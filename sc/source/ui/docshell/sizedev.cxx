/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



#include <sfx2/printer.hxx>
#include <vcl/virdev.hxx>

#include "sizedev.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "inputopt.hxx"

//------------------------------------------------------------------

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
        bOwner = sal_True;
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
