/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_Titles.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:40:53 $
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
#include "precompiled_chart2.hxx"

#include "res_Titles.hxx"
#include "res_Titles.hrc"
#include "ResId.hxx"
#include "TitleDialogData.hxx"

//.............................................................................
namespace chart
{
//.............................................................................

TitleResources::TitleResources( Window* pWindow )
    : m_aFT_Main(pWindow, SchResId(FT_MAINTITLE))
    , m_aEd_Main(pWindow, SchResId(ED_MAINTITLE))
    , m_aFT_Sub(pWindow, SchResId(FT_SUBTITLE))
    , m_aEd_Sub(pWindow, SchResId(ED_SUBTITLE))
    , m_aFT_XAxis(pWindow, SchResId(FT_TITLE_X_AXIS))
    , m_aEd_XAxis(pWindow, SchResId(ED_X_AXIS))
    , m_aFT_YAxis(pWindow, SchResId(FT_TITLE_Y_AXIS))
    , m_aEd_YAxis(pWindow, SchResId(ED_Y_AXIS))
    , m_aFT_ZAxis(pWindow, SchResId(FT_TITLE_Z_AXIS))
    , m_aEd_ZAxis(pWindow, SchResId(ED_Z_AXIS))
{
}

TitleResources::~TitleResources()
{
}

void TitleResources::SetUpdateDataHdl( const Link& rLink )
{
    ULONG nTimeout = 4*EDIT_UPDATEDATA_TIMEOUT;

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
}

bool TitleResources::IsModified()
{
    return m_aEd_Main.IsModified()
        || m_aEd_Sub.IsModified()
        || m_aEd_XAxis.IsModified()
        || m_aEd_YAxis.IsModified()
        || m_aEd_ZAxis.IsModified();
}

void TitleResources::ClearModifyFlag()
{
    m_aEd_Main.ClearModifyFlag();
    m_aEd_Sub.ClearModifyFlag();
    m_aEd_XAxis.ClearModifyFlag();
    m_aEd_YAxis.ClearModifyFlag();
    m_aEd_ZAxis.ClearModifyFlag();
}

void TitleResources::writeToResources( const TitleDialogData& rInput )
{
    m_aFT_Main.Enable( rInput.aPossibilityList[0] );
    m_aFT_Sub.Enable( rInput.aPossibilityList[1] );
    m_aFT_XAxis.Enable( rInput.aPossibilityList[2] );
    m_aFT_YAxis.Enable( rInput.aPossibilityList[3] );
    m_aFT_ZAxis.Enable( rInput.aPossibilityList[4] );

    m_aEd_Main.Enable( rInput.aPossibilityList[0] );
    m_aEd_Sub.Enable( rInput.aPossibilityList[1] );
    m_aEd_XAxis.Enable( rInput.aPossibilityList[2] );
    m_aEd_YAxis.Enable( rInput.aPossibilityList[3] );
    m_aEd_ZAxis.Enable( rInput.aPossibilityList[4] );

    m_aEd_Main.SetText(rInput.aTextList[0]);
    m_aEd_Sub.SetText(rInput.aTextList[1]);
    m_aEd_XAxis.SetText(rInput.aTextList[2]);
    m_aEd_YAxis.SetText(rInput.aTextList[3]);
    m_aEd_ZAxis.SetText(rInput.aTextList[4]);
}

void TitleResources::readFromResources( TitleDialogData& rOutput )
{
    rOutput.aExistenceList[0] = static_cast<sal_Bool>(m_aEd_Main.GetText().Len()!=0);
    rOutput.aExistenceList[1] = static_cast<sal_Bool>(m_aEd_Sub.GetText().Len()!=0);
    rOutput.aExistenceList[2] = static_cast<sal_Bool>(m_aEd_XAxis.GetText().Len()!=0);
    rOutput.aExistenceList[3] = static_cast<sal_Bool>(m_aEd_YAxis.GetText().Len()!=0);
    rOutput.aExistenceList[4] = static_cast<sal_Bool>(m_aEd_ZAxis.GetText().Len()!=0);

    rOutput.aTextList[0] = m_aEd_Main.GetText();
    rOutput.aTextList[1] = m_aEd_Sub.GetText();
    rOutput.aTextList[2] = m_aEd_XAxis.GetText();
    rOutput.aTextList[3] = m_aEd_YAxis.GetText();
    rOutput.aTextList[4] = m_aEd_ZAxis.GetText();
}

//.............................................................................
} //namespace chart
//.............................................................................

