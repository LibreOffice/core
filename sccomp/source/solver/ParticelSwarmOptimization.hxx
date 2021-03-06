/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once

#include <vector>
#include <random>
#include <limits>

struct Particle
{
    Particle(size_t nDimensionality)
        : mVelocity(nDimensionality)
        , mPosition(nDimensionality)
        , mCurrentFitness(std::numeric_limits<float>::lowest())
        , mBestPosition(nDimensionality)
        , mBestFitness(std::numeric_limits<float>::lowest())
    {
    }

    std::vector<double> mVelocity;

    std::vector<double> mPosition;
    double mCurrentFitness;

    std::vector<double> mBestPosition;
    double mBestFitness;
};

template <typename DataProvider> class ParticleSwarmOptimizationAlgorithm
{
private:
    // inertia
    static constexpr double constWeight = 0.729;
    // cognitive coefficient
    static constexpr double c1 = 1.49445;
    // social  coefficient
    static constexpr double c2 = 1.49445;

    static constexpr double constAcceptedPrecision = 0.000000001;

    DataProvider& mrDataProvider;

    size_t mnNumOfParticles;

    std::vector<Particle> maSwarm;

    std::random_device maRandomDevice;
    std::mt19937 maGenerator;
    size_t mnDimensionality;

    std::uniform_real_distribution<> maRandom01;

    std::vector<double> maBestPosition;
    double mfBestFitness;
    int mnGeneration;
    int mnLastChange;

public:
    ParticleSwarmOptimizationAlgorithm(DataProvider& rDataProvider, size_t nNumOfParticles)
        : mrDataProvider(rDataProvider)
        , mnNumOfParticles(nNumOfParticles)
        , maGenerator(maRandomDevice())
        , mnDimensionality(mrDataProvider.getDimensionality())
        , maRandom01(0.0, 1.0)
        , maBestPosition(mnDimensionality)
        , mfBestFitness(std::numeric_limits<float>::lowest())
        , mnGeneration(0)
        , mnLastChange(0)
    {
    }

    std::vector<double> const& getResult() { return maBestPosition; }

    int getGeneration() { return mnGeneration; }

    int getLastChange() { return mnLastChange; }

    void initialize()
    {
        mnGeneration = 0;
        mnLastChange = 0;
        maSwarm.clear();

        mfBestFitness = std::numeric_limits<float>::lowest();

        maSwarm.reserve(mnNumOfParticles);
        for (size_t i = 0; i < mnNumOfParticles; i++)
        {
            maSwarm.emplace_back(mnDimensionality);
            Particle& rParticle = maSwarm.back();

            mrDataProvider.initializeVariables(rParticle.mPosition, maGenerator);
            mrDataProvider.initializeVariables(rParticle.mVelocity, maGenerator);

            for (size_t k = 0; k < mnDimensionality; k++)
            {
                rParticle.mPosition[k] = mrDataProvider.clampVariable(k, rParticle.mPosition[k]);
            }

            rParticle.mCurrentFitness = mrDataProvider.calculateFitness(rParticle.mPosition);

            for (size_t k = 0; k < mnDimensionality; k++)
            {
                rParticle.mPosition[k] = mrDataProvider.clampVariable(k, rParticle.mPosition[k]);
            }

            rParticle.mBestPosition.insert(rParticle.mBestPosition.begin(),
                                           rParticle.mPosition.begin(), rParticle.mPosition.end());
            rParticle.mBestFitness = rParticle.mCurrentFitness;

            if (rParticle.mCurrentFitness > mfBestFitness)
            {
                mfBestFitness = rParticle.mCurrentFitness;
                maBestPosition.insert(maBestPosition.begin(), rParticle.mPosition.begin(),
                                      rParticle.mPosition.end());
            }
        }
    }

    bool next()
    {
        bool bBestChanged = false;

        for (Particle& rParticle : maSwarm)
        {
            double fRandom1 = maRandom01(maGenerator);
            double fRandom2 = maRandom01(maGenerator);

            for (size_t k = 0; k < mnDimensionality; k++)
            {
                rParticle.mVelocity[k]
                    = (constWeight * rParticle.mVelocity[k])
                      + (c1 * fRandom1 * (rParticle.mBestPosition[k] - rParticle.mPosition[k]))
                      + (c2 * fRandom2 * (maBestPosition[k] - rParticle.mPosition[k]));

                mrDataProvider.clampVariable(k, rParticle.mVelocity[k]);

                rParticle.mPosition[k] += rParticle.mVelocity[k];
                rParticle.mPosition[k] = mrDataProvider.clampVariable(k, rParticle.mPosition[k]);
            }

            rParticle.mCurrentFitness = mrDataProvider.calculateFitness(rParticle.mPosition);

            if (rParticle.mCurrentFitness > rParticle.mBestFitness)
            {
                rParticle.mBestFitness = rParticle.mCurrentFitness;
                rParticle.mBestPosition.insert(rParticle.mBestPosition.begin(),
                                               rParticle.mPosition.begin(),
                                               rParticle.mPosition.end());
            }

            if (rParticle.mCurrentFitness > mfBestFitness)
            {
                if (std::abs(rParticle.mCurrentFitness - mfBestFitness) > constAcceptedPrecision)
                {
                    bBestChanged = true;
                    mnLastChange = mnGeneration;
                }
                maBestPosition.insert(maBestPosition.begin(), rParticle.mPosition.begin(),
                                      rParticle.mPosition.end());
                mfBestFitness = rParticle.mCurrentFitness;
            }
        }
        mnGeneration++;
        return bBestChanged;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
