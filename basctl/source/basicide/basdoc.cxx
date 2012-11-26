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
#include "precompiled_basctl.hxx"


#include <ide_pch.hxx>
#include <svx/svxids.hrc>

#define GLOBALOVERFLOW2

#include <sfx2/docfac.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <vcl/status.hxx>

#include <svx/xmlsecctrl.hxx>

#include <basdoc.hxx>

#define BasicDocShell
#include <basslots.hxx>

#include "basicmod.hxx"
#include "unomodel.hxx"

DBG_NAME(BasicDocShell);

SFX_IMPL_OBJECTFACTORY( BasicDocShell, SvGlobalName(), SFXOBJECTSHELL_STD_NORMAL, "sbasic" )

SFX_IMPL_INTERFACE( BasicDocShell, SfxObjectShell, IDEResId( 0 ) )
{
    SFX_STATUSBAR_REGISTRATION( IDEResId( SID_BASICIDE_STATUSBAR ) );
}

BasicDocShell::BasicDocShell()
    :SfxObjectShell( SFXMODEL_DISABLE_EMBEDDED_SCRIPTS | SFXMODEL_DISABLE_DOCUMENT_RECOVERY )
{
    pPrinter = 0;
    SetPool( &SFX_APP()->GetPool() );
    SetBaseModel( new SIDEModel(this) );
}

__EXPORT BasicDocShell::~BasicDocShell()
{
    delete pPrinter;
}

SfxPrinter* BasicDocShell::GetPrinter( sal_Bool bCreate )
{
    if ( !pPrinter && bCreate )
        pPrinter = new SfxPrinter( new SfxItemSet( GetPool(), SID_PRINTER_NOTFOUND_WARN , SID_PRINTER_NOTFOUND_WARN ) );

    return pPrinter;
}

void BasicDocShell::SetPrinter( SfxPrinter* pPr )
{
    if ( pPr != pPrinter )
    {
        delete pPrinter;
        pPrinter = pPr;
    }
}

void BasicDocShell::FillClass( SvGlobalName*, sal_uInt32*, String*, String*, String*, sal_Int32, sal_Bool bTemplate) const
{
    (void)bTemplate;
    DBG_ASSERT( bTemplate == sal_False, "No template for Basic" );
}

void BasicDocShell::Draw( OutputDevice *, const JobSetup &, sal_uInt16 )
{}

