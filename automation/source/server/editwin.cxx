/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_automation.hxx"

#if OSL_DEBUG_LEVEL > 1
#include <vcl/svapp.hxx>
#include "editwin.hxx"


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

sal_Bool EditWindow::Close()
{
    if ( pImpWorkWindow )
    {
        delete pImpWorkWindow;
        pImpWorkWindow = NULL;
    }
    return sal_True;
}

void EditWindow::Show()
{
    if ( Check() )
        pImpWorkWindow->Show();
    else
        bShowWin = sal_True;
}

void EditWindow::Hide()
{
    if ( Check() )
        pImpWorkWindow->Hide();
    else
        bShowWin = sal_False;
}

EditWindow::EditWindow( WorkWindow *pParent, const String &rName, WinBits iWstyle )
: pImpWorkWindow(NULL)
, pMemParent(pParent)
, aMemName(rName)
, iMemWstyle(iWstyle)
, nTextLen(0)
, bQuiet(sal_False)
{
}

EditWindow::~EditWindow()
{
    Close();
}

sal_Bool EditWindow::Check()
{
    if ( ! pImpWorkWindow && Application::IsInExecute() )
    {
        pImpWorkWindow = new ImpWorkWindow( pMemParent, aMemName, iMemWstyle  );
        pImpWorkWindow->m_aInhalt.SetText( aMemPreWinText );
        nTextLen = aMemPreWinText.Len();
        aMemPreWinText.Erase();
        if ( bShowWin )
            pImpWorkWindow->Show();
        return sal_True;
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
        nTextLen = nTextLen + aText.Len();
        pImpWorkWindow->m_aInhalt.SetSelection( Selection( SELECTION_MAX, SELECTION_MAX ) );
    }
    else
    {
        aMemPreWinText += aText;
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
