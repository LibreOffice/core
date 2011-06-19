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
#ifndef _UNTOOLS_UCBSTREAMHELPER_HXX
#define _UNTOOLS_UCBSTREAMHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include "unotools/unotoolsdllapi.h"

#include <tools/stream.hxx>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace task
            {
                class XInteractionHandler;
            }
            namespace io
            {
                class XStream;
                class XInputStream;
            }
        }
    }
}

#define NS_UNO ::com::sun::star::uno
#define NS_IO ::com::sun::star::io
#define NS_TASK ::com::sun::star::task

class String;
namespace utl
{
    class UcbLockBytesHandler;

    class UNOTOOLS_DLLPUBLIC UcbStreamHelper : public SvStream
    {
    public:
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                NS_UNO::Reference < NS_TASK::XInteractionHandler >,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( const String& rFileName, StreamMode eOpenMode,
                                sal_Bool bFileExists,
                                UcbLockBytesHandler* pHandler=0, sal_Bool bForceSynchron=sal_True );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XInputStream > xStream );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XStream > xStream );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XInputStream > xStream, sal_Bool bCloseStream );
        static SvStream*    CreateStream( NS_UNO::Reference < NS_IO::XStream > xStream, sal_Bool bCloseStream );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
