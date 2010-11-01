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

#ifndef SD_FRAMEWORK_CHANGE_OPERATION_QUEUE_HXX
#define SD_FRAMEWORK_CHANGE_OPERATION_QUEUE_HXX

#include <com/sun/star/drawing/framework/XConfigurationChangeRequest.hpp>

#include <list>

namespace sd { namespace framework {


/** The ChangeRequestQueue stores the pending requests for changes to the
    requested configuration.  It is the task of the
    ChangeRequestQueueProcessor to process these requests.
*/
class ChangeRequestQueue
    : public ::std::list<com::sun::star::uno::Reference<
        ::com::sun::star::drawing::framework::XConfigurationChangeRequest> >
{
public:
    /** Create an empty queue.
    */
    ChangeRequestQueue (void);
};


} } // end of namespace sd::framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
