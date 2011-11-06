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


#ifndef _EDITWIN_HXX
#define _EDITWIN_HXX

#if OSL_DEBUG_LEVEL > 1

#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif

class ImpWorkWindow;

class EditWindow
{
protected:
    ImpWorkWindow   *pImpWorkWindow;
    sal_Bool check();

    WorkWindow  *pMemParent;
    String      aMemName;
    WinBits     iMemWstyle;

    String      aMemPreWinText;
    sal_Bool        bShowWin;

    xub_StrLen      nTextLen;   // aus Performanzgründen eigene Länge mitführen

public:
    EditWindow( WorkWindow *pParent, const UniString &rName = UniString( RTL_CONSTASCII_USTRINGPARAM ( "Debug" ) ), WinBits iWstyle = WB_HSCROLL | WB_VSCROLL );
    virtual ~EditWindow();

    void Clear();
    void Show();
    void Hide();
    void AddText( const String &rNew );
    void AddText( const sal_Char* rNew );

    virtual sal_Bool Close(); // derived
    sal_Bool bQuiet;
    sal_Bool Check();
};

#endif
#endif

