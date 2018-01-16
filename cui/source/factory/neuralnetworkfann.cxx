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

#include "neuralnetworkfann.hxx"

NeuralNetworkFann::NeuralNetworkFann(sal_uInt32 nLayers, const sal_uInt32* nLayer) :
    ann(fann_create_standard_array(nLayers, nLayer), fann_destroy),
    data(nullptr, fann_destroy_train)
{
}

NeuralNetworkFann::NeuralNetworkFann(const OUString& file) :
    ann(fann_create_from_file(OUStringToOString(file, RTL_TEXTENCODING_ASCII_US).pData->buffer), fann_destroy),
    data(nullptr, fann_destroy_train)
{
}

void NeuralNetworkFann::SetActivationFunction(ActivationFunction function)
{
    fann_activationfunc_enum func;

    switch (function)
    {
        case ActivationFunction::SIGMOID:
        {
            func = FANN_SIGMOID_SYMMETRIC;
            break;
        }
        default:
        {
            return;
        }
    }
    fann_set_activation_function_hidden(ann.get(), func);
    fann_set_activation_function_output(ann.get(), func);
}

void NeuralNetworkFann::SetTrainingAlgorithm(TrainingAlgorithm algorithm)
{
    fann_train_enum algo;

    switch (algorithm)
    {
        case TrainingAlgorithm::INCREMENTAL:
        {
            algo = FANN_TRAIN_INCREMENTAL;
            break;
        }
        default:
        {
            return;
        }
    }
    fann_set_training_algorithm(ann.get(), algo);
}

void NeuralNetworkFann::SetLearningRate(float rate)
{
   fann_set_learning_rate(ann.get(), rate);
}

#include <iostream>

void NeuralNetworkFann::InitTraining(sal_uInt32 nExamples)
{
    data.reset(fann_create_train(nExamples, fann_get_num_input(ann.get()), fann_get_num_output(ann.get())));
}

void NeuralNetworkFann::Save(const OUString& file)
{
    OString o = OUStringToOString(file, RTL_TEXTENCODING_ASCII_US);
    fann_save(ann.get(), o.pData->buffer);
}

sal_uInt32 NeuralNetworkFann::GetNumInput()
{
    return fann_get_num_input(ann.get());
}

float* NeuralNetworkFann::GetInput(sal_uInt32 nIeme)
{
    return &data->input[nIeme][0];
}

sal_uInt32 NeuralNetworkFann::GetNumOutput()
{
    return fann_get_num_output(ann.get());
}

float* NeuralNetworkFann::GetOutput(sal_uInt32 nIeme)
{
    return &data->output[nIeme][0];
}

float* NeuralNetworkFann::Run(float *data_input)
{
    return fann_run(ann.get(), data_input);
}

void NeuralNetworkFann::Train(sal_uInt32 nEpochs, float error)
{
    fann_train_on_data(ann.get(), data.get(), nEpochs, 1, error);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
