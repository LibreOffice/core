/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dump8.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2005-11-16 13:54:06 $
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


#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#include <sot/storage.hxx>

// globale Vars

char** pPara;

// Deklarationen

int PrepareConvert( String& rName, String& rOutName, String& rMess );
int DoConvert( const String& rName, BYTE nVersion );
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
    BYTE nVersion = 8;

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

    BOOL bOk =     !aMess.Len()
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



