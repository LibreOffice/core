/*************************************************************************
 *
 *  $RCSfile: inetstrm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:08 $
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

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_CHAR_H_
#include <rtl/char.h>
#endif
#ifndef _RTL_MEMORY_H_
#include <rtl/memory.h>
#endif

#ifndef _CACHESTR_HXX
#include <cachestr.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <debug.hxx>
#endif
#ifndef _TOOLS_INETMSG_HXX
#include <inetmsg.hxx>
#endif
#ifndef _TOOLS_INETSTRM_HXX
#include <inetstrm.hxx>
#endif
#ifndef _ZCODEC_HXX
#include <zcodec.hxx>
#endif

#include <ctype.h> // toupper

#define CONSTASCII_STRINGPARAM(a) (a), RTL_TEXTENCODING_ASCII_US

/*=======================================================================
 *
 * INetMessageEncodeQPStream Interface.
 * (Quoted-Printable Encoding)
 *
 *=====================================================================*/
class INetMessageEncodeQPStream_Impl : public INetMessageIStream
{
    SvStream               *pMsgStrm;

    ULONG                   nMsgBufSiz;
    sal_Char               *pMsgBuffer;
    sal_Char               *pMsgRead;
    sal_Char               *pMsgWrite;

    ULONG                   nTokBufSiz;
    sal_Char               *pTokBuffer;
    sal_Char               *pTokRead;
    sal_Char               *pTokWrite;

    INetMessageStreamState  eState;
    BOOL                    bDone;

    virtual int GetMsgLine (
        sal_Char *pData, ULONG nSize, void *pCtx = NULL);

public:
    INetMessageEncodeQPStream_Impl (ULONG nMsgBufferSize = 1024);
    virtual ~INetMessageEncodeQPStream_Impl (void);
};

/*=====================================================================
 *
 * INetMessageDecodeQPStream Interface.
 * (Quoted-Printable Decoding)
 *
 *====================================================================*/
class INetMessageDecodeQPStream_Impl : public INetMessageOStream
{
    INetMessageStreamState  eState;
    SvMemoryStream         *pMsgBuffer;

    ULONG                   nTokBufLen;
    sal_Char                pTokBuffer[4];

    virtual int PutMsgLine (
        const sal_Char *pData, ULONG nSize, void *pCtx = NULL);

public:
    INetMessageDecodeQPStream_Impl (void);
    virtual ~INetMessageDecodeQPStream_Impl (void);
};

/*======================================================================
 *
 * INetMessageEncode64Stream Interface.
 * (Base64 Encoding)
 *
 *====================================================================*/
class INetMessageEncode64Stream_Impl : public INetMessageIStream
{
    SvStream  *pMsgStrm;

    ULONG      nMsgBufSiz;
    sal_uInt8 *pMsgBuffer;
    sal_uInt8 *pMsgRead;
    sal_uInt8 *pMsgWrite;

    ULONG      nTokBufSiz;
    sal_Char  *pTokBuffer;
    sal_Char  *pTokRead;
    sal_Char  *pTokWrite;

    BOOL       bDone;

    virtual int GetMsgLine (
        sal_Char *pData, ULONG nSize, void *pCtx = NULL);

public:
    INetMessageEncode64Stream_Impl (ULONG nMsgBufferSize = 2048);
    virtual ~INetMessageEncode64Stream_Impl (void);
};

/*======================================================================
 *
 * INetMessageDecode64Stream Interface.
 * (Base64 Decoding)
 *
 *====================================================================*/
class INetMessageDecode64Stream_Impl : public INetMessageOStream
{
    INetMessageStreamState  eState;

    ULONG                   nMsgBufSiz;
    sal_Char               *pMsgBuffer;
    sal_Char               *pMsgRead;
    sal_Char               *pMsgWrite;

    virtual int PutMsgLine (
        const sal_Char *pData, ULONG nSize, void *pCtx = NULL);

public:
    INetMessageDecode64Stream_Impl (ULONG nMsgBufferSize = 128);
    virtual ~INetMessageDecode64Stream_Impl (void);
};

/*=========================================================================
 *
 * INetIStream Implementation.
 *
 *=======================================================================*/
/*
 * INetIStream.
 */
INetIStream::INetIStream (ULONG nIBufferSize)
{
}

/*
 * ~INetIStream.
 */
INetIStream::~INetIStream (void)
{
}

/*
 * Read.
 */
int INetIStream::Read (sal_Char *pData, ULONG nSize, void *pCtx)
{
    return GetData (pData, nSize, pCtx);
}

/*
 * Decode64.
 */
void INetIStream::Decode64 (SvStream& rIn, SvStream& rOut)
{
    INetMessage aMsg;
    aMsg.SetDocumentLB(new SvAsyncLockBytes(&rOut, FALSE));

    INetMessageDecode64Stream_Impl aStream (8192);
    aStream.SetTargetMessage (&aMsg);

    sal_Char* pBuf = new sal_Char[8192];

    int nRead = 0;
    while ((nRead = rIn.Read (pBuf, 8192)) > 0)
        aStream.Write( pBuf, nRead );
    aStream.Write ("\r\n", 2);

    delete[] pBuf;
}

/*
 * Encode64.
 */
void INetIStream::Encode64 (SvStream& rIn, SvStream& rOut)
{
    INetMessage aMsg;
    aMsg.SetDocumentLB (
        new SvLockBytes (&rIn, FALSE));

    INetMessageEncode64Stream_Impl aStream (8192);
    aStream.SetSourceMessage (&aMsg);

    sal_Char* pBuf = new sal_Char[8192];

    int nRead = 0;
    while ((nRead = aStream.Read (pBuf, 8192)) > 0)
        rOut.Write( pBuf, nRead );

    delete[] pBuf;
}

/*=========================================================================
 *
 * INetOStream Implementation.
 *
 *=======================================================================*/
/*
 * INetOStream.
 */
INetOStream::INetOStream (ULONG nOBufferSize)
{
}

/*
 * ~INetOStream.
 */
INetOStream::~INetOStream (void)
{
}

/*
 * Write.
 */
int INetOStream::Write (const sal_Char *pData, ULONG nSize, void *pCtx)
{
    return PutData (pData, nSize, pCtx);
}

/*=========================================================================
 *
 * INetMessageIStream Implementation.
 *
 *=======================================================================*/
/*
 * INetMessageIStream.
 */
INetMessageIStream::INetMessageIStream (ULONG nBufferSize)
    : INetIStream (0),
      pSourceMsg       (NULL),
      bHeaderGenerated (FALSE),
      nBufSiz          (nBufferSize),
      pMsgStrm         (NULL),
      pMsgBuffer       (new SvMemoryStream)
{
    pMsgBuffer->SetStreamCharSet (RTL_TEXTENCODING_ASCII_US);
    pBuffer = new sal_Char[nBufSiz];
    pRead = pWrite = pBuffer;
}

/*
 * ~INetMessageIStream.
 */
INetMessageIStream::~INetMessageIStream (void)
{
    delete [] pBuffer;
    delete pMsgBuffer;
    delete pMsgStrm;
}

/*
 * GetData.
 */
