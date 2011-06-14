/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2010 Oracle and/or its affiliates.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* This file is part of OpenOffice.org.
*
* OpenOffice.org is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License version 3
* only, as published by the Free Software Foundation.
*
* OpenOffice.org is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License version 3 for more details
* (a copy is included in the LICENSE file that accompanied this code).
*
* You should have received a copy of the GNU Lesser General Public License
* version 3 along with OpenOffice.org.  If not, see
* <http://www.openoffice.org/license.html>
* for a copy of the LGPLv3 License.
*
************************************************************************/

#ifndef INCLUDED_CONFIGMGR_SOURCE_CHILDACCESS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_CHILDACCESS_HXX

#include "sal/config.h"

#include <memory>
#include <vector>

#include "com/sun/star/container/XChild.hpp"
#include "com/sun/star/lang/NoSupportException.hpp"
#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "boost/shared_ptr.hpp"
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
        rtl::Reference< Access > const & parent, rtl::OUString const & name,
        rtl::Reference< Node > const & node);

    ChildAccess(
        Components & components, rtl::Reference< RootAccess > const & root,
        rtl::Reference< Node > const & node);

    virtual Path getAbsolutePath();
    virtual Path getRelativePath();

    virtual rtl::OUString getRelativePathRepresentation();
    virtual rtl::Reference< Node > getNode();

    virtual bool isFinalized();

    virtual rtl::OUString getNameInternal();

    virtual rtl::Reference< RootAccess > getRootAccess();
    virtual rtl::Reference< Access > getParentAccess();

    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    virtual com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
    SAL_CALL getParent()
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setParent(
        com::sun::star::uno::Reference< com::sun::star::uno::XInterface >
            const &)
        throw (
            com::sun::star::lang::NoSupportException,
            com::sun::star::uno::RuntimeException);

    virtual sal_Int64 SAL_CALL getSomething(
        com::sun::star::uno::Sequence< sal_Int8 > const & aIdentifier)
        throw (com::sun::star::uno::RuntimeException);

    void bind(
        rtl::Reference< RootAccess > const & root,
        rtl::Reference< Access > const & parent, rtl::OUString const & name)
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
        std::vector< rtl::OUString > * services);

    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
        com::sun::star::uno::Type const & aType)
        throw (com::sun::star::uno::RuntimeException);

    rtl::Reference< RootAccess > root_;
    rtl::Reference< Access > parent_; // null iff free node
    rtl::OUString name_;
    rtl::Reference< Node > node_;
    std::auto_ptr< com::sun::star::uno::Any > changedValue_;
    bool inTransaction_;
        // to determine if a free node can be inserted underneath some root
    boost::shared_ptr<osl::Mutex> lock_;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
