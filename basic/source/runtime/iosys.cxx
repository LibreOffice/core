/*************************************************************************
 *
 *  $RCSfile: iosys.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:11 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#include <osl/security.h>

#include "runtime.hxx"

#ifdef _USE_UNO

// <-- encoding
#ifdef UNX
#include <alloca.h>
#endif
#ifdef WNT
#include <malloc.h>
#define alloca _alloca
#endif
#include <ctype.h>
#include <rtl/byteseq.hxx>
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
// encoding -->

#include <unotools/processfactory.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/bridge/XBridge.hpp>
#include <com/sun/star/bridge/XBridgeFactory.hpp>

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::ucb;
using namespace com::sun::star::io;
using namespace com::sun::star::bridge;

#endif /* _USE_UNO */

#pragma hdrstop
#include "iosys.hxx"
#include "sbintern.hxx"

#include "segmentc.hxx"
#pragma SW_SEGMENT_CLASS( SBRUNTIME, SBRUNTIME_CODE )

// Der Input-Dialog:

class SbiInputDialog : public ModalDialog {
    Edit aInput;
    OKButton aOk;
    CancelButton aCancel;
    String aText;
    DECL_LINK( Ok, Window * );
    DECL_LINK( Cancel, Window * );
public:
    SbiInputDialog( Window*, const String& );
    const String& GetInput() { return aText; }
};

SbiInputDialog::SbiInputDialog( Window* pParent, const String& rPrompt )
            :ModalDialog( pParent, WB_SVLOOK | WB_MOVEABLE | WB_CLOSEABLE ),
             aInput( this, WB_SVLOOK | WB_LEFT | WB_BORDER ),
             aOk( this ), aCancel( this )
{
    SetText( rPrompt );
    aOk.SetClickHdl( LINK( this, SbiInputDialog, Ok ) );
    aCancel.SetClickHdl( LINK( this, SbiInputDialog, Cancel ) );
    SetMapMode( MapMode( MAP_APPFONT ) );

    Point aPt = LogicToPixel( Point( 50, 50 ) );
    Size  aSz = LogicToPixel( Size( 145, 65 ) );
    SetPosSizePixel( aPt, aSz );
    aPt = LogicToPixel( Point( 10, 10 ) );
    aSz = LogicToPixel( Size( 120, 12 ) );
    aInput.SetPosSizePixel( aPt, aSz );
    aPt = LogicToPixel( Point( 15, 30 ) );
    aSz = LogicToPixel( Size( 45, 15) );
    aOk.SetPosSizePixel( aPt, aSz );
    aPt = LogicToPixel( Point( 80, 30 ) );
    aSz = LogicToPixel( Size( 45, 15) );
    aCancel.SetPosSizePixel( aPt, aSz );

    aInput.Show();
    aOk.Show();
    aCancel.Show();
}

IMPL_LINK_INLINE_START( SbiInputDialog, Ok, Window *, pWindow )
{
    aText = aInput.GetText();
    EndDialog( 1 );
    return 0;
}
IMPL_LINK_INLINE_END( SbiInputDialog, Ok, Window *, pWindow )

IMPL_LINK_INLINE_START( SbiInputDialog, Cancel, Window *, pWindow )
{
    EndDialog( 0 );
    return 0;
}
IMPL_LINK_INLINE_END( SbiInputDialog, Cancel, Window *, pWindow )

//////////////////////////////////////////////////////////////////////////

SbiStream::SbiStream()
    : pStrm( 0 )
{
}

SbiStream::~SbiStream()
{
    delete pStrm;
}

// Ummappen eines SvStream-Fehlers auf einen StarBASIC-Code

void SbiStream::MapError()
{
    if( pStrm )
     switch( pStrm->GetError() )
     {
        case SVSTREAM_OK:
            nError = 0; break;
        case SVSTREAM_FILE_NOT_FOUND:
            nError = SbERR_FILE_NOT_FOUND; break;
        case SVSTREAM_PATH_NOT_FOUND:
            nError = SbERR_PATH_NOT_FOUND; break;
        case SVSTREAM_TOO_MANY_OPEN_FILES:
            nError = SbERR_TOO_MANY_FILES; break;
        case SVSTREAM_ACCESS_DENIED:
            nError = SbERR_ACCESS_DENIED; break;
        case SVSTREAM_INVALID_PARAMETER:
            nError = SbERR_BAD_ARGUMENT; break;
        case SVSTREAM_OUTOFMEMORY:
            nError = SbERR_NO_MEMORY; break;
        default:
            nError = SbERR_IO_ERROR; break;
    }
}

