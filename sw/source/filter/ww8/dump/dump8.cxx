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



