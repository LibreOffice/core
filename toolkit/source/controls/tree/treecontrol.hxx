/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_TOOLKIT_SOURCE_CONTROLS_TREE_TREECONTROL_HXX
#define INCLUDED_TOOLKIT_SOURCE_CONTROLS_TREE_TREECONTROL_HXX

#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

namespace toolkit {


// = UnoTreeModel

class UnoTreeModel : public UnoControlModel
{
protected:
    css::uno::Any ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&   SAL_CALL getInfoHelper() override;

public:
    explicit UnoTreeModel( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& i_factory );
    UnoTreeModel( const UnoTreeModel& rModel );

    UnoControlModel* Clone() const override;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("stardiv.Toolkit.TreeControlModel"); }

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override
    {
        auto s(UnoControlModel::getSupportedServiceNames());
        s.realloc(s.getLength() + 1);
        s[s.getLength() - 1] = "com.sun.star.awt.tree.TreeControlModel";
        return s;
    }
};

} // toolkit

#endif // _ INCLUDED_TOOLKIT_SOURCE_CONTROLS_TREE_TREECONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