#ifdef _USE_UNO

// TODO: Code is copied from daemons2/source/uno/asciiEncoder.cxx

namespace basicEncoder
{
    enum EncodeMechanism
    {
        ENCODE_ALL,
        WAS_ENCODED,
        NOT_CANONIC
    };

    enum DecodeMechanism
    {
        NO_DECODE,
        DECODE_TO_IURI,
        DECODE_WITH_CHARSET
    };

    enum EscapeType
    {
        ESCAPE_NO,
        ESCAPE_OCTET,
        ESCAPE_UTF32
    };

    inline bool isUSASCII(sal_uInt32 nChar)
    {
        return nChar <= 0x7F;
    }

    inline bool isDigit(sal_uInt32 nChar)
    {
        return nChar >= '0' && nChar <= '9';
    }

    inline int getHexWeight(sal_uInt32 nChar)
    {
        return isDigit(nChar) ? int(nChar - '0') :
            nChar >= 'A' && nChar <= 'F' ? int(nChar - 'A' + 10) :
            nChar >= 'a' && nChar <= 'f' ? int(nChar - 'a' + 10) : -1;
    }

    inline bool isHighSurrogate(sal_uInt32 nUTF16)
    {
        return nUTF16 >= 0xD800 && nUTF16 <= 0xDBFF;
    }

    inline bool isLowSurrogate(sal_uInt32 nUTF16)
    {
        return nUTF16 >= 0xDC00 && nUTF16 <= 0xDFFF;
    }

