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

#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/menu.hxx>
#include <svl/lstner.hxx>
#include <vector>
#include "svx/galbrws.hxx"

#include <boost/function.hpp>

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

namespace svx { namespace sidebar { class GalleryControl; } }

class GalleryBrowser1 : public Control, SfxListener
{
    friend class GalleryBrowser;
    friend class svx::sidebar::GalleryControl;
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

    ::boost::function<sal_Bool(const KeyEvent&,Window*)> maKeyInputHandler;
    ::boost::function<void(void)> maThemeSlectionHandler;

    void                    ImplAdjustControls();
    sal_uIntPtr                 ImplInsertThemeEntry( const GalleryThemeEntry* pEntry );
    void                    ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData );
    void                    ImplGetExecuteVector(::std::vector< sal_uInt16 >& o_aExec);
    void                    ImplExecute( sal_uInt16 nId );
    void                    ImplGalleryThemeProperties( const OUString & rThemeName, bool bCreateNew );
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

                            GalleryBrowser1(
                                Window* pParent,
                                const ResId& rResId,
                                Gallery* pGallery,
                                const ::boost::function<sal_Bool(const KeyEvent&,Window*)>& rKeyInputHandler,
                                const ::boost::function<void(void)>& rThemeSlectionHandler);
                            ~GalleryBrowser1();

    void                    SelectTheme( const OUString& rThemeName ) { mpThemes->SelectEntry( rThemeName ); SelectThemeHdl( NULL ); }
    void                    SelectTheme( sal_uIntPtr nThemePos ) { mpThemes->SelectEntryPos( (sal_uInt16) nThemePos ); SelectThemeHdl( NULL ); }
    OUString                GetSelectedTheme() { return mpThemes->GetEntryCount() ? mpThemes->GetEntry( mpThemes->GetSelectEntryPos() ) : OUString(); }

    void                    ShowContextMenu();
    sal_Bool                    KeyInput( const KeyEvent& rKEvt, Window* pWindow );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
