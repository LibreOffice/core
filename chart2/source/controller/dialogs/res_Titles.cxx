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
#include "precompiled_chart2.hxx"

#include "res_Titles.hxx"
#include "res_Titles.hrc"
#include "ResId.hxx"
#include "TitleDialogData.hxx"

 #ifndef _SVT_CONTROLDIMS_HRC_
 #include <svtools/controldims.hrc>
 #endif

//.............................................................................
namespace chart
{
//.............................................................................

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
    rOutput.aExistenceList[0] = static_cast<sal_Bool>(m_aEd_Main.GetText().Len()!=0);
    rOutput.aExistenceList[1] = static_cast<sal_Bool>(m_aEd_Sub.GetText().Len()!=0);
    rOutput.aExistenceList[2] = static_cast<sal_Bool>(m_aEd_XAxis.GetText().Len()!=0);
    rOutput.aExistenceList[3] = static_cast<sal_Bool>(m_aEd_YAxis.GetText().Len()!=0);
    rOutput.aExistenceList[4] = static_cast<sal_Bool>(m_aEd_ZAxis.GetText().Len()!=0);
    rOutput.aExistenceList[5] = static_cast<sal_Bool>(m_aEd_SecondaryXAxis.GetText().Len()!=0);
    rOutput.aExistenceList[6] = static_cast<sal_Bool>(m_aEd_SecondaryYAxis.GetText().Len()!=0);

    rOutput.aTextList[0] = m_aEd_Main.GetText();
    rOutput.aTextList[1] = m_aEd_Sub.GetText();
    rOutput.aTextList[2] = m_aEd_XAxis.GetText();
    rOutput.aTextList[3] = m_aEd_YAxis.GetText();
    rOutput.aTextList[4] = m_aEd_ZAxis.GetText();
    rOutput.aTextList[5] = m_aEd_SecondaryXAxis.GetText();
    rOutput.aTextList[6] = m_aEd_SecondaryYAxis.GetText();
}

//.............................................................................
} //namespace chart
//.............................................................................

