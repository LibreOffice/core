/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "neuralnetworkinternal.hxx"

#include <iostream>
#include <fstream>

NeuralNetworkInternal::NeuralNetworkInternal(sal_uInt32 nLayers, const sal_uInt32* nLayer) :
    n(nLayers, std::vector<Neuron>()),
    learning_rate(0.2),
    learning_rate_alpha(0.4),
    learning_input(),
    learning_output()
{
    for (sal_uInt32 i = 0; i < nLayers; i++)
    {
        n[i].resize(nLayer[i]+1); // One neuron is biais.

        if (i != nLayers-1)
        {
            for (sal_uInt32 j = 0; j < n[i].size() /*with biais*/; j++)
            {
                n[i][j].w.resize(nLayer[i+1]); /*no biais*/
                n[i][j].dw.resize(nLayer[i+1]);
                for (sal_uInt32 k = 0; k < nLayer[i+1]; k++)
                {
                    // Between [0;.1]
                    n[i][j].w[k] = (float)std::rand() / (float)RAND_MAX *0.2 - 0.1;
                }
            }
        }
    }
}

NeuralNetworkInternal::NeuralNetworkInternal(const OUString& file) :
    n(), learning_rate(0.), learning_rate_alpha(0.), learning_input(),
    learning_output()
{
    OString o = OUStringToOString(file, RTL_TEXTENCODING_ASCII_US);
    const char * o_str = o.pData->buffer;

    std::ifstream infile (o_str, std::ifstream::binary);
    sal_uInt32 tmpInt;

    infile.read(reinterpret_cast<char *>(&learning_rate), sizeof(learning_rate));
    infile.read(reinterpret_cast<char *>(&learning_rate_alpha), sizeof(learning_rate_alpha));
    infile.read(reinterpret_cast<char *>(&tmpInt), sizeof(tmpInt));
    n.resize(tmpInt);

    for (sal_uInt32 i = 0; i < n.size(); i++)
    {
        infile.read(reinterpret_cast<char *>(&tmpInt), sizeof(tmpInt));
        n[i].resize(tmpInt);
    }

    for (sal_uInt32 i = 0; i < n.size(); i++)
    {
        if (i != n.size()-1)
        {
            for (sal_uInt32 j = 0; j < n[i].size(); j++)
            {
                n[i][j].w.resize(n[i+1].size());
                n[i][j].dw.resize(n[i+1].size());
            }
        }
    }

    for (sal_uInt32 i = 0; i < n.size(); i++)
        for (sal_uInt32 j = 0; j < n[i].size(); j++)
        {
            infile.read(reinterpret_cast<char *>(&n[i][j].w[0]), n[i][j].w.size()*sizeof(n[i][j].w[0]));
            infile.read(reinterpret_cast<char *>(&n[i][j].dw[0]), n[i][j].dw.size()*sizeof(n[i][j].dw[0]));
        }

    for (sal_uInt32 i = 0; i < n.size(); i++)
        for (sal_uInt32 j = 0; j < n[i].size(); j++)
        {
            infile.read(reinterpret_cast<char *>(&n[i][j].f), sizeof(n[i][j].f));
            infile.read(reinterpret_cast<char *>(&n[i][j].stp), sizeof(n[i][j].stp));
        }
}

void NeuralNetworkInternal::SetActivationFunction(ActivationFunction function)
{
    FunctionTrans func;

    switch (function)
    {
        case ActivationFunction::SIGMOID:
        {
            func = FunctionTrans::Sigmoid;
            break;
        }
        case ActivationFunction::SIGMOID_SYMMETRIC:
        {
            func = FunctionTrans::SigmoidSymmetric;
            break;
        }
        default:
        {
            return;
        }
    }

    for (sal_uInt32 i = 1; i < n.size(); i++)
    {
        for (sal_uInt32 j = 0; j < n[i].size(); j++)
        {
            n[i][j].f = func;
        }
    }
}

void NeuralNetworkInternal::SetTrainingAlgorithm(TrainingAlgorithm algorithm)
{
    return;
}

void NeuralNetworkInternal::SetLearningRate(float rate)
{
    learning_rate = rate;
}

