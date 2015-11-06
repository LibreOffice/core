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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_SWRENAMEXNAMEDDLG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_SWRENAMEXNAMEDDLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <actctrl.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/container/XNamed.hpp>

class SwRenameXNamedDlg : public ModalDialog
{
    VclPtr<Edit>     m_pNewNameED;
    TextFilter m_aTextFilter;
    VclPtr<OKButton> m_pOk;

    css::uno::Reference< css::container::XNamed > &   xNamed;
    css::uno::Reference< css::container::XNameAccess > & xNameAccess;
    css::uno::Reference< css::container::XNameAccess >   xSecondAccess;
    css::uno::Reference< css::container::XNameAccess >   xThirdAccess;

    DECL_LINK_TYPED(OkHdl, Button*, void);
    DECL_LINK_TYPED(ModifyHdl, Edit&, void);

public:
    SwRenameXNamedDlg( vcl::Window* pParent,
                    css::uno::Reference< css::container::XNamed > & xNamed,
                    css::uno::Reference< css::container::XNameAccess > & xNameAccess );
    virtual ~SwRenameXNamedDlg();
    virtual void dispose() override;

    void SetForbiddenChars(const OUString& rSet)
    {
        m_aTextFilter.SetForbiddenChars(rSet);
    }

    void SetAlternativeAccess(
            css::uno::Reference< css::container::XNameAccess > & xSecond,
            css::uno::Reference< css::container::XNameAccess > & xThird )
    {
        xSecondAccess = xSecond;
        xThirdAccess = xThird;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
