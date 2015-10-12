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

#ifndef INCLUDED_FILTER_SOURCE_FLASH_IMPSWFDIALOG_HXX
#define INCLUDED_FILTER_SOURCE_FLASH_IMPSWFDIALOG_HXX

#include <com/sun/star/uno/Sequence.h>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svtools/stdctrl.hxx>

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>

#include <vcl/FilterConfigItem.hxx>


// - ImpSWFDialog -


class ResMgr;
namespace vcl { class Window; }

class ImpSWFDialog : public ModalDialog
{
private:
    VclPtr<NumericField> mpNumFldQuality;
    VclPtr<CheckBox>     mpCheckExportAll;
    VclPtr<CheckBox>     mpCheckExportBackgrounds;
    VclPtr<CheckBox>     mpCheckExportBackgroundObjects;
    VclPtr<CheckBox>     mpCheckExportSlideContents;
    VclPtr<CheckBox>     mpCheckExportSound;
    VclPtr<CheckBox>     mpCheckExportOLEAsJPEG;
    VclPtr<CheckBox>     mpCheckExportMultipleFiles;

    FilterConfigItem    maConfigItem;

    DECL_LINK_TYPED( OnToggleCheckbox, CheckBox&, void );

public:
    ImpSWFDialog( vcl::Window* pParent,
                  css::uno::Sequence< css::beans::PropertyValue >& rFilterData );
    virtual ~ImpSWFDialog();
    virtual void dispose() override;

    css::uno::Sequence< css::beans::PropertyValue > GetFilterData();
};

#endif // INCLUDED_FILTER_SOURCE_FLASH_IMPSWFDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