void NeuralNetworkInternal::InitTraining(sal_uInt32 nExamples)
{
    learning_input.resize(nExamples, std::vector<float>(n[0].size(), 0));
    learning_output.resize(nExamples, std::vector<float>(n.back().size(), 0));
}

sal_uInt32 NeuralNetworkInternal::GetNumInput()
{
    return n[0].size();
}

float* NeuralNetworkInternal::GetInput(sal_uInt32 nIeme)
{
    if (nIeme >= learning_input.size())
        return nullptr;
    return &learning_input[nIeme][0];
}

sal_uInt32 NeuralNetworkInternal::GetNumOutput()
{
    return n.back().size();
}

float* NeuralNetworkInternal::GetOutput(sal_uInt32 nIeme)
{
    if (nIeme >= learning_output.size())
        return nullptr;
    return &learning_output[nIeme][0];
}

void NeuralNetworkInternal::Train(sal_uInt32 nEpochs, float error)
{
    for (sal_uInt32 l = 0; l < nEpochs; l++)
    {
        for (sal_uInt32 m = 0; m < learning_input.size(); m++)
        {
            Run(&learning_input[m][0], nullptr);

            if (l == nEpochs-1)
            {
                for (sal_uInt32 i = 0; i < n.back().size(); i++)
                    std::cout << n.back()[i].a << " ";
                std::cout << std::endl;
            }

            // error
            float errori = 0.f;
            for (sal_uInt32 i = 0; i < n.back().size()-1 /*no biais*/; i++)
            {
                errori += (learning_input[m][i] - n.back()[i].a)*(learning_input[m][i] - n.back()[i].a);
            }
            errori = errori / (n.back().size()-1);
            errori = sqrt(errori);
            std::cout << "error " << l << "," << m << ":" << errori << std::endl;

            // Calcul de sM
            // s: ann->train_errors
            for (sal_uInt32 i = 0; i < n.back().size()-1 /*no biais*/; i++)
            {
                float diff = learning_output[m][i] - n.back()[i].a;
                switch (n.back()[i].f)
                {
                    case FunctionTrans::Sigmoid:
                        {
                            n.back()[i].s = 2.*n.back()[i].stp*n.back()[i].a*(1.-n.back()[i].a)*diff;
                            break;
                        }
                    case FunctionTrans::SigmoidSymmetric:
                        {
                            // If Symmetric: diff/2.
                            diff = diff / 2.;
                            n.back()[i].s = n.back()[i].stp*(1.-n.back()[i].a*n.back()[i].a)*diff;
                            break;
                        }

                        /*
ftLineaire  : RNA.s[High(RNA.s)][I] := -2*(SortiesTest[I] - RNA.a[High(RNA.a)][I]);

ftTanHyperb : RNA.s[High(RNA.s)][I] := -2*(1-RNA.a[High(RNA.a)][I]*RNA.a[High(RNA.a)][I])*

(SortiesTest[I] - RNA.a[High(RNA.a)][I])
*/
                }
            }

            // Calcul des sk
            // fann_activation_derived
            for (sal_uInt32 i = n.size()-2 /*No Output layer*/; i > 0; i--)
            {
                for (sal_uInt32 j = 0; j < n[i].size() /*with biais*/; j++)
                {
                    float sum = 0.f;
                    //connection_rate >= 1
                    for (sal_uInt32 k = 0; k < n[i+1].size()-1 /*no biais*/; k++)
                    {
                        sum += n[i][j].w[k] * n[i+1][k].s;
                    }
                    switch (n[i][j].f)
                    {
                        case FunctionTrans::Sigmoid:
                            {
                                n[i][j].s = sum * 2. * n[i][j].stp * n[i][j].a * (1. - n[i][j].a);
                                break;
                            }
                        case FunctionTrans::SigmoidSymmetric:
                            {
                                n[i][j].s = sum * n[i][j].stp * (1. - n[i][j].a*n[i][j].a);
                                break;
                            }
                        default:
                            {
                                return;
                            }

                            // Cette ligne est volontairement commentée car pour une fonction linéaire,
                            // il aurait fallu mettre RNA.s[I][J] := RNA.s[I][J] * 1, ce qui ne sert à rien
                            //        ftLineaire : RNA.s[I][J] := RNA.s[I][J] * 1;

                            //        ftTanHyperb : RNA.s[I][J] := RNA.s[I][J] * (1 - RNA.a[I][J]*RNA.a[I][J]);
                    }
                }
            }

            // Modifications des poids et des biais
            for (sal_uInt32 i = n.size()-1 /*No input layer*/; i > 0; --i)
            {
                for (sal_uInt32 j = 0; j < n[i].size() - 1 /*no biais*/; j++)
                {
                    for (sal_uInt32 k = 0; k < n[i-1].size() /*with biais*/; k++)
                    {
                        float delta = learning_rate * n[i][j].s * n[i-1][k].a + learning_rate_alpha * n[i-1][k].dw[j];
                        n[i-1][k].dw[j] = delta;
                        n[i-1][k].w[j] += delta;
                    }
                }
            }
        }
    }
}

