/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAMENUBARS_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAMENUBARS_HXX

#include <ooo/vba/excel/XMenuBars.hpp>
#include <ooo/vba/excel/XMenuBar.hpp>
#include <ooo/vba/XCommandBars.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
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
    virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException) override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw (css::uno::RuntimeException) override;
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;

    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException) override;
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& aIndex, const css::uno::Any& /*aIndex2*/ ) throw( css::uno::RuntimeException) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAMENUBARS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
