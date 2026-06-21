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

#include <utility>
#include <vector>
#include <random>

// Where a mutated value that lands outside a variable's bounds is sent. Clamp
// pins it to the nearest bound. Wrap folds it back into the range, keeping its
// offset from the lower bound.
enum class DifferentialBound
{
    Clamp,
    Wrap
};

// The base the mutant is built around. RandomMember picks a random particle
// (classic differential evolution). GlobalBest uses the swarm's best
// position, which pulls trials toward the best.
enum class DifferentialBase
{
    RandomMember,
    GlobalBest
};

// One differential evolution generation, a reusable engine any swarm-based
// solver can drive. Each call advances the bound swarm by one generation: for
// every particle it builds a trial by binomial crossover, where each chosen
// dimension takes base + weight * (first - second) kept inside the bounds, and
// keeps the trial only when it improves on the particle (greedy selection).
template <typename DataProvider> class DEAlgorithm
{
    Swarm<DataProvider>& mrSwarm;
    double mfWeight;
    double mfCrossover;
    DifferentialBound meBound;
    DifferentialBase meBase;
    std::uniform_real_distribution<> maRandom01;
    std::uniform_int_distribution<> maRandomParticle;
    std::uniform_int_distribution<> maRandomDimension;

public:
    DEAlgorithm(Swarm<DataProvider>& rSwarm, double fWeight, double fCrossover,
                DifferentialBound eBound, DifferentialBase eBase)
        : mrSwarm(rSwarm)
        , mfWeight(fWeight)
        , mfCrossover(fCrossover)
        , meBound(eBound)
        , meBase(eBase)
        , maRandom01(0.0, 1.0)
        , maRandomParticle(0, rSwarm.numParticles() > 0 ? int(rSwarm.numParticles() - 1) : 0)
        , maRandomDimension(0, rSwarm.dimensionality() > 0 ? int(rSwarm.dimensionality() - 1) : 0)
    {
    }

    bool next() { return mrSwarm.advance(*this); }

    void step(Particle& rParticle, size_t nParticleIndex)
    {
        DataProvider& rDataProvider = mrSwarm.dataProvider();
        std::mt19937& rGenerator = mrSwarm.generator();
        std::vector<Particle>& rParticles = mrSwarm.particles();

        // The difference vector is taken from two distinct other particles. The
        // mutant base is either a third random particle or the swarm's best.
        size_t nFirst;
        size_t nSecond;
        const std::vector<double>* pBase;
        if (meBase == DifferentialBase::RandomMember)
        {
            size_t nBaseParticle;
            do
            {
                nBaseParticle = maRandomParticle(rGenerator);
            } while (nBaseParticle == nParticleIndex);
            do
            {
                nFirst = maRandomParticle(rGenerator);
            } while (nFirst == nParticleIndex || nFirst == nBaseParticle);
            do
            {
                nSecond = maRandomParticle(rGenerator);
            } while (nSecond == nParticleIndex || nSecond == nBaseParticle || nSecond == nFirst);
            pBase = &rParticles[nBaseParticle].mPosition;
        }
        else
        {
            do
            {
                nFirst = maRandomParticle(rGenerator);
            } while (nFirst == nParticleIndex);
            do
            {
                nSecond = maRandomParticle(rGenerator);
            } while (nSecond == nParticleIndex || nSecond == nFirst);
            pBase = &mrSwarm.bestPosition();
        }

        const std::vector<double>& rBase = *pBase;
        const std::vector<double>& rFirst = rParticles[nFirst].mPosition;
        const std::vector<double>& rSecond = rParticles[nSecond].mPosition;

        std::vector<double> aTrial = rParticle.mPosition;
        const size_t nDimensionality = rParticle.mPosition.size();
        const size_t nForced = maRandomDimension(rGenerator);
        for (size_t nDimension = 0; nDimension < nDimensionality; ++nDimension)
        {
            if (nDimension == nForced || maRandom01(rGenerator) < mfCrossover)
            {
                double fMutant
                    = rBase[nDimension] + mfWeight * (rFirst[nDimension] - rSecond[nDimension]);
                aTrial[nDimension] = meBound == DifferentialBound::Wrap
                                         ? rDataProvider.boundVariable(nDimension, fMutant)
                                         : rDataProvider.clampVariable(nDimension, fMutant);
            }
        }

        // Greedy selection: keep the trial only if it improves on the particle.
        double fTrialFitness = rDataProvider.calculateFitness(aTrial);
        if (fTrialFitness > rParticle.mCurrentFitness)
        {
            for (size_t nDimension = 0; nDimension < nDimensionality; ++nDimension)
                rParticle.mVelocity[nDimension]
                    = aTrial[nDimension] - rParticle.mPosition[nDimension];
            rParticle.mPosition = std::move(aTrial);
            rParticle.mCurrentFitness = fTrialFitness;
        }
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
