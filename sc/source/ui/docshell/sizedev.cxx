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

#include <sfx2/printer.hxx>
#include <vcl/virdev.hxx>

#include "sizedev.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "inputopt.hxx"



ScSizeDeviceProvider::ScSizeDeviceProvider( ScDocShell* pDocSh )
{
    sal_Bool bTextWysiwyg = SC_MOD()->GetInputOptions().GetTextWysiwyg();
    if ( bTextWysiwyg )
    {
        pDevice = pDocSh->GetPrinter();
        bOwner = false;

        aOldMapMode = pDevice->GetMapMode();
        pDevice->SetMapMode( MAP_PIXEL );       // GetNeededSize needs pixel MapMode
        // printer has right DigitLanguage already
    }
    else
    {
        pDevice = new VirtualDevice;
        pDevice->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
        bOwner = true;
    }

    Point aLogic = pDevice->LogicToPixel( Point(1000,1000), MAP_TWIP );
    nPPTX = aLogic.X() / 1000.0;
    nPPTY = aLogic.Y() / 1000.0;

    if ( !bTextWysiwyg )
        nPPTX /= pDocSh->GetOutputFactor();
}

ScSizeDeviceProvider::~ScSizeDeviceProvider()
{
    if (bOwner)
        delete pDevice;
    else
        pDevice->SetMapMode( aOldMapMode );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
