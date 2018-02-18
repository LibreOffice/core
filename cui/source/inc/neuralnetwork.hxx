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
#ifndef INCLUDED_CUI_SOURCE_FACTORY_NEURALNETWORK_HXX
#define INCLUDED_CUI_SOURCE_FACTORY_NEURALNETWORK_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>

class AbstractNeuralNetwork
{
public:
    enum class ActivationFunction {
        SIGMOID, SIGMOID_SYMMETRIC
    };
    enum class TrainingAlgorithm {
        INCREMENTAL
    };
    static AbstractNeuralNetwork * CreateFactory(sal_uInt32 nLayers, const sal_uInt32* nLayer);
    static AbstractNeuralNetwork * CreateFactory(const OUString& file);
    virtual void SetActivationFunction(ActivationFunction function) = 0;
    virtual void SetTrainingAlgorithm(TrainingAlgorithm algorithm) = 0;
    virtual void SetLearningRate(float rate) = 0;

    virtual void InitTraining(sal_uInt32 nExamples) = 0;
    virtual sal_uInt32 GetNumInput() = 0;
    virtual float* GetInput(sal_uInt32 nIeme) = 0;
    virtual sal_uInt32 GetNumOutput() = 0;
    virtual float* GetOutput(sal_uInt32 nIeme) = 0;

    virtual void Train(sal_uInt32 nEpochs, float error) = 0;
    virtual void Run(float *data_input, float* result) = 0;
    virtual void Save(const OUString& file) = 0;

    virtual ~AbstractNeuralNetwork(){}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
