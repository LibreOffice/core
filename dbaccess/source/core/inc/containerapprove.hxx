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

#ifndef DBACCESS_CONTAINERAPPROVE_HXX
#define DBACCESS_CONTAINERAPPROVE_HXX

/** === begin UNO includes === **/
#include <com/sun/star/uno/XInterface.hpp>
/** === end UNO includes === **/

#include <boost/shared_ptr.hpp>

namespace dbaccess
{

    //====================================================================
    //= IContainerApprove
    //====================================================================
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
            const ::rtl::OUString& _rName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxElement
        ) = 0;
    };

    typedef ::boost::shared_ptr< IContainerApprove >    PContainerApprove;

} // namespace dbaccess

#endif // DBACCESS_CONTAINERAPPROVE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
