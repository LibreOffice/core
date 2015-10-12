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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_RETRIEVEINPUTSTREAM_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_RETRIEVEINPUTSTREAM_HXX

#include <observablethread.hxx>
#include <rtl/ustring.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <retrievedinputstreamdata.hxx>

/** class for a thread to retrieve an input stream given by an URL

    OD 2007-01-29 #i73788#
*/
class SwAsyncRetrieveInputStreamThread : public ObservableThread
{
    public:

        static ::rtl::Reference< ObservableThread > createThread(
                    const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                    const OUString& rLinkedURL, const OUString& rReferer );

        virtual ~SwAsyncRetrieveInputStreamThread();

    protected:

        virtual void threadFunction() override;

    private:

        SwAsyncRetrieveInputStreamThread( const SwRetrievedInputStreamDataManager::tDataKey nDataKey,
                                          const OUString& rLinkedURL,
                                          const OUString& rReferer );

        const SwRetrievedInputStreamDataManager::tDataKey mnDataKey;
        const OUString mrLinkedURL;
        const OUString mrReferer;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
