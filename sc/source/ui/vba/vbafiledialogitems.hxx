/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAFILEDIALOGSELECTEDITEMS
#define INCLUDED_SC_SOURCE_UI_VBA_VBAFILEDIALOGSELECTEDITEMS

#include <ooo/vba//excel/XFileDialogSelectedItems.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <vbahelper/vbacollectionimpl.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>

typedef CollTestImplHelper< ov::excel::XFileDialogSelectedItems > FileDialogSelectedItems_BASE;

class ScVbaFileDialogSelectedItems : public FileDialogSelectedItems_BASE
{
public:
    ScVbaFileDialogSelectedItems( const css::uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext, const css::uno::Reference< css::container::XIndexAccess >& xIndexAccess);

    // XEnumerationAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;
    virtual css::uno::Any createCollectionObject( const css::uno::Any& aSource ) override;

    // Methods
    virtual css::uno::Any SAL_CALL Item( const css::uno::Any& Index, const css::uno::Any& /*Index2*/ ) override;
    virtual OUString SAL_CALL Add( const OUString& sPath ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual css::uno::Sequence<OUString> getServiceNames() override;
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAFILEDIALOGSELECTEDITEMS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
