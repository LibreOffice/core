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
#ifndef INCLUDED_UNOTOOLS_UCBSTREAMHELPER_HXX
#define INCLUDED_UNOTOOLS_UCBSTREAMHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <unotools/unotoolsdllapi.h>

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

namespace utl
{
    class UcbLockBytesHandler;

    class UNOTOOLS_DLLPUBLIC UcbStreamHelper : public SvStream
    {
    public:
        static SvStream*    CreateStream( const OUString& rFileName, StreamMode eOpenMode );
        static SvStream*    CreateStream( const OUString& rFileName, StreamMode eOpenMode,
                                          const css::uno::Reference < css::task::XInteractionHandler >& );
        static SvStream*    CreateStream( const OUString& rFileName, StreamMode eOpenMode,
                                          bool bFileExists );
        static SvStream*    CreateStream( const css::uno::Reference < css::io::XInputStream >& xStream );
        static SvStream*    CreateStream( const css::uno::Reference < css::io::XStream >& xStream );
        static SvStream*    CreateStream( const css::uno::Reference < css::io::XInputStream >& xStream, bool bCloseStream );
        static SvStream*    CreateStream( const css::uno::Reference < css::io::XStream >& xStream, bool bCloseStream );
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
