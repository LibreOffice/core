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

#ifndef INCLUDED_CONFIGMGR_SOURCE_ROOTACCESS_HXX
#define INCLUDED_CONFIGMGR_SOURCE_ROOTACCESS_HXX

#include "sal/config.h"

#include <set>
#include <vector>

#include "com/sun/star/lang/WrappedTargetException.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/util/ChangesSet.hpp"
#include "com/sun/star/util/XChangesBatch.hpp"
#include "com/sun/star/util/XChangesNotifier.hpp"
#include "boost/shared_ptr.hpp"
#include "rtl/ref.hxx"
#include "rtl/ustring.hxx"
#include "sal/types.h"

#include "access.hxx"
#include "modifications.hxx"
#include "path.hxx"

namespace com { namespace sun { namespace star {
    namespace uno {
        class Any;
        class Type;
    }
    namespace util { class XChangesListener; }
} } }

namespace configmgr {

class Broadcaster;
class Components;
class Node;

class RootAccess:
    public Access, public com::sun::star::util::XChangesNotifier,
    public com::sun::star::util::XChangesBatch
{
public:
    RootAccess(
        Components & components, OUString const & pathRepresenation,
        OUString const & locale, bool update);

    virtual Path getAbsolutePath() SAL_OVERRIDE;

    virtual void initBroadcaster(
        Modifications::Node const & modifications, Broadcaster * broadcaster) SAL_OVERRIDE;

    virtual void SAL_CALL acquire() throw () SAL_OVERRIDE;

    virtual void SAL_CALL release() throw () SAL_OVERRIDE;

    OUString getAbsolutePathRepresentation();

    OUString getLocale() const;

    bool isUpdate() const;

    void setAlive(bool b);

    virtual void SAL_CALL addChangesListener(
        com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
            const & aListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL removeChangesListener(
        com::sun::star::uno::Reference< com::sun::star::util::XChangesListener >
            const & aListener)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual void SAL_CALL commitChanges()
        throw (
            com::sun::star::lang::WrappedTargetException,
            com::sun::star::uno::RuntimeException,
            std::exception) SAL_OVERRIDE;

    virtual sal_Bool SAL_CALL hasPendingChanges()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual com::sun::star::util::ChangesSet SAL_CALL getPendingChanges()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

private:
    virtual ~RootAccess();

    virtual Path getRelativePath() SAL_OVERRIDE;

    virtual OUString getRelativePathRepresentation() SAL_OVERRIDE;

    virtual rtl::Reference< Node > getNode() SAL_OVERRIDE;

    virtual bool isFinalized() SAL_OVERRIDE;

    virtual OUString getNameInternal() SAL_OVERRIDE;

    virtual rtl::Reference< RootAccess > getRootAccess() SAL_OVERRIDE;

    virtual rtl::Reference< Access > getParentAccess() SAL_OVERRIDE;

    virtual void addTypes(std::vector< com::sun::star::uno::Type > * types)
        const SAL_OVERRIDE;

    virtual void addSupportedServiceNames(
        std::vector< OUString > * services) SAL_OVERRIDE;

    virtual void initDisposeBroadcaster(Broadcaster * broadcaster) SAL_OVERRIDE;

    virtual void clearListeners() throw () SAL_OVERRIDE;

    virtual com::sun::star::uno::Any SAL_CALL queryInterface(
        com::sun::star::uno::Type const & aType)
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    virtual OUString SAL_CALL getImplementationName()
        throw (com::sun::star::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    typedef
        std::multiset<
            com::sun::star::uno::Reference<
                com::sun::star::util::XChangesListener > >
        ChangesListeners;

    OUString pathRepresentation_;
    OUString locale_;
    Path path_;
    rtl::Reference< Node > node_;
    OUString name_;
    ChangesListeners changesListeners_;

    boost::shared_ptr<osl::Mutex> lock_;

    bool update_:1;
    bool finalized_:1;
    bool alive_:1;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
