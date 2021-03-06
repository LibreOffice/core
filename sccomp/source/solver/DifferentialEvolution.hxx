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

struct Individual
{
    std::vector<double> mVariables;
};

template <typename DataProvider> class DifferentialEvolutionAlgorithm
{
    static constexpr double mnDifferentialWeight = 0.5; // [0, 2]
    static constexpr double mnCrossoverProbability = 0.9; // [0, 1]

    static constexpr double constAcceptedPrecision = 0.000000001;

    DataProvider& mrDataProvider;

    size_t mnPopulationSize;
    std::vector<Individual> maPopulation;

    std::random_device maRandomDevice;
    std::mt19937 maGenerator;
    size_t mnDimensionality;

    std::uniform_int_distribution<> maRandomPopulation;
    std::uniform_int_distribution<> maRandomDimensionality;
    std::uniform_real_distribution<> maRandom01;

    Individual maBestCandidate;
    double mfBestFitness;
    int mnGeneration;
    int mnLastChange;

public:
    DifferentialEvolutionAlgorithm(DataProvider& rDataProvider, size_t nPopulationSize)
        : mrDataProvider(rDataProvider)
        , mnPopulationSize(nPopulationSize)
        , maGenerator(maRandomDevice())
        , mnDimensionality(mrDataProvider.getDimensionality())
        , maRandomPopulation(0, mnPopulationSize - 1)
        , maRandomDimensionality(0, mnDimensionality - 1)
        , maRandom01(0.0, 1.0)
        , mfBestFitness(std::numeric_limits<double>::lowest())
        , mnGeneration(0)
        , mnLastChange(0)
    {
    }

    std::vector<double> const& getResult() { return maBestCandidate.mVariables; }

    int getGeneration() { return mnGeneration; }

    int getLastChange() { return mnLastChange; }

    void initialize()
    {
        mnGeneration = 0;
        mnLastChange = 0;
        maPopulation.clear();
        maBestCandidate.mVariables.clear();

        // Initialize population with individuals that have been initialized with uniform random
        // noise
        // uniform noise means random value inside your search space
        maPopulation.reserve(mnPopulationSize);
        for (size_t i = 0; i < mnPopulationSize; ++i)
        {
            maPopulation.emplace_back();
            Individual& rIndividual = maPopulation.back();
            mrDataProvider.initializeVariables(rIndividual.mVariables, maGenerator);
        }
    }

    // Calculate one generation
    bool next()
    {
        bool bBestChanged = false;

        for (size_t agentIndex = 0; agentIndex < mnPopulationSize; ++agentIndex)
        {
            // calculate new candidate solution

            // pick random point from population
            size_t x = agentIndex; // randomPopulation(generator);
            size_t a, b, c;

            // create a copy of chosen random agent in population
            Individual& rOriginal = maPopulation[x];
            Individual aCandidate(rOriginal);

            // pick three different random points from population
            do
            {
                a = maRandomPopulation(maGenerator);
            } while (a == x);

            do
            {
                b = maRandomPopulation(maGenerator);
            } while (b == x || b == a);

            do
            {
                c = maRandomPopulation(maGenerator);

            } while (c == x || c == a || c == b);

            size_t randomIndex = maRandomDimensionality(maGenerator);

            for (size_t index = 0; index < mnDimensionality; ++index)
            {
                double randomCrossoverProbability = maRandom01(maGenerator);
                if (index == randomIndex || randomCrossoverProbability < mnCrossoverProbability)
                {
                    double fVarA = maPopulation[a].mVariables[index];
                    double fVarB = maPopulation[b].mVariables[index];
                    double fVarC = maPopulation[c].mVariables[index];

                    double fNewValue = fVarA + mnDifferentialWeight * (fVarB - fVarC);
                    fNewValue = mrDataProvider.boundVariable(index, fNewValue);
                    aCandidate.mVariables[index] = fNewValue;
                }
            }

            double fCandidateFitness = mrDataProvider.calculateFitness(aCandidate.mVariables);

            // see if is better than original, if so replace
            if (fCandidateFitness > mrDataProvider.calculateFitness(rOriginal.mVariables))
            {
                maPopulation[x] = aCandidate;

                if (fCandidateFitness > mfBestFitness)
                {
                    if (std::abs(fCandidateFitness - mfBestFitness) > constAcceptedPrecision)
                    {
                        bBestChanged = true;
                        mnLastChange = mnGeneration;
                    }
                    mfBestFitness = fCandidateFitness;
                    maBestCandidate = maPopulation[x];
                }
            }
        }
        mnGeneration++;
        return bBestChanged;
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
