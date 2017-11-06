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

#ifndef INCLUDED_CUI_SOURCE_INC_SHOWCOLS_HXX
#define INCLUDED_CUI_SOURCE_INC_SHOWCOLS_HXX

#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>

#include <vcl/button.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>


//  FmShowColsDialog


class FmShowColsDialog final : public ModalDialog
{
    VclPtr<ListBox>        m_pList;
    VclPtr<OKButton>       m_pOK;

    css::uno::Reference< css::container::XIndexAccess >   m_xColumns;

public:
    FmShowColsDialog(vcl::Window* pParent);
    virtual ~FmShowColsDialog() override;
    virtual void dispose() override;

    void SetColumns(const css::uno::Reference< css::container::XIndexContainer>& xCols);

private:
    DECL_LINK(OnClickedOk, Button*, void);
};

#endif // INCLUDED_CUI_SOURCE_INC_SHOWCOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
