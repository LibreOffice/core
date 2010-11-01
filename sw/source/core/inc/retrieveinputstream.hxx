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
#ifndef _RETRIEVEINPUTSTREAM_HXX
#define _RETRIEVEINPUTSTREAM_HXX

#include <observablethread.hxx>
#include <tools/string.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <retrievedinputstreamdata.hxx>

/** class for a thread to retrieve an input stream given by an URL

    OD 2007-01-29 #i73788#

    @author OD
*/
class SwAsyncRetrieveInputStreamThread : public ObservableThread
{
    public:

        static ::rtl::Reference< ObservableThread > createThread(
                    const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                    const String& rLinkedURL );

        virtual ~SwAsyncRetrieveInputStreamThread();

    protected:

        virtual void threadFunction();

    private:

        SwAsyncRetrieveInputStreamThread( const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                                          const String& rLinkedURL );

        const SwRetrievedInputStreamDataManager::tDataKey mnDataKey;
        const String mrLinkedURL;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
