/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableWindowTitle.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:29:22 $
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
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif
#ifndef DBAUI_QUERYDESIGNVIEW_HXX
#include "QueryDesignView.hxx"
#endif
#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif

#include <algorithm>

using namespace dbaui;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
//==================================================================
// class OTableWindowTitle
//==================================================================
DBG_NAME(OTableWindowTitle)
//------------------------------------------------------------------------------
OTableWindowTitle::OTableWindowTitle( OTableWindow* pParent ) :
     FixedText( pParent, WB_3DLOOK|WB_CENTER|WB_NOLABEL )
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
    m_pTabWin = NULL;
}

//------------------------------------------------------------------------------
void OTableWindowTitle::GetFocus()
{
    if(m_pTabWin)
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
    if(m_pTabWin)
    {
        String aHelpText = m_pTabWin->GetComposedName();
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
            if ( m_pTabWin )
                m_pTabWin->Command( rEvt );
            else
                Control::Command(rEvt);
        }
    }
}

//------------------------------------------------------------------------------
//void OTableWindowTitle::Paint( const Rectangle& rRect )
//{
//  //////////////////////////////////////////////////////////////////////
//  // Ausgabe des Textes in der Titelzeile
//  Rectangle aWinRect( Point(0,0), GetSizePixel() );
//  DrawText( aWinRect, GetText(), TEXT_DRAW_CENTER|TEXT_DRAW_VCENTER|TEXT_DRAW_ENDELLIPSIS );
//}

//------------------------------------------------------------------------------
void OTableWindowTitle::KeyInput( const KeyEvent& rEvt )
{
    if ( m_pTabWin )
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
            if(m_pTabWin->GetSizePixel() != aSize)
            {
                m_pTabWin->SetSizePixel(aSize);

                OJoinTableView* pView = static_cast<OJoinTableView*>(m_pTabWin->getTableView());
                OSL_ENSURE(pView,"No OJoinTableView!");
                const ::std::vector<OTableConnection*>* pConns = pView->getTableConnections();
                ::std::for_each(pConns->begin(),
                                pConns->end(),
                                ::std::mem_fun(&OTableConnection::RecalcLines));

                pView->InvalidateConnections();
                pView->getDesignView()->getController()->setModified(sal_True);
                pView->Invalidate(INVALIDATE_NOCHILDREN);
            }
        }
        else
        {
            Point aPos = rEvt.GetPosPixel();
            aPos = OutputToScreenPixel( aPos );
            OJoinTableView* pView = static_cast<OJoinTableView*>(m_pTabWin->getTableView());
            OSL_ENSURE(pView,"No OJoinTableView!");
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




