/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
        Components & components, rtl::OUString const & pathRepresenation,
        rtl::OUString const & locale, bool update);

    virtual Path getAbsolutePath();

    virtual void initBroadcaster(
        Modifications::Node const & modifications, Broadcaster * broadcaster);

    virtual void SAL_CALL acquire() throw ();

    virtual void SAL_CALL release() throw ();

    rtl::OUString getAbsolutePathRepresentation();

    rtl::OUString getLocale() const;

    bool isUpdate() const;

private:
    virtual ~RootAccess();

    virtual Path getRelativePath();

    virtual rtl::OUString getRelativePathRepresentation();

    virtual rtl::Reference< Node > getNode();

    virtual bool isFinalized();

    virtual rtl::OUString getNameInternal();

    virtual rtl::Reference< RootAccess > getRootAccess();

    virtual rtl::Reference< Access > getParentAccess();

    virtual void addTypes(std::vector< com::sun::star::uno::Type > * types)
        const;

    virtual void addSupportedServiceNames(
        std::vector< rtl::OUString > * services);

    virtual void initDisposeBroadcaster(Broadcaster * broadcaster);

    virtual void clearListeners() throw ();

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

    typedef
        std::multiset<
            com::sun::star::uno::Reference<
                com::sun::star::util::XChangesListener > >
        ChangesListeners;

    rtl::OUString pathRepresentation_;
    rtl::OUString locale_;
    bool update_;
    Path path_;
    rtl::Reference< Node > node_;
    rtl::OUString name_;
    bool finalized_;
    ChangesListeners changesListeners_;
};

}

#endif
