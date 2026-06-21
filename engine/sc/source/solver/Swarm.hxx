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

#include <cmath>
#include <vector>
#include <random>
#include <limits>

struct Particle
{
    Particle(size_t nDimensionality)
        : mVelocity(nDimensionality)
        , mPosition(nDimensionality)
        , mCurrentFitness(std::numeric_limits<double>::lowest())
        , mBestPosition(nDimensionality)
        , mBestFitness(std::numeric_limits<double>::lowest())
    {
    }

    std::vector<double> mVelocity;

    std::vector<double> mPosition;
    double mCurrentFitness;

    std::vector<double> mBestPosition;
    double mBestFitness;
};

// The shared state of a swarm search: the particles, the best position and
// fitness found so far, and the generation counters. It owns the random
// generator, builds and rebuilds the swarm, and folds each particle's fitness
// into the bests. A per-generation algorithm advances it through this shared
// scaffolding rather than each one repeating it.
template <typename DataProvider> class Swarm
{
    static constexpr double constAcceptedPrecision = 0.000000001;

    DataProvider& mrDataProvider;
    size_t mnNumOfParticles;

    std::random_device maRandomDevice;
    std::mt19937 maGenerator;
    size_t mnDimensionality;

    std::vector<Particle> maParticles;
    std::vector<double> maBestPosition;
    double mfBestFitness;
    int mnGeneration;
    int mnLastChange;

public:
    Swarm(DataProvider& rDataProvider, size_t nNumOfParticles)
        : mrDataProvider(rDataProvider)
        , mnNumOfParticles(nNumOfParticles)
        , maGenerator(maRandomDevice())
        , mnDimensionality(mrDataProvider.getDimensionality())
        , maBestPosition(mnDimensionality)
        , mfBestFitness(std::numeric_limits<double>::lowest())
        , mnGeneration(0)
        , mnLastChange(0)
    {
    }

    DataProvider& dataProvider() { return mrDataProvider; }
    std::mt19937& generator() { return maGenerator; }
    std::vector<Particle>& particles() { return maParticles; }
    const std::vector<double>& bestPosition() const { return maBestPosition; }
    size_t dimensionality() const { return mnDimensionality; }
    size_t numParticles() const { return mnNumOfParticles; }

    std::vector<double> const& getResult() const { return maBestPosition; }
    int getGeneration() const { return mnGeneration; }
    int getLastChange() const { return mnLastChange; }
    double getBestFitness() const { return mfBestFitness; }

    void initialize()
    {
        mnGeneration = 0;
        mnLastChange = 0;
        mfBestFitness = std::numeric_limits<double>::lowest();

        maParticles.clear();
        maParticles.reserve(mnNumOfParticles);
        for (size_t nParticleIndex = 0; nParticleIndex < mnNumOfParticles; ++nParticleIndex)
        {
            maParticles.emplace_back(mnDimensionality);
            seedParticle(maParticles.back());

            Particle& rParticle = maParticles.back();
            if (rParticle.mCurrentFitness > mfBestFitness)
            {
                mfBestFitness = rParticle.mCurrentFitness;
                maBestPosition = rParticle.mPosition;
            }
        }
    }

    // Rebuild the swarm from a fresh random spread to escape a local optimum it
    // has collapsed into, seeding one particle at the best position found so far
    // so it is not lost. The no-change counter is reset to give the search a
    // fresh window to improve in.
    void restart()
    {
        mnLastChange = mnGeneration;

        maParticles.clear();
        maParticles.reserve(mnNumOfParticles);
        for (size_t nParticleIndex = 0; nParticleIndex < mnNumOfParticles; ++nParticleIndex)
        {
            maParticles.emplace_back(mnDimensionality);
            seedParticle(maParticles.back());
        }

        if (!maBestPosition.empty() && !maParticles.empty())
        {
            Particle& rFirst = maParticles.front();
            rFirst.mPosition = maBestPosition;
            rFirst.mBestPosition = maBestPosition;
            rFirst.mCurrentFitness = mfBestFitness;
            rFirst.mBestFitness = mfBestFitness;
        }
    }

    // Advance the swarm by one generation: apply the algorithm's step to every
    // particle, fold the result into the bests, and count the generation. The
    // step leaves the particle at its new position and current fitness. Returns
    // whether the global best improved.
    template <typename Algorithm> bool advance(Algorithm& rAlgorithm)
    {
        bool bBestChanged = false;
        for (size_t nParticleIndex = 0; nParticleIndex < maParticles.size(); ++nParticleIndex)
        {
            rAlgorithm.step(maParticles[nParticleIndex], nParticleIndex);
            if (registerParticle(maParticles[nParticleIndex]))
                bBestChanged = true;
        }
        ++mnGeneration;
        return bBestChanged;
    }

private:
    // Fill a fresh particle with a random clamped position and its fitness.
    void seedParticle(Particle& rParticle)
    {
        mrDataProvider.initializeVariables(rParticle.mPosition, maGenerator);
        for (size_t nDimension = 0; nDimension < mnDimensionality; ++nDimension)
            rParticle.mPosition[nDimension]
                = mrDataProvider.clampVariable(nDimension, rParticle.mPosition[nDimension]);

        rParticle.mCurrentFitness = mrDataProvider.calculateFitness(rParticle.mPosition);
        rParticle.mBestPosition = rParticle.mPosition;
        rParticle.mBestFitness = rParticle.mCurrentFitness;
    }

    // Fold a particle's current fitness into its personal best and the swarm's
    // global best. Returns whether the global best moved by more than the
    // accepted precision, recording the generation when it did.
    bool registerParticle(Particle& rParticle)
    {
        bool bBestChanged = false;

        if (rParticle.mCurrentFitness > rParticle.mBestFitness)
        {
            rParticle.mBestFitness = rParticle.mCurrentFitness;
            rParticle.mBestPosition = rParticle.mPosition;
        }

        if (rParticle.mCurrentFitness > mfBestFitness)
        {
            if (std::abs(rParticle.mCurrentFitness - mfBestFitness) > constAcceptedPrecision)
            {
                bBestChanged = true;
                mnLastChange = mnGeneration;
            }
            maBestPosition = rParticle.mPosition;
            mfBestFitness = rParticle.mCurrentFitness;
        }
        return bBestChanged;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
