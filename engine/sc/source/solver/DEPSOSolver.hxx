/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include "Swarm.hxx"
#include "PSOAlgorithm.hxx"
#include "DEAlgorithm.hxx"

#include <vector>

// A hybrid of particle swarm optimization and differential evolution that
// alternates between the two from one generation to the next over a single
// swarm. An even generation runs one full particle swarm generation: inertia
// plus a pull toward each particle's own best and the swarm's best. An odd
// generation runs one full differential evolution generation whose mutant base
// is the swarm's best, kept only when it improves on the particle. Each
// generation costs one fitness evaluation per particle, the same as either
// method alone, so alternating combines the social convergence of particle
// swarm optimization with the selection pressure and diversity of differential
// evolution without doubling the cost.
//
// It owns no algorithm code of its own: it drives the PSOAlgorithm and
// DEAlgorithm engines, both bound to its swarm.
template <typename DataProvider> class DEPSOSolver
{
    // differential evolution mutation weight and crossover rate
    static constexpr double constDifferentialWeight = 0.6;
    static constexpr double constCrossover = 0.9;

    Swarm<DataProvider> maSwarm;
    PSOAlgorithm<DataProvider> maParticleSwarm;
    DEAlgorithm<DataProvider> maDifferential;

public:
    DEPSOSolver(DataProvider& rDataProvider, size_t nNumOfParticles)
        : maSwarm(rDataProvider, nNumOfParticles)
        , maParticleSwarm(maSwarm)
        , maDifferential(maSwarm, constDifferentialWeight, constCrossover, DifferentialBound::Clamp,
                         DifferentialBase::GlobalBest)
    {
    }

    void initialize() { maSwarm.initialize(); }
    void restart() { maSwarm.restart(); }

    bool next()
    {
        // Differential evolution needs a difference vector, so fall back to a
        // particle swarm generation when there are too few particles.
        bool bDifferential = maSwarm.numParticles() >= 3 && (maSwarm.getGeneration() % 2 == 1);
        return bDifferential ? maDifferential.next() : maParticleSwarm.next();
    }

    std::vector<double> const& getResult() const { return maSwarm.getResult(); }
    int getGeneration() const { return maSwarm.getGeneration(); }
    int getLastChange() const { return maSwarm.getLastChange(); }
    double getBestFitness() const { return maSwarm.getBestFitness(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