void NeuralNetworkInternal::Run(float *data_input, float *result)
{
    // Calcul de a0
    for (sal_uInt32 i = 0; i < n[0].size(); i++)
        n[0][i].a = data_input[i];

    // Calcul des ak

    for (sal_uInt32 i = 1; i < n.size(); i++)
    {
        for (sal_uInt32 j = 0; j < n[i].size()-1 /*no biais*/; j++)
        {
            n[i][j].sum = 0;
            for (sal_uInt32 k = 0; k < n[i-1].size() /*with biais*/; k++)
                n[i][j].sum += n[i-1][k].a * n[i-1][k].w[j];
            n[i][j].sum *= n[i][j].stp;
            switch (n[i][j].f)
            {
                case FunctionTrans::Sigmoid:
                    {
                        n[i][j].a = 1.f / (1.f + exp(-2.f*n[i][j].sum));
                        break;
                    }
                case FunctionTrans::SigmoidSymmetric:
                    {
                        n[i][j].a = 2.f/(1.f + exp(-2.f/n[i][j].sum)) - 1.f;
                        break;
                    }
                default:
                    {
                        return;
                    }
            }
        }
    }


    if (result == nullptr)
    {
        return;
    }
    for (sal_uInt32 i = 0; i < n.back().size(); i++)
        result[i] = n.back()[i].a;
}

void NeuralNetworkInternal::Save(const OUString& file)
{
    OString o = OUStringToOString(file, RTL_TEXTENCODING_ASCII_US);
    const char * o_str = o.pData->buffer;

    std::ofstream outfile (o_str, std::ofstream::binary);

    outfile.write(reinterpret_cast<const char *>(&learning_rate), sizeof(learning_rate));
    outfile.write(reinterpret_cast<const char *>(&learning_rate_alpha), sizeof(learning_rate_alpha));

    sal_uInt32 tmpInt = n.size();
    outfile.write(reinterpret_cast<const char *>(&tmpInt), sizeof(tmpInt));

    for (sal_uInt32 i = 0; i < n.size(); i++)
    {
        tmpInt = n[i].size();
        outfile.write(reinterpret_cast<const char *>(&tmpInt), sizeof(tmpInt));
    }

    for (sal_uInt32 i = 0; i < n.size(); i++)
        for (sal_uInt32 j = 0; j < n[i].size(); j++)
        {
            outfile.write(reinterpret_cast<const char *>(&n[i][j].w[0]), n[i][j].w.size()*sizeof(n[i][j].w[0]));
            outfile.write(reinterpret_cast<const char *>(&n[i][j].dw[0]), n[i][j].dw.size()*sizeof(n[i][j].dw[0]));
        }

    for (sal_uInt32 i = 0; i < n.size(); i++)
        for (sal_uInt32 j = 0; j < n[i].size(); j++)
        {
            outfile.write(reinterpret_cast<const char *>(&n[i][j].f), sizeof(n[i][j].f));
            outfile.write(reinterpret_cast<const char *>(&n[i][j].stp), sizeof(n[i][j].stp));
        }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