    sal_uInt32 getHexDigit(int nWeight)
    {
        OSL_ASSERT(nWeight >= 0 && nWeight < 16);
        static sal_Char const aDigits[16]
            = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C',
                'D', 'E', 'F' };
        return aDigits[nWeight];
    }

    inline void appendEscape(rtl::OUStringBuffer & rTheText,
                             sal_Char cEscapePrefix, sal_uInt32 nOctet)
    {
        rTheText.append(sal_Unicode(cEscapePrefix));
        rTheText.append(sal_Unicode(getHexDigit(int(nOctet >> 4))));
        rTheText.append(sal_Unicode(getHexDigit(int(nOctet & 15))));
    }

    inline sal_uInt32 getUTF32Character(sal_Unicode const *& rBegin,
                                        sal_Unicode const * pEnd)
    {
        OSL_ASSERT(rBegin && rBegin < pEnd);
        if (rBegin + 1 < pEnd && rBegin[0] >= 0xD800 && rBegin[0] <= 0xDBFF
            && rBegin[1] >= 0xDC00 && rBegin[1] <= 0xDFFF)
        {
            sal_uInt32 nUTF32 = sal_uInt32(*rBegin++ & 0x3FF) << 10;
            return (nUTF32 | (*rBegin++ & 0x3FF)) + 0x10000;
        }
        else
            return *rBegin++;
    }

    sal_uInt32 getUTF32(sal_Unicode const *& rBegin, sal_Unicode const * pEnd,
                        bool bOctets, sal_Char cEscapePrefix,
                        EncodeMechanism eMechanism, rtl_TextEncoding eCharset,
                        EscapeType & rEscapeType)
    {
        OSL_ASSERT(rBegin < pEnd);
        sal_uInt32 nUTF32 = bOctets ? *rBegin++ : getUTF32Character(rBegin, pEnd);
        switch (eMechanism)
        {
        case ENCODE_ALL:
            rEscapeType = ESCAPE_NO;
            break;

        case WAS_ENCODED:
        {
            int nWeight1;
            int nWeight2;
            if (nUTF32 == cEscapePrefix && rBegin + 1 < pEnd
                && (nWeight1 = getHexWeight(rBegin[0])) >= 0
                && (nWeight2 = getHexWeight(rBegin[1])) >= 0)
            {
                rBegin += 2;
                nUTF32 = nWeight1 << 4 | nWeight2;
                switch (eCharset)
                {
                default:
                    OSL_ASSERT(false);
                case RTL_TEXTENCODING_ASCII_US:
                    rEscapeType
                        = isUSASCII(nUTF32) ? ESCAPE_UTF32 : ESCAPE_OCTET;
                    break;

                case RTL_TEXTENCODING_ISO_8859_1:
                    rEscapeType = ESCAPE_UTF32;
                    break;

                case RTL_TEXTENCODING_UTF8:
                    if (isUSASCII(nUTF32))
                        rEscapeType = ESCAPE_UTF32;
                    else
                    {
                        if (nUTF32 >= 0xC0 && nUTF32 <= 0xF4)
                        {
                            sal_uInt32 nEncoded;
                            int nShift;
                            sal_uInt32 nMin;
                            if (nUTF32 <= 0xDF)
                            {
                                nEncoded = (nUTF32 & 0x1F) << 6;
                                nShift = 0;
                                nMin = 0x80;
                            }
                            else if (nUTF32 <= 0xEF)
                            {
                                nEncoded = (nUTF32 & 0x0F) << 12;
                                nShift = 6;
                                nMin = 0x800;
                            }
                            else
                            {
                                nEncoded = (nUTF32 & 0x07) << 18;
                                nShift = 12;
                                nMin = 0x10000;
                            }
                            sal_Unicode const * p = rBegin;
                            bool bUTF8 = true;
                            for (;;)
                            {
                                if (p + 2 >= pEnd || p[0] != cEscapePrefix
                                    || (nWeight1 = getHexWeight(p[1])) < 0
                                    || (nWeight2 = getHexWeight(p[2])) < 0
                                    || nWeight1 < 8)
                                {
                                    bUTF8 = false;
                                    break;
                                }
                                p += 3;
                                nEncoded
                                    |= ((nWeight1 & 3) << 4 | nWeight2)
                                                       << nShift;
                                if (nShift == 0)
                                    break;
                                nShift -= 6;
                            }
                            if (bUTF8 && nEncoded >= nMin
                                && !isHighSurrogate(nEncoded)
                                && !isLowSurrogate(nEncoded)
                                && nEncoded <= 0x10FFFF)
                            {
                                rBegin = p;
                                nUTF32 = nEncoded;
                                rEscapeType = ESCAPE_UTF32;
                                break;
                            }
                        }
                        rEscapeType = ESCAPE_OCTET;
                    }
                    break;
                }
            }
            else
                rEscapeType = ESCAPE_NO;
            break;
        }

        case NOT_CANONIC:
        {
            int nWeight1;
            int nWeight2;
            if (nUTF32 == cEscapePrefix && rBegin + 1 < pEnd
                && ((nWeight1 = getHexWeight(rBegin[0])) >= 0)
                && ((nWeight2 = getHexWeight(rBegin[1])) >= 0))
            {
                rBegin += 2;
                nUTF32 = nWeight1 << 4 | nWeight2;
                rEscapeType = ESCAPE_OCTET;
            }
            else
                rEscapeType = ESCAPE_NO;
            break;
        }
        }
        return nUTF32;
    }

    static rtl::OUString decodeImpl(sal_Unicode const * pBegin,
                         sal_Unicode const * pEnd, sal_Char cEscapePrefix,
                         DecodeMechanism eMechanism,
                         rtl_TextEncoding eCharset)
    {
        switch (eMechanism)
        {
        case NO_DECODE:
            return rtl::OUString(pBegin, pEnd - pBegin);

        case DECODE_TO_IURI:
            eCharset = RTL_TEXTENCODING_UTF8;
            break;
        }
        rtl::OUStringBuffer aResult;
        while (pBegin < pEnd)
        {
            EscapeType eEscapeType;
            sal_uInt32 nUTF32 = getUTF32(pBegin, pEnd, false, cEscapePrefix,
                                         WAS_ENCODED, eCharset, eEscapeType);
            switch (eEscapeType)
            {
            case ESCAPE_NO:
                aResult.append(sal_Unicode(nUTF32));
                break;

            case ESCAPE_OCTET:
                appendEscape(aResult, cEscapePrefix, nUTF32);
                break;

            case ESCAPE_UTF32:
                if (eMechanism == DECODE_TO_IURI && isUSASCII(nUTF32))
                    appendEscape(aResult, cEscapePrefix, nUTF32);
                else
                    aResult.append(sal_Unicode(nUTF32));
                break;
            }
        }
        return aResult.makeStringAndClear();
    }


    OUString AsciiEncoder::decodeUnoUrlParamValue(rtl::OUString const & rSource)
    {
        return decodeImpl(rSource.getStr(), rSource.getStr() + rSource.getLength(),
                      '%', DECODE_WITH_CHARSET, RTL_TEXTENCODING_UTF8);
    }

}