int INetMessageIStream::GetData (sal_Char *pData, ULONG nSize, void *pCtx)
{
    if (pSourceMsg == NULL) return INETSTREAM_STATUS_ERROR;

    sal_Char *pWBuf = pData;
    sal_Char *pWEnd = pData + nSize;

    while (pWBuf < pWEnd)
    {
        // Caller's buffer not yet filled.
        ULONG n = pRead - pWrite;
        if (n > 0)
        {
            // Bytes still in buffer.
            ULONG m = pWEnd - pWBuf;
            if (m < n) n = m;
            for (ULONG i = 0; i < n; i++) *pWBuf++ = *pWrite++;
        }
        else
        {
            // Buffer empty. Reset to <Begin-of-Buffer>.
            pRead = pWrite = pBuffer;

            // Read next message line.
            int nRead = GetMsgLine (pBuffer, nBufSiz, pCtx);
            if (nRead > 0)
            {
                // Set read pointer.
                pRead = pBuffer + nRead;
            }
            else
            {
                if (!bHeaderGenerated)
                {
                    // Header generated. Insert empty line.
                    bHeaderGenerated = TRUE;
                    *pRead++ = '\r';
                    *pRead++ = '\n';
                }
                else
                {
                    // Body generated.
                    return (pWBuf - pData);
                }
            }
        }
    }
    return (pWBuf - pData);
}

/*
 * GetMsgLine.
 */
int INetMessageIStream::GetMsgLine (sal_Char *pData, ULONG nSize, void *pCtx)
{
    if (pSourceMsg == NULL) return INETSTREAM_STATUS_ERROR;

    sal_Char *pWBuf = pData;
    sal_Char *pWEnd = pData + nSize;

    if (!bHeaderGenerated)
    {
        ULONG i, n;

        if (pMsgBuffer->Tell() == 0)
        {
            // Insert formatted header into buffer.
            n = pSourceMsg->GetHeaderCount();
            for (i = 0; i < n; i++)
            {
                INetMessageHeader aHeader (pSourceMsg->GetHeaderField(i));
                if (aHeader.GetValue().Len())
                {
                    // NYI: Folding long lines.
                    *pMsgBuffer << (sal_Char*)(aHeader.GetName().GetBuffer());
                    *pMsgBuffer << ": ";
                    *pMsgBuffer << (sal_Char*)(aHeader.GetValue().GetBuffer());
                    *pMsgBuffer << "\r\n";
                }
            }

            pMsgWrite = (sal_Char *)(pMsgBuffer->GetData());
            pMsgRead  = pMsgWrite + pMsgBuffer->Tell();
        }

        n = pMsgRead - pMsgWrite;
        if (n > 0)
        {
            // Move to caller.
            if (nSize < n) n = nSize;
            for (i = 0; i < n; i++) *pWBuf++ = *pMsgWrite++;
        }
        else
        {
            // Reset buffer.
            pMsgBuffer->Seek (STREAM_SEEK_TO_BEGIN);
        }
    }
    else
    {
        if (pSourceMsg->GetDocumentLB())
        {
            if (pMsgStrm == NULL)
                pMsgStrm = new SvStream (pSourceMsg->GetDocumentLB());

            ULONG nRead = pMsgStrm->Read (pWBuf, (pWEnd - pWBuf));
            pWBuf += nRead;
        }
    }
    return (pWBuf - pData);
}

/*=========================================================================
 *
 * INetMessageOStream Implementation.
 *
 *=======================================================================*/
/*
 * INetMessageOStream.
 */
INetMessageOStream::INetMessageOStream (void)
    : INetOStream (0),
      pTargetMsg    (NULL),
      bHeaderParsed (FALSE),
      eOState       (INETMSG_EOL_BEGIN),
      pMsgBuffer    (new SvMemoryStream)
{
}

/*
 * ~INetMessageOStream.
 */
INetMessageOStream::~INetMessageOStream (void)
{
    if (pMsgBuffer->Tell() > 0)
        PutMsgLine ((const sal_Char *) pMsgBuffer->GetData(), pMsgBuffer->Tell());
    delete pMsgBuffer;

    if (pTargetMsg)
    {
        SvOpenLockBytes *pLB =
            PTR_CAST (SvOpenLockBytes, pTargetMsg->GetDocumentLB());
        if (pLB)
        {
            pLB->Flush();
            pLB->Terminate();
        }
    }
}

/*
 * PutData.
 * (Simple Field Parsing (RFC822, Appendix B)).
 */
int INetMessageOStream::PutData (
    const sal_Char *pData, ULONG nSize, void *pCtx)
{
    if (pTargetMsg == NULL) return INETSTREAM_STATUS_ERROR;

    const sal_Char *pStop = (pData + nSize);

    while (!bHeaderParsed && (pData < pStop))
    {
        if (eOState == INETMSG_EOL_BEGIN)
        {
            if ((*pData == '\r') || (*pData == '\n'))
            {
                /*
                 * Empty Line. Separates header fields from message body.
                 * Skip this and any 2nd line break character (if any).
                 */
                pData++;
                if ((pData < pStop) && ((*pData == '\r') || (*pData == '\n')))
                    pData++;

                // Emit any buffered last header field.
                if (pMsgBuffer->Tell() > 0)
                {
                    *pMsgBuffer << '\0';
                    int status = PutMsgLine (
                        (const sal_Char *) pMsgBuffer->GetData(), pMsgBuffer->Tell(), pCtx);
                    if (status != INETSTREAM_STATUS_OK) return status;
                }

                // Reset to begin.
                eOState = INETMSG_EOL_BEGIN;
                pMsgBuffer->Seek (STREAM_SEEK_TO_BEGIN);

                // Mark header parsed.
                bHeaderParsed = TRUE;
            }
            else if ((*pData == ' ') || (*pData == '\t'))
            {
                // Continuation line. Unfold multi-line field-body.
                *pMsgBuffer << ' ';
                pData++;
            }
            else
            {
                // Begin of new header field.
                if (pMsgBuffer->Tell() > 0)
                {
                    // Emit buffered header field now.
                    *pMsgBuffer << '\0';
                    int status = PutMsgLine (
                        (const sal_Char *) pMsgBuffer->GetData(), pMsgBuffer->Tell(), pCtx);
                    if (status != INETSTREAM_STATUS_OK) return status;
                }

                // Reset to begin of buffer.
                pMsgBuffer->Seek (STREAM_SEEK_TO_BEGIN);

                // Insert current character into buffer.
                *pMsgBuffer << *pData++;
            }

            // Search for next line break character.
            if (!bHeaderParsed) eOState = INETMSG_EOL_SCR;
        }
        else if (eOState == INETMSG_EOL_FCR)
        {
            // Skip line break character.
            pData++;

            // Mark begin of line.
            eOState = INETMSG_EOL_BEGIN;
        }
        else if ((*pData == '\r') || (*pData == '\n'))
        {
            if (*pData == '\r') pData++;
            eOState = INETMSG_EOL_FCR;
        }
        else if (rtl_char_isWhitespace (*pData & 0x7f))
        {
            // Any <LWS> is folded into a single <SP> character.
            sal_Char c = *((const sal_Char *) pMsgBuffer->GetData() + pMsgBuffer->Tell() - 1);
            if (!rtl_char_isWhitespace (c & 0x7f)) *pMsgBuffer << ' ';

            // Skip over this <LWS> character.
            pData++;
        }
        else
        {
            // Any other character is inserted into line buffer.
            *pMsgBuffer << *pData++;
        }
    }

    if (bHeaderParsed && (pData < pStop))
    {
        // Put message body down-stream.
        return PutMsgLine (pData, (pStop - pData), pCtx);
    }

    return INETSTREAM_STATUS_OK;
}

