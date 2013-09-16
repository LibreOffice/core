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
#ifndef _RETRIEVEINPUTSTREAMCONSUMER_HXX
#define _RETRIEVEINPUTSTREAMCONSUMER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <osl/interlck.h>
#include <retrievedinputstreamdata.hxx>

class SwGrfNode;

/** class to provide creation of a thread to retrieve an input stream given by
    an URL and to consume the retrieved input stream.

    #i73788#
*/
class SwAsyncRetrieveInputStreamThreadConsumer
{
    public:

        SwAsyncRetrieveInputStreamThreadConsumer( SwGrfNode& rGrfNode );
        ~SwAsyncRetrieveInputStreamThreadConsumer();

        /** method to create thread
        */
        void CreateThread( const OUString& rURL );

        /** method called to provide the retrieved input stream to the thread Consumer
        */
        void ApplyInputStream(
            com::sun::star::uno::Reference<com::sun::star::io::XInputStream> xInputStream,
            const sal_Bool bIsStreamReadOnly );

    private:

        SwGrfNode& mrGrfNode;
        oslInterlockedCount mnThreadID;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
