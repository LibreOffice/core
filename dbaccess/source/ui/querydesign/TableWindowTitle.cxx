/*************************************************************************
 *
 *  $RCSfile: TableWindowTitle.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-02 11:05:23 $
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
#ifndef DBAUI_TABLEWINDOWTITLE_HXX
#include "TableWindowTitle.hxx"
#endif
#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_QUERYTABLEVIEW_HXX
#include "QueryTableView.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_HELP_HXX
#include <vcl/help.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _CNTIDS_HRC
#include <sfx2/cntids.hrc>
#endif
#ifndef DBAUI_TABLEWINDOWLISTBOX_HXX
#include "TableWindowListBox.hxx"
#endif
#ifndef DBAUI_TABLECONNECTION_HXX
#include "TableConnection.hxx"
#endif
#ifndef _DBU_RESOURCE_HRC_
#include "dbu_resource.hrc"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif

using namespace dbaui;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
//==================================================================
// class OTableWindowTitle
//==================================================================
DBG_NAME(OTableWindowTitle);
//------------------------------------------------------------------------------
OTableWindowTitle::OTableWindowTitle( OTableWindow* pParent ) :
     Control( pParent, WB_3DLOOK )
    ,m_pTabWin( pParent )
{
    DBG_CTOR(OTableWindowTitle,NULL);
    // Hintergrund- und Textfarbe setzen
    StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
    SetBackground(Wallpaper(Color(aSystemStyle.GetFaceColor())));
    SetTextColor(aSystemStyle.GetButtonTextColor());

    Font aFont( GetFont() );
    aFont.SetTransparent( TRUE );
    SetFont( aFont );
}

//------------------------------------------------------------------------------
OTableWindowTitle::~OTableWindowTitle()
{
    DBG_DTOR(OTableWindowTitle,NULL);
}

//------------------------------------------------------------------------------
void OTableWindowTitle::GetFocus()
{
    m_pTabWin->GetFocus();
}

//------------------------------------------------------------------------------
void OTableWindowTitle::LoseFocus()
{
    m_pTabWin->LoseFocus();
}
//------------------------------------------------------------------------------
void OTableWindowTitle::RequestHelp( const HelpEvent& rHEvt )
{
    Reference<XPropertySet> xProp(m_pTabWin->GetTable(),UNO_QUERY);

    if(xProp.is())
    {
        ::rtl::OUString aTableName;
        xProp->getPropertyValue(PROPERTY_NAME) >>= aTableName;
        String aHelpText = aTableName.getStr();
        if( aHelpText.Len())
        {
            // Hilfe anzeigen
            Rectangle aItemRect(Point(0,0),GetSizePixel());
            aItemRect = LogicToPixel( aItemRect );
            Point aPt = OutputToScreenPixel( aItemRect.TopLeft() );
            aItemRect.Left()   = aPt.X();
            aItemRect.Top()    = aPt.Y();
            aPt = OutputToScreenPixel( aItemRect.BottomRight() );
            aItemRect.Right()  = aPt.X();
            aItemRect.Bottom() = aPt.Y();
            if( rHEvt.GetMode() == HELPMODE_BALLOON )
                Help::ShowBalloon( this, aItemRect.Center(), aItemRect, aHelpText);
            else
                Help::ShowQuickHelp( this, aItemRect, aHelpText );
        }
    }
}

//------------------------------------------------------------------------------
void OTableWindowTitle::Command( const CommandEvent& rEvt )
{
    switch( rEvt.GetCommand() )
    {
        case COMMAND_CONTEXTMENU:
        {
            GrabFocus();
            OQueryTableView* pView = static_cast<OQueryTableView*>(m_pTabWin->getTableView());
            OSL_ENSURE(pView,"No OQueryTableView!");
            if (!pView->getDesignView()->getController()->isReadOnly())
            {
                Point aPoint = rEvt.GetMousePosPixel();
                PopupMenu aContextMenu( ModuleRes(RID_QUERYCOLPOPUPMENU) );
                switch (aContextMenu.Execute(this, aPoint))
                {
                case SID_DELETE:
                    m_pTabWin->Remove();

                    break;
                default:
                    Control::Command(rEvt);
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
void OTableWindowTitle::Paint( const Rectangle& rRect )
{
    //////////////////////////////////////////////////////////////////////
    // Ausgabe des Textes in der Titelzeile
    Rectangle aWinRect( Point(0,0), GetSizePixel() );
    DrawText( aWinRect, GetText(), TEXT_DRAW_CENTER|TEXT_DRAW_VCENTER|TEXT_DRAW_ENDELLIPSIS );
}

//------------------------------------------------------------------------------
void OTableWindowTitle::KeyInput( const KeyEvent& rEvt )
{
    m_pTabWin->KeyInput( rEvt );
}

//------------------------------------------------------------------------------
void OTableWindowTitle::MouseButtonDown( const MouseEvent& rEvt )
{
    if( rEvt.IsLeft() )
    {
        if( rEvt.GetClicks() == 2)
        {
            Size aSize(GetTextWidth(GetText()) + 20,
                        m_pTabWin->GetSizePixel().Height() - m_pTabWin->GetListBox()->GetSizePixel().Height());

            aSize.Height() += (m_pTabWin->GetListBox()->GetEntryCount() + 2) * m_pTabWin->GetListBox()->GetEntryHeight();
            m_pTabWin->SetSizePixel(aSize);

            OQueryTableView* pView = static_cast<OQueryTableView*>(m_pTabWin->getTableView());
            OSL_ENSURE(pView,"No OQueryTableView!");
            ::std::vector<OTableConnection*>* pConns = pView->GetTabConnList();
            for(::std::vector<OTableConnection*>::iterator aIter = pConns->begin();aIter != pConns->end();++aIter)
                (*aIter)->RecalcLines();

            pView->InvalidateConnections();
            pView->getDesignView()->getController()->setModified(sal_True);
        }
        else
        {
            Point aPos = rEvt.GetPosPixel();
            aPos = OutputToScreenPixel( aPos );
            OQueryTableView* pView = static_cast<OQueryTableView*>(m_pTabWin->getTableView());
            OSL_ENSURE(pView,"No OQueryTableView!");
            pView->NotifyTitleClicked( static_cast<OTableWindow*>(GetParent()), aPos );
        }
        GrabFocus();
    }
    else
        Control::MouseButtonDown( rEvt );
}


//------------------------------------------------------------------------------
void OTableWindowTitle::DataChanged(const DataChangedEvent& rDCEvt)
{
    if (rDCEvt.GetType() == DATACHANGED_SETTINGS)
    {
        // nehmen wir den worst-case an : die Farben haben sich geaendert, also
        // mich anpassen
        StyleSettings aSystemStyle = Application::GetSettings().GetStyleSettings();
        SetBackground(Wallpaper(Color(aSystemStyle.GetFaceColor())));
        SetTextColor(aSystemStyle.GetButtonTextColor());
    }
}
// -----------------------------------------------------------------------------
void OTableWindowTitle::StateChanged( StateChangedType nType )
{
    Window::StateChanged( nType );

    if ( nType == STATE_CHANGE_ZOOM )
    {
        const StyleSettings& rStyleSettings = GetSettings().GetStyleSettings();

        Font aFont = rStyleSettings.GetGroupFont();
        if ( IsControlFont() )
            aFont.Merge( GetControlFont() );
        SetZoomedPointFont( aFont );

        Resize();
    }
}




