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

#include <sfx2/app.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/printer.hxx>
#include <sfx2/objface.hxx>
#include <sfx2/objsh.hxx>
#include <svl/itemset.hxx>

#include "unomodel.hxx"

#include <basdoc.hxx>
#define BasicDocShell     // This CANNOT come before basdoc apparently
#include <basslots.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svx/svxids.hrc>

TYPEINIT1(BasicDocShell, SfxObjectShell);
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

BasicDocShell::~BasicDocShell()
{
    delete pPrinter;
}

SfxPrinter* BasicDocShell::GetPrinter( bool bCreate )
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
    DBG_ASSERT( !bTemplate, "No template for Basic" );
}

void BasicDocShell::Draw( OutputDevice *, const JobSetup &, sal_uInt16 )
{}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
