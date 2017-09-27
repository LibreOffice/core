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
#ifndef INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGECOLUMNCONTROL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_SIDEBAR_PAGECOLUMNCONTROL_HXX

#include <svx/tbxctl.hxx>
#include <vcl/button.hxx>
#include <svx/pageitem.hxx>
#include <editeng/sizeitem.hxx>
#include <svl/intitem.hxx>

namespace sw { namespace sidebar {

class PageColumnControl : public SfxPopupWindow
{
public:
    explicit PageColumnControl(sal_uInt16 nId, vcl::Window* pParent);

    virtual ~PageColumnControl() override;
    virtual void dispose() override;

private:
    VclPtr<PushButton> m_pOneColumn;
    VclPtr<PushButton> m_pTwoColumns;
    VclPtr<PushButton> m_pThreeColumns;
    VclPtr<PushButton> m_pLeft;
    VclPtr<PushButton> m_pRight;
    VclPtr<PushButton> m_pMoreButton;

    static void ExecuteColumnChange( const sal_uInt16 nColumnType );

    DECL_LINK( ColumnButtonClickHdl_Impl, Button*, void );
    DECL_LINK( MoreButtonClickHdl_Impl, Button*, void );
};

} } // end of namespace sw::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
