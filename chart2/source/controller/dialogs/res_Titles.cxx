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

#include "res_Titles.hxx"
#include "ResId.hxx"
#include "TitleDialogData.hxx"
#include <svtools/controldims.hrc>

namespace chart
{

TitleResources::TitleResources( VclBuilderContainer& rBuilder, bool bShowSecondaryAxesTitle )
{
    rBuilder.get( m_pFT_Main, "labelMainTitle" );
    rBuilder.get( m_pFT_Sub, "labelSubTitle" );
    rBuilder.get( m_pEd_Main, "maintitle" );
    rBuilder.get( m_pEd_Sub, "subtitle" );

    rBuilder.get( m_pFT_XAxis, "labelPrimaryXaxis" );
    rBuilder.get( m_pFT_YAxis, "labelPrimaryYaxis" );
    rBuilder.get( m_pFT_ZAxis, "labelPrimaryZaxis" );
    rBuilder.get( m_pEd_XAxis, "primaryXaxis" );
    rBuilder.get( m_pEd_YAxis, "primaryYaxis" );
    rBuilder.get( m_pEd_ZAxis, "primaryZaxis" );

    rBuilder.get( m_pFT_SecondaryXAxis, "labelSecondaryXAxis" );
    rBuilder.get( m_pFT_SecondaryYAxis, "labelSecondaryYAxis" );
    rBuilder.get( m_pEd_SecondaryXAxis, "secondaryXaxis" );
    rBuilder.get( m_pEd_SecondaryYAxis, "secondaryYaxis" );

    m_pFT_SecondaryXAxis->Show( bShowSecondaryAxesTitle );
    m_pFT_SecondaryYAxis->Show( bShowSecondaryAxesTitle );
    m_pEd_SecondaryXAxis->Show( bShowSecondaryAxesTitle );
    m_pEd_SecondaryYAxis->Show( bShowSecondaryAxesTitle );
}

TitleResources::~TitleResources()
{
}

void TitleResources::SetUpdateDataHdl( const Link<Edit&,void>& rLink )
{
    sal_uLong nTimeout = 4*EDIT_UPDATEDATA_TIMEOUT;

    m_pEd_Main->EnableUpdateData( nTimeout );
    m_pEd_Main->SetUpdateDataHdl( rLink );

    m_pEd_Sub->EnableUpdateData( nTimeout );
    m_pEd_Sub->SetUpdateDataHdl( rLink );

    m_pEd_XAxis->EnableUpdateData( nTimeout );
    m_pEd_XAxis->SetUpdateDataHdl( rLink );

    m_pEd_YAxis->EnableUpdateData( nTimeout );
    m_pEd_YAxis->SetUpdateDataHdl( rLink );

    m_pEd_ZAxis->EnableUpdateData( nTimeout );
    m_pEd_ZAxis->SetUpdateDataHdl( rLink );

    m_pEd_SecondaryXAxis->EnableUpdateData( nTimeout );
    m_pEd_SecondaryXAxis->SetUpdateDataHdl( rLink );

    m_pEd_SecondaryYAxis->EnableUpdateData( nTimeout );
    m_pEd_SecondaryYAxis->SetUpdateDataHdl( rLink );
}

bool TitleResources::IsModified()
{
    return m_pEd_Main->IsModified()
        || m_pEd_Sub->IsModified()
        || m_pEd_XAxis->IsModified()
        || m_pEd_YAxis->IsModified()
        || m_pEd_ZAxis->IsModified()
        || m_pEd_SecondaryXAxis->IsModified()
        || m_pEd_SecondaryYAxis->IsModified();
}

void TitleResources::ClearModifyFlag()
{
    m_pEd_Main->ClearModifyFlag();
    m_pEd_Sub->ClearModifyFlag();
    m_pEd_XAxis->ClearModifyFlag();
    m_pEd_YAxis->ClearModifyFlag();
    m_pEd_ZAxis->ClearModifyFlag();
    m_pEd_SecondaryXAxis->ClearModifyFlag();
    m_pEd_SecondaryYAxis->ClearModifyFlag();
}

void TitleResources::writeToResources( const TitleDialogData& rInput )
{
    m_pFT_Main->Enable( rInput.aPossibilityList[0] );
    m_pFT_Sub->Enable( rInput.aPossibilityList[1] );
    m_pFT_XAxis->Enable( rInput.aPossibilityList[2] );
    m_pFT_YAxis->Enable( rInput.aPossibilityList[3] );
    m_pFT_ZAxis->Enable( rInput.aPossibilityList[4] );
    m_pFT_SecondaryXAxis->Enable( rInput.aPossibilityList[5] );
    m_pFT_SecondaryYAxis->Enable( rInput.aPossibilityList[6] );

    m_pEd_Main->Enable( rInput.aPossibilityList[0] );
    m_pEd_Sub->Enable( rInput.aPossibilityList[1] );
    m_pEd_XAxis->Enable( rInput.aPossibilityList[2] );
    m_pEd_YAxis->Enable( rInput.aPossibilityList[3] );
    m_pEd_ZAxis->Enable( rInput.aPossibilityList[4] );
    m_pEd_SecondaryXAxis->Enable( rInput.aPossibilityList[5] );
    m_pEd_SecondaryYAxis->Enable( rInput.aPossibilityList[6] );

    m_pEd_Main->SetText(rInput.aTextList[0]);
    m_pEd_Sub->SetText(rInput.aTextList[1]);
    m_pEd_XAxis->SetText(rInput.aTextList[2]);
    m_pEd_YAxis->SetText(rInput.aTextList[3]);
    m_pEd_ZAxis->SetText(rInput.aTextList[4]);
    m_pEd_SecondaryXAxis->SetText(rInput.aTextList[5]);
    m_pEd_SecondaryYAxis->SetText(rInput.aTextList[6]);
}

void TitleResources::readFromResources( TitleDialogData& rOutput )
{
    rOutput.aExistenceList[0] = !m_pEd_Main->GetText().isEmpty();
    rOutput.aExistenceList[1] = !m_pEd_Sub->GetText().isEmpty();
    rOutput.aExistenceList[2] = !m_pEd_XAxis->GetText().isEmpty();
    rOutput.aExistenceList[3] = !m_pEd_YAxis->GetText().isEmpty();
    rOutput.aExistenceList[4] = !m_pEd_ZAxis->GetText().isEmpty();
    rOutput.aExistenceList[5] = !m_pEd_SecondaryXAxis->GetText().isEmpty();
    rOutput.aExistenceList[6] = !m_pEd_SecondaryYAxis->GetText().isEmpty();

    rOutput.aTextList[0] = m_pEd_Main->GetText();
    rOutput.aTextList[1] = m_pEd_Sub->GetText();
    rOutput.aTextList[2] = m_pEd_XAxis->GetText();
    rOutput.aTextList[3] = m_pEd_YAxis->GetText();
    rOutput.aTextList[4] = m_pEd_ZAxis->GetText();
    rOutput.aTextList[5] = m_pEd_SecondaryXAxis->GetText();
    rOutput.aTextList[6] = m_pEd_SecondaryYAxis->GetText();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
