/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strucvt.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 13:54:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

// =======================================================================

void UniString::InitStringRes( const char* pUTF8Str, sal_Int32 nLen )
{
    DBG_CTOR( UniString, DbgCheckUniString );
    OSL_ENSURE(nLen <= STRING_MAXLEN, "Overflowing UniString");

    mpData = NULL;
    rtl_string2UString( (rtl_uString **)(&mpData),
                        pUTF8Str, nLen,
                        RTL_TEXTENCODING_UTF8,
                        RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_MAPTOPRIVATE |
                        RTL_TEXTTOUNICODE_FLAGS_MBUNDEFINED_DEFAULT |
                        RTL_TEXTTOUNICODE_FLAGS_INVALID_DEFAULT );
}

// =======================================================================

UniString::UniString( const ByteString& rByteStr, rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( UniString, DbgCheckUniString );
    DBG_CHKOBJ( &rByteStr, ByteString, DbgCheckByteString );

    mpData = NULL;
    rtl_string2UString( (rtl_uString **)(&mpData),
                        rByteStr.mpData->maStr, rByteStr.mpData->mnLen,
                        eTextEncoding, nCvtFlags );
}

// -----------------------------------------------------------------------

UniString::UniString( const ByteString& rByteStr, xub_StrLen nPos, xub_StrLen nLen,
                      rtl_TextEncoding eTextEncoding, sal_uInt32 nCvtFlags )
{
    DBG_CTOR( UniString, DbgCheckUniString );
    DBG_CHKOBJ( &rByteStr, ByteString, DbgCheckByteString );

    // Stringlaenge ermitteln
    if ( nPos > rByteStr.mpData->mnLen )
        nLen = 0;
    else
    {
        // Laenge korrigieren, wenn noetig
        sal_Int32 nMaxLen = rByteStr.mpData->mnLen-nPos;
        if ( nLen > nMaxLen )
            nLen = static_cast< xub_StrLen >(nMaxLen);
    }

    mpData = NULL;
    rtl_string2UString( (rtl_uString **)(&mpData),
                        rByteStr.mpData->maStr+nPos, nLen,
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

UniString UniString::intern() const
{
    UniString aStr;

    rtl_uString_intern( reinterpret_cast<rtl_uString **>(&aStr.mpData),
                        (rtl_uString *)(&mpData) );

    return aStr;
}

// =======================================================================

#ifndef _TOOLS_RC_HXX
#include <rc.hxx>
#endif
#ifndef _TOOLS_RCID_H
#include <rcid.h>
#endif

UniString::UniString( const ResId& rResId )
{
    rResId.SetRT( RSC_STRING );
    ResMgr* pResMgr = rResId.GetResMgr();
    if ( !pResMgr )
        pResMgr = Resource::GetResManager();

    mpData = NULL;
    if ( pResMgr->GetResource( rResId ) )
    {
        // String laden
        RSHEADER_TYPE * pResHdr = (RSHEADER_TYPE*)pResMgr->GetClass();
        //sal_uInt32 nLen = pResHdr->GetLocalOff() - sizeof( RSHEADER_TYPE );

        sal_Int32 nStringLen = rtl_str_getLength( (char*)(pResHdr+1) );
        InitStringRes( (const char*)(pResHdr+1), nStringLen );

        sal_uInt32 nSize = sizeof( RSHEADER_TYPE )
            + sal::static_int_cast< sal_uInt32 >(nStringLen) + 1;
        nSize += nSize % 2;
        pResMgr->Increment( nSize );
    }
    else
    {
        STRING_NEW((STRING_TYPE **)&mpData);

#if OSL_DEBUG_LEVEL > 0
        *this = UniString::CreateFromAscii( "<resource id " );
        Append( UniString::CreateFromInt32( rResId.GetId() ) );
        AppendAscii( " not found>" );
#endif
    }


    ResHookProc pImplResHookProc = ResMgr::GetReadStringHook();
    if ( pImplResHookProc )
        pImplResHookProc( *this );
}

