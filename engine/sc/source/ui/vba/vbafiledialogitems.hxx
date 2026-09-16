/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/excel/XFileDialogSelectedItems.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

typedef CollTestImplHelper< ov::excel::XFileDialogSelectedItems > FileDialogSelectedItems_BASE;

class ScVbaFileDialogSelectedItems final : public FileDialogSelectedItems_BASE
{
    const std::vector<OUString> m_sItems;
public:
    std::vector<OUString> const& getItems()
    {
        return m_sItems;
    }

    ScVbaFileDialogSelectedItems( const cpo::uno::Reference< ov::XHelperInterface >& xParent,
            const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext,
            std::vector<OUString>&& sItems);

    // XEnumerationAccess
    virtual cpo::uno::Type getElementType() override;
    virtual cpo::uno::Reference< css::container::XEnumeration > createEnumeration() override;
    virtual cpo::uno::Any createCollectionObject( const cpo::uno::Any& aSource ) override;

    // Methods
    virtual cpo::uno::Any Item( const cpo::uno::Any& Index, const cpo::uno::Any& /*Index2*/ ) override;
    virtual sal_Int32 getCount() override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
