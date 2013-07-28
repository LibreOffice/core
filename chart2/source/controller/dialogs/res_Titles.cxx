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
#include "res_Titles.hrc"
#include "ResId.hxx"
#include "TitleDialogData.hxx"
#include <svtools/controldims.hrc>

namespace chart
{

TitleResources::TitleResources( Window* pWindow, bool bShowSecondaryAxesTitle )
    : m_aFT_Main(pWindow, SchResId(FT_MAINTITLE))
    , m_aEd_Main(pWindow, SchResId(ED_MAINTITLE))
    , m_aFT_Sub(pWindow, SchResId(FT_SUBTITLE))
    , m_aEd_Sub(pWindow, SchResId(ED_SUBTITLE))
    , m_aFL_Axes(pWindow, SchResId(FL_AXES))
    , m_aFT_XAxis(pWindow, SchResId(FT_TITLE_X_AXIS))
    , m_aEd_XAxis(pWindow, SchResId(ED_X_AXIS))
    , m_aFT_YAxis(pWindow, SchResId(FT_TITLE_Y_AXIS))
    , m_aEd_YAxis(pWindow, SchResId(ED_Y_AXIS))
    , m_aFT_ZAxis(pWindow, SchResId(FT_TITLE_Z_AXIS))
    , m_aEd_ZAxis(pWindow, SchResId(ED_Z_AXIS))
    , m_aFL_SecondaryAxes(pWindow, SchResId(FL_SECONDARY_AXES))
    , m_aFT_SecondaryXAxis(pWindow, SchResId(FT_TITLE_SECONDARY_X_AXIS))
    , m_aEd_SecondaryXAxis(pWindow, SchResId(ED_SECONDARY_X_AXIS))
    , m_aFT_SecondaryYAxis(pWindow, SchResId(FT_TITLE_SECONDARY_Y_AXIS))
    , m_aEd_SecondaryYAxis(pWindow, SchResId(ED_SECONDARY_Y_AXIS))
{
     long nMaxTextWidth = ::std::max( m_aFT_Main.CalcMinimumSize().Width(),m_aFT_Sub.CalcMinimumSize().Width() ) ;
     nMaxTextWidth = ::std::max( nMaxTextWidth, m_aFT_XAxis.CalcMinimumSize().Width() ) ;
     nMaxTextWidth = ::std::max( nMaxTextWidth, m_aFT_YAxis.CalcMinimumSize().Width() ) ;
     nMaxTextWidth = ::std::max( nMaxTextWidth, m_aFT_ZAxis.CalcMinimumSize().Width() ) ;
     if( bShowSecondaryAxesTitle )
     {
         nMaxTextWidth = ::std::max( nMaxTextWidth, m_aFT_SecondaryXAxis.CalcMinimumSize().Width() ) ;
         nMaxTextWidth = ::std::max( nMaxTextWidth, m_aFT_SecondaryYAxis.CalcMinimumSize().Width() ) ;
     }

     Size aControlDistance( m_aFT_Main.LogicToPixel( Size(RSC_SP_CTRL_DESC_X,RSC_SP_CTRL_GROUP_Y), MapMode(MAP_APPFONT) ) );
     long nEditFieldXPos = m_aFT_Main.GetPosPixel().X() + nMaxTextWidth + aControlDistance.Width();
     long nEditFieldWidth = m_aEd_Main.GetPosPixel().X() + m_aEd_Main.GetSizePixel().Width() - nEditFieldXPos;

     if( nEditFieldWidth > 10 )
     {
         Size aFTSize( m_aFT_Main.GetSizePixel() );
         aFTSize.Width() = nMaxTextWidth;
         m_aFT_Main.SetSizePixel(aFTSize);
          m_aFT_Sub.SetSizePixel(aFTSize);
         m_aFT_XAxis.SetSizePixel(aFTSize);
         m_aFT_YAxis.SetSizePixel(aFTSize);
         m_aFT_ZAxis.SetSizePixel(aFTSize);
         if( bShowSecondaryAxesTitle )
         {
             m_aFT_SecondaryXAxis.SetSizePixel(aFTSize);
             m_aFT_SecondaryYAxis.SetSizePixel(aFTSize);
         }

         m_aEd_Main.SetPosPixel( Point( nEditFieldXPos, m_aEd_Main.GetPosPixel().Y() ) );
         m_aEd_Sub.SetPosPixel( Point( nEditFieldXPos, m_aEd_Sub.GetPosPixel().Y() ) );
         m_aEd_XAxis.SetPosPixel( Point( nEditFieldXPos, m_aEd_XAxis.GetPosPixel().Y() ) );
         m_aEd_YAxis.SetPosPixel( Point( nEditFieldXPos, m_aEd_YAxis.GetPosPixel().Y() ) );
         m_aEd_ZAxis.SetPosPixel( Point( nEditFieldXPos, m_aEd_ZAxis.GetPosPixel().Y() ) );
         if(bShowSecondaryAxesTitle)
         {
             m_aEd_SecondaryXAxis.SetPosPixel( Point( nEditFieldXPos, m_aEd_SecondaryXAxis.GetPosPixel().Y() ) );
             m_aEd_SecondaryYAxis.SetPosPixel( Point( nEditFieldXPos, m_aEd_SecondaryYAxis.GetPosPixel().Y() ) );
         }

         Size aEditSize( m_aEd_Main.GetSizePixel() );
         aEditSize.Width() = nEditFieldWidth;
         m_aEd_Main.SetSizePixel( aEditSize );
         m_aEd_Sub.SetSizePixel( aEditSize );
         m_aEd_XAxis.SetSizePixel( aEditSize );
         m_aEd_YAxis.SetSizePixel( aEditSize );
         m_aEd_ZAxis.SetSizePixel( aEditSize );
         if(bShowSecondaryAxesTitle)
         {
             m_aEd_SecondaryXAxis.SetSizePixel( aEditSize );
             m_aEd_SecondaryYAxis.SetSizePixel( aEditSize );
         }
     }

     m_aFT_SecondaryXAxis.Show( bShowSecondaryAxesTitle );
     m_aEd_SecondaryXAxis.Show( bShowSecondaryAxesTitle );
     m_aFT_SecondaryYAxis.Show( bShowSecondaryAxesTitle );
     m_aEd_SecondaryYAxis.Show( bShowSecondaryAxesTitle );
}

TitleResources::~TitleResources()
{
}

void TitleResources::SetUpdateDataHdl( const Link& rLink )
{
    sal_uLong nTimeout = 4*EDIT_UPDATEDATA_TIMEOUT;

    m_aEd_Main.EnableUpdateData( nTimeout );
    m_aEd_Main.SetUpdateDataHdl( rLink );

    m_aEd_Sub.EnableUpdateData( nTimeout );
    m_aEd_Sub.SetUpdateDataHdl( rLink );

    m_aEd_XAxis.EnableUpdateData( nTimeout );
    m_aEd_XAxis.SetUpdateDataHdl( rLink );

    m_aEd_YAxis.EnableUpdateData( nTimeout );
    m_aEd_YAxis.SetUpdateDataHdl( rLink );

    m_aEd_ZAxis.EnableUpdateData( nTimeout );
    m_aEd_ZAxis.SetUpdateDataHdl( rLink );

    m_aEd_SecondaryXAxis.EnableUpdateData( nTimeout );
    m_aEd_SecondaryXAxis.SetUpdateDataHdl( rLink );

    m_aEd_SecondaryYAxis.EnableUpdateData( nTimeout );
    m_aEd_SecondaryYAxis.SetUpdateDataHdl( rLink );
}

bool TitleResources::IsModified()
{
    return m_aEd_Main.IsModified()
        || m_aEd_Sub.IsModified()
        || m_aEd_XAxis.IsModified()
        || m_aEd_YAxis.IsModified()
        || m_aEd_ZAxis.IsModified()
        || m_aEd_SecondaryXAxis.IsModified()
        || m_aEd_SecondaryYAxis.IsModified();
}

void TitleResources::ClearModifyFlag()
{
    m_aEd_Main.ClearModifyFlag();
    m_aEd_Sub.ClearModifyFlag();
    m_aEd_XAxis.ClearModifyFlag();
    m_aEd_YAxis.ClearModifyFlag();
    m_aEd_ZAxis.ClearModifyFlag();
    m_aEd_SecondaryXAxis.ClearModifyFlag();
    m_aEd_SecondaryYAxis.ClearModifyFlag();
}

void TitleResources::writeToResources( const TitleDialogData& rInput )
{
    m_aFT_Main.Enable( rInput.aPossibilityList[0] );
    m_aFT_Sub.Enable( rInput.aPossibilityList[1] );
    m_aFT_XAxis.Enable( rInput.aPossibilityList[2] );
    m_aFT_YAxis.Enable( rInput.aPossibilityList[3] );
    m_aFT_ZAxis.Enable( rInput.aPossibilityList[4] );
    m_aFT_SecondaryXAxis.Enable( rInput.aPossibilityList[5] );
    m_aFT_SecondaryYAxis.Enable( rInput.aPossibilityList[6] );

    m_aEd_Main.Enable( rInput.aPossibilityList[0] );
    m_aEd_Sub.Enable( rInput.aPossibilityList[1] );
    m_aEd_XAxis.Enable( rInput.aPossibilityList[2] );
    m_aEd_YAxis.Enable( rInput.aPossibilityList[3] );
    m_aEd_ZAxis.Enable( rInput.aPossibilityList[4] );
    m_aEd_SecondaryXAxis.Enable( rInput.aPossibilityList[5] );
    m_aEd_SecondaryYAxis.Enable( rInput.aPossibilityList[6] );

    m_aEd_Main.SetText(rInput.aTextList[0]);
    m_aEd_Sub.SetText(rInput.aTextList[1]);
    m_aEd_XAxis.SetText(rInput.aTextList[2]);
    m_aEd_YAxis.SetText(rInput.aTextList[3]);
    m_aEd_ZAxis.SetText(rInput.aTextList[4]);
    m_aEd_SecondaryXAxis.SetText(rInput.aTextList[5]);
    m_aEd_SecondaryYAxis.SetText(rInput.aTextList[6]);
}

void TitleResources::readFromResources( TitleDialogData& rOutput )
{
    rOutput.aExistenceList[0] = static_cast<sal_Bool>(!m_aEd_Main.GetText().isEmpty());
    rOutput.aExistenceList[1] = static_cast<sal_Bool>(!m_aEd_Sub.GetText().isEmpty());
    rOutput.aExistenceList[2] = static_cast<sal_Bool>(!m_aEd_XAxis.GetText().isEmpty());
    rOutput.aExistenceList[3] = static_cast<sal_Bool>(!m_aEd_YAxis.GetText().isEmpty());
    rOutput.aExistenceList[4] = static_cast<sal_Bool>(!m_aEd_ZAxis.GetText().isEmpty());
    rOutput.aExistenceList[5] = static_cast<sal_Bool>(!m_aEd_SecondaryXAxis.GetText().isEmpty());
    rOutput.aExistenceList[6] = static_cast<sal_Bool>(!m_aEd_SecondaryYAxis.GetText().isEmpty());

    rOutput.aTextList[0] = m_aEd_Main.GetText();
    rOutput.aTextList[1] = m_aEd_Sub.GetText();
    rOutput.aTextList[2] = m_aEd_XAxis.GetText();
    rOutput.aTextList[3] = m_aEd_YAxis.GetText();
    rOutput.aTextList[4] = m_aEd_ZAxis.GetText();
    rOutput.aTextList[5] = m_aEd_SecondaryXAxis.GetText();
    rOutput.aTextList[6] = m_aEd_SecondaryYAxis.GetText();
}

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
