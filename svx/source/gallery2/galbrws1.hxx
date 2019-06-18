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


class GalleryButton : public PushButton
{
private:

    virtual void    KeyInput( const KeyEvent& rKEvt ) override;

public:

                    GalleryButton( GalleryBrowser1* pParent, WinBits nWinBits );
};


class GalleryThemeListBox final : public ListBox
{
    void            InitSettings();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

public:

                    GalleryThemeListBox( GalleryBrowser1* pParent, WinBits nWinBits );
};


class Gallery;
class GalleryThemeEntry;
class GalleryTheme;
class VclAbstractDialog;
struct ExchangeData;
class SfxItemSet;

namespace svx { namespace sidebar { class GalleryControl; } }

class GalleryBrowser1 : public Control, public SfxListener
{
    friend class GalleryBrowser;
    friend class svx::sidebar::GalleryControl;
    friend class GalleryThemeListBox;
    using Window::KeyInput;

private:

    VclPtr<GalleryButton>        maNewTheme;
    VclPtr<GalleryThemeListBox>  mpThemes;
    VclPtr<VclAbstractDialog>   mpThemePropertiesDialog; // to keep it alive during execution
    Gallery*                mpGallery;
    std::unique_ptr<ExchangeData> mpExchangeData;
    std::unique_ptr<SfxItemSet>   mpThemePropsDlgItemSet;

    Image                   aImgNormal;
    Image                   aImgDefault;
    Image                   aImgReadOnly;

    ::std::function<sal_Bool (const KeyEvent&,Window*)> const maKeyInputHandler;
    ::std::function<void ()> const maThemeSlectionHandler;

    void                    ImplAdjustControls();
    sal_uIntPtr             ImplInsertThemeEntry( const GalleryThemeEntry* pEntry );
    static void             ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData );
    void                    ImplGetExecuteVector(std::vector<OString>& o_aExec);
    void                    ImplExecute(const OString &rIdent);
    void                    ImplGalleryThemeProperties( const OUString & rThemeName, bool bCreateNew );
    void                    EndNewThemePropertiesDlgHdl(sal_Int32 nResult);
    void                    EndThemePropertiesDlgHdl(sal_Int32 nResult);
    void                    ImplEndGalleryThemeProperties(bool bCreateNew, sal_Int32 nResult);

    // Control
    virtual void            Resize() override;
    virtual void            GetFocus() override;

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            DECL_LINK( ClickNewThemeHdl, Button*, void );
                            DECL_LINK( SelectThemeHdl, ListBox&, void );
                            DECL_LINK( ShowContextMenuHdl, void*, void );
                            DECL_LINK( PopupMenuHdl, Menu*, bool );
                            DECL_LINK( DestroyThemePropertiesDlgHdl, void*, void );

public:

                            GalleryBrowser1(
                                vcl::Window* pParent,
                                Gallery* pGallery,
                                const ::std::function<sal_Bool (const KeyEvent&,Window*)>& rKeyInputHandler,
                                const ::std::function<void ()>& rThemeSlectionHandler);
                            virtual ~GalleryBrowser1() override;
    virtual void            dispose() override;

    void                    SelectTheme( sal_uInt16 nThemePos ) { mpThemes->SelectEntryPos( nThemePos ); SelectThemeHdl( *mpThemes ); }
    OUString                GetSelectedTheme() const { return mpThemes->GetEntryCount() ? mpThemes->GetSelectedEntry() : OUString(); }

    void                    ShowContextMenu();
    bool                    KeyInput( const KeyEvent& rKEvt, vcl::Window* pWindow );
};

#endif // INCLUDED_SVX_SOURCE_GALLERY2_GALBRWS1_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
