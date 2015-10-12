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

#ifndef INCLUDED_CONFIGMGR_SOURCE_CHILDACCESS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_CHILDACCESS_HXX

#include <sal/config.h>

#include <memory>
#include <vector>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ref.hxx>
#include <sal/types.h>

#include "access.hxx"
#include "path.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
    class Type;
    class XInterface;
} } } }

namespace configmgr {

class Components;
class Modifications;
class Node;
class RootAccess;

class ChildAccess:
    public Access, public css::container::XChild,
    public css::lang::XUnoTunnel
{
public:
    static css::uno::Sequence< sal_Int8 > getTunnelId();

    ChildAccess(
        Components & components, rtl::Reference< RootAccess > const & root,
        rtl::Reference< Access > const & parent, OUString const & name,
        rtl::Reference< Node > const & node);

    ChildAccess(
        Components & components, rtl::Reference< RootAccess > const & root,
        rtl::Reference< Node > const & node);

    virtual Path getAbsolutePath() override;
    virtual Path getRelativePath() override;

    virtual OUString getRelativePathRepresentation() override;
    virtual rtl::Reference< Node > getNode() override;

    virtual bool isFinalized() override;

    virtual OUString getNameInternal() override;

    virtual rtl::Reference< RootAccess > getRootAccess() override;
    virtual rtl::Reference< Access > getParentAccess() override;

    virtual void SAL_CALL acquire() throw () override;
    virtual void SAL_CALL release() throw () override;

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL getParent()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL setParent(
        css::uno::Reference< css::uno::XInterface > const &)
        throw (
            css::lang::NoSupportException,
            css::uno::RuntimeException, std::exception) override;

    virtual sal_Int64 SAL_CALL getSomething(
        css::uno::Sequence< sal_Int8 > const & aIdentifier)
        throw (css::uno::RuntimeException, std::exception) override;

    void bind(
        rtl::Reference< RootAccess > const & root,
        rtl::Reference< Access > const & parent, OUString const & name)
        throw ();

    void unbind() throw ();

    bool isInTransaction() const { return inTransaction_; }
    void committed();
    void setNode(rtl::Reference< Node > const & node);

    void setProperty(
        css::uno::Any const & value,
        Modifications * localModifications);

    css::uno::Any asValue();
    static bool asSimpleValue(const rtl::Reference< Node > &rNode,
                              css::uno::Any &value,
                              Components &components);

    void commitChanges(bool valid, Modifications * globalModifications);

private:
    virtual ~ChildAccess();

    virtual void addTypes(
        std::vector< css::uno::Type > * types) const override;

    virtual void addSupportedServiceNames(
        std::vector< OUString > * services) override;

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & aType)
        throw (css::uno::RuntimeException, std::exception) override;

    rtl::Reference< RootAccess > root_;
    rtl::Reference< Access > parent_; // null if free node
    OUString name_;
    rtl::Reference< Node > node_;
    std::unique_ptr< css::uno::Any > changedValue_;
    bool inTransaction_;
        // to determine if a free node can be inserted underneath some root
    std::shared_ptr<osl::Mutex> lock_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
