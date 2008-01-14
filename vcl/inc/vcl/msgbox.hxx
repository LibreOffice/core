/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: msgbox.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:03:45 $
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

#ifndef _SV_MSGBOX_HXX
#define _SV_MSGBOX_HXX

#ifndef _SV_SV_H
#include <vcl/sv.h>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

#ifndef _SV_BTNDLG_HXX
#include <vcl/btndlg.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif
class FixedText;
class FixedImage;
class CheckBox;

// -----------------
// - MessBox-Types -
// -----------------

// Return-Werte von Execute
//!!! bei Aenderungen \basic\source\runtime\methods.cxx msgbox anpassen

#define RET_OK               TRUE
#define RET_CANCEL           FALSE
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
    USHORT              mnSoundType;
    BOOL                mbHelpBtn;
    BOOL                mbSound;
    CheckBox*           mpCheckBox;
    XubString           maCheckBoxText;
    BOOL                mbCheck;

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

    BOOL                SetModeImage( const Image& rImage, BmpColorMode eMode = BMP_COLOR_NORMAL );
    const Image&        GetModeImage( BmpColorMode eMode = BMP_COLOR_NORMAL ) const;

    void                SetDefaultCheckBoxText();
    void                SetCheckBoxText( const XubString& rText ) { maCheckBoxText = rText;}
    const XubString&    GetCheckBoxText() const { return maCheckBoxText;}
    void                SetCheckBoxState( BOOL bCheck );
    BOOL                GetCheckBoxState() const;

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

    static Image        GetStandardImage();
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
};

#endif  // _SV_MSGBOX_HXX
