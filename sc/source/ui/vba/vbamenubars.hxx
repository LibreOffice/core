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

#include <ooo/vba/excel/XMenuBars.hpp>
#include <ooo/vba/excel/XMenuBar.hpp>
#include <ooo/vba/XCommandBars.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XMenuBars > MenuBars_BASE;

class ScVbaMenuBars : public MenuBars_BASE
{
private:
    css::uno::Reference< ov::XCommandBars > m_xCommandBars;

public:
    ScVbaMenuBars( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< ov::XCommandBars >& xCommandBars ) throw (css::uno::RuntimeException);
    virtual ~ScVbaMenuBars();

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException);
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException);
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource );

    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException);
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& aIndex, const css::uno::Any& /*aIndex2*/ ) throw( css::uno::RuntimeException);
    // XHelperInterface
    virtual rtl::OUString getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};

#endif//SC_VBA_MENUBARS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
