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

#include "sal/config.h"

#include <vector>

#include "boost/scoped_ptr.hpp"
#include "boost/shared_ptr.hpp"
#include "com/sun/star/container/XChild.hpp"
#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"

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
    public Access, public com::sun::star::container::XChild,
    public com::sun::star::lang::XUnoTunnel
{
public:
    static com::sun::star::uno::Sequence< sal_Int8 > getTunnelId();

    ChildAccess(
        Components & components, rtl::Reference< RootAccess > const & root,
        rtl::Reference< Access > const & parent, OUString const & name,
        rtl::Reference< Node > const & node);

    ChildAccess(
        Components & components, rtl::Reference< RootAccess > const & root,
        rtl::Reference< Node > const & node);

    virtual Path getAbsolutePath();
    virtual Path getRelativePath();

    virtual OUString getRelativePathRepresentation();
    virtual rtl::Reference< Node > getNode();

    virtual bool isFinalized();

    virtual OUString getNameInternal();

    virtual rtl::Reference< RootAccess > getRootAccess();
    virtual rtl::Reference< Access > getParentAccess();

    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL getParent()
        throw (com::sun::star::uno::RuntimeException, std::exception);

    virtual void SAL_CALL setParent(
        com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
            const &)
        throw (
            com::sun::star::lang::NoSupportException,
            com::sun::star::uno::RuntimeException, std::exception);

    virtual sal_Int64 SAL_CALL getSomething(
        com::sun::star::uno::Sequence< sal_Int8 > const & aIdentifier)
        throw (com::sun::star::uno::RuntimeException, std::exception);

    void bind(
        rtl::Reference< RootAccess > const & root,
        rtl::Reference< Access > const & parent, OUString const & name)
        throw ();

    void unbind() throw ();

    bool isInTransaction() const { return inTransaction_; }
    void committed();
    void setNode(rtl::Reference< Node > const & node);

    void setProperty(
        com::sun::star::uno::Any const & value,
        Modifications * localModifications);

    com::sun::star::uno::Any asValue();

    void commitChanges(bool valid, Modifications * globalModifications);

private:
    virtual ~ChildAccess();

    virtual void addTypes(
        std::vector< com::sun::star::uno::Type > * types) const;

    virtual void addSupportedServiceNames(
        std::vector< OUString > * services);

    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
        com::sun::star::uno::Type const & aType)
        throw (com::sun::star::uno::RuntimeException, std::exception);

    rtl::Reference< RootAccess > root_;
    rtl::Reference< Access > parent_; // null if free node
    OUString name_;
    rtl::Reference< Node > node_;
    boost::scoped_ptr< com::sun::star::uno::Any > changedValue_;
    bool inTransaction_;
        // to determine if a free node can be inserted underneath some root
    boost::shared_ptr<osl::Mutex> lock_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