OUString findUserInDescription( const OUString& aDescription )
{
    OUString user;

    sal_Int32 index;
    sal_Int32 lastIndex = 0;

//#ifdef DEBUG
    //OString tmp = OUStringToOString(aDescription, RTL_TEXTENCODING_ASCII_US);
    //OSL_TRACE("Portal_XConnector %s\n", tmp.getStr());
//#endif

    do
    {
        index = aDescription.indexOf((sal_Unicode) ',', lastIndex);
        //OSL_TRACE("Portal_XConnector %d last_index %d\n", index, lastIndex);
        OUString token = (index == -1) ? aDescription.copy(lastIndex) : aDescription.copy(lastIndex, index - lastIndex);

//#ifdef DEBUG
        //OString token_tmp = OUStringToOString(token, RTL_TEXTENCODING_ASCII_US);
        //OSL_TRACE("Portal_XConnector - token %s\n", token_tmp.getStr());
//#endif

        lastIndex = index + 1;

        sal_Int32 eindex = token.indexOf((sal_Unicode)'=');
        OUString left = token.copy(0, eindex).toLowerCase().trim();
        OUString right = basicEncoder::AsciiEncoder::decodeUnoUrlParamValue(token.copy(eindex + 1).trim());

//#ifdef DEBUG
        //OString left_tmp = OUStringToOString(left, RTL_TEXTENCODING_ASCII_US);
        //OSL_TRACE("Portal_XConnector - left %s\n", left_tmp.getStr());
        //OString right_tmp = OUStringToOString(right, RTL_TEXTENCODING_ASCII_US);
        //OSL_TRACE("Portal_XConnector - right %s\n", right_tmp.getStr());
//#endif

        if(left.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("user"))))
        {
            user = right;
            break;
        }
    }
    while(index != -1);

    return user;

    /*
    ORef<IPortalConnector> connector;

    Reference<XConnection> xConnection;

    OUString protocol;
    connector = getPortalConnector(protocol);
    if(connector.isValid())
    {
        ORef<IConnection> connection;

        OUString server;
        if(host.getLength()) // let the server empty when there is no host
        {
            server += host;
            server += OUString(RTL_CONSTASCII_USTRINGPARAM(":"));
            server += port;
        }

        RC state;

        if(user.getLength() && !ticket.getLength()) // if there is a user and no ticket
        {
            state = connector->connectToService(user, password, server, service, connection);
        }
        else
        {
            ByteSequence byteSequence_ticket = AsciiEncoder::decode(ticket);

            state = connector->connectToService(user, byteSequence_ticket, server, service, connection);
        }

        if(state == E_None)
            xConnection = new Portal_XConnection(connection);
        else
            throw ConnectionSetupException(OUString(RTL_CONSTASCII_USTRINGPARAM("Portal_XConnector::connect: could not connect")), Reference<XInterface>());
    }
    else
        throw ConnectionSetupException(OUString(RTL_CONSTASCII_USTRINGPARAM("Portal_XConnector::connect: couldn't get connector")), Reference<XInterface>());

    return xConnection;
    */
}

#endif



BOOL needSecurityRestrictions( void )
{
#ifdef _USE_UNO
    static BOOL bNeedInit = TRUE;
    static BOOL bRetVal = TRUE;

    if( bNeedInit )
    {
        bNeedInit = FALSE;

        // Get system user to compare to portal user
        oslSecurity aSecurity = osl_getCurrentSecurity();
        OUString aSystemUser;
        sal_Bool bRet = osl_getUserName( aSecurity, &aSystemUser.pData );
        if( !bRet )
        {
            // No valid security! -> Secure mode!
            return TRUE;
        }

        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( !xSMgr.is() )
            return TRUE;
        Reference< XBridgeFactory > xBridgeFac( xSMgr->createInstance
            ( OUString::createFromAscii( "com.sun.star.bridge.BridgeFactory" ) ), UNO_QUERY );

        Sequence< Reference< XBridge > > aBridgeSeq;
        sal_Int32 nBridgeCount = 0;
        if( xBridgeFac.is() )
        {
            aBridgeSeq = xBridgeFac->getExistingBridges();
            nBridgeCount = aBridgeSeq.getLength();
        }

        if( nBridgeCount == 0 )
        {
            // No bridges -> local
            bRetVal = FALSE;
            return bRetVal;
        }

        // Iterate through all bridges to find (portal) user property
        const Reference< XBridge >* pBridges = aBridgeSeq.getConstArray();
        bRetVal = FALSE;    // Now only TRUE if user different from portal user is found
        sal_Int32 i;
        for( i = 0 ; i < nBridgeCount ; i++ )
        {
            const Reference< XBridge >& rxBridge = pBridges[ i ];
            OUString aDescription = rxBridge->getDescription();
            OUString aPortalUser = findUserInDescription( aDescription );
            if( aPortalUser.getLength() > 0 )
            {
                // User Found, compare to system user
                if( aPortalUser == aSystemUser )
                {
                    // Same user -> system security is ok, bRetVal stays FALSE
                    break;
                }
                else
                {
                    // Different user -> Secure mode!
                    bRetVal = TRUE;
                    break;
                }
            }
        }
        // No user found or PortalUser != SystemUser -> Secure mode! (Keep default value)
    }

    return bRetVal;
#else
    return FALSE;
#endif
}

