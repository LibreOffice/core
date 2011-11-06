/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

