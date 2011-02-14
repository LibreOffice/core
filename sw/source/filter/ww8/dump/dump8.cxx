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
#include "precompiled_sw.hxx"


#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>

#include <sot/storage.hxx>

// globale Vars

char** pPara;

// Deklarationen

int PrepareConvert( String& rName, String& rOutName, String& rMess );
int DoConvert( const String& rName, sal_uInt8 nVersion );
void DeInit();



// -----------------------------------------------------------------------

class MyApp : public Application
{
public:
    void Main();
};

// -----------------------------------------------------------------------

MyApp aMyApp;

// -----------------------------------------------------------------------

class MyWin : public WorkWindow
{
    String& rMessg;
public:
         MyWin( Window* pParent, WinBits aWinStyle, String& rMess ) :
             WorkWindow(pParent, aWinStyle), rMessg( rMess ) {}

    void Paint( const Rectangle& );
    void Resize();

    void MouseButtonDown( const MouseEvent& rMEvt );
    void KeyInput( const KeyEvent& rKEvt );
};

// -----------------------------------------------------------------------

extern SvStorageStreamRef xStrm;

void MyApp::Main()
{
    SvFactory::Init();
    String aMess, aName, aOutName;
    sal_uInt8 nVersion = 8;

    int nArgs = GetCommandLineParamCount();
    if (nArgs)
    {
        aName = GetCommandLineParam( 0 );
        if (aName.Search('.') == STRING_NOTFOUND)
            aName.AppendAscii( ".doc" );
        if (nArgs >= 2)
        {
            aOutName = GetCommandLineParam( 1 );
            if (nArgs > 2)
            {
                nVersion = GetCommandLineParam( 2 ).ToInt32();
                if( 6 > nVersion || 8 < nVersion )
                {
                    aMess.AssignAscii( "Aufruf: Dump1 InFile [OutFile] [6|7|8]" );
                }
            }
        }
        else
        {
            aOutName = aName;
            aOutName.Erase(aOutName.Search('.'));
            aOutName.AppendAscii( ".dmp" );
        }
    }

    sal_Bool bOk =     !aMess.Len()
                && !PrepareConvert( aName, aOutName, aMess )
                && !DoConvert( aName, nVersion  );

    if( !bOk )
    {
        MyWin aMainWin( NULL, WB_APP | WB_STDWORK, aMess );

        aMainWin.SetText( String::CreateFromAscii( "WW8-Dumper" ));
        aMainWin.Show();
        aMainWin.Invalidate();

        Execute();
    }
    DeInit();
//  SvFactory::DeInit();
}

// -----------------------------------------------------------------------

void MyWin::Paint( const Rectangle& )
{
    String aText( String::CreateFromAscii( "Dumper fuer WinWord-Dateien !\n"
            "Die Wandlung ging schief. Ist es wirklich ein WW-File ?" ));

    Size  aWinSize  = GetOutputSizePixel();
    Size  aTextSize( GetTextWidth( aText ), GetTextHeight());
    Point aPos( aWinSize.Width() / 2  - aTextSize.Width() / 2,
                aWinSize.Height() / 2 - aTextSize.Height() / 2 );

    DrawText( aPos, aText );

    aPos.Y() += 20;
    DrawText( aPos, rMessg );
}

// -----------------------------------------------------------------------

void MyWin::Resize()
{
    Invalidate();
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonDown( const MouseEvent& )
{
}

// -----------------------------------------------------------------------

void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    WorkWindow::KeyInput( rKEvt );
}



