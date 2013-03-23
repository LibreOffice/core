/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): Pei Feng Lin <pflin@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#pragma once
#if 1

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

    virtual css::uno::Any SAL_CALL Menus( const css::uno::Any& aIndex ) throw (css::script::BasicErrorException, css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif//SC_VBA_MENUBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
