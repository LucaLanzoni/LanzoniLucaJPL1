//
// PROJECT CHRONO - http://projectchrono.org
//
// Copyright (c) 2010-2011 Alessandro Tasora
// Copyright (c) 2013 Project Chrono
// All rights reserved.
//
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file at the top level of the distribution
// and at http://projectchrono.org/license-chrono.txt.
//

///////////////////////////////////////////////////
//
//   Demo code about
//
//     - using the ChParticleEmitter to create
//       a cluster of random shapes
//     - apply custom force field to particles
//     - use Irrlicht to display objects
//
//
//	 CHRONO
//   ------
//   Multibody dinamics engine
//
// ------------------------------------------------
//             www.deltaknowledge.com
// ------------------------------------------------
///////////////////////////////////////////////////

#include "chrono/physics/ChSystem.h"
#include "chrono/particlefactory/ChParticleEmitter.h"
#include "chrono/assets/ChTexture.h"
#include "chrono/assets/ChColorAsset.h"
#include "chrono/physics/ChBodyEasy.h"

#include "chrono_irrlicht/ChIrrApp.h"

// for M_PI
#define _USE_MATH_DEFINES
#include <math.h>


// Use the main namespace of Chrono, and other chrono namespaces
using namespace chrono;
using namespace chrono::particlefactory;
using namespace chrono::geometry;
using namespace chrono::irrlicht;

// Use the main namespaces of Irrlicht
using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;
using namespace irr::io;
using namespace irr::gui;

//     A callback executed at each particle creation can be attached to the emitter.
//     For example, we need that new particles will be bound to Irrlicht visualization:

class MyCreatorForAll : public ChCallbackPostCreation {
public:
	virtual void PostCreation(std::shared_ptr<ChBody> mbody, ChCoordsys<> mcoords, ChRandomShapeCreator& mcreator) {
		// optional: add further assets, ex for improving visualization:
		auto mtexture = std::make_shared<ChTexture>();
		mtexture->SetTextureFilename(GetChronoDataFile("bluwhite.png"));
		mbody->AddAsset(mtexture);

		// Enable Irrlicht visualization for all particles
		airrlicht_application->AssetBind(mbody);
		airrlicht_application->AssetUpdate(mbody);

		// Other stuff, ex. disable gyroscopic forces for increased integrator stabilty
		mbody->SetNoGyroTorque(true);
	}
	ChIrrApp* airrlicht_application;
};

