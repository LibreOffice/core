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



#ifndef _SV_MSGBOX_HXX
#define _SV_MSGBOX_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/btndlg.hxx>
#include <vcl/image.hxx>
#include <vcl/bitmap.hxx>
class FixedText;
class FixedImage;
class CheckBox;

// -----------------
// - MessBox-Types -
// -----------------

// Return-Werte von Execute
//!!! bei Aenderungen \basic\source\runtime\methods.cxx msgbox anpassen

#define RET_OK               sal_True
#define RET_CANCEL           sal_False
#define RET_YES              2
#define RET_NO               3
#define RET_RETRY            4
#define RET_IGNORE           5

#define BUTTONID_OK          RET_OK
#define BUTTONID_CANCEL      RET_CANCEL
#define BUTTONID_YES         RET_YES
#define BUTTONID_NO          RET_NO
#define BUTTONID_RETRY       RET_RETRY
#define BUTTONID_IGNORE      RET_IGNORE
#define BUTTONID_HELP        10

// -----------
// - MessBox -
// -----------

class VCL_DLLPUBLIC MessBox : public ButtonDialog
{
protected:
    FixedText*          mpFixedText;
    FixedImage*         mpFixedImage;
    XubString           maMessText;
    Image               maImage;
    Image               maImageHC;
    sal_uInt16              mnSoundType;
    sal_Bool                mbHelpBtn;
    sal_Bool                mbSound;
    CheckBox*           mpCheckBox;
    XubString           maCheckBoxText;
    sal_Bool                mbCheck;

    SAL_DLLPRIVATE void ImplInitMessBoxData();
    SAL_DLLPRIVATE void ImplInitButtons();
    SAL_DLLPRIVATE void ImplPosControls();

protected:
    SAL_DLLPRIVATE void ImplLoadRes( const ResId& rResId );
                        MessBox( WindowType nType );

public:
                        MessBox( Window* pParent, WinBits nStyle,
                                 const XubString& rTitle, const XubString& rMessage );
                        MessBox( Window* pParent, const ResId& rResId );
                        ~MessBox();

    virtual void        StateChanged( StateChangedType nStateChange );

    void                SetMessText( const XubString& rText ) { maMessText = rText; }
    const XubString&    GetMessText() const { return maMessText; }

    void                SetImage( const Image& rImage ) { maImage = rImage; }
    const Image&        GetImage() const { return maImage; }

    sal_Bool                SetModeImage( const Image& rImage, BmpColorMode eMode = BMP_COLOR_NORMAL );
    const Image&        GetModeImage( BmpColorMode eMode = BMP_COLOR_NORMAL ) const;

    void                SetDefaultCheckBoxText();
    void                SetCheckBoxText( const XubString& rText ) { maCheckBoxText = rText;}
    const XubString&    GetCheckBoxText() const { return maCheckBoxText;}
    void                SetCheckBoxState( sal_Bool bCheck );
    sal_Bool                GetCheckBoxState() const;

    virtual Size        GetOptimalSize(WindowSizeType eType) const;
};

// -----------
// - InfoBox -
// -----------

class VCL_DLLPUBLIC InfoBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitInfoBoxData();

public:
                        InfoBox( Window* pParent, const XubString& rMessage );
                        InfoBox( Window* pParent, const ResId & rResId );
                        InfoBox( Window* pParent, WinBits nStyle,
                                const XubString& rMessage );

    static Image        GetStandardImage();
    static Image        GetStandardImageHC();
};

// --------------
// - WarningBox -
// --------------

class VCL_DLLPUBLIC WarningBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitWarningBoxData();

public:
                        WarningBox( Window* pParent, WinBits nStyle,
                                    const XubString& rMessage );
                        WarningBox( Window* pParent, const ResId& rResId );

    void                SetDefaultCheckBoxText();

    static Image        GetStandardImage();
    static Image        GetStandardImageHC();
};

// ------------
// - ErrorBox -
// ------------

class VCL_DLLPUBLIC ErrorBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitErrorBoxData();

public:
                        ErrorBox( Window* pParent, WinBits nStyle,
                                  const XubString& rMessage );
                        ErrorBox( Window* pParent, const ResId& rResId );

    static Image        GetStandardImage();
    static Image        GetStandardImageHC();
};

// ------------
// - QueryBox -
// ------------

class VCL_DLLPUBLIC QueryBox : public MessBox
{
private:
    SAL_DLLPRIVATE void ImplInitQueryBoxData();

public:
                        QueryBox( Window* pParent, WinBits nStyle,
                                  const XubString& rMessage );
                        QueryBox( Window* pParent, const ResId& rResId );

    void                SetDefaultCheckBoxText();

    static Image        GetStandardImage();
    static Image        GetStandardImageHC();
};

#endif  // _SV_MSGBOX_HXX
