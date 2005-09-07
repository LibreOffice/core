/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: editwin.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:23:11 $
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

#if OSL_DEBUG_LEVEL > 1

#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _EDITWIN_HXX
#include "editwin.hxx"
#endif


class ImpWorkWindow : public WorkWindow
{
public:
    MultiLineEdit   m_aInhalt;
    ImpWorkWindow( WorkWindow *pParent, const UniString &rName, WinBits );
    ~ImpWorkWindow();
    void Resize();
};

ImpWorkWindow::ImpWorkWindow( WorkWindow *pParent, const String &rName, WinBits iWstyle )
: WorkWindow( pParent , WB_SIZEMOVE )
, m_aInhalt( this, iWstyle )
{
    m_aInhalt.Show();
    SetText(rName);
    SetPosSizePixel( Point( 1,40 ), Size(500,150) );
    Resize();
}

ImpWorkWindow::~ImpWorkWindow()
{
    Hide();
}

void ImpWorkWindow::Resize()
{
    m_aInhalt.SetPosSizePixel( Point(), GetOutputSizePixel() );
}

BOOL EditWindow::Close()
{
    if ( pImpWorkWindow )
    {
        delete pImpWorkWindow;
        pImpWorkWindow = NULL;
    }
    return TRUE;
}

void EditWindow::Show()
{
    if ( Check() )
        pImpWorkWindow->Show();
    else
        bShowWin = TRUE;
}

void EditWindow::Hide()
{
    if ( Check() )
        pImpWorkWindow->Hide();
    else
        bShowWin = FALSE;
}

EditWindow::EditWindow( WorkWindow *pParent, const String &rName, WinBits iWstyle )
: pMemParent(pParent)
, aMemName(rName)
, iMemWstyle(iWstyle)
, bQuiet(FALSE)
, pImpWorkWindow(NULL)
, nTextLen(0)
{
}

EditWindow::~EditWindow()
{
    Close();
}

BOOL EditWindow::Check()
{
    if ( ! pImpWorkWindow && Application::IsInExecute() )
    {
        pImpWorkWindow = new ImpWorkWindow( pMemParent, aMemName, iMemWstyle  );
        pImpWorkWindow->m_aInhalt.SetText( aMemPreWinText );
        nTextLen = aMemPreWinText.Len();
        aMemPreWinText.Erase();
        if ( bShowWin )
            pImpWorkWindow->Show();
        return TRUE;
    }
    return pImpWorkWindow != NULL;
}

void EditWindow::Clear()
{
    if ( Check() )
    {
        pImpWorkWindow->m_aInhalt.SetText( String() );
        nTextLen = 0;
    }
    aMemPreWinText.Erase();
}

void EditWindow::AddText( const sal_Char* rNew )
{
    AddText( UniString::CreateFromAscii( rNew ) );
}

void EditWindow::AddText( const String &rNew )
{
    if ( bQuiet ) return;

    String aText = rNew;
    aText.ConvertLineEnd();

    if ( Check() )
    {
        if ( nTextLen > 5000 )
        {
            pImpWorkWindow->m_aInhalt.SetText( pImpWorkWindow->m_aInhalt.GetText().Erase(0,1000) );
            nTextLen = pImpWorkWindow->m_aInhalt.GetText().Len();       // Absolut, um Fehler sonstwo auszubügeln
        }


        pImpWorkWindow->m_aInhalt.SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
        pImpWorkWindow->m_aInhalt.ReplaceSelected( aText );
        nTextLen += aText.Len();
        pImpWorkWindow->m_aInhalt.SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
    }
    else
    {
        aMemPreWinText += aText;
    }
}

#endif