/*
 * PutMsgLine.
 */
int INetMessageOStream::PutMsgLine (
    const sal_Char *pData, ULONG nSize, void *pCtx)
{
    // Check for message container.
    if (pTargetMsg == NULL) return INETSTREAM_STATUS_ERROR;

    // Check for header or body.
    if (!IsHeaderParsed())
    {
        ByteString aField (pData);
        USHORT nPos = aField.Search (':');
        if (nPos != STRING_NOTFOUND)
        {
            ByteString aName (
                aField.Copy (0, nPos));
            ByteString aValue (
                aField.Copy (nPos + 1, aField.Len() - nPos + 1));
            aValue.EraseLeadingChars (' ');

            pTargetMsg->SetHeaderField (
                INetMessageHeader (aName, aValue));
        }
    }
    else
    {
        SvOpenLockBytes *pLB =
            PTR_CAST(SvOpenLockBytes, pTargetMsg->GetDocumentLB());
        if (pLB == NULL)
            return INETSTREAM_STATUS_WOULDBLOCK;

        ULONG nDocSiz = pTargetMsg->GetDocumentSize();
        ULONG nWrite  = 0;

        ErrCode status = pLB->FillAppend ((sal_Char *)pData, nSize, &nWrite);
        pTargetMsg->SetDocumentSize (nDocSiz + nWrite);

        if (nWrite < nSize) return INETSTREAM_STATUS_ERROR;
    }
    return INETSTREAM_STATUS_OK;
}

/*=========================================================================
 *
 * INetMessageIOStream Implementation.
 *
 *=======================================================================*/
/*
 * INetMessageIOStream.
 */
INetMessageIOStream::INetMessageIOStream (ULONG nBufferSize)
    : INetMessageIStream (nBufferSize),
      INetMessageOStream ()
{
}

/*
 * ~INetMessageIOStream.
 */
INetMessageIOStream::~INetMessageIOStream (void)
{
}

/*=======================================================================
 *
 * INetMessageEncodeQPStream_Impl Implementation.
 * (Quoted-Printable Encoding)
 *
 *=====================================================================*/
static const sal_Char hex2pr[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F'
};

static const sal_Char ebcdic[] = {
    '!', '"', '#', '$', '@', '[', '\\', ']', '^', '`', '{', '|', '}', '~'
};

/*
 * INetMessageEncodeQPStream_Impl.
 */
INetMessageEncodeQPStream_Impl::INetMessageEncodeQPStream_Impl (
    ULONG nMsgBufferSize)
    : INetMessageIStream (),
      pMsgStrm   (NULL),
      nMsgBufSiz (nMsgBufferSize),
      nTokBufSiz (80),
      eState     (INETMSG_EOL_SCR),
      bDone      (FALSE)
{
    GenerateHeader (FALSE);

    pMsgBuffer = new sal_Char[nMsgBufSiz];
    pMsgRead = pMsgWrite = pMsgBuffer;

    pTokBuffer = new sal_Char[nTokBufSiz];
    pTokRead = pTokWrite = pTokBuffer;
}

/*
 * ~INetMessageEncodeQPStream_Impl.
 */
INetMessageEncodeQPStream_Impl::~INetMessageEncodeQPStream_Impl (void)
{
    delete pMsgStrm;
    delete [] pMsgBuffer;
    delete [] pTokBuffer;
}

/*
 * GetMsgLine.
 */
int INetMessageEncodeQPStream_Impl::GetMsgLine (
    sal_Char *pData, ULONG nSize, void *pCtx)
{
    INetMessage *pMsg = GetSourceMessage ();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    if (pMsg->GetDocumentLB() == NULL) return 0;
    if (pMsgStrm == NULL) pMsgStrm = new SvStream (pMsg->GetDocumentLB());

    sal_Char *pWBuf = pData;
    while (pWBuf < (pData + nSize))
    {
        // Caller's buffer not yet filled.
        if ((pMsgRead - pMsgWrite) > 0)
        {
            // Bytes still in message buffer.
            if ((eState != INETMSG_EOL_BEGIN) &&
                ((pTokRead - pTokBuffer) < 72))
            {
                // Token buffer not yet filled.
                if (eState == INETMSG_EOL_FCR)
                {
                    eState = INETMSG_EOL_BEGIN;
                    if (*pMsgWrite != '\n')
                    {
                        // Convert orphant <CR> into <CR><LF> sequence.
                        *pTokRead++ = '\n';
                    }
                    *pTokRead++ = *pMsgWrite++;
                }
                else if ((*pMsgWrite == ' ') || (*pMsgWrite == '\t'))
                {
                    eState = INETMSG_EOL_FSP;
                    *pTokRead++ = *pMsgWrite++;
                }
                else if (*pMsgWrite == '\r')
                {
                    // Found <CR>.
                    if (eState == INETMSG_EOL_FSP)
                    {
                        // Encode last (trailing space) character.
                        sal_uInt8 c = (sal_uInt8)(*(--pTokRead));
                        *pTokRead++ = '=';
                        *pTokRead++ = hex2pr[((c & 0xf0) >> 4)];
                        *pTokRead++ = hex2pr[((c & 0x0f)     )];
                    }
                    eState = INETMSG_EOL_FCR;
                    *pTokRead++ = *pMsgWrite++;
                }
                else if (*pMsgWrite == '\n')
                {
                    // Found <LF> only.
                    if (eState == INETMSG_EOL_FSP)
                    {
                        // Encode last (trailing space) character.
                        sal_uInt8 c = (sal_uInt8)(*(--pTokRead));
                        *pTokRead++ = '=';
                        *pTokRead++ = hex2pr[((c & 0xf0) >> 4)];
                        *pTokRead++ = hex2pr[((c & 0x0f)     )];
                    }
                    eState = INETMSG_EOL_BEGIN;

                    // Convert orphant <LF> into <CR><LF> sequence.
                    *pTokRead++ = '\r';
                    *pTokRead++ = *pMsgWrite++;
                }
                else if (*pMsgWrite == '=')
                {
                    // Escape character itself MUST be encoded, of course.
                    sal_uInt8 c = (sal_uInt8)(*pMsgWrite++);
                    *pTokRead++ = '=';
                    *pTokRead++ = hex2pr[((c & 0xf0) >> 4)];
                    *pTokRead++ = hex2pr[((c & 0x0f)     )];

                    eState = INETMSG_EOL_SCR;
                }
                else if (((sal_uInt8)(*pMsgWrite) > 0x20) &&
                         ((sal_uInt8)(*pMsgWrite) < 0x7f)    )
                {
                    /*
                     * Some printable ASCII character.
                     * (Encode EBCDIC special characters (NYI)).
                     */
                    *pTokRead++ = *pMsgWrite++;
                    eState = INETMSG_EOL_SCR;
                }
                else
                {
                    // Encode any other character.
                    sal_uInt8 c = (sal_uInt8)(*pMsgWrite++);
                    *pTokRead++ = '=';
                    *pTokRead++ = hex2pr[((c & 0xf0) >> 4)];
                    *pTokRead++ = hex2pr[((c & 0x0f)     )];

                    eState = INETMSG_EOL_SCR;
                }
            }
            else
            {
                // Check for maximum line length.
                if (eState != INETMSG_EOL_BEGIN)
                {
                    // Insert soft line break.
                    *pTokRead++ = '=';
                    *pTokRead++ = '\r';
                    *pTokRead++ = '\n';

                    eState = INETMSG_EOL_BEGIN;
                }

                // Copy to caller's buffer.
                if ((pTokRead - pTokWrite) > 0)
                {
                    // Bytes still in token buffer.
                    *pWBuf++ = *pTokWrite++;
                }
                else
                {
                    // Token buffer empty. Reset to <Begin-of-Buffer>.
                    pTokRead = pTokWrite = pTokBuffer;
                    eState = INETMSG_EOL_SCR;
                }
            }
        }
        else
        {
            // Message buffer empty. Reset to <Begin-of-Buffer>.
            pMsgRead = pMsgWrite = pMsgBuffer;

            // Read next message block.
            ULONG nRead = pMsgStrm->Read (pMsgBuffer, nMsgBufSiz);
            if (nRead > 0)
            {
                // Set read pointer.
                pMsgRead = (pMsgBuffer + nRead);
            }
            else
            {
                // Nothing more ro read.
                if (!bDone)
                {
                    // Append final <CR><LF> and mark we're done.
                    *pTokRead++ = '\r';
                    *pTokRead++ = '\n';

                    bDone = TRUE;
                }
                else
                {
                    // Already done all encoding.
                    if ((pTokRead - pTokWrite) > 0)
                    {
                        // Bytes still in token buffer.
                        *pWBuf++ = *pTokWrite++;
                    }
                    else
                    {
                        // Token buffer empty. Reset to <Begin-of-Buffer>.
                        pTokRead = pTokWrite = pTokBuffer;

                        // Return.
                        return (pWBuf - pData);
                    }
                }
            }
        }
    }
    return (pWBuf - pData);
}

