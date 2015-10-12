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

#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>

#include <svtools/valueset.hxx>

#include <sfx2/tbxctrl.hxx>

#include <vector>

class FmFormModel;
class SdrView;
namespace vcl { class Window; }
class SdrTextObj;
class SdrObject;
class SdrModel;

class SfxBindings;



namespace svx
{

class SAL_WARN_UNUSED FontworkCharacterSpacingDialog : public ModalDialog
{
    VclPtr<MetricField> m_pMtrScale;

public:
    FontworkCharacterSpacingDialog( vcl::Window* pParent, sal_Int32 nScale );
    virtual ~FontworkCharacterSpacingDialog();
    virtual void dispose() override;

    sal_Int32 getScale() const;
};

class SVX_DLLPUBLIC SAL_WARN_UNUSED FontWorkGalleryDialog : public ModalDialog
{
    VclPtr<ValueSet>    mpCtlFavorites;
    VclPtr<OKButton>    mpOKButton;

    sal_uInt16          mnThemeId;

    SdrView*            mpSdrView;

    DECL_LINK_TYPED( DoubleClickFavoriteHdl, ValueSet*, void );
    DECL_LINK_TYPED( ClickOKHdl, Button*, void );

    SdrObject**         mppSdrObject;
    SdrModel*           mpDestModel;

    void            initFavorites(sal_uInt16 nThemeId);
    void            insertSelectedFontwork();
    void            fillFavorites(sal_uInt16 nThemeId);

    std::vector< BitmapEx> maFavoritesHorizontal;

public:
    FontWorkGalleryDialog( SdrView* pView, vcl::Window* pParent, sal_uInt16 nSID );
    virtual ~FontWorkGalleryDialog();
    virtual void dispose() override;

    // SJ: if the SdrObject** is set, the SdrObject is not inserted into the page when executing the dialog
    void SetSdrObjectRef( SdrObject**, SdrModel* pModel );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
