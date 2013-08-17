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

#ifndef DBACCESS_CONTAINERAPPROVE_HXX
#define DBACCESS_CONTAINERAPPROVE_HXX

#include <com/sun/star/uno/XInterface.hpp>

#include <boost/shared_ptr.hpp>

namespace dbaccess
{

    // IContainerApprove
    /** interface for approving elements to be inserted into a container

        On the long run, one could imagine that this interface completely encapsulates
        container/element approvals in all our various container classes herein (document
        containers, definition containers, table containers, query containers,
        command definition containers, bookmark containers). This would decrease coupling
        of the respective classes.
     */
    class SAL_NO_VTABLE IContainerApprove
    {
    public:
        virtual ~IContainerApprove() {}

        /** approves a given element for insertion into the container
            @param  _rName
                specifies the name under which the element is going to be inserted
            @param  _rxElement
                specifies the element which is going to be inserted
            @throws Exception
                if the name or the object are invalid, or not eligible for insertion
                into the container
        */
        virtual void SAL_CALL   approveElement(
            const OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement
        ) = 0;
    };

    typedef ::boost::shared_ptr< IContainerApprove >    PContainerApprove;

} // namespace dbaccess

#endif // DBACCESS_CONTAINERAPPROVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