/*=====================================================================
 *
 * INetMessageDecodeQPStream_Impl Implementation.
 * (Quoted-Printable Decoding)
 *
 *====================================================================*/
static const sal_uInt8 pr2hex[128] = {
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,

    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,

    0x10, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,

    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
    0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10
};

/*
 * INetMessageDecodeQPStream_Impl.
 */
INetMessageDecodeQPStream_Impl::INetMessageDecodeQPStream_Impl (void)
    : INetMessageOStream (),
      eState     (INETMSG_EOL_BEGIN),
      pMsgBuffer (new SvMemoryStream),
      nTokBufLen (0)
{
    ParseHeader (FALSE);
}

/*
 * ~INetMessageDecodeQPStream_Impl.
 */
INetMessageDecodeQPStream_Impl::~INetMessageDecodeQPStream_Impl (void)
{
    delete pMsgBuffer;
}

/*
 * PutMsgLine.
 */
int INetMessageDecodeQPStream_Impl::PutMsgLine (
    const sal_Char *pData, ULONG nSize, void *pCtx)
{
    INetMessage *pMsg = GetTargetMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    SvOpenLockBytes * pLB = PTR_CAST(SvOpenLockBytes, pMsg->GetDocumentLB());
    if (pLB == NULL) return INETSTREAM_STATUS_WOULDBLOCK;

    const sal_Char *pStop = pData + nSize;
    while (pData < pStop)
    {
        if (eState == INETMSG_EOL_FESC)
        {
            *(pTokBuffer + nTokBufLen++) = toupper (*pData);
            pData++;
            if (nTokBufLen == 2)
            {
                if ((*pTokBuffer == '\r') || (*pTokBuffer == '\n'))
                {
                    // Soft line break (=<CR><LF>). Emit buffer now.
                    eState = INETMSG_EOL_BEGIN;
                }
                else
                {
                    // Decode token.
                    *pMsgBuffer << sal_uInt8 (
                        (pr2hex[(int)(pTokBuffer[0] & 0x7f)] << 4) |
                        (pr2hex[(int)(pTokBuffer[1] & 0x7f)] & 15)   );

                    // Search for next <CR>.
                    eState = INETMSG_EOL_SCR;
                }

                // Reset token buffer.
                nTokBufLen = 0;
            }
        }
        else if (*pData == '=')
        {
            // Found escape character.
            pData++;
            eState = INETMSG_EOL_FESC;
        }
        else if (eState == INETMSG_EOL_FCR)
        {
            *pMsgBuffer << *pData++;
            eState = INETMSG_EOL_BEGIN;
        }
        else if (*pData == '\r')
        {
            *pMsgBuffer << *pData++;
            eState = INETMSG_EOL_FCR;
        }
        else
        {
            *pMsgBuffer << *pData++;
        }

        if (eState == INETMSG_EOL_BEGIN)
        {
            ULONG nRead = pMsgBuffer->Tell();
            if (nRead > 0)
            {
                // Emit buffer.
                ULONG nDocSiz = pMsg->GetDocumentSize();
                ULONG nWrite  = 0;

                pLB->FillAppend (
                    (sal_Char *)(pMsgBuffer->GetData()), nRead, &nWrite);
                pMsg->SetDocumentSize (nDocSiz + nWrite);

                if (nWrite < nRead) return INETSTREAM_STATUS_ERROR;

                pMsgBuffer->Seek (STREAM_SEEK_TO_BEGIN);
            }
            eState = INETMSG_EOL_SCR;
        }
    }
    return INETSTREAM_STATUS_OK;
}

/*======================================================================
 *
 * INetMessageEncode64Stream_Impl Implementation.
 * (Base64 Encoding)
 *
 *====================================================================*/
