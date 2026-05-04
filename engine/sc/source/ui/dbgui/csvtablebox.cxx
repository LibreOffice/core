/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <csvtablebox.hxx>
#include <osl/diagnose.h>
#include <comphelper/kit.hxx>

ScCsvTableBox::ScCsvTableBox(weld::Builder& rBuilder)
    : mxGrid(new ScCsvGrid(maData, this))
{
    if (comphelper::COKit::isActive())
        mxGridWeld.reset(new weld::CustomClientWeld(rBuilder, u"csvgrid"_ustr, *mxGrid));

    mbFixedMode = false;
    mnFixedWidth = 1;

    Link<ScCsvControl&,void> aLink = LINK( this, ScCsvTableBox, CsvCmdHdl );
    mxGrid->SetCmdHdl( aLink );
}

ScCsvTableBox::~ScCsvTableBox()
{
    if (mbFixedMode)
        mxGrid->SaveFixedWidthList();
}

// common table box handling --------------------------------------------------

void ScCsvTableBox::Refresh()
{
    mxGrid->DisableRepaint();
    if (mbFixedMode)
    {
        mxGrid->Execute( CSVCMD_SETPOSCOUNT, mnFixedWidth );
        mxGrid->SetSplits( maFixedSplits );
        mxGrid->SetColumnStates( std::vector(maFixColStates) );
    }
    else
    {
        mxGrid->Execute( CSVCMD_SETPOSCOUNT, 1 );
        mxGrid->Execute( CSVCMD_NEWCELLTEXTS );
        mxGrid->SetColumnStates( std::vector(maSepColStates) );
    }
    mxGrid->EnableRepaint();
}

void ScCsvTableBox::SetSeparatorsMode()
{
    if( !mbFixedMode )
        return;

    // rescue data for fixed width mode
    mnFixedWidth = mxGrid->GetPosCount();
    maFixColStates = mxGrid->GetColumnStates();
    maFixedSplits = mxGrid->GetSplits();
    mxGrid->SaveFixedWidthList();
    // switch to separators mode
    mbFixedMode = false;
    // reset and reinitialize controls
    Refresh();
}

void ScCsvTableBox::SetFixedWidthMode()
{
    if( mbFixedMode )
        return;

    // rescue data for separators mode
    maSepColStates = mxGrid->GetColumnStates();
    // switch to fixed width mode
    mbFixedMode = true;
    maFixedSplits = ScCsvGrid::LoadFixedWidthList();
    // reset and reinitialize controls
    Refresh();
}

// cell contents --------------------------------------------------------------

void ScCsvTableBox::SetUniStrings(
        const std::vector<OUString>& rTextLines, const OUString& rSepChars,
        sal_Unicode cTextSep, bool bMergeSep, bool bRemoveSpace )
{
    mxGrid->DisableRepaint();
    sal_Int32 nLineCount = static_cast<sal_Int32>(rTextLines.size());
    for( sal_Int32 nLine = 0; nLine < nLineCount; ++nLine )
    {
        if( mbFixedMode )
            mxGrid->ImplSetTextLineFix( nLine, rTextLines[nLine] );
        else
            mxGrid->ImplSetTextLineSep( nLine, rTextLines[nLine], rSepChars, cTextSep, bMergeSep, bRemoveSpace );
    }
    mxGrid->EnableRepaint();
}

// column settings ------------------------------------------------------------

void ScCsvTableBox::InitTypes(const weld::ComboBox& rListBox)
{
    const sal_Int32 nTypeCount = rListBox.get_count();
    std::vector<OUString> aTypeNames( nTypeCount );
    for( sal_Int32 nIndex = 0; nIndex < nTypeCount; ++nIndex )
        aTypeNames[ nIndex ] = rListBox.get_text( nIndex );
    mxGrid->SetTypeNames( std::move(aTypeNames) );
}

void ScCsvTableBox::FillColumnData( ScAsciiOptions& rOptions ) const
{
    if( mbFixedMode )
        mxGrid->FillColumnDataFix( rOptions );
    else
        mxGrid->FillColumnDataSep( rOptions );
}

// event handling -------------------------------------------------------------

IMPL_LINK( ScCsvTableBox, CsvCmdHdl, ScCsvControl&, rCtrl, void )
{
    const ScCsvCmd& rCmd = rCtrl.GetCmd();
    ScCsvCmdType eType = rCmd.GetType();
    sal_Int32 nParam1 = rCmd.GetParam1();

    switch( eType )
    {
        case CSVCMD_REPAINT:
            if( !mxGrid->IsNoRepaint() )
                mxGrid->Invalidate();
        break;

        case CSVCMD_SETPOSCOUNT:
        {
            const sal_Int32 nOldPosCount = maData.mnPosCount;
            maData.mnPosCount = std::max( nParam1, sal_Int32( 1 ) );
            if( maData.mnPosCount != nOldPosCount )
                mxGrid->ApplyPosCountChange( nOldPosCount );
        }
        break;

        case CSVCMD_NEWCELLTEXTS:
            if( mbFixedMode )
                mxGrid->Execute( CSVCMD_UPDATECELLTEXTS );
            else
            {
                mxGrid->DisableRepaint();
                ScCsvColStateVec aStates( mxGrid->GetColumnStates() );
                mxGrid->Execute( CSVCMD_SETPOSCOUNT, 1 );
                mxGrid->Execute( CSVCMD_UPDATECELLTEXTS );
                mxGrid->SetColumnStates( std::move(aStates) );
                mxGrid->EnableRepaint();
            }
        break;
        case CSVCMD_UPDATECELLTEXTS:
            maUpdateTextHdl.Call( *this );
        break;
        case CSVCMD_SETCOLUMNTYPE:
            mxGrid->SetSelColumnType( nParam1 );
        break;
        case CSVCMD_EXPORTCOLUMNTYPE:
            maColTypeHdl.Call( *this );
        break;
        case CSVCMD_SETFIRSTIMPORTLINE:
            mxGrid->SetFirstImportedLine( nParam1 );
        break;

        case CSVCMD_INSERTSPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::InsertSplit - invalid call" );
            mxGrid->InsertSplit( nParam1 );
        break;
        case CSVCMD_REMOVESPLIT:
            OSL_ENSURE( mbFixedMode, "ScCsvTableBox::CsvCmdHdl::RemoveSplit - invalid call" );
            mxGrid->RemoveSplit( nParam1 );
        break;
        case CSVCMD_TOGGLESPLIT:
            mxGrid->Execute( mxGrid->HasSplit( nParam1 ) ? CSVCMD_REMOVESPLIT : CSVCMD_INSERTSPLIT, nParam1 );
        break;
        default:
        break;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
