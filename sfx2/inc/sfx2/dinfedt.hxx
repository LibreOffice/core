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


#ifndef _SFX_DINFEDT_HXX
#define _SFX_DINFEDT_HXX

// include ---------------------------------------------------------------

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>

// class InfoEdit_Impl ---------------------------------------------------

class InfoEdit_Impl : public Edit
{
public:
    InfoEdit_Impl( Window* pParent, const ResId& rResId ) :
        Edit( pParent, rResId ) {}

    virtual void    KeyInput( const KeyEvent& rKEvent );
};

// class SfxDocInfoEditDlg -----------------------------------------------

class SfxDocInfoEditDlg : public ModalDialog
{
private:
    FixedLine       aInfoFL;
    InfoEdit_Impl   aInfo1ED;
    InfoEdit_Impl   aInfo2ED;
    InfoEdit_Impl   aInfo3ED;
    InfoEdit_Impl   aInfo4ED;
    OKButton        aOkBT;
    CancelButton    aCancelBT;
    HelpButton      aHelpBtn;

public:
    SfxDocInfoEditDlg( Window* pParent );

    void    SetText1( const String &rStr) { aInfo1ED.SetText( rStr ); }
    void    SetText2( const String &rStr) { aInfo2ED.SetText( rStr ); }
    void    SetText3( const String &rStr) { aInfo3ED.SetText( rStr ); }
    void    SetText4( const String &rStr) { aInfo4ED.SetText( rStr ); }

    String  GetText1() const { return aInfo1ED.GetText(); }
    String  GetText2() const { return aInfo2ED.GetText(); }
    String  GetText3() const { return aInfo3ED.GetText(); }
    String  GetText4() const { return aInfo4ED.GetText(); }
};

#endif

