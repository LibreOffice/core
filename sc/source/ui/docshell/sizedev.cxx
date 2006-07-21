/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sizedev.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-07-21 13:45:56 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
    BOOL bTextWysiwyg = SC_MOD()->GetInputOptions().GetTextWysiwyg();
    if ( bTextWysiwyg )
    {
        pDevice = pDocSh->GetPrinter();
        bOwner = FALSE;

        aOldMapMode = pDevice->GetMapMode();
        pDevice->SetMapMode( MAP_PIXEL );       // GetNeededSize needs pixel MapMode
        // printer has right DigitLanguage already
    }
    else
    {
        pDevice = new VirtualDevice;
        pDevice->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
        bOwner = TRUE;
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

