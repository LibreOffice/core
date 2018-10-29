/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <externaltransformationuno.hxx>

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <document.hxx>

using namespace com::sun::star;

ScExternalTransformationObj::ScExternalTransformationObj(ScDocShell* pDocShell, SCTAB nTab)
    : mnTab(nTab)
    , mpDocShell(pDocShell)
{
    pDocShell->GetDocument().AddUnoObject(*this);
}

ScExternalTransformationObj::~ScExternalTransformationObj()
{
    if (mpDocShell)
        mpDocShell->GetDocument().RemoveUnoObject(*this);
}

void SAL_CALL ScExternalTransformationObj::Transform() {}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
