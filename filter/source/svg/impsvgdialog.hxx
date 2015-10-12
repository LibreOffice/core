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

#ifndef INCLUDED_FILTER_SOURCE_SVG_IMPSVGDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_SVG_IMPSVGDIALOG_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <svtools/stdctrl.hxx>
#include <vcl/FilterConfigItem.hxx>

#define SVG_EXPORTFILTER_CONFIGPATH         "Office.Common/Filter/SVG/Export/"
#define SVG_PROP_TINYPROFILE                "TinyMode"
#define SVG_PROP_EMBEDFONTS                 "EmbedFonts"
#define SVG_PROP_NATIVEDECORATION           "UseNativeTextDecoration"
#define SVG_PROP_OPACITY                    "Opacity"
#define SVG_PROP_POSITIONED_CHARACTERS      "UsePositionedCharacters"


// - ImpSVGDialog -


class ResMgr;
namespace vcl { class Window; }

class ImpSVGDialog : public ModalDialog
{
private:

    VclPtr<FixedLine>           maFI;
    VclPtr<CheckBox>            maCBTinyProfile;
    VclPtr<CheckBox>            maCBEmbedFonts;
    VclPtr<CheckBox>            maCBUseNativeDecoration;

    VclPtr<OKButton>            maBTOK;
    VclPtr<CancelButton>        maBTCancel;
    VclPtr<HelpButton>          maBTHelp;

    FilterConfigItem            maConfigItem;
    bool                        mbOldNativeDecoration;

    DECL_LINK_TYPED( OnToggleCheckbox, CheckBox&, void );

public:

    ImpSVGDialog( vcl::Window* pParent, /*ResMgr& rResMgr,*/
                  css::uno::Sequence< css::beans::PropertyValue >& rFilterData );
    virtual ~ImpSVGDialog();
    virtual void dispose() override;

    css::uno::Sequence< css::beans::PropertyValue > GetFilterData();
};

#endif // INCLUDED_FILTER_SOURCE_SVG_IMPSVGDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
