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

#include <vector>
#include <random>

// One particle swarm generation, a reusable engine any swarm-based solver can
// drive. Each call advances the bound swarm by one generation: every particle's
// velocity is pulled by inertia toward its own best and the swarm's best, then
// its position steps and is clamped.
template <typename DataProvider> class PSOAlgorithm
{
    // inertia
    static constexpr double constWeight = 0.729;
    // cognitive coefficient
    static constexpr double constCognitive = 1.49445;
    // social coefficient
    static constexpr double constSocial = 1.49445;

    Swarm<DataProvider>& mrSwarm;
    std::uniform_real_distribution<> maRandom01;

public:
    explicit PSOAlgorithm(Swarm<DataProvider>& rSwarm)
        : mrSwarm(rSwarm)
        , maRandom01(0.0, 1.0)
    {
    }

    bool next() { return mrSwarm.advance(*this); }

    // Advance one particle: pull its velocity by inertia toward its own best and
    // the swarm's best, then step and clamp its position. The move depends only
    // on the particle and the swarm's best, so the particle index is unused.
    void step(Particle& rParticle, size_t /*nParticleIndex*/)
    {
        DataProvider& rDataProvider = mrSwarm.dataProvider();
        std::mt19937& rGenerator = mrSwarm.generator();
        const std::vector<double>& rGlobalBest = mrSwarm.bestPosition();

        double fRandom1 = maRandom01(rGenerator);
        double fRandom2 = maRandom01(rGenerator);

        const size_t nDimensionality = rParticle.mPosition.size();
        for (size_t nDimension = 0; nDimension < nDimensionality; ++nDimension)
        {
            rParticle.mVelocity[nDimension]
                = (constWeight * rParticle.mVelocity[nDimension])
                  + (constCognitive * fRandom1
                     * (rParticle.mBestPosition[nDimension] - rParticle.mPosition[nDimension]))
                  + (constSocial * fRandom2
                     * (rGlobalBest[nDimension] - rParticle.mPosition[nDimension]));

            // The inertia weight below one keeps the velocity bounded on its
            // own, so only the position is clamped, which holds the particle
            // inside its bounds.
            rParticle.mPosition[nDimension] = rDataProvider.clampVariable(
                nDimension, rParticle.mPosition[nDimension] + rParticle.mVelocity[nDimension]);
        }
        rParticle.mCurrentFitness = rDataProvider.calculateFitness(rParticle.mPosition);
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