int main(int argc, char* argv[]) {
	// Create a ChronoENGINE physical system
	ChSystem mphysicalSystem;

	// Create the Irrlicht visualization (open the Irrlicht device,
	// bind a simple user interface, etc. etc.)
	ChIrrApp application(&mphysicalSystem, L"Particle emitter", core::dimension2d<u32>(800, 600), false);

	// Easy shortcuts to add camera, lights, logo and sky in Irrlicht scene:
	ChIrrWizard::add_typical_Logo(application.GetDevice());
	ChIrrWizard::add_typical_Sky(application.GetDevice());
	ChIrrWizard::add_typical_Lights(application.GetDevice());
	ChIrrWizard::add_typical_Camera(application.GetDevice(), core::vector3df(0, 7, -10));

	//
	// CREATE THE SYSTEM OBJECTS
	//

	// Example: create a ChBody rigid body (trick: using the ChBodyEasyXXYYZZ
	// functions it also sets mass and inertia tensor for you, and collision
	// and visualization shapes are added automatically)

	auto msphereBody = std::make_shared<ChBodyEasySphere>(3.1,    // radius size
		5000,   // density
		true,   // collide enable?
		true);  // visualization?
	msphereBody->SetPos(ChVector<>(0, 0, 0));
	msphereBody->GetMaterialSurface()->SetFriction(0.2f);


	// optional: add further assets, ex for improving visualization:
	auto mtexture = std::make_shared<ChTexture>();
	mtexture->SetTextureFilename(GetChronoDataFile("concrete.jpg"));
	msphereBody->AddAsset(mtexture);

	mphysicalSystem.Add(msphereBody);

	// Ok, creating particles using ChBody or the ChBodyEasyXXYYZZ shortcuts
	// can be enough, ex. if you put in a for() loop you can create a cluster.
	// However there is an easier way to the creation of cluster of particles,
	// namely the ChEmitter helper class. Here we show hof to use it.

	//Emitter
	/*
	// Create an emitter:

	ChParticleEmitter emitter;

	// Ok, that object will take care of generating particle flows for you.
	// It accepts a lot of settings, for creating many different types of particle
	// flows, like fountains, outlets of various shapes etc.
	// For instance, set the flow rate, etc:

	emitter.ParticlesPerSecond() = 2000;

	emitter.SetUseParticleReservoir(true);
	emitter.ParticleReservoirAmount() = 0;

	// Our ChParticleEmitter object, among the main settings, it requires
	// that you give him four 'randomizer' objects: one is in charge of
	// generating random shapes, one is in charge of generating
	// random positions, one for random alignements, and one for random velocities.
	// In the following we need to instance such objects. (There are many ready-to-use
	// randomizer objects already available in chrono, but note that you could also
	// inherit your own class from these randomizers if the choice is not enough).

	// ---Initialize the randomizer for POSITIONS: random points in a large cube
	auto emitter_positions = std::make_shared<ChRandomParticlePositionOnGeometry>();

	std::shared_ptr<ChGeometry> sampled_cube(new ChBox(VNULL, ChMatrix33<>(QUNIT), ChVector<>(50, 50, 50)));
	emitter_positions->SetGeometry(sampled_cube);

	emitter.SetParticlePositioner(emitter_positions);

	// ---Initialize the randomizer for ALIGNMENTS
	auto emitter_rotations = std::make_shared<ChRandomParticleAlignmentUniform>();
	emitter.SetParticleAligner(emitter_rotations);

	// ---Initialize the randomizer for VELOCITIES, with statistical distribution
	auto mvelo = std::make_shared<ChRandomParticleVelocityAnyDirection>();
	mvelo->SetModulusDistribution(std::make_shared<ChMinMaxDistribution>(0.0, 0.5));
	emitter.SetParticleVelocity(mvelo);

	// ---Initialize the randomizer for ANGULAR VELOCITIES, with statistical distribution
	auto mangvelo = std::make_shared<ChRandomParticleVelocityAnyDirection>();
	mangvelo->SetModulusDistribution(std::make_shared<ChMinMaxDistribution>(0.0, 0.2));
	emitter.SetParticleAngularVelocity(mangvelo);

	// ---Initialize the randomizer for CREATED SHAPES, with statistical distribution

	
	// Create a ChRandomShapeCreator object (ex. here for sphere particles)
	auto mcreator_spheres = std::make_shared<ChRandomShapeCreatorSpheres>();
	mcreator_spheres->SetDiameterDistribution(std::make_shared<ChZhangDistribution>(0.4, 0.2));
	mcreator_spheres->SetDensityDistribution(std::make_shared<ChConstantDistribution>(1600));

	// Finally, tell to the emitter that it must use the creator above:
	emitter.SetParticleCreator(mcreator_spheres);
	*/
	
	/*
	// ..as an alternative: create odd shapes with convex hulls, like faceted fragments:
	auto mcreator_hulls = std::make_shared<ChRandomShapeCreatorConvexHulls>();
	mcreator_hulls->SetNpoints(15);
	mcreator_hulls->SetChordDistribution(std::make_shared<ChZhangDistribution>(1.3, 0.4));
	mcreator_hulls->SetDensityDistribution(std::make_shared<ChConstantDistribution>(1600));
	emitter.SetParticleCreator(mcreator_hulls);
	*/

	/*
	// --- Optional: what to do by default on ALL newly created particles?
	//     A callback executed at each particle creation can be attached to the emitter.
	//     For example, we need that new particles will be bound to Irrlicht visualization:

	// a- define a class that implement your custom PostCreation method (see top of source file)
	// b- create the callback object...
	MyCreatorForAll* mcreation_callback = new MyCreatorForAll;
	// c- set callback own data that he might need...
	mcreation_callback->airrlicht_application = &application;
	// d- attach the callback to the emitter!
	emitter.SetCallbackPostCreation(mcreation_callback);
	*/

	// Class for a custom asset (ex. an optical property)
	class MyProperty : public ChAsset {
	public:
		double reflection;
	};

	
	///// Create particles 'the easy way'
	for (int i = 0; i <= 0; ++i) {

		/// make a sphere

		auto my_sphere = std::make_shared<ChBodyEasySphere>(0.1, 500, true, true);
		my_sphere->GetMaterialSurface()->SetFriction(0.2f);

		mphysicalSystem.Add(my_sphere);

		my_sphere->SetPos(ChVector<>(i + i*7 + 7, 0 , 0));
		double G_const = 6.67384e-11;   //G_const = 6.67384e-11 the true one
		double mu = msphereBody->GetMass() * G_const;
		double virad = mu / my_sphere->GetPos().x;
		double vi = sqrt(virad);
		my_sphere->SetPos_dt(ChVector<>(0 , vi, 0));

		auto my_color = std::make_shared<ChColorAsset>(0,1,0); // rgb
		my_sphere->AddAsset(my_color);
		
		/*
		/// make a convex hull

		std::vector<ChVector<>> set_of_points;
		set_of_points.push_back(ChVector<>(0.40, 0.00, 0.00));
		set_of_points.push_back(ChVector<>(0.00, 0.4, 0.00));
		set_of_points.push_back(ChVector<>(0.00, 0.00, 0.4));
		set_of_points.push_back(ChVector<>(0.00, 0.00, 0.00));
		set_of_points.push_back(ChVector<>(0.00, 0.00, 0.4));

		auto my_hull = std::make_shared<ChBodyEasyConvexHull>(
			set_of_points,
			1000,  //  density
			true,  //  collide?
			true); //  visualize?	
		
		mphysicalSystem.Add(my_hull);
		
		my_hull->SetPos(ChVector<>(ChRandom(), ChRandom(), ChRandom()) * 10);
		my_hull->SetRot(ChQuaternion<>(ChRandom(), ChRandom(), ChRandom(), ChRandom()).GetNormalized() );
		*/

		// create a custom asset
		auto shape_property = std::make_shared<MyProperty>();
		shape_property->reflection = ChRandom()*0;
		//my_hull->AddAsset(shape_property);
		//msphereBody->AddAsset(shape_property);
		my_sphere->AddAsset(shape_property);

		double period = 2 * M_PI * pow(my_sphere->GetPos().x, 1.5) / sqrt(mu);

		GetLog() << "mu: " << mu << "\n";
		GetLog() << "period: " << period << "\n";
		GetLog() << "central mass: " << msphereBody->GetMass() << "\n";
		GetLog() << "orbiting mass: " << my_sphere->GetMass() << "\n";
	}
	


	// Use this function for adding a ChIrrNodeAsset to all already created items (ex. a floor, a wall, etc.)
	// Otherwise use application.AssetBind(myitem); on a per-item basis, as in the creation callback.
	application.AssetBindAll();

	// Use this function for 'converting' assets into Irrlicht meshes
	application.AssetUpdateAll();

	// Modify some setting of the physical system for the simulation, if you want
	mphysicalSystem.SetLcpSolverType(ChSystem::LCP_ITERATIVE_SOR);  // try also other ChSystem::LCP_XXXYYYZZZ stuff
	mphysicalSystem.SetIterLCPmaxItersSpeed(40);
	mphysicalSystem.SetIterLCPmaxItersStab(5);

	// Turn off default -9.8 downward gravity
	mphysicalSystem.Set_G_acc(ChVector<>(0, 0, 0));	//posizione forza di gravit�

	application.SetStepManage(true);
	application.SetTimestep(2);


	//
	// THE SOFT-REAL-TIME CYCLE
	//
	ChStreamOutAsciiFile my_results("results.txt"); // apre un file
	int save_each = 10;

	double chronoTime = 0;
	while (application.GetDevice()->run()) { //(chronoTime < 1000) {//
		//chronoTime += 0.01;
		application.GetVideoDriver()->beginScene(true, true, SColor(255, 140, 161, 192));

		application.DrawAll();

		// Create particle flow
		//emitter.EmitParticles(mphysicalSystem, application.GetTimestep());

		// Apply custom forcefield (brute force approach..)
		// A) reset 'user forces accumulators':
		for (unsigned int i = 0; i < mphysicalSystem.Get_bodylist()->size(); i++) {
			auto abody = (*mphysicalSystem.Get_bodylist())[i];
			abody->Empty_forces_accumulators();
		}

		// B) store user computed force:
		// double G_constant = 6.67384e-11; // gravitational constant
		double G_constant = 6.67384e-11;  // gravitational constant - HACK to speed up simulation
		for (unsigned int i = 0; i < mphysicalSystem.Get_bodylist()->size(); i++) {
			auto abodyA = (*mphysicalSystem.Get_bodylist())[i];
			for (unsigned int j = i + 1; j < mphysicalSystem.Get_bodylist()->size(); j++) {
				auto abodyB = (*mphysicalSystem.Get_bodylist())[j];
				ChVector<> D_attract = abodyB->GetPos() - abodyA->GetPos();
				double r_attract = D_attract.Length();
				double f_attract = G_constant * (abodyA->GetMass() * abodyB->GetMass()) / (pow(r_attract, 2));
				ChVector<> F_attract = (D_attract / r_attract) * f_attract;

				abodyA->Accumulate_force(F_attract, abodyA->GetPos(), false);
				abodyB->Accumulate_force(-F_attract, abodyB->GetPos(), false);
			}
		}

		if (mphysicalSystem.GetStepcount() % save_each == 0)
			my_results << mphysicalSystem.GetChTime() << " ";

		for (unsigned int i = 0; i < mphysicalSystem.Get_bodylist()->size(); i++) {
			auto abodyA = (*mphysicalSystem.Get_bodylist())[i];

			for (auto generic_asset : abodyA->GetAssets()) {
				if (auto optical_asset = std::dynamic_pointer_cast<MyProperty>(generic_asset)) {
					// if we found a MyProperty apply a force!
					abodyA->Accumulate_force( ChVector<>(0, optical_asset->reflection,0), abodyA->GetPos(), false);
					// ..also save x y z on disk
					if (mphysicalSystem.GetStepcount() % save_each == 0) {
						my_results
							<< abodyA->GetPos().x << " "
							<< abodyA->GetPos().y << " "
							<< abodyA->GetPos().z << " "
							<< abodyA->GetPos_dt().x << " "
							<< abodyA->GetPos_dt().y << " "
							<< abodyA->GetPos_dt().z << " "
							<< abodyA->GetPos_dtdt().x << " "
							<< abodyA->GetPos_dtdt().y << " "
							<< abodyA->GetPos_dtdt().z << " ";
					}
				}
			}
		}

		// Perform the integration timestep
		application.DoStep();

		application.GetVideoDriver()->endScene();

		if (mphysicalSystem.GetStepcount() % save_each == 0)
			my_results << "\n";
	}

	return 0;
}
