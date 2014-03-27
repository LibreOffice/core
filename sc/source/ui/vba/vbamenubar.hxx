/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef SC_VBA_MENUBAR_HXX
#define SC_VBA_MENUBAR_HXX

#include <ooo/vba/excel/XMenuBar.hpp>
#include <ooo/vba/XCommandBar.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceImpl1< ov::excel::XMenuBar > MenuBar_BASE;

class ScVbaMenuBar : public MenuBar_BASE
{
private:
    css::uno::Reference< ov::XCommandBar > m_xCommandBar;

public:
    ScVbaMenuBar( const css::uno::Reference< ov::XHelperInterface > xParent, const css::uno::Reference< css::uno::XComponentContext > xContext, const css::uno::Reference< ov::XCommandBar >& xCommandBar ) throw( css::uno::RuntimeException );

    virtual css::uno::Any SAL_CALL Menus( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XHelperInterface
    virtual OUString getServiceImplName() SAL_OVERRIDE;
    virtual css::uno::Sequence<OUString> getServiceNames() SAL_OVERRIDE;
};
#endif//SC_VBA_MENUBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
