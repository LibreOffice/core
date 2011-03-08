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
#ifndef _URP_PROPERTY_HXX_
#define _URP_PROPERTY_HXX_
#include <bridges/remote/remote.h>
#include <rtl/ustring.hxx>
#include <rtl/byteseq.hxx>

namespace bridges_urp
{
    struct Properties
    {
        ::rtl::ByteSequence seqBridgeID;
        sal_Int32           nTypeCacheSize;
        sal_Int32           nOidCacheSize;
        sal_Int32           nTidCacheSize;
        ::rtl::OUString     sSupportedVersions;
        ::rtl::OUString     sVersion;
        sal_Int32           nFlushBlockSize;
        sal_Int32           nOnewayTimeoutMUSEC;
        sal_Bool            bSupportsMustReply;
        sal_Bool            bSupportsSynchronous;
        sal_Bool            bSupportsMultipleSynchronous;
        sal_Bool            bClearCache;
        sal_Bool            bNegotiate;
        sal_Bool            bForceSynchronous;
        sal_Bool            bCurrentContext;

        inline Properties()
            : nTypeCacheSize( 256 )
            , nOidCacheSize( 256 )
            , nTidCacheSize( 256 )
            , sSupportedVersions( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "1.0" ) ) )
            , sVersion( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "1.0" )))
            , nFlushBlockSize( 4*1024 )
            , nOnewayTimeoutMUSEC( 10000 )
            , bSupportsMustReply( sal_False )
            , bSupportsSynchronous( sal_False )
            , bSupportsMultipleSynchronous( sal_False )
            , bClearCache( sal_False )
            , bNegotiate( sal_True )
            , bForceSynchronous( sal_True )
            , bCurrentContext( sal_False )
            {}

        inline Properties & SAL_CALL operator = ( const Properties &props )
        {
            seqBridgeID                  = props.seqBridgeID;
            nTypeCacheSize               = props.nTypeCacheSize;
            nOidCacheSize                = props.nOidCacheSize;
            nTidCacheSize                = props.nTidCacheSize;
            sSupportedVersions           = props.sSupportedVersions;
            sVersion                     = props.sVersion;
            nFlushBlockSize              = props.nFlushBlockSize;
            nOnewayTimeoutMUSEC          = props.nOnewayTimeoutMUSEC;
            bSupportsMustReply           = props.bSupportsMustReply;
            bSupportsSynchronous         = props.bSupportsSynchronous;
            bSupportsMultipleSynchronous = props.bSupportsMultipleSynchronous;
            bClearCache                  = props.bClearCache;
            bNegotiate                   = props.bNegotiate;
            bForceSynchronous            = props.bForceSynchronous;
            bCurrentContext              = props.bCurrentContext;
            return *this;
        }
    };
} // end namespace bridges_urp
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
