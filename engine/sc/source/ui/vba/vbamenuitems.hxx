/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/excel/XMenuItems.hpp>
#include <vbahelper/vbacollectionimpl.hxx>

namespace ooo::vba { class XCommandBarControls; }

typedef CollTestImplHelper< ov::excel::XMenuItems > MenuItems_BASE;

class ScVbaMenuItems : public MenuItems_BASE
{
private:
    cpo::uno::Reference< ov::XCommandBarControls > m_xCommandBarControls;

public:
    /// @throws cpo::uno::RuntimeException
    ScVbaMenuItems( const cpo::uno::Reference< ov::XHelperInterface >& xParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& xContext, cpo::uno::Reference< ov::XCommandBarControls > xCommandBarControls );

    // XEnumerationAccess
    virtual cpo::uno::Type getElementType() override;
    virtual cpo::uno::Reference< css::container::XEnumeration > createEnumeration() override;
    virtual cpo::uno::Any createCollectionObject( const cpo::uno::Any& aSource ) override;

    // Methods
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any Item( const cpo::uno::Any& Index, const cpo::uno::Any& /*Index2*/ ) override;
    virtual cpo::uno::Reference< ov::excel::XMenuItem > Add( const OUString& Caption, const cpo::uno::Any& OnAction, const cpo::uno::Any& ShortcutKey, const cpo::uno::Any& Before, const cpo::uno::Any& Restore, const cpo::uno::Any& StatusBar, const cpo::uno::Any& HelpFile, const cpo::uno::Any& HelpContextID ) override;
    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
