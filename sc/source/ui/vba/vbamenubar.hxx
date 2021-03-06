/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/excel/XMenuBar.hpp>
#include <ooo/vba/XCommandBar.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XMenuBar > MenuBar_BASE;

class ScVbaMenuBar : public MenuBar_BASE
{
private:
    css::uno::Reference< ov::XCommandBar > m_xCommandBar;

public:
    /// @throws css::uno::RuntimeException
    ScVbaMenuBar( const css::uno::Reference< ov::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< ov::XCommandBar >& rCommandBar );

    virtual css::uno::Any SAL_CALL Menus( const css::uno::Any& aIndex ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
