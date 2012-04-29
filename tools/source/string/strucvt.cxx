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

UniString::UniString( const rtl::OString& rByteStr, rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( UniString, DbgCheckUniString );

    mpData = NULL;
    rtl_string2UString( (rtl_uString **)(&mpData),
                        rByteStr.getStr(), rByteStr.getLength(),
                        eTextEncoding, nCvtFlags );
}

// -----------------------------------------------------------------------

UniString::UniString( const char* pByteStr,
                      rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( UniString, DbgCheckUniString );
    DBG_ASSERT( pByteStr, "UniString::UniString() - pByteStr is NULL" );

    mpData = NULL;
    rtl_string2UString( (rtl_uString **)(&mpData),
                        pByteStr, ImplStringLen( pByteStr ),
                        eTextEncoding, nCvtFlags );
}

// -----------------------------------------------------------------------

UniString::UniString( const char* pByteStr, xub_StrLen nLen,
                      rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( UniString, DbgCheckUniString );
    DBG_ASSERT( pByteStr, "UniString::UniString() - pByteStr is NULL" );

    if ( nLen == STRING_LEN )
        nLen = ImplStringLen( pByteStr );

    mpData = NULL;
    rtl_string2UString( (rtl_uString **)(&mpData),
                        pByteStr, nLen,
                        eTextEncoding, nCvtFlags );
}

// =======================================================================

UniString::UniString( const rtl::OUString& rStr )
    : mpData(NULL)
{
    DBG_CTOR( UniString, DbgCheckUniString );

    OSL_ENSURE(rStr.pData->length < STRING_MAXLEN,
               "Overflowing rtl::OUString -> UniString cut to zero length");


    if (rStr.pData->length < STRING_MAXLEN)
    {
        mpData = reinterpret_cast< UniStringData * >(const_cast< rtl::OUString & >(rStr).pData);
        STRING_ACQUIRE((STRING_TYPE *)mpData);
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }
}

// -----------------------------------------------------------------------

UniString& UniString::Assign( const rtl::OUString& rStr )
{
    DBG_CHKTHIS( UniString, DbgCheckUniString );

    OSL_ENSURE(rStr.pData->length < STRING_MAXLEN,
               "Overflowing rtl::OUString -> UniString cut to zero length");


    if (rStr.pData->length < STRING_MAXLEN)
    {
        STRING_RELEASE((STRING_TYPE *)mpData);
        mpData = reinterpret_cast< UniStringData * >(const_cast< rtl::OUString & >(rStr).pData);
        STRING_ACQUIRE((STRING_TYPE *)mpData);
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }

    return *this;
}

// =======================================================================

#include <rtl/ustrbuf.hxx>
#include <tools/rc.hxx>
#include <tools/rcid.h>

UniString::UniString( const ResId& rResId )
    : mpData(NULL)
{
    rtl::OUString sStr(rResId.toString());

    DBG_CTOR( UniString, DbgCheckUniString );

    OSL_ENSURE(sStr.pData->length < STRING_MAXLEN,
               "Overflowing rtl::OUString -> UniString cut to zero length");

    if (sStr.pData->length < STRING_MAXLEN)
    {
        mpData = reinterpret_cast< UniStringData * >(sStr.pData);
        STRING_ACQUIRE((STRING_TYPE *)mpData);
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);
    }


}

rtl::OUString ResId::toString() const
{
    SetRT( RSC_STRING );
    ResMgr* pResMgr = GetResMgr();

    if ( !pResMgr || !pResMgr->GetResource( *this ) )
    {
        rtl::OUString sRet;

#if OSL_DEBUG_LEVEL > 0
        sRet = rtl::OUStringBuffer().
            appendAscii(RTL_CONSTASCII_STRINGPARAM("<resource id ")).
            append(static_cast<sal_Int32>(GetId())).
            appendAscii(RTL_CONSTASCII_STRINGPARAM(" not found>")).
            makeStringAndClear();
#endif

        if( pResMgr )
            pResMgr->PopContext();

        return sRet;
    }

    // String loading
    RSHEADER_TYPE * pResHdr = (RSHEADER_TYPE*)pResMgr->GetClass();

    sal_Int32 nStringLen = rtl_str_getLength( (char*)(pResHdr+1) );
    rtl::OUString sRet((const char*)(pResHdr+1), nStringLen, RTL_TEXTENCODING_UTF8);

    sal_uInt32 nSize = sizeof( RSHEADER_TYPE )
        + sal::static_int_cast< sal_uInt32 >(nStringLen) + 1;
    nSize += nSize % 2;
    pResMgr->Increment( nSize );

    ResHookProc pImplResHookProc = ResMgr::GetReadStringHook();
    if ( pImplResHookProc )
        sRet = pImplResHookProc(sRet);
    return sRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
