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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGESIZECONTROL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGESIZECONTROL_HXX

#include <i18nutil/paper.hxx>

#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <svtools/unitconv.hxx>
#include <svx/tbxctl.hxx>
#include <vcl/layout.hxx>
#include <vcl/lstbox.hxx>

#include <vector>
#include <svtools/valueset.hxx>

namespace svx { namespace sidebar {
    class ValueSetWithTextControl;
} }
class ValueSet;

namespace sw { namespace sidebar {

class PageSizeControl : public SfxPopupWindow
{
public:
    explicit PageSizeControl(sal_uInt16 nId, vcl::Window* pParent);
    virtual ~PageSizeControl() override;
    virtual void dispose() override;

private:
    VclPtr<VclVBox> maContainer;
    VclPtr<svx::sidebar::ValueSetWithTextControl> mpSizeValueSet;
    VclPtr<PushButton> maMoreButton;
    // hidden metric field
    VclPtr<MetricField> maWidthHeightField;

    std::vector< Paper > maPaperList;

    static void ExecuteSizeChange( const Paper ePaper );

    DECL_LINK(ImplSizeHdl, ::ValueSet*, void);
    DECL_LINK(MoreButtonClickHdl_Impl, Button*, void);
};

} } // end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
