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

#ifndef INCLUDED_CONFIGMGR_SOURCE_ROOTACCESS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_ROOTACCESS_HXX

#include "sal/config.h"

#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/util/ChangesSet.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "com/sun/star/util/XChangesNotifier.hpp"
#include "cppuhelper/implbase2.hxx"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "access.hxx"

namespace com { namespace sun { namespace star {
    namespace uno {
        class Any;
        class Type;
    }
    namespace util { class XChangesListener; }
} } }

namespace configmgr {

class Node;

typedef
    cppu::ImplInheritanceHelper2<
        Access, com::sun::star::util::XChangesNotifier,
        com::sun::star::util::XChangesBatch >
    RootAccessBase;

class RootAccess: public RootAccessBase {
public:
    RootAccess(
        rtl::OUString const & path, rtl::OUString const & locale, bool update);

    rtl::OUString getLocale() const;

    bool isUpdate() const;

private:
    virtual ~RootAccess();

    virtual rtl::OUString getPath();

    virtual rtl::Reference< Node > getNode();

    virtual bool isFinalized();

    virtual rtl::OUString name();

    virtual rtl::Reference< RootAccess > getRootAccess();

    virtual rtl::Reference< Access > getParentAccess();

    virtual void addSupportedServiceNames(
        std::vector< rtl::OUString > * services);

    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
        com::sun::star::uno::Type const & aType)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addChangesListener(
        com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
            const & aListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeChangesListener(
        com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
            const & aListener)
        throw (com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL commitChanges()
        throw (
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL hasPendingChanges()
        throw (com::sun::star::uno::RuntimeException);

    virtual com::sun::star::util::ChangesSet SAL_CALL getPendingChanges()
        throw (com::sun::star::uno::RuntimeException);

    rtl::OUString path_;
    rtl::OUString locale_;
    bool update_;
    bool finalized_;
    rtl::Reference< Node > node_;
    rtl::OUString name_;
};

}

#endif
