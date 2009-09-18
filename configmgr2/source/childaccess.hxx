/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
*
* $RCSfile: code,v $
*
* $Revision: 1.4 $
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
#include "cppuhelper/implbase2.hxx"
#include "osl/interlck.h"
#include "rtl/ref.hxx"
#include "sal/types.h"

#include "access.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
    class XInterface;
} } } }

namespace configmgr {

class Node;
class RootAccess;

class ChildAccess:
    public cppu::ImplInheritanceHelper2<
        Access, com::sun::star::container::XChild,
        com::sun::star::lang::XUnoTunnel >
{
public:
    static com::sun::star::uno::Sequence< sal_Int8 > getTunnelId();

    ChildAccess(
        rtl::Reference< RootAccess > const & root,
        rtl::Reference< Access > const & parent, rtl::OUString const & name,
        rtl::Reference< Node > const & node);

    ChildAccess(
        rtl::Reference< RootAccess > const & root,
        rtl::Reference< Node > const & node);

    virtual rtl::OUString getAbsolutePath();

    virtual rtl::OUString getRelativePath();

    virtual rtl::Reference< Node > getNode();

    virtual bool isFinalized();

    virtual rtl::OUString getNameInternal();

    virtual rtl::Reference< RootAccess > getRootAccess();

    virtual rtl::Reference< Access > getParentAccess();

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

    oslInterlockedCount acquireCounting();

    void releaseNondeleting();

    void bind(
        rtl::Reference< RootAccess > const & root,
        rtl::Reference< Access > const & parent, rtl::OUString const & name)
        throw ();

    void unbind() throw ();

    bool isInTransaction() const { return inTransaction_; }

    void committed();

    void setNode(rtl::Reference< Node > const & node);

    void setProperty(com::sun::star::uno::Any const & value);

    com::sun::star::uno::Any asValue();

    void commitChanges(bool valid);

private:
    virtual ~ChildAccess();

    virtual void addSupportedServiceNames(
        std::vector< rtl::OUString > * services);

    rtl::Reference< RootAccess > root_;
    rtl::Reference< Access > parent_; // null iff free node
    rtl::OUString name_;
    rtl::Reference< Node > node_;
    std::auto_ptr< com::sun::star::uno::Any > changedValue_;
    bool inTransaction_;
        // to determine if a free node can be inserted underneath some root
};

}

#endif
