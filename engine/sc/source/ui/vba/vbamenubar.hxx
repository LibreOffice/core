/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
    cpo::uno::Reference< ov::XCommandBar > m_xCommandBar;

public:
    /// @throws cpo::uno::RuntimeException
    ScVbaMenuBar( const cpo::uno::Reference< ov::XHelperInterface >& rParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& rContext, cpo::uno::Reference< ov::XCommandBar > xCommandBar );

    virtual cpo::uno::Any Menus( const cpo::uno::Any& aIndex ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
