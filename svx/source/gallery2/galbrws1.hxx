/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_SOURCE_GALLERY2_GALBRWS1_HXX
#define INCLUDED_SVX_SOURCE_GALLERY2_GALBRWS1_HXX

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <svl/lstner.hxx>
#include <vector>

#include <functional>

class GalleryBrowser1;

// - GalleryButton -

class GalleryButton : public PushButton
{
private:

    virtual void    KeyInput( const KeyEvent& rKEvt ) override;

public:

                    GalleryButton( GalleryBrowser1* pParent, WinBits nWinBits );
};

// - GalleryThemeListBox -

class GalleryThemeListBox : public ListBox
{
protected:

    void            InitSettings();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

public:

                    GalleryThemeListBox( GalleryBrowser1* pParent, WinBits nWinBits );
};

// - GalleryBrowser1 -

class Gallery;
class GalleryThemeEntry;
class GalleryTheme;
class VclAbstractDialog2;
struct ExchangeData;
class SfxItemSet;

namespace svx { namespace sidebar { class GalleryControl; } }

class GalleryBrowser1 : public Control, SfxListener
{
    friend class GalleryBrowser;
    friend class svx::sidebar::GalleryControl;
    friend class GalleryThemeListBox;
    using Control::Notify;
    using Window::KeyInput;

private:

    VclPtr<GalleryButton>   maNewTheme;
    VclPtr<GalleryThemeListBox>  mpThemes;
    Gallery*                mpGallery;
    ExchangeData*           mpExchangeData;
    SfxItemSet*             mpThemePropsDlgItemSet;

    Image                   aImgNormal;
    Image                   aImgDefault;
    Image                   aImgReadOnly;
    Image                   aImgImported;

    ::std::function<sal_Bool (const KeyEvent&,Window*)> maKeyInputHandler;
    ::std::function<void ()> maThemeSlectionHandler;

    void                    ImplAdjustControls();
    sal_uIntPtr             ImplInsertThemeEntry( const GalleryThemeEntry* pEntry );
    static void             ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData );
    void                    ImplGetExecuteVector(::std::vector< sal_uInt16 >& o_aExec);
    void                    ImplExecute( sal_uInt16 nId );
    void                    ImplGalleryThemeProperties( const OUString & rThemeName, bool bCreateNew );
    void                    ImplEndGalleryThemeProperties(Dialog* pDialog, bool bCreateNew);

    // Control
    virtual void            Resize() override;
    virtual void            GetFocus() override;

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            DECL_LINK_TYPED( ClickNewThemeHdl, Button*, void );
                            DECL_LINK_TYPED( SelectThemeHdl, ListBox&, void );
                            DECL_LINK_TYPED( ShowContextMenuHdl, void*, void );
                            DECL_LINK_TYPED( PopupMenuHdl, Menu*, bool );
                            DECL_LINK_TYPED( EndNewThemePropertiesDlgHdl, Dialog&, void );
                            DECL_LINK_TYPED( EndThemePropertiesDlgHdl, Dialog&, void );
                            DECL_LINK_TYPED( DestroyThemePropertiesDlgHdl, void*, void );

public:

                            GalleryBrowser1(
                                vcl::Window* pParent,
                                Gallery* pGallery,
                                const ::std::function<sal_Bool (const KeyEvent&,Window*)>& rKeyInputHandler,
                                const ::std::function<void ()>& rThemeSlectionHandler);
                            virtual ~GalleryBrowser1();
    virtual void            dispose() override;

    void                    SelectTheme( sal_uInt16 nThemePos ) { mpThemes->SelectEntryPos( nThemePos ); SelectThemeHdl( *mpThemes ); }
    OUString                GetSelectedTheme() { return mpThemes->GetEntryCount() ? mpThemes->GetSelectEntry() : OUString(); }

    void                    ShowContextMenu();
    bool                    KeyInput( const KeyEvent& rKEvt, vcl::Window* pWindow );
};

#endif // INCLUDED_SVX_SOURCE_GALLERY2_GALBRWS1_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
