/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_FORMULA_FUNCVARARGS_H
#define INCLUDED_FORMULA_FUNCVARARGS_H

/** Used to indicate a variable number of parameters for the Function Wizard.

    VAR_ARGS if variable number of parameters, or VAR_ARGS+number if number of
    fixed parameters and variable arguments following.

    @see formula::ParaWin
    @see ScFuncDescCore

    @NOTE: the value can't be easily changed. If changed then
    reportdesign/source/ui/misc/FunctionHelper.cxx
    FunctionDescription::getVarArgsStart() has to provide some backward
    compatibility for implicit API stability.

    @NOTE: also
    reportbuilder/java/org/libreoffice/report/pentaho/StarFunctionDescription.java
    uses a hard coded value in StarFunctionDescription::getArguments() for
    functionDescription.isInfiniteParameterCount() which though looks like it
    could be easily adapted.
 */
#define VAR_ARGS            30

/** Used to indicate a variable number of paired parameters for the Function Wizard.

    PAIRED_VAR_ARGS if variable number of paired parameters, or
    PAIRED_VAR_ARGS+number if number of fixed parameters and variable paired
    arguments following.

    @see VAR_ARGS
 */
#define PAIRED_VAR_ARGS     (VAR_ARGS + VAR_ARGS)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
