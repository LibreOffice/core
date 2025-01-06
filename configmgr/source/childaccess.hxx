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

#pragma once

#include <sal/config.h>

#include <memory>
#include <optional>
#include <vector>

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ref.hxx>
#include <sal/types.h>

#include "access.hxx"

namespace configmgr {

class Components;
class Modifications;
class Node;
class RootAccess;

class ChildAccess:
    public Access, public css::container::XChild
{
public:

    ChildAccess(
        Components & components, rtl::Reference< RootAccess > const & root,
        rtl::Reference< Access > const & parent, OUString name,
        rtl::Reference< Node > const & node);

    ChildAccess(
        Components & components, rtl::Reference< RootAccess > const & root,
        rtl::Reference< Node > const & node);

    virtual std::vector<OUString> getAbsolutePath() override;
    virtual std::vector<OUString> getRelativePath() override;

    virtual OUString getRelativePathRepresentation() override;
    virtual const rtl::Reference< Node > & getNode() override;

    virtual bool isFinalized() override;

    virtual const OUString & getNameInternal() override;

    virtual rtl::Reference< RootAccess > getRootAccess() override;
    virtual rtl::Reference< Access > getParentAccess() override;

    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    virtual css::uno::Reference< css::uno::XInterface >
    SAL_CALL getParent() override;

    virtual void SAL_CALL setParent(
        css::uno::Reference< css::uno::XInterface > const &) override;

    void bind(
        rtl::Reference< RootAccess > const & root,
        rtl::Reference< Access > const & parent, OUString const & name)
        noexcept;

    void unbind() noexcept;

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
    virtual ~ChildAccess() override;

    virtual void addTypes(
        std::vector< css::uno::Type > * types) const override;

    virtual void addSupportedServiceNames(
        std::vector<OUString> * services) override;

    virtual css::uno::Any SAL_CALL queryInterface(
        css::uno::Type const & aType) override;

    rtl::Reference< RootAccess > root_;
    rtl::Reference< Access > parent_; // null if free node
    OUString name_;
    rtl::Reference< Node > node_;
    std::optional< css::uno::Any > changedValue_;
    bool inTransaction_;
        // to determine if a free node can be inserted underneath some root
    std::shared_ptr<osl::Mutex> lock_;
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
