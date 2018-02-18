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
#ifndef INCLUDED_CUI_SOURCE_FACTORY_NEURALNETWORKINTERNAL_HXX
#define INCLUDED_CUI_SOURCE_FACTORY_NEURALNETWORKINTERNAL_HXX

#include "neuralnetwork.hxx"

#include <fann.h>
#include <vector>

class NeuralNetworkInternal : public AbstractNeuralNetwork
{
public:
    NeuralNetworkInternal(sal_uInt32 nLayers, const sal_uInt32* nLayer);
    NeuralNetworkInternal(const OUString& file);

    void SetActivationFunction(ActivationFunction function) override;
    void SetTrainingAlgorithm(TrainingAlgorithm algorithm) override;
    void SetLearningRate(float rate) override;

    void InitTraining(sal_uInt32 nExamples) override;
    sal_uInt32 GetNumInput() override;
    float* GetInput(sal_uInt32 nIeme) override;
    sal_uInt32 GetNumOutput() override;
    float* GetOutput(sal_uInt32 nIeme) override;

    void Train(sal_uInt32 nEpochs, float error) override;
    void Run(float *data_input, float* result) override;
    void Save(const OUString& file) override;

    virtual ~NeuralNetworkInternal(){}

    virtual void * GetTrain(){return nullptr;}

private:
    enum class FunctionTrans
    {
        Sigmoid,
        SigmoidSymmetric
    };
    struct Neuron
    {
        // value of neuron.
        // fann: value
        // 0: input.
        // last: output.
        float a;
        // weight
        // fann: weights[first_con to last_con-2]
        // Useless for n[0]
        std::vector<float> w; // Number of neuron of the next layer without biais.
        std::vector<float> dw; // Number of neuron of the next layer without biais.
        // biais.
        // fann: weights[last_con-1]
        // function
        // Useless for n[0]
        FunctionTrans f;
        // Steepness.
        float stp;

        // temporary field.
        // gradient
        float s;
        float sum;

        // b : between [-.1;.1]
        Neuron() : a(0.), w(), f(FunctionTrans::Sigmoid), stp(1.f), sum(0) {}
    };
    std::vector<std::vector<Neuron>> n; // One neuron is biais.
    float learning_rate;
    float learning_rate_alpha;
    std::vector<std::vector<float>> learning_input;
    std::vector<std::vector<float>> learning_output;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
