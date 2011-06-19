/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svtools.hxx"
#include <svtools/filterutils.hxx>
#include <rtl/ustrbuf.hxx>

namespace svt
{
//........................................................................

    using namespace ::com::sun::star;

    rtl::OUString lcl_createStringFromArray( const char* pcCharArr, sal_uInt32 nBufSize, bool bIsCompressed )
    {
        rtl::OUStringBuffer aBuffer;
        if( bIsCompressed )
        {
            // buffer contains compressed Unicode, not encoded bytestring
            sal_Int32 nStrLen = static_cast< sal_Int32 >( nBufSize );
            aBuffer.setLength( nStrLen );
            const char* pcCurrChar = pcCharArr;
            for( sal_Int32 nChar = 0; nChar < nStrLen; ++nChar, ++pcCurrChar )
                /*  *pcCurrChar may contain negative values and therefore MUST be
                    casted to unsigned char, before assigned to a sal_Unicode. */
                aBuffer.setCharAt( nChar, static_cast< unsigned char >( *pcCurrChar ) );
        }
        else
        {
            // buffer contains Little-Endian Unicode
            sal_Int32 nStrLen = static_cast< sal_Int32 >( nBufSize ) / 2;
            aBuffer.setLength( nStrLen );
            const char* pcCurrChar = pcCharArr;
            for( sal_Int32 nChar = 0; nChar < nStrLen; ++nChar )
            {
                /*  *pcCurrChar may contain negative values and therefore MUST be
                    casted to unsigned char, before assigned to a sal_Unicode. */
                sal_Unicode cChar = static_cast< unsigned char >( *pcCurrChar++ );
                cChar |= (static_cast< unsigned char >( *pcCurrChar++ ) << 8);
                aBuffer.setCharAt( nChar, cChar );
            }
        }
        return aBuffer.makeStringAndClear();
    }

    rtl::OUString BinFilterUtils::CreateOUStringFromUniStringArray( const char* pcCharArr, sal_uInt32 nBufSize )
    {
        return lcl_createStringFromArray( pcCharArr, nBufSize, false );
    }

    rtl::OUString BinFilterUtils::CreateOUStringFromStringArray( const char* pcCharArr, sal_uInt32 nBufSize )
    {
        return lcl_createStringFromArray( pcCharArr, nBufSize, true );
    }
//........................................................................
} // namespace svt
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
