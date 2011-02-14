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

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <svl/lstner.hxx>
#include <vector>
#include "svx/galbrws.hxx"

// -----------------
// - GalleryButton -
// -----------------

class GalleryButton : public PushButton
{
private:

    virtual void    KeyInput( const KeyEvent& rKEvt );

public:

                    GalleryButton( GalleryBrowser1* pParent, WinBits nWinBits );
                    ~GalleryButton();
};

// -----------------------
// - GalleryThemeListBox -
// -----------------------

class GalleryThemeListBox : public ListBox
{
protected:

    void            InitSettings();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:

                    GalleryThemeListBox( GalleryBrowser1* pParent, WinBits nWinBits );
                    ~GalleryThemeListBox();
};

// -------------------
// - GalleryBrowser1 -
// -------------------

class Gallery;
class GalleryThemeEntry;
class GalleryTheme;
class VclAbstractDialog2;
struct ExchangeData;
class SfxItemSet;

class GalleryBrowser1 : public Control, SfxListener
{
    friend class GalleryBrowser;
    friend class GalleryThemeListBox;
    using Control::Notify;
    using Window::KeyInput;

private:

    GalleryButton           maNewTheme;
    GalleryThemeListBox*    mpThemes;
    Gallery*                mpGallery;
    ExchangeData*           mpExchangeData;
    SfxItemSet*             mpThemePropsDlgItemSet;

    Image                   aImgNormal;
    Image                   aImgDefault;
    Image                   aImgReadOnly;
    Image                   aImgImported;

    void                    ImplAdjustControls();
    sal_uIntPtr                 ImplInsertThemeEntry( const GalleryThemeEntry* pEntry );
    void                    ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData );
    ::std::vector< sal_uInt16 > ImplGetExecuteVector();
    void                    ImplExecute( sal_uInt16 nId );
    void                    ImplGalleryThemeProperties( const String & rThemeName, bool bCreateNew );
    void                    ImplEndGalleryThemeProperties( VclAbstractDialog2* pDialog, bool bCreateNew );

    // Control
    virtual void            Resize();
    virtual void            GetFocus();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

                            DECL_LINK( ClickNewThemeHdl, void* );
                            DECL_LINK( SelectThemeHdl, void* );
                            DECL_LINK( ShowContextMenuHdl, void* );
                            DECL_LINK( PopupMenuHdl, Menu* );
                            DECL_LINK( EndNewThemePropertiesDlgHdl, VclAbstractDialog2* );
                            DECL_LINK( EndThemePropertiesDlgHdl, VclAbstractDialog2* );
                            DECL_LINK( DestroyThemePropertiesDlgHdl, VclAbstractDialog2* );

public:

                            GalleryBrowser1( GalleryBrowser* pParent, const ResId& rResId, Gallery* pGallery );
                            ~GalleryBrowser1();

    void                    SelectTheme( const String& rThemeName ) { mpThemes->SelectEntry( rThemeName ); SelectThemeHdl( NULL ); }
    void                    SelectTheme( sal_uIntPtr nThemePos ) { mpThemes->SelectEntryPos( (sal_uInt16) nThemePos ); SelectThemeHdl( NULL ); }
    String                  GetSelectedTheme() { return mpThemes->GetEntryCount() ? mpThemes->GetEntry( mpThemes->GetSelectEntryPos() ) : String(); }

    void                    ShowContextMenu();
    sal_Bool                    KeyInput( const KeyEvent& rKEvt, Window* pWindow );
};
