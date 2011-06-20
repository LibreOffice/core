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
#ifndef _RETRIEVEINPUTSTREAMCONSUMER_HXX
#define _RETRIEVEINPUTSTREAMCONSUMER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <osl/interlck.h>
#include <retrievedinputstreamdata.hxx>

class SwGrfNode;
class String;
class SwAsyncRetrieveInputStreamThread;

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
        void CreateThread( const String& rURL );

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
