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

#include <vector>

// Standalone particle swarm solver: a swarm driven by the particle swarm engine
// every generation.
template <typename DataProvider> class ParticleSwarmOptimizationSolver
{
    Swarm<DataProvider> maSwarm;
    PSOAlgorithm<DataProvider> maAlgorithm;

public:
    ParticleSwarmOptimizationSolver(DataProvider& rDataProvider, size_t nNumOfParticles)
        : maSwarm(rDataProvider, nNumOfParticles)
        , maAlgorithm(maSwarm)
    {
    }

    void initialize() { maSwarm.initialize(); }
    void restart() { maSwarm.restart(); }
    bool next() { return maAlgorithm.next(); }

    std::vector<double> const& getResult() const { return maSwarm.getResult(); }
    int getGeneration() const { return maSwarm.getGeneration(); }
    int getLastChange() const { return maSwarm.getLastChange(); }
    double getBestFitness() const { return maSwarm.getBestFitness(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
