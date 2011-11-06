/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        bOwner = sal_False;

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