static const sal_Char six2pr[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

/*
 * INetMessageEncode64Stream_Impl.
 */
INetMessageEncode64Stream_Impl::INetMessageEncode64Stream_Impl (
    ULONG nMsgBufferSize)
    : INetMessageIStream (),
      pMsgStrm   (NULL),
      nMsgBufSiz (nMsgBufferSize),
      nTokBufSiz (80),
      bDone      (FALSE)
{
    GenerateHeader (FALSE);

    pMsgBuffer = new sal_uInt8[nMsgBufSiz];
    pMsgRead = pMsgWrite = pMsgBuffer;

    pTokBuffer = new sal_Char[nTokBufSiz];
    pTokRead = pTokWrite = pTokBuffer;
}

/*
 * ~INetMessageEncode64Stream_Impl.
 */
INetMessageEncode64Stream_Impl::~INetMessageEncode64Stream_Impl (void)
{
    delete pMsgStrm;
    delete [] pMsgBuffer;
    delete [] pTokBuffer;
}

/*
 * GetMsgLine.
 */
int INetMessageEncode64Stream_Impl::GetMsgLine (
    sal_Char *pData, ULONG nSize, void *pCtx)
{
    INetMessage *pMsg = GetSourceMessage ();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    if (pMsg->GetDocumentLB() == NULL) return 0;
    if (pMsgStrm == NULL) pMsgStrm = new SvStream (pMsg->GetDocumentLB());

    sal_Char *pWBuf = pData;
    while (pWBuf < (pData + nSize))
    {
        // Caller's buffer not yet filled.
        if ((pMsgRead - pMsgWrite) > 0)
        {
            // Bytes still in message buffer.
            if ((pTokRead - pTokBuffer) < 72)
            {
                // Token buffer not yet filled.
                switch ((pTokRead - pTokBuffer) % 4)
                {
                    case 0:
                        *pTokRead++ = six2pr[(int)(*pMsgWrite >> 2)];
                        break;

                    case 1:
                        *pTokRead++ = six2pr[
                            (int)(((*pMsgWrite << 4) & 060) |
                                  (((*(pMsgWrite + 1)) >> 4) & 017))];
                        pMsgWrite++;
                        break;

                    case 2:
                        *pTokRead++ = six2pr[
                            (int)(((*pMsgWrite << 2) & 074) |
                                  (((*(pMsgWrite + 1)) >> 6) & 003))];
                        pMsgWrite++;
                        break;

                    default: // == case 3
                        *pTokRead++ = six2pr[(int)(*pMsgWrite & 077)];
                        pMsgWrite++;
                        break;
                }
            }
            else if ((pTokRead - pTokBuffer) == 72)
            {
                // Maximum line length. Append <CR><LF>.
                *pTokRead++ = '\r';
                *pTokRead++ = '\n';
            }
            else
            {
                if ((pTokRead - pTokWrite) > 0)
                {
                    // Bytes still in token buffer.
                    *pWBuf++ = *pTokWrite++;
                }
                else
                {
                    // Token buffer empty. Reset to <Begin-of-Buffer>.
                    pTokRead = pTokWrite = pTokBuffer;
                }
            }
        }
        else
        {
            // Message buffer empty. Reset to <Begin-of-Buffer>.
            pMsgRead = pMsgWrite = pMsgBuffer;

            // Read next message block.
            ULONG nRead = pMsgStrm->Read (pMsgBuffer, nMsgBufSiz);
            if (nRead > 0)
            {
                // Set read pointer.
                pMsgRead = (pMsgBuffer + nRead);
            }
            else
            {
                // Nothing more to read.
                if (!bDone)
                {
                    // Append pad character(s) and final <CR><LF>.
                    switch ((pTokRead - pTokBuffer) % 4)
                    {
                        case 2:
                            *pTokRead++ = '=';
                            // Fall through for 2nd pad character.

                        case 3:
                            *pTokRead++ = '=';
                            break;

                        default:
                            break;
                    }
                    *pTokRead++ = '\r';
                    *pTokRead++ = '\n';

                    // Mark we're done.
                    bDone = TRUE;
                }
                else
                {
                    // Already done all encoding.
                    if ((pTokRead - pTokWrite) > 0)
                    {
                        // Bytes still in token buffer.
                        *pWBuf++ = *pTokWrite++;
                    }
                    else
                    {
                        // Token buffer empty. Reset to <Begin-of-Buffer>.
                        pTokRead = pTokWrite = pTokBuffer;

                        // Reset done flag, if everything has been done.
                        // if (pWBuf == pData) bDone = FALSE;

                        // Return.
                        return (pWBuf - pData);
                    }
                }
            }
        }
    } // while (pWBuf < (pData + nSize))
    return (pWBuf - pData);
}

/*======================================================================
 *
 * INetMessageDecode64Stream_Impl Implementation.
 * (Base64 Decoding)
 *
 *====================================================================*/
static const sal_uInt8 pr2six[256] = {
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,

    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x3E, 0x40, 0x40, 0x40, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,

    0x40, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0x40, 0x40, 0x40, 0x40, 0x40,

    0x40, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0x40, 0x40, 0x40, 0x40, 0x40,

    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,

    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,

    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,

    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40
};

/*
 * INetMessageDecode64Stream_Impl.
 */
INetMessageDecode64Stream_Impl::INetMessageDecode64Stream_Impl (
    ULONG nMsgBufferSize)
    : INetMessageOStream (),
      eState     (INETMSG_EOL_SCR),
      nMsgBufSiz (nMsgBufferSize)
{
    ParseHeader (FALSE);

    pMsgBuffer = new sal_Char[nMsgBufSiz];
    pMsgRead = pMsgWrite = pMsgBuffer;
}

/*
 * ~INetMessageDecode64Stream_Impl.
 */
INetMessageDecode64Stream_Impl::~INetMessageDecode64Stream_Impl (void)
{
    delete [] pMsgBuffer;
}

/*
 * PutMsgLine.
 */
int INetMessageDecode64Stream_Impl::PutMsgLine (
    const sal_Char *pData, ULONG nSize, void *pCtx)
{
    INetMessage *pMsg = GetTargetMessage ();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    SvOpenLockBytes * pLB = PTR_CAST(SvOpenLockBytes, pMsg->GetDocumentLB());
    if (pLB == NULL) return INETSTREAM_STATUS_WOULDBLOCK;

    const sal_Char *pStop = (pData + nSize);
    while (pData < pStop)
    {
        if (pr2six[(int)(*pData)] > 63)
        {
            /*
             * Character not in base64 alphabet.
             * Check for <End-of-Stream> or Junk.
             */
            if (*pData == '=')
            {
                // Final pad character -> Done.
                ULONG nDocSiz = pMsg->GetDocumentSize();
                ULONG nRead   = pMsgWrite - pMsgBuffer;
                ULONG nWrite  = 0;

                pLB->FillAppend (pMsgBuffer, nRead, &nWrite);
                pMsg->SetDocumentSize (nDocSiz + nWrite);

                if (nWrite < nRead)
                    return INETSTREAM_STATUS_ERROR;
                else
                    return INETSTREAM_STATUS_LOADED;
            }
            else if (eState == INETMSG_EOL_FCR)
            {
                // Skip any line break character.
                if ((*pData == '\r') || (*pData == '\n')) pData++;

                // Store decoded message buffer contents.
                ULONG nDocSiz = pMsg->GetDocumentSize();
                ULONG nRead   = pMsgWrite - pMsgBuffer;
                ULONG nWrite  = 0;

                pLB->FillAppend (pMsgBuffer, nRead, &nWrite);
                pMsg->SetDocumentSize (nDocSiz + nWrite);

                if (nWrite < nRead) return INETSTREAM_STATUS_ERROR;

                // Reset to <Begin-of-Buffer>.
                pMsgWrite = pMsgBuffer;
                eState = INETMSG_EOL_SCR;
            }
            else if ((*pData == '\r') || (*pData == '\n'))
            {
                // Skip any line break character.
                pData++;
                eState = INETMSG_EOL_FCR;
            }
            else
            {
                // Skip any junk character (may be transmission error).
                pData++;
            }
        }
        else
        {
            // Decode any other character into message buffer.
            switch ((pMsgRead - pMsgBuffer) % 4)
            {
                case 0:
                    *pMsgWrite    = (pr2six[(int)(*pData++)] << 2);
                    pMsgRead++;
                    break;

                case 1:
                    *pMsgWrite++ |= (pr2six[(int)(*pData  )] >> 4);
                    *pMsgWrite    = (pr2six[(int)(*pData++)] << 4);
                    pMsgRead++;
                    break;

                case 2:
                    *pMsgWrite++ |= (pr2six[(int)(*pData  )] >> 2);
                    *pMsgWrite    = (pr2six[(int)(*pData++)] << 6);
                    pMsgRead++;
                    break;

                default: // == case 3
                    *pMsgWrite++ |= (pr2six[(int)(*pData++)]);
                    pMsgRead = pMsgBuffer;
                    break;
            } // switch ((pMsgRead - pMsgBuffer) % 4)
        }
    } // while (pData < pStop)
    return INETSTREAM_STATUS_OK;
}

/*=========================================================================
 *
 * INetMIMEMessageStream Implementation.
 *
 *=======================================================================*/
/*
 * INetMIMEMessageStream.
 */
INetMIMEMessageStream::INetMIMEMessageStream (ULONG nBufferSize)
    : INetMessageIOStream (nBufferSize),
      eState      (INETMSG_EOL_BEGIN),
      nChildIndex (0),
      pChildStrm  (NULL),
      eEncoding   (INETMSG_ENCODING_BINARY),
      pEncodeStrm (NULL),
      pDecodeStrm (NULL),
      pMsgBuffer  (NULL)
{
}

/*
 * ~INetMIMEMessageStream.
 */
INetMIMEMessageStream::~INetMIMEMessageStream (void)
{
    delete pChildStrm;
    delete pEncodeStrm;
    delete pDecodeStrm;
    delete pMsgBuffer;
}

/*
 * GetMsgEncoding.
 */
INetMessageEncoding
INetMIMEMessageStream::GetMsgEncoding (const String& rContentType)
{
    if ((rContentType.CompareIgnoreCaseToAscii ("message"  , 7) == 0) ||
        (rContentType.CompareIgnoreCaseToAscii ("multipart", 9) == 0)    )
        return INETMSG_ENCODING_7BIT;

    if (rContentType.CompareIgnoreCaseToAscii ("text", 4) == 0)
    {
        if (rContentType.CompareIgnoreCaseToAscii ("text/plain", 10) == 0)
        {
            if (rContentType.GetTokenCount ('=') > 1)
            {
                String aCharset (rContentType.GetToken (1, '='));
                aCharset.EraseLeadingChars (' ');
                aCharset.EraseLeadingChars ('"');

                if (aCharset.CompareIgnoreCaseToAscii ("us-ascii", 8) == 0)
                    return INETMSG_ENCODING_7BIT;
                else
                    return INETMSG_ENCODING_QUOTED;
            }
            else
                return INETMSG_ENCODING_7BIT;
        }
        else
            return INETMSG_ENCODING_QUOTED;
    }

    return INETMSG_ENCODING_BASE64;
}

/*
 * GetMsgLine.
 * (Message Generator).
 */
int INetMIMEMessageStream::GetMsgLine (
    sal_Char *pData, ULONG nSize, void *pCtx)
{
    // Check for message container.
    INetMIMEMessage *pMsg = GetSourceMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    // Check for header or body.
    if (!IsHeaderGenerated())
    {
        if (eState == INETMSG_EOL_BEGIN)
        {
            // Prepare special header fields.
            if (pMsg->GetParent())
            {
                String aPCT (pMsg->GetParent()->GetContentType());
                if (aPCT.CompareIgnoreCaseToAscii ("message/rfc822", 14) == 0)
                    pMsg->SetMIMEVersion (
                        String(CONSTASCII_STRINGPARAM("1.0")));
                else
                    pMsg->SetMIMEVersion (String());
            }
            else
            {
                pMsg->SetMIMEVersion (String(CONSTASCII_STRINGPARAM("1.0")));
            }

            // Check ContentType.
            String aContentType (pMsg->GetContentType());
            if (aContentType.Len())
            {
                // Determine default Content-Type.
                String aDefaultType;
                pMsg->GetDefaultContentType (aDefaultType);

                if (aDefaultType.CompareIgnoreCaseToAscii (
                    aContentType, aContentType.Len()) == 0)
                {
                    // No need to specify default.
                    pMsg->SetContentType (String());
                }
            }

            // Check Encoding.
            String aEncoding (pMsg->GetContentTransferEncoding());
            if (aEncoding.Len())
            {
                // Use given Encoding.
                if (aEncoding.CompareIgnoreCaseToAscii (
                    "base64", 6) == 0)
                    eEncoding = INETMSG_ENCODING_BASE64;
                else if (aEncoding.CompareIgnoreCaseToAscii (
                    "quoted-printable", 16) == 0)
                    eEncoding = INETMSG_ENCODING_QUOTED;
                else
                    eEncoding = INETMSG_ENCODING_7BIT;
            }
            else
            {
                // Use default Encoding for (given|default) Content-Type.
                if (aContentType.Len() == 0)
                {
                    // Determine default Content-Type.
                    pMsg->GetDefaultContentType (aContentType);
                }
                eEncoding = GetMsgEncoding (aContentType);
            }

            // Set Content-Transfer-Encoding header.
            if (eEncoding == INETMSG_ENCODING_BASE64)
            {
                // Base64.
                pMsg->SetContentTransferEncoding (
                    String(CONSTASCII_STRINGPARAM("base64")));
            }
            else if (eEncoding == INETMSG_ENCODING_QUOTED)
            {
                // Quoted-Printable.
                pMsg->SetContentTransferEncoding (
                    String(CONSTASCII_STRINGPARAM("quoted-printable")));
            }
            else
            {
                // No need to specify default.
                pMsg->SetContentTransferEncoding (String());
            }

            // Mark we're done.
            eState = INETMSG_EOL_DONE;
        }

        // Generate the message header.
        int nRead = INetMessageIOStream::GetMsgLine (pData, nSize, pCtx);
        if (nRead <= 0)
        {
            // Reset state.
            eState = INETMSG_EOL_BEGIN;
        }
        return nRead;
    }
    else
    {
        // Generate the message body.
        if (pMsg->IsContainer())
        {
            // Encapsulated message body.
            while (eState == INETMSG_EOL_BEGIN)
            {
                if (pChildStrm == NULL)
                {
                    INetMIMEMessage *pChild = pMsg->GetChild (nChildIndex);
                    if (pChild)
                    {
                        // Increment child index.
                        nChildIndex++;

                        // Create child stream.
                        pChildStrm = new INetMIMEMessageStream;
                        pChildStrm->SetSourceMessage (pChild);

                        if (pMsg->IsMultipart())
                        {
                            // Insert multipart delimiter.
                            ByteString aDelim ("--");
                            aDelim += pMsg->GetMultipartBoundary();
                            aDelim += "\r\n";

                            rtl_copyMemory (
                                pData, aDelim.GetBuffer(), aDelim.Len());
                            return aDelim.Len();
                        }
                    }
                    else
                    {
                        // No more parts. Mark we're done.
                        eState = INETMSG_EOL_DONE;
                        nChildIndex = 0;

                        if (pMsg->IsMultipart())
                        {
                            // Insert close delimiter.
                            ByteString aDelim ("--");
                            aDelim += pMsg->GetMultipartBoundary();
                            aDelim += "--\r\n";

                            rtl_copyMemory (
                                pData, aDelim.GetBuffer(), aDelim.Len());
                            return aDelim.Len();
                        }
                    }
                }
                else
                {
                    // Read current child stream.
                    int nRead = pChildStrm->Read (pData, nSize, pCtx);
                    if (nRead > 0)
                    {
                        return nRead;
                    }
                    else
                    {
                        // Cleanup exhausted child stream.
                        delete pChildStrm;
                        pChildStrm = NULL;
                    }
                }
            }
            return 0;
        }
        else
        {
            // Single part message body.
            if (pMsg->GetDocumentLB() == NULL)
            {
                // Empty message body.
                return 0;
            }
            else
            {
                // Check whether message body needs to be encoded.
                if (eEncoding == INETMSG_ENCODING_7BIT)
                {
                    // No Encoding.
                    return INetMessageIOStream::GetMsgLine (
                        pData, nSize, pCtx);
                }
                else
                {
                    // Apply appropriate Encoding.
                    while (eState == INETMSG_EOL_BEGIN)
                    {
                        if (pEncodeStrm == NULL)
                        {
                            // Create encoder stream.
                            if (eEncoding == INETMSG_ENCODING_QUOTED)
                            {
                                // Quoted-Printable Encoding.
                                pEncodeStrm
                                 = new INetMessageEncodeQPStream_Impl;
                            }
                            else
                            {
                                // Base64 Encoding.
                                pEncodeStrm
                                 = new INetMessageEncode64Stream_Impl;
                            }
                            pEncodeStrm->SetSourceMessage (pMsg);
                        }

                        // Read encoded message.
                        int nRead = pEncodeStrm->Read (pData, nSize, pCtx);
                        if (nRead > 0)
                        {
                            return nRead;
                        }
                        else
                        {
                            // Cleanup exhausted encoder stream.
                            delete pEncodeStrm;
                            pEncodeStrm = NULL;

                            // Mark we're done.
                            eState = INETMSG_EOL_DONE;
                        }
                    }
                    return 0;
                }
            }
        }
    }
    return INETSTREAM_STATUS_ERROR; // Never reached.
}

/*
 * PutMsgLine.
 * (Message Parser).
 */
int INetMIMEMessageStream::PutMsgLine (
    const sal_Char *pData, ULONG nSize, void *pCtx)
{
    // Check for message container.
    INetMIMEMessage *pMsg = GetTargetMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    // Check for header or body.
    if (!IsHeaderParsed())
    {
        // Parse the message header.
        int nRet = INetMessageIOStream::PutMsgLine (pData, nSize, pCtx);
        return nRet;
    }
    else
    {
        pMsg->SetHeaderParsed();
        // Parse the message body.
        if (pMsg->IsContainer())
        {

            // Content-Transfer-Encoding MUST be "7bit" (RFC1521).
            if (pMsg->IsMessage())
            {
                if( !pChildStrm )
                {
                    // Encapsulated message.
                    pMsg->SetChildCount( pMsg->GetChildCount() + 1);
                    INetMIMEMessage* pNewMessage = new INetMIMEMessage;
                    pNewMessage->SetDocumentLB (
                        new SvAsyncLockBytes(new SvCacheStream, FALSE));
                    pMsg->AttachChild( *pNewMessage, TRUE );

                    // Encapsulated message body. Create message parser stream.
                    pChildStrm = new INetMIMEMessageStream;
                    pChildStrm->SetTargetMessage ( pNewMessage );

                    // Initialize control variables.
                    eState = INETMSG_EOL_BEGIN;
                }
                if ( nSize > 0)
                {
                    // Bytes still in buffer. Put message down-stream.
                    int status = pChildStrm->Write(
                        pData, nSize, NULL );
                    if (status != INETSTREAM_STATUS_OK)
                        return status;
                }

                return INetMessageIOStream::PutMsgLine (
                    pData, nSize, pCtx);
            }
            else
            {

                // Multipart message body. Initialize multipart delimiters.
                // Multipart message.
                if (pMsg->GetMultipartBoundary().Len() == 0)
                {
                    // Determine boundary.
                    ByteString aType (
                        pMsg->GetContentType(), RTL_TEXTENCODING_ASCII_US);
                    ByteString aLowerType (aType);
                    aLowerType.ToLowerAscii();

                    USHORT nPos = aLowerType.Search ("boundary=");
                    ByteString aBoundary (aType.Copy (nPos + 9));

                    aBoundary.EraseLeadingAndTrailingChars (' ');
                    aBoundary.EraseLeadingAndTrailingChars ('"');

                    // Save boundary.
                    pMsg->SetMultipartBoundary (aBoundary);
                }

                ByteString aPlainDelim (pMsg->GetMultipartBoundary());
                ByteString aDelim ("--");
                aDelim += aPlainDelim;

                ByteString aPlainClose (aPlainDelim);
                aPlainClose += "--";

                ByteString aClose (aDelim);
                aClose += "--";

                if (pMsgBuffer == NULL) pMsgBuffer = new SvMemoryStream;
                pMsgBuffer->Write (pData, nSize);
                ULONG nBufSize = pMsgBuffer->Tell();

                const sal_Char* pChar;
                const sal_Char* pOldPos;
                for( pOldPos = pChar = (const sal_Char *) pMsgBuffer->GetData(); nBufSize--;
                     pChar++ )
                {
                    int status;
                    if( *pChar == '\r' || *pChar == '\n' )
                    {
                        if( aDelim.CompareTo (pOldPos, aDelim.Len())
                            != COMPARE_EQUAL &&
                            aClose.CompareTo (pOldPos, aClose.Len())
                            != COMPARE_EQUAL &&
                            aPlainDelim.CompareTo (pOldPos, aPlainDelim.Len())
                            != COMPARE_EQUAL &&
                            aPlainClose.CompareTo(pOldPos, aPlainClose.Len())
                            != COMPARE_EQUAL )
                        {
                            if( nBufSize &&
                                ( pChar[1] == '\r' || pChar[1] == '\n' ) )
                                nBufSize--, pChar++;
                            if( pChildStrm )
                            {
                                status = pChildStrm->Write(
                                    pOldPos, pChar - pOldPos + 1 );
                                if( status != INETSTREAM_STATUS_OK )
                                    return status;
                            }
                            else
                                DBG_ERRORFILE( "Die Boundary nicht gefunden" );
                            status = INetMessageIOStream::PutMsgLine(
                                pOldPos, pChar - pOldPos + 1, pCtx );
                            if( status != INETSTREAM_STATUS_OK )
                                return status;
                            pOldPos = pChar + 1;
                        }
                        else
                        {
                            if( nBufSize &&
                                ( pChar[1] == '\r' || pChar[1] == '\n' ) )
                                nBufSize--, pChar++;
                            pOldPos = pChar + 1;
                            DELETEZ( pChildStrm );

                            if (aClose.CompareTo (pOldPos, aClose.Len())
                                != COMPARE_EQUAL &&
                                aPlainClose.CompareTo (pOldPos, aClose.Len())
                                != COMPARE_EQUAL )
                            {
                                // Encapsulated message.
                                pMsg->SetChildCount(pMsg->GetChildCount() + 1);
                                INetMIMEMessage* pNewMessage =
                                    new INetMIMEMessage;
                                pNewMessage->SetDocumentLB (
                                    new SvAsyncLockBytes (
                                        new SvCacheStream, FALSE));

                                pMsg->AttachChild( *pNewMessage, TRUE );

                                // Encapsulated message body. Create message parser stream.
                                pChildStrm = new INetMIMEMessageStream;
                                pChildStrm->SetTargetMessage ( pNewMessage );

                                // Initialize control variables.
                            }
                            eState = INETMSG_EOL_BEGIN;
                            status = INetMessageIOStream::PutMsgLine(
                                pOldPos, pChar - pOldPos + 1, pCtx );
                            if( status != INETSTREAM_STATUS_OK )
                                return status;
                        }
                    }
                }
                if( pOldPos < pChar )
                {
                    SvMemoryStream* pNewStream = new SvMemoryStream;
                    pNewStream->Write( pOldPos, pChar - pOldPos );
                    SvMemoryStream* pTmp = pMsgBuffer;
                    pMsgBuffer = pNewStream;
                    delete pTmp;
                }
                else
                {
                    pMsgBuffer->Seek( 0L );
                    pMsgBuffer->SetStreamSize( 0 );
                }
                return INETSTREAM_STATUS_OK;
            }
        }
        else
        {
            /*
             * Single part message.
             * Remove any ContentTransferEncoding.
             */
            if (pMsg->GetContentType().Len() == 0)
            {
                String aDefaultCT;
                pMsg->GetDefaultContentType (aDefaultCT);
                pMsg->SetContentType (aDefaultCT);
            }

            if (eEncoding == INETMSG_ENCODING_BINARY)
            {
                String aEncoding (pMsg->GetContentTransferEncoding());
                if (aEncoding.CompareIgnoreCaseToAscii (
                    "base64", 6) == COMPARE_EQUAL)
                    eEncoding = INETMSG_ENCODING_BASE64;
                else if (aEncoding.CompareIgnoreCaseToAscii (
                    "quoted-printable", 16) == COMPARE_EQUAL)
                    eEncoding = INETMSG_ENCODING_QUOTED;
                else
                    eEncoding = INETMSG_ENCODING_7BIT;
            }

            if (eEncoding == INETMSG_ENCODING_7BIT)
            {
                // No decoding necessary.
                return INetMessageIOStream::PutMsgLine (
                    pData, nSize, pCtx);
            }
            else
            {
                if (pDecodeStrm == NULL)
                {
                    if (eEncoding == INETMSG_ENCODING_QUOTED)
                        pDecodeStrm = new INetMessageDecodeQPStream_Impl;
                    else
                        pDecodeStrm = new INetMessageDecode64Stream_Impl;

                    pDecodeStrm->SetTargetMessage (pMsg);
                }
                return pDecodeStrm->Write (pData, nSize, pCtx);
            }
        }
    }
}

/*=========================================================================
 *
 * INetMessageDecodeGZStream_Impl Implementation.
 * (GZIP Decoding).
 *
 *=======================================================================*/
class INetMessageDecodeGZStream_Impl : public INetMessageOStream
{
    SvMemoryStream m_aStrm;
    GZCodec        m_aCodec;
    sal_uInt8      m_pBuffer[1024];

    virtual int PutMsgLine (
        const sal_Char *pData, ULONG nSize, void *pCtx = NULL);

public:
    INetMessageDecodeGZStream_Impl (void);
};

/*
 * INetMessageDecodeGZStream_Impl.
 */
INetMessageDecodeGZStream_Impl::INetMessageDecodeGZStream_Impl (void)
    : INetMessageOStream()
{
    ParseHeader (FALSE);
    m_aCodec.BeginCompression();
}

/*
 * PutMsgLine.
 */
int INetMessageDecodeGZStream_Impl::PutMsgLine (
    const sal_Char *pData, ULONG nSize, void *pCtx)
{
    INetMessage *pMsg = GetTargetMessage();
    if (pMsg == NULL)
        return INETSTREAM_STATUS_ERROR;

    SvOpenLockBytes * pLB = PTR_CAST(SvOpenLockBytes, pMsg->GetDocumentLB());
    if (pLB == NULL)
        return INETSTREAM_STATUS_WOULDBLOCK;

    m_aStrm.Seek (STREAM_SEEK_TO_BEGIN);
    m_aStrm.Write (pData, nSize);

    m_aStrm.Seek (STREAM_SEEK_TO_BEGIN);
    long nRead = 0;

    // Decode into buffer.
    m_aCodec.SetBreak (nSize);
    while ((nRead = m_aCodec.Read (m_aStrm, m_pBuffer, sizeof(m_pBuffer))) > 0)
    {
        // Emit Buffer.
        ULONG nDocSize = pMsg->GetDocumentSize();
        ULONG nWrite   = 0;

        pLB->FillAppend (m_pBuffer, nRead, &nWrite);
        pMsg->SetDocumentSize (nDocSize + nWrite);

        if ((long)nWrite < nRead)
            return INETSTREAM_STATUS_ERROR;
    }

    if (m_aCodec.IsFinished())
        return INETSTREAM_STATUS_LOADED;
    else if (nRead < 0)
        return INETSTREAM_STATUS_ERROR;
    else
        return INETSTREAM_STATUS_OK;
}

/*=========================================================================
 *
 * INetHTTPMessageStream Implementation.
 *
 *=======================================================================*/
/*
 * INetHTTPMessageStream.
 */
INetHTTPMessageStream::INetHTTPMessageStream (ULONG nBufferSize)
    : INetMIMEMessageStream (nBufferSize),
      m_eState              (INETMSG_EOL_BEGIN),
      m_pDecodeStrm         (NULL)
{
}

/*
 * ~INetHTTPMessageStream.
 */
INetHTTPMessageStream::~INetHTTPMessageStream (void)
{
    delete m_pDecodeStrm;
}

/*
 * GetMsgLine.
 * (Message Generator).
 */
int INetHTTPMessageStream::GetMsgLine (
    sal_Char *pData, ULONG nSize, void *pCtx)
{
    // Check for header generation.
    if (!IsHeaderGenerated())
    {
        if (m_eState == INETMSG_EOL_BEGIN)
        {
            // Check for message container.
            INetHTTPMessage *pMsg = GetSourceMessage();
            if (pMsg == NULL)
                return INETSTREAM_STATUS_ERROR;

            // Set Accept-Encoding header.
            pMsg->SetAcceptEncoding (
                String ("gzip", RTL_TEXTENCODING_ASCII_US));

            // Mark done.
            m_eState = INETMSG_EOL_DONE;
        }
    }

    // Initial approximation: Generate as plain message (w/o encoding).
    return INetMessageIOStream::GetMsgLine (pData, nSize, pCtx);
}

/*
 * PutMsgLine.
 * (Message Parser).
 */
int INetHTTPMessageStream::PutMsgLine (
    const sal_Char *pData, ULONG nSize, void *pCtx)
{
    // Check for header or body.
    if (!IsHeaderParsed())
    {
        // Parse the message header.
        return INetMessageIOStream::PutMsgLine (pData, nSize, pCtx);
    }
    else
    {
        // Parse the message body.
        if (m_eState == INETMSG_EOL_BEGIN)
        {
            // Check for message container.
            INetHTTPMessage *pMsg = GetTargetMessage();
            if (pMsg == NULL)
                return INETSTREAM_STATUS_ERROR;

            // Obtain ContentEncoding.
            String aEncoding (pMsg->GetContentEncoding());
            aEncoding.EraseLeadingChars (' ');

            // Check for supported encoding.
            if ((aEncoding.CompareIgnoreCaseToAscii ("gzip")   == 0) ||
                (aEncoding.CompareIgnoreCaseToAscii ("x-gzip") == 0)   )
            {
                // Initialize decode stream.
                m_pDecodeStrm = new INetMessageDecodeGZStream_Impl;
                m_pDecodeStrm->SetTargetMessage (pMsg);
            }

            // Mark done.
            m_eState = INETMSG_EOL_DONE;
        }

        // Check for Decode stream.
        if (m_pDecodeStrm)
        {
            // Decode.
            return m_pDecodeStrm->Write (pData, nSize, pCtx);
        }
        else
        {
            // No decoding necessary.
            return INetMessageIOStream::PutMsgLine (pData, nSize, pCtx);
        }
    }
}

