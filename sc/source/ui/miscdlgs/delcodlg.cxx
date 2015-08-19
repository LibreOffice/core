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

#undef SC_DLLIMPLEMENTATION

#include "delcodlg.hxx"
#include "scresid.hxx"
#include "miscdlgs.hrc"

bool       ScDeleteContentsDlg::bPreviousAllCheck = false;
InsertDeleteFlags ScDeleteContentsDlg::nPreviousChecks   = (IDF_DATETIME | IDF_STRING  |
                                                 IDF_NOTE     | IDF_FORMULA |
                                                 IDF_VALUE);

ScDeleteContentsDlg::ScDeleteContentsDlg( vcl::Window* pParent,
                                          InsertDeleteFlags nCheckDefaults ) :
    ModalDialog     ( pParent, "DeleteContentsDialog", "modules/scalc/ui/deletecontents.ui" ),

    bObjectsDisabled( false )
{
    get( aBtnDelAll, "deleteall" );
    get( aBtnDelStrings, "text" );
    get( aBtnDelNumbers, "numbers" );
    get( aBtnDelDateTime, "datetime" );
    get( aBtnDelFormulas, "formulas" );
    get( aBtnDelNotes, "comments" );
    get( aBtnDelAttrs, "formats" );
    get( aBtnDelObjects, "objects" );
    get( aBtnOk, "ok" );

    if ( nCheckDefaults != IDF_NONE )
    {
        ScDeleteContentsDlg::nPreviousChecks = nCheckDefaults;
        ScDeleteContentsDlg::bPreviousAllCheck = false;
    }

    aBtnDelAll->Check     ( ScDeleteContentsDlg::bPreviousAllCheck );
    aBtnDelStrings->Check ( IS_SET( IDF_STRING,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelNumbers->Check ( IS_SET( IDF_VALUE,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelDateTime->Check( IS_SET( IDF_DATETIME,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelFormulas->Check( IS_SET( IDF_FORMULA,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelNotes->Check   ( IS_SET( IDF_NOTE,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelAttrs->Check   ( IS_SET( IDF_ATTRIB,
                                   ScDeleteContentsDlg::nPreviousChecks ) );
    aBtnDelObjects->Check ( IS_SET( IDF_OBJECTS,
                                   ScDeleteContentsDlg::nPreviousChecks ) );

    DisableChecks( aBtnDelAll->IsChecked() );

    aBtnDelAll->SetClickHdl( LINK( this, ScDeleteContentsDlg, DelAllHdl ) );
}

ScDeleteContentsDlg::~ScDeleteContentsDlg()
{
    disposeOnce();
}

void ScDeleteContentsDlg::dispose()
{
    aBtnDelAll.clear();
    aBtnDelStrings.clear();
    aBtnDelNumbers.clear();
    aBtnDelDateTime.clear();
    aBtnDelFormulas.clear();
    aBtnDelNotes.clear();
    aBtnDelAttrs.clear();
    aBtnDelObjects.clear();
    aBtnOk.clear();
    ModalDialog::dispose();
}

InsertDeleteFlags ScDeleteContentsDlg::GetDelContentsCmdBits() const
{
    ScDeleteContentsDlg::nPreviousChecks = IDF_NONE;

    if ( aBtnDelStrings->IsChecked() )
        ScDeleteContentsDlg::nPreviousChecks  = IDF_STRING;
    if ( aBtnDelNumbers->IsChecked() )
        ScDeleteContentsDlg::nPreviousChecks |= IDF_VALUE;
    if ( aBtnDelDateTime->IsChecked())
        ScDeleteContentsDlg::nPreviousChecks |= IDF_DATETIME;
    if ( aBtnDelFormulas->IsChecked())
        ScDeleteContentsDlg::nPreviousChecks |= IDF_FORMULA;
    if ( aBtnDelNotes->IsChecked()   )
        ScDeleteContentsDlg::nPreviousChecks |= IDF_NOTE;
    if ( aBtnDelAttrs->IsChecked()   )
        ScDeleteContentsDlg::nPreviousChecks |= IDF_ATTRIB;
    if ( aBtnDelObjects->IsChecked() )
        ScDeleteContentsDlg::nPreviousChecks |= IDF_OBJECTS;

    ScDeleteContentsDlg::bPreviousAllCheck = aBtnDelAll->IsChecked();

    return ( (ScDeleteContentsDlg::bPreviousAllCheck)
                ? IDF_ALL
                : ScDeleteContentsDlg::nPreviousChecks );
}

void ScDeleteContentsDlg::DisableChecks( bool bDelAllChecked )
{
    if ( bDelAllChecked )
    {
        aBtnDelStrings->Disable();
        aBtnDelNumbers->Disable();
        aBtnDelDateTime->Disable();
        aBtnDelFormulas->Disable();
        aBtnDelNotes->Disable();
        aBtnDelAttrs->Disable();
        aBtnDelObjects->Disable();
    }
    else
    {
        aBtnDelStrings->Enable();
        aBtnDelNumbers->Enable();
        aBtnDelDateTime->Enable();
        aBtnDelFormulas->Enable();
        aBtnDelNotes->Enable();
        aBtnDelAttrs->Enable();
        if (bObjectsDisabled)
            aBtnDelObjects->Disable();
        else
            aBtnDelObjects->Enable();
    }
}

void ScDeleteContentsDlg::DisableObjects()
{
    bObjectsDisabled = true;
    aBtnDelObjects->Check(false);
    aBtnDelObjects->Disable();
}

IMPL_LINK_NOARG_TYPED(ScDeleteContentsDlg, DelAllHdl, Button*, void)
{
    DisableChecks( aBtnDelAll->IsChecked() );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
