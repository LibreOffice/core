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

#ifndef INCLUDED_CONFIGMGR_CHILDACCESS_HXX
#define INCLUDED_CONFIGMGR_CHILDACCESS_HXX

#include "sal/config.h"

#include "com/sun/star/lang/XUnoTunnel.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "cppuhelper/implbase1.hxx"
#include "rtl/ref.hxx"
#include "sal/types.h"

#include "access.hxx"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
} } } }

namespace configmgr {

class Node;
class RootAccess;

class ChildAccess:
    public cppu::ImplInheritanceHelper1<
        Access, com::sun::star::lang::XUnoTunnel >
{
public:
    static com::sun::star::uno::Sequence< sal_Int8 > getTunnelId();

    ChildAccess(
        RootAccess * root, Access * parent,
        rtl::Reference< Node > const & node);

    ChildAccess(
        rtl::Reference< RootAccess > const & root,
        rtl::Reference< Node > const & node);

    virtual rtl::Reference< Node > getNode();

    virtual rtl::Reference< RootAccess > getRoot();

    Access * getParentAccess() const;

    void bind(RootAccess * root, Access * parent) throw ();

    void unbind() throw ();

    com::sun::star::uno::Any asValue();

private:
    virtual ~ChildAccess();

    virtual sal_Int64 SAL_CALL getSomething(
        com::sun::star::uno::Sequence< sal_Int8 > const & aIdentifier)
        throw (com::sun::star::uno::RuntimeException);

    RootAccess * root_;
    rtl::Reference< RootAccess > acquiredRoot_; // only for free nodes (= root_)
    Access * parent_; // non-null iff non-free node
    rtl::Reference< Node > node_;
};

}

#endif
