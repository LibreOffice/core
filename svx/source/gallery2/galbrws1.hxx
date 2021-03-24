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

#pragma once

#include <svl/lstner.hxx>
#include <vcl/weld.hxx>
#include <vector>

#include <functional>

class GalleryBrowser1;


class Gallery;
class GalleryThemeEntry;
class GalleryTheme;
class VclAbstractDialog;
struct ExchangeData;
class SfxItemSet;

namespace svx::sidebar { class GalleryControl; }

class GalleryBrowser1 final : public SfxListener
{
    friend class GalleryBrowser;
    friend class svx::sidebar::GalleryControl;

private:

    std::unique_ptr<weld::Button> mxNewTheme;
    std::unique_ptr<weld::TreeView> mxThemes;
    std::unique_ptr<weld::Button> mxMoreGalleries;
    Gallery* mpGallery;
    std::unique_ptr<ExchangeData> mpExchangeData;
    std::unique_ptr<SfxItemSet> mpThemePropsDlgItemSet;

    OUString aImgNormal;
    OUString aImgDefault;
    OUString aImgReadOnly;

    ::std::function<void ()> maThemeSelectionHandler;

    void                    ImplInsertThemeEntry( const GalleryThemeEntry* pEntry );
    static void             ImplFillExchangeData( const GalleryTheme* pThm, ExchangeData& rData );
    void                    ImplGetExecuteVector(std::vector<OString>& o_aExec);
    void                    ImplExecute(std::string_view rIdent);
    void                    ImplGalleryThemeProperties( std::u16string_view rThemeName, bool bCreateNew );
    void                    EndNewThemePropertiesDlgHdl(sal_Int32 nResult);
    void                    EndThemePropertiesDlgHdl(sal_Int32 nResult);
    void                    ImplEndGalleryThemeProperties(bool bCreateNew, sal_Int32 nResult);

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

                            DECL_LINK( ClickNewThemeHdl, weld::Button&, void );
                            DECL_LINK( SelectThemeHdl, weld::TreeView&, void );
                            DECL_LINK( PopupMenuHdl, const CommandEvent&, bool );
                            DECL_LINK( KeyInputHdl, const KeyEvent&, bool );
                            DECL_STATIC_LINK( GalleryBrowser1, OnMoreGalleriesClick, weld::Button&, void );

public:

                            GalleryBrowser1(
                                weld::Builder& rBuilder,
                                Gallery* pGallery,
                                const ::std::function<void ()>& rThemeSelectionHandler);

                            ~GalleryBrowser1();

    void                    SelectTheme( sal_uInt16 nThemePos ) { mxThemes->select( nThemePos ); SelectThemeHdl( *mxThemes ); }
    OUString                GetSelectedTheme() const { return mxThemes->get_selected_text(); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
