/*************************************************************************
 *
 *  $RCSfile: dump8.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jp $ $Date: 2000-10-24 14:01:34 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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


#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif

#ifndef _FACTORY_HXX //autogen
#include <so3/factory.hxx>
#endif

#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif

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




/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww8/dump/dump8.cxx,v 1.2 2000-10-24 14:01:34 jp Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.1.1.1  2000/09/18 17:14:59  hr
      initial import

      Revision 1.8  2000/09/18 16:05:02  willem.vandorp
      OpenOffice header added.

      Revision 1.7  2000/02/14 14:40:00  jp
      #70473# changes for unicode

      Revision 1.6  2000/01/25 10:44:55  khz
      change: update the dumper according to import improvements

      Revision 1.5  1999/08/16 10:22:38  JP
      neu: Dump EscherObjects


      Rev 1.4   16 Aug 1999 12:22:38   JP
   neu: Dump EscherObjects

      Rev 1.3   15 Jun 1999 14:19:16   JP
   new: ListTable and other Contents

      Rev 1.2   25 Jun 1998 15:43:46   KHZ
   Strukturaenderung fuer PAPX FKPs

      Rev 1.1   10 Jun 1998 17:22:32   KHZ
   Zwischenstand-Sicherung Dumper

      Rev 1.0   27 May 1998 15:29:52   KHZ
   Initial revision.

*************************************************************************/

