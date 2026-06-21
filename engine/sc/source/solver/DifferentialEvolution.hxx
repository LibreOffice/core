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
#include "DEAlgorithm.hxx"

#include <vector>

// Standalone differential evolution solver: a swarm driven by the differential
// evolution engine every generation, mutating around a random member.
template <typename DataProvider> class DifferentialEvolutionSolver
{
    static constexpr double constDifferentialWeight = 0.5; // [0, 2]
    static constexpr double constCrossoverProbability = 0.9; // [0, 1]

    Swarm<DataProvider> maSwarm;
    DEAlgorithm<DataProvider> maAlgorithm;

public:
    DifferentialEvolutionSolver(DataProvider& rDataProvider, size_t nNumOfParticles)
        : maSwarm(rDataProvider, nNumOfParticles)
        , maAlgorithm(maSwarm, constDifferentialWeight, constCrossoverProbability,
                      DifferentialBound::Wrap, DifferentialBase::RandomMember)
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