// Returns TRUE if UNO is available, otherwise the old
// file system implementation has to be used
// (Implemented in iosys.cxx)
BOOL hasUno( void )
{
#ifdef _USE_UNO
    static BOOL bNeedInit = TRUE;
    static BOOL bRetVal = TRUE;

    if( bNeedInit )
    {
        bNeedInit = FALSE;
        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( !xSMgr.is() )
            bRetVal = FALSE;
    }
    return bRetVal;
#else
    return FALSE;
#endif
}


#ifdef _USE_UNO

class UCBStream : public SvStream
{
    Reference< XInputStream >   xIS;
    Reference< XOutputStream >  xOS;
    Reference< XStream >        xS;
    Reference< XSeekable >      xSeek;
public:
                    UCBStream( Reference< XInputStream > & xIS );
                    UCBStream( Reference< XOutputStream > & xOS );
                    UCBStream( Reference< XStream > & xS );
                    ~UCBStream();
    virtual ULONG   GetData( void* pData, ULONG nSize );
    virtual ULONG   PutData( const void* pData, ULONG nSize );
    virtual ULONG   SeekPos( ULONG nPos );
    virtual void    FlushData();
    virtual void    SetSize( ULONG nSize );
};

/*
ULONG UCBErrorToSvStramError( ucb::IOErrorCode nError )
{
    ULONG eReturn = ERRCODE_IO_GENERAL;
    switch( nError )
    {
        case ucb::IOErrorCode_ABORT:                eReturn = SVSTREAM_GENERALERROR; break;
        case ucb::IOErrorCode_NOT_EXISTING:         eReturn = SVSTREAM_FILE_NOT_FOUND; break;
        case ucb::IOErrorCode_NOT_EXISTING_PATH:    eReturn = SVSTREAM_PATH_NOT_FOUND; break;
        case ucb::IOErrorCode_OUT_OF_FILE_HANDLES:  eReturn = SVSTREAM_TOO_MANY_OPEN_FILES; break;
        case ucb::IOErrorCode_ACCESS_DENIED:        eReturn = SVSTREAM_ACCESS_DENIED; break;
        case ucb::IOErrorCode_LOCKING_VIOLATION:    eReturn = SVSTREAM_SHARING_VIOLATION; break;

        case ucb::IOErrorCode_INVALID_ACCESS:       eReturn = SVSTREAM_INVALID_ACCESS; break;
        case ucb::IOErrorCode_CANT_CREATE:          eReturn = SVSTREAM_CANNOT_MAKE; break;
        case ucb::IOErrorCode_INVALID_PARAMETER:    eReturn = SVSTREAM_INVALID_PARAMETER; break;

        case ucb::IOErrorCode_CANT_READ:            eReturn = SVSTREAM_READ_ERROR; break;
        case ucb::IOErrorCode_CANT_WRITE:           eReturn = SVSTREAM_WRITE_ERROR; break;
        case ucb::IOErrorCode_CANT_SEEK:            eReturn = SVSTREAM_SEEK_ERROR; break;
        case ucb::IOErrorCode_CANT_TELL:            eReturn = SVSTREAM_TELL_ERROR; break;

        case ucb::IOErrorCode_OUT_OF_MEMORY:        eReturn = SVSTREAM_OUTOFMEMORY; break;

        case SVSTREAM_FILEFORMAT_ERROR:             eReturn = SVSTREAM_FILEFORMAT_ERROR; break;
        case ucb::IOErrorCode_WRONG_VERSION:        eReturn = SVSTREAM_WRONGVERSION;
        case ucb::IOErrorCode_OUT_OF_DISK_SPACE:    eReturn = SVSTREAM_DISK_FULL; break;

        case ucb::IOErrorCode_BAD_CRC:              eReturn = ERRCODE_IO_BADCRC; break;
    }
    return eReturn;
}
*/

UCBStream::UCBStream( Reference< XInputStream > & rStm )
    : xIS( rStm )
    , xSeek( rStm, UNO_QUERY )
{
}

UCBStream::UCBStream( Reference< XOutputStream > & rStm )
    : xOS( rStm )
    , xSeek( rStm, UNO_QUERY )
{
}

