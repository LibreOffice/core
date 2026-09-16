/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <ooo/vba/excel/XMenuItem.hpp>
#include <ooo/vba/XCommandBarControl.hpp>
#include <vbahelper/vbahelperinterface.hxx>

typedef InheritedHelperInterfaceWeakImpl< ov::excel::XMenuItem > MenuItem_BASE;

class ScVbaMenuItem : public MenuItem_BASE
{
private:
    cpo::uno::Reference< ov::XCommandBarControl > m_xCommandBarControl;

public:
    /// @throws cpo::uno::RuntimeException
    ScVbaMenuItem( const cpo::uno::Reference< ov::XHelperInterface >& rParent, const cpo::uno::Reference< cpo::uno::XComponentContext >& rContext, cpo::uno::Reference< ov::XCommandBarControl > xCommandBarControl );

    virtual OUString getCaption() override;
    virtual void setCaption( const OUString& _caption ) override;
    virtual OUString getOnAction() override;
    virtual void setOnAction( const OUString& _onaction ) override;

    virtual void Delete(  ) override;

    // XHelperInterface
    virtual OUString getServiceImplName() override;
    virtual cpo::uno::Sequence<OUString> getServiceNames() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
