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


// =======================================================================

ByteString::ByteString( const rtl::OString& rStr )
    : mpData(NULL)
{
    DBG_CTOR( ByteString, DbgCheckByteString );

    OSL_ENSURE(rStr.pData->length < STRING_MAXLEN,
               "Overflowing rtl::OString -> ByteString cut to zero length");

    if (rStr.pData->length < STRING_MAXLEN)
    {
        mpData = reinterpret_cast< ByteStringData * >(const_cast< rtl::OString & >(rStr).pData);
        STRING_ACQUIRE((STRING_TYPE *)mpData);
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
}

// -----------------------------------------------------------------------

ByteString& ByteString::Assign( const rtl::OString& rStr )
{
    DBG_CHKTHIS( ByteString, DbgCheckByteString );

    OSL_ENSURE(rStr.pData->length < STRING_MAXLEN,
               "Overflowing rtl::OString -> ByteString cut to zero length");

    if (rStr.pData->length < STRING_MAXLEN)
    {
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = reinterpret_cast< ByteStringData * >(const_cast< rtl::OString & >(rStr).pData);
        STRING_ACQUIRE((STRING_TYPE *)mpData);
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }

    return *this;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