UCBStream::UCBStream( Reference< XStream > & rStm )
    : xS( rStm )
    , xSeek( rStm, UNO_QUERY )
{
}


UCBStream::~UCBStream()
{
    try
    {
        if( xIS.is() )
            xIS->closeInput();
        else if( xOS.is() )
            xOS->closeOutput();
        else if( xS.is() )
            xS->closeStream();
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
}

ULONG   UCBStream::GetData( void* pData, ULONG nSize )
{
    try
    {
        if( xIS.is() )
        {
            Sequence<sal_Int8> aData;
            nSize = xIS->readBytes( aData, nSize );
            rtl_copyMemory( pData, aData.getConstArray(), nSize );
            return nSize;
        }
        else if( xS.is() )
        {
            Sequence<sal_Int8> aData;
            nSize = xS->readBytes( aData, nSize );
            rtl_copyMemory( pData, aData.getConstArray(), nSize );
            return nSize;
        }
        else
            SetError( ERRCODE_IO_GENERAL );
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    return 0;
}

ULONG   UCBStream::PutData( const void* pData, ULONG nSize )
{
    try
    {
        if( xOS.is() )
        {
            Sequence<sal_Int8> aData( (const sal_Int8 *)pData, nSize );
            xOS->writeBytes( aData );
            return nSize;
        }
        else if( xS.is() )
        {
            Sequence<sal_Int8> aData( (const sal_Int8 *)pData, nSize );
            xS->writeBytes( aData );
            return nSize;
        }
        else
            SetError( ERRCODE_IO_GENERAL );
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    return 0;
}

ULONG   UCBStream::SeekPos( ULONG nPos )
{
    if( !nPos )
        return 0;
    try
    {
        if( xSeek.is() )
        {
            sal_Int32 nLen = xSeek->getLength();
            if( nPos > nLen )
                nPos = nLen;
            xSeek->seek( nPos );
            return nPos;
        }
        else
            SetError( ERRCODE_IO_GENERAL );
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
    return 0;
}

void    UCBStream::FlushData()
{
    try
    {
        if( xOS.is() )
            xOS->flush();
        else if( xS.is() )
            xS->flush();
        else
            SetError( ERRCODE_IO_GENERAL );
    }
    catch( Exception & )
    {
        SetError( ERRCODE_IO_GENERAL );
    }
}

void    UCBStream::SetSize( ULONG nSize )
{
    DBG_ERROR( "not allowed to call from basic" )
    SetError( ERRCODE_IO_GENERAL );
}

#endif

// Oeffnen eines Streams
SbError SbiStream::Open
( short nCh, const ByteString& rName, short nStrmMode, short nFlags, short nL )
{
    nMode   = nFlags;
    nLen    = nL;
    nChan   = nCh;
    nLine   = 0;
    nExpandOnWriteTo = 0;
    if( ( nStrmMode & ( STREAM_READ|STREAM_WRITE ) ) == STREAM_READ )
        nStrmMode |= STREAM_NOCREATE;
    String aNameStr( rName, gsl_getSystemTextEncoding() );
#ifdef _USE_UNO
    if( hasUno() )
    {
        Reference< XMultiServiceFactory > xSMgr = getProcessServiceFactory();
        if( xSMgr.is() )
        {
            Reference< XSimpleFileAccess >
                xSFI( xSMgr->createInstance( OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ), UNO_QUERY );
            if( xSFI.is() )
            {
                try
            {

                if( (nStrmMode & (STREAM_READ | STREAM_WRITE)) == (STREAM_READ | STREAM_WRITE) )
                {
                    Reference< XStream > xIS = xSFI->openFileReadWrite( aNameStr );
                    pStrm = new UCBStream( xIS );
                }
                else if( nStrmMode & STREAM_WRITE )
                {
                    Reference< XStream > xIS = xSFI->openFileReadWrite( aNameStr );
                    pStrm = new UCBStream( xIS );
                    // Open for writing is not implemented in ucb yet!!!
                    //Reference< XOutputStream > xIS = xSFI->openFileWrite( aNameStr );
                    //pStrm = new UCBStream( xIS );
                }
                else //if( nStrmMode & STREAM_READ )
                {
                    Reference< XInputStream > xIS = xSFI->openFileRead( aNameStr );
                    pStrm = new UCBStream( xIS );
                }
                }
                catch( Exception & )
                {
                }
            }
        }
    }

#endif
    if( !pStrm )
        pStrm = new SvFileStream( aNameStr, nStrmMode );
    if( IsAppend() )
        pStrm->Seek( STREAM_SEEK_TO_END );
    MapError();
    if( nError )
        delete pStrm, pStrm = NULL;
    return nError;
}

SbError SbiStream::Close()
{
    if( pStrm )
    {
        if( !hasUno() )
            ((SvFileStream *)pStrm)->Close();
        MapError();
        delete pStrm;
        pStrm = NULL;
    }
    nChan = 0;
    return nError;
}

SbError SbiStream::Read( ByteString& rBuf, USHORT n )
{
    nExpandOnWriteTo = 0;
    if( IsText() )
    {
        pStrm->ReadLine( rBuf );
        nLine++;
    }
    else
    {
        if( !n ) n = nLen;
        if( !n )
            return nError = SbERR_BAD_RECORD_LENGTH;
        rBuf.Fill( n, ' ' );
        pStrm->Read( (void*)rBuf.GetBuffer(), n );
    }
    MapError();
    if( !nError && pStrm->IsEof() )
        nError = SbERR_READ_PAST_EOF;
    return nError;
}

SbError SbiStream::Read( char& ch )
{
    nExpandOnWriteTo = 0;
    if( !aLine.Len() )
    {
        Read( aLine, 0 );
        aLine += '\n';
    }
    ch = aLine.GetBuffer()[0];
    aLine.Erase( 0, 1 );
    return nError;
}

void SbiStream::ExpandFile()
{
    if ( nExpandOnWriteTo )
    {
        ULONG nCur = pStrm->Seek(STREAM_SEEK_TO_END);
        if( nCur < nExpandOnWriteTo )
        {
            ULONG nDiff = nExpandOnWriteTo - nCur;
            char c = 0;
            while( nDiff-- )
                *pStrm << c;
        }
        else
        {
            pStrm->Seek( nExpandOnWriteTo );
        }
        nExpandOnWriteTo = 0;
    }
}

SbError SbiStream::Write( const ByteString& rBuf, USHORT n )
{
    ExpandFile();
    if( IsAppend() )
        pStrm->Seek( STREAM_SEEK_TO_END );

    if( IsText() )
    {
        aLine += rBuf;
        // Raus damit, wenn das Ende ein LF ist, aber CRLF vorher
        // strippen, da der SvStrm ein CRLF anfuegt!
        USHORT n = aLine.Len();
        if( n && aLine.GetBuffer()[ --n ] == 0x0A )
        {
            aLine.Erase( n );
            if( n && aLine.GetBuffer()[ --n ] == 0x0D )
                aLine.Erase( n );
            pStrm->WriteLines( aLine );
            aLine.Erase();
        }
    }
    else
    {
        if( !n ) n = nLen;
        if( !n )
            return nError = SbERR_BAD_RECORD_LENGTH;
        pStrm->Write( rBuf.GetBuffer(), n );
        MapError();
    }
    return nError;
}

//////////////////////////////////////////////////////////////////////////

// Zugriff auf das aktuelle I/O-System:

SbiIoSystem* SbGetIoSystem()
{
    SbiInstance* pInst = pINST;
    return pInst ? pInst->GetIoSystem() : NULL;
}

//////////////////////////////////////////////////////////////////////////

SbiIoSystem::SbiIoSystem()
{
    for( short i = 0; i < CHANNELS; i++ )
        pChan[ i ] = NULL;
    nChan  = 0;
    nError = 0;
}

SbiIoSystem::~SbiIoSystem()
{
    Shutdown();
}

SbError SbiIoSystem::GetError()
{
    SbError n = nError; nError = 0;
    return n;
}

void SbiIoSystem::Open
    ( short nCh, const ByteString& rName, short nMode, short nFlags, short nLen )
{
    nError = 0;
    if( nCh >= CHANNELS || !nCh )
        nError = SbERR_BAD_CHANNEL;
    else if( pChan[ nCh ] )
        nError = SbERR_FILE_ALREADY_OPEN;
    else
    {
        pChan[ nCh ] = new SbiStream;
        nError = pChan[ nCh ]->Open( nCh, rName, nMode, nFlags, nLen );
        if( nError )
            delete pChan[ nCh ], pChan[ nCh ] = NULL;
    }
    nChan = 0;
}

// Aktuellen Kanal schliessen

void SbiIoSystem::Close()
{
    if( !nChan )
        nError = SbERR_BAD_CHANNEL;
    else if( !pChan[ nChan ] )
        nError = SbERR_BAD_CHANNEL;
    else
    {
        nError = pChan[ nChan ]->Close();
        delete pChan[ nChan ];
        pChan[ nChan ] = NULL;
    }
    nChan = 0;
}

// Shutdown nach Programmlauf

void SbiIoSystem::Shutdown()
{
    for( short i = 1; i < CHANNELS; i++ )
    {
        if( pChan[ i ] )
        {
            USHORT n = pChan[ i ]->Close();
            delete pChan[ i ];
            pChan[ i ] = NULL;
            if( n && !nError )
                nError = n;
        }
    }
    nChan = 0;
    // Noch was zu PRINTen?
    if( aOut.Len() )
    {
        String aOutStr( aOut, gsl_getSystemTextEncoding() );
#if defined GCC
        Window* pParent = Application::GetDefModalDialogParent();
        MessBox( pParent, WinBits( WB_OK ), String(), aOutStr ).Execute();
#else
        MessBox( GetpApp()->GetDefModalDialogParent(), WinBits( WB_OK ), String(), aOutStr ).Execute();
#endif
    }
    aOut.Erase();
}

// Aus aktuellem Kanal lesen

void SbiIoSystem::Read( ByteString& rBuf, short n )
{
    if( !nChan )
        ReadCon( rBuf );
    else if( !pChan[ nChan ] )
        nError = SbERR_BAD_CHANNEL;
    else
        nError = pChan[ nChan ]->Read( rBuf, n );
}

char SbiIoSystem::Read()
{
    char ch = ' ';
    if( !nChan )
    {
        if( !aIn.Len() )
        {
            ReadCon( aIn );
            aIn += '\n';
        }
        ch = aIn.GetBuffer()[0];
        aIn.Erase( 0, 1 );
    }
    else if( !pChan[ nChan ] )
        nError = SbERR_BAD_CHANNEL;
    else
        nError = pChan[ nChan ]->Read( ch );
    return ch;
}

void SbiIoSystem::Write( const ByteString& rBuf, short n )
{
    if( !nChan )
        WriteCon( rBuf );
    else if( !pChan[ nChan ] )
        nError = SbERR_BAD_CHANNEL;
    else
        nError = pChan[ nChan ]->Write( rBuf, n );
}

short SbiIoSystem::NextChannel()
{
    for( short i = 1; i < CHANNELS; i++ )
    {
        if( !pChan[ i ] )
            return i;
    }
    nError = SbERR_TOO_MANY_FILES;
    return CHANNELS;
}

// nChannel == 0..CHANNELS-1

SbiStream* SbiIoSystem::GetStream( short nChannel ) const
{
    SbiStream* pRet = 0;
    if( nChannel >= 0 && nChannel < CHANNELS )
        pRet = pChan[ nChannel ];
    return pRet;
}

void SbiIoSystem::CloseAll(void)
{
    for( short i = 1; i < CHANNELS; i++ )
    {
        if( pChan[ i ] )
        {
            USHORT n = pChan[ i ]->Close();
            delete pChan[ i ];
            pChan[ i ] = NULL;
            if( n && !nError )
                nError = n;
        }
    }
}

/***************************************************************************
*
*   Console Support
*
***************************************************************************/

// Einlesen einer Zeile von der Console

void SbiIoSystem::ReadCon( ByteString& rIn )
{
    String aPromptStr( aPrompt, gsl_getSystemTextEncoding() );
    SbiInputDialog aDlg( NULL, aPromptStr );
    if( aDlg.Execute() )
        rIn = ByteString( aDlg.GetInput(), gsl_getSystemTextEncoding() );
    else
        nError = SbERR_USER_ABORT;
    aPrompt.Erase();
}

// Ausgabe einer MessageBox, wenn im Console-Puffer ein CR ist

void SbiIoSystem::WriteCon( const ByteString& rText )
{
    aOut += rText;
    USHORT n1 = aOut.Search( '\n' );
    USHORT n2 = aOut.Search( '\r' );
    if( n1 != STRING_NOTFOUND || n2 != STRING_NOTFOUND )
    {
        if( n1 == STRING_NOTFOUND ) n1 = n2;
        else
        if( n2 == STRING_NOTFOUND ) n2 = n1;
        if( n1 > n2 ) n1 = n2;
        ByteString s( aOut.Copy( 0, n1 ) );
        aOut.Erase( 0, n1 );
        while( aOut.GetBuffer()[0] == '\n' || aOut.GetBuffer()[0] == '\r' )
            aOut.Erase( 0, 1 );
        String aStr( s, RTL_TEXTENCODING_ASCII_US );
        if( !MessBox( GetpApp()->GetDefModalDialogParent(),
                      WinBits( WB_OK_CANCEL | WB_DEF_OK ),
                      String(), aStr ).Execute() )
            nError = SbERR_USER_ABORT;
    }
}

