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

#ifndef INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_FP_HXX
#define INCLUDED_BRIDGES_SOURCE_CPP_UNO_CC5_SOLARIS_SPARC64_FP_HXX

#include "sal/config.h"

extern "C" {

void fp_loadf0(float *);
void fp_loadf1(float *);
void fp_loadf2(float *);
void fp_loadf3(float *);
void fp_loadf4(float *);
void fp_loadf5(float *);
void fp_loadf6(float *);
void fp_loadf7(float *);
void fp_loadf9(float *);
void fp_loadf11(float *);
void fp_loadf13(float *);
void fp_loadf15(float *);
void fp_loadf17(float *);
void fp_loadf19(float *);
void fp_loadf21(float *);
void fp_loadf23(float *);
void fp_loadf25(float *);
void fp_loadf27(float *);
void fp_loadf29(float *);
void fp_loadf31(float *);

void fp_storef0(float *);
void fp_storef1(float *);
void fp_storef2(float *);
void fp_storef3(float *);
void fp_storef4(float *);
void fp_storef5(float *);
void fp_storef6(float *);
void fp_storef7(float *);
void fp_storef9(float *);
void fp_storef11(float *);
void fp_storef13(float *);
void fp_storef15(float *);
void fp_storef17(float *);
void fp_storef19(float *);
void fp_storef21(float *);
void fp_storef23(float *);
void fp_storef25(float *);
void fp_storef27(float *);
void fp_storef29(float *);
void fp_storef31(float *);

void fp_loadd0(double *);
void fp_loadd2(double *);
void fp_loadd4(double *);
void fp_loadd6(double *);
void fp_loadd8(double *);
void fp_loadd10(double *);
void fp_loadd12(double *);
void fp_loadd14(double *);
void fp_loadd16(double *);
void fp_loadd18(double *);
void fp_loadd20(double *);
void fp_loadd22(double *);
void fp_loadd24(double *);
void fp_loadd26(double *);
void fp_loadd28(double *);
void fp_loadd30(double *);

void fp_stored0(double *);
void fp_stored2(double *);
void fp_stored4(double *);
void fp_stored6(double *);
void fp_stored8(double *);
void fp_stored10(double *);
void fp_stored12(double *);
void fp_stored14(double *);
void fp_stored16(double *);
void fp_stored18(double *);
void fp_stored20(double *);
void fp_stored22(double *);
void fp_stored24(double *);
void fp_stored26(double *);
void fp_stored28(double *);
void fp_stored30(double *);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
