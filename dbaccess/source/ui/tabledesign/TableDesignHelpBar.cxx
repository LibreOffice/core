/*************************************************************************
 *
 *  $RCSfile: TableDesignHelpBar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-02 09:54:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef DBAUI_TABLEDESIGNHELPBAR_HXX
#include "TableDesignHelpBar.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#ifndef _DBA_DBACCESS_HELPID_HRC_
#include "dbaccess_helpid.hrc"
#endif

using namespace dbaui;
#define STANDARD_MARGIN                 6
//==================================================================
// class OTableDesignHelpBar
//==================================================================
DBG_NAME(OTableDesignHelpBar);
//------------------------------------------------------------------------------
OTableDesignHelpBar::OTableDesignHelpBar( Window* pParent ) :
     TabPage( pParent, WB_3DLOOK )
{
    DBG_CTOR(OTableDesignHelpBar,NULL);
    m_pTextWin = new MultiLineEdit( this, WB_VSCROLL | WB_LEFT | WB_BORDER | WB_NOTABSTOP | WB_READONLY);
    m_pTextWin->SetHelpId(HID_TABLE_DESIGN_HELP_WINDOW);
    m_pTextWin->SetReadOnly();
    m_pTextWin->SetControlBackground( GetSettings().GetStyleSettings().GetFaceColor() );
    m_pTextWin->Show();
}

//------------------------------------------------------------------------------
OTableDesignHelpBar::~OTableDesignHelpBar()
{
    DBG_DTOR(OTableDesignHelpBar,NULL);
    delete m_pTextWin;
    m_pTextWin = NULL;
}

//------------------------------------------------------------------------------
void OTableDesignHelpBar::SetHelpText( const String& rText )
{
    DBG_CHKTHIS(OTableDesignHelpBar,NULL);
    if(m_pTextWin)
        m_pTextWin->SetText( rText );
    Invalidate();
}

//------------------------------------------------------------------------------
void OTableDesignHelpBar::Resize()
{
    DBG_CHKTHIS(OTableDesignHelpBar,NULL);
    //////////////////////////////////////////////////////////////////////
    // Abmessungen parent window
    Size aOutputSize( GetOutputSizePixel() );

    //////////////////////////////////////////////////////////////////////
    // TextWin anpassen
    if(m_pTextWin)
        m_pTextWin->SetPosSizePixel( Point(STANDARD_MARGIN+1, STANDARD_MARGIN+1),
            Size(aOutputSize.Width()-(2*STANDARD_MARGIN)-2,
                 aOutputSize.Height()-(2*STANDARD_MARGIN)-2) );

}

//------------------------------------------------------------------------------
long OTableDesignHelpBar::PreNotify( NotifyEvent& rNEvt )
{
    if (rNEvt.GetType() == EVENT_LOSEFOCUS)
        SetHelpText(String());
    return TabPage::PreNotify(rNEvt);
}

//==================================================================
// class OFieldPropTabCtrl
//==================================================================
DBG_NAME(OFieldPropTabCtrl);
//==================================================================
OFieldPropTabCtrl::OFieldPropTabCtrl( Window* pParent, WinBits nWinStyle ) :
    TabControl( pParent, nWinStyle )
{
    DBG_CTOR(OFieldPropTabCtrl,NULL);
}

//------------------------------------------------------------------------------
OFieldPropTabCtrl::~OFieldPropTabCtrl()
{
    DBG_DTOR(OFieldPropTabCtrl,NULL);
}



