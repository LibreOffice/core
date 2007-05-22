/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: res_Titles.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 17:41:13 $
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
#ifndef _CHART2_RES_TITLES_HXX
#define _CHART2_RES_TITLES_HXX

#include "TitleDialogData.hxx"

#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class TitleResources
{
public:
    TitleResources( Window* pParent );
    virtual ~TitleResources();

    void writeToResources( const TitleDialogData& rInput );
    void readFromResources( TitleDialogData& rOutput );

    void SetUpdateDataHdl( const Link& rLink );
    bool IsModified();
    void ClearModifyFlag();

private:
    FixedText           m_aFT_Main;
    Edit                m_aEd_Main;
    FixedText           m_aFT_Sub;
    Edit                m_aEd_Sub;
    FixedText           m_aFT_XAxis;
    Edit                m_aEd_XAxis;
    FixedText           m_aFT_YAxis;
    Edit                m_aEd_YAxis;
    FixedText           m_aFT_ZAxis;
    Edit                m_aEd_ZAxis;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
