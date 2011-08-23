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

#ifdef PCH
#endif

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <bf_sfx2/printer.hxx>
#include <vcl/virdev.hxx>

#include "sizedev.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "inputopt.hxx"
namespace binfilter {

//------------------------------------------------------------------

/*N*/ ScSizeDeviceProvider::ScSizeDeviceProvider( ScDocShell* pDocSh )
/*N*/ {
/*N*/ 	BOOL bTextWysiwyg = SC_MOD()->GetInputOptions().GetTextWysiwyg();
/*N*/ 	if ( bTextWysiwyg )
/*N*/ 	{
/*?*/ 		pDevice = pDocSh->GetPrinter();
/*?*/ 		bOwner = FALSE;
/*?*/ 
/*?*/ 		aOldMapMode = pDevice->GetMapMode();
/*?*/ 		pDevice->SetMapMode( MAP_PIXEL );		// GetNeededSize needs pixel MapMode
/*?*/ 		// printer has right DigitLanguage already
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pDevice = new VirtualDevice;
/*N*/ 		pDevice->SetDigitLanguage( SC_MOD()->GetOptDigitLanguage() );
/*N*/ 		bOwner = TRUE;
/*N*/ 	}
/*N*/ 
/*N*/ 	Point aLogic = pDevice->LogicToPixel( Point(1000,1000), MAP_TWIP );
/*N*/ 	nPPTX = aLogic.X() / 1000.0;
/*N*/ 	nPPTY = aLogic.Y() / 1000.0;
/*N*/ 
/*N*/ 	if ( !bTextWysiwyg )
/*N*/ 		nPPTX /= pDocSh->GetOutputFactor();
/*N*/ }

/*N*/ ScSizeDeviceProvider::~ScSizeDeviceProvider()
/*N*/ {
/*N*/ 	if (bOwner)
/*N*/ 		delete pDevice;
/*N*/ 	else
/*?*/ 		pDevice->SetMapMode( aOldMapMode );
/*N*/ }

}
