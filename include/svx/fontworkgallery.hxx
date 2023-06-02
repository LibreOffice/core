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


#ifndef INCLUDED_SVX_FONTWORKGALLERY_HXX
#define INCLUDED_SVX_FONTWORKGALLERY_HXX

#include <svx/svxdllapi.h>
#include <vcl/weld.hxx>
#include <map>
#include <vector>

class SdrView;
class SdrObject;
class SdrModel;

namespace svx
{

class SAL_WARN_UNUSED FontworkCharacterSpacingDialog final : public weld::GenericDialogController
{
    std::unique_ptr<weld::MetricSpinButton> m_xMtrScale;

public:
    FontworkCharacterSpacingDialog(weld::Window* pParent, sal_Int32 nScale);
    virtual ~FontworkCharacterSpacingDialog() override;

    sal_Int32 getScale() const;
};

class SAL_WARN_UNUSED SVXCORE_DLLPUBLIC FontWorkGalleryDialog final : public weld::GenericDialogController
{
    sal_uInt16          mnThemeId;
    SdrView&            mrSdrView;

    bool                mbInsertIntoPage;
    rtl::Reference<SdrObject> mxSdrObject;
    SdrModel*           mpDestModel;

    std::vector<VclPtr< VirtualDevice >> maFavoritesHorizontal;
    // mapping between item ID and item title
    std::map<OUString, OUString> maIdToTitleMap;

    std::unique_ptr<weld::IconView> maCtlFavorites;
    std::unique_ptr<weld::Button> mxOKButton;

    css::uno::Reference<css::frame::XFrame> mxFrame;

    void            initFavorites(sal_uInt16 nThemeId);
    void            insertSelectedFontwork();
    void            fillFavorites(sal_uInt16 nThemeId);

    DECL_DLLPRIVATE_LINK(DoubleClickFavoriteHdl, weld::IconView&, bool);
    DECL_DLLPRIVATE_LINK(ClickOKHdl, weld::Button&, void );
    DECL_DLLPRIVATE_LINK(QueryTooltipHandler, const weld::TreeIter&, OUString);

public:
    FontWorkGalleryDialog(weld::Window* pParent, SdrView& rView,
                          css::uno::Reference<css::frame::XFrame> xFrame);
    virtual ~FontWorkGalleryDialog() override;

    // SJ: if the SdrObject** is set, the SdrObject is not inserted into the page when executing the dialog
    void SetSdrObjectRef( SdrModel* pModel );
    SdrObject* GetSdrObjectRef() { return mxSdrObject.get(); }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
