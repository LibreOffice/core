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

#include <comphelper/string.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <rtl/strbuf.hxx>
#include <tools/inetmsg.hxx>
#include <tools/inetstrm.hxx>

#include <ctype.h>

/** Quoted-Printable Encoding */
class INetMessageEncodeQPStream_Impl : public INetMessageIStream
{
    SvStream*               pMsgStrm;

    sal_uIntPtr             nMsgBufSiz;
    sal_Char*               pMsgBuffer;
    sal_Char*               pMsgRead;
    sal_Char*               pMsgWrite;

    sal_uIntPtr             nTokBufSiz;
    sal_Char*               pTokBuffer;
    sal_Char*               pTokRead;
    sal_Char*               pTokWrite;

    INetMessageStreamState  eState;
    bool                    bDone;

    virtual int GetMsgLine(sal_Char* pData, sal_uIntPtr nSize) SAL_OVERRIDE;

public:
    explicit INetMessageEncodeQPStream_Impl(sal_uIntPtr nMsgBufferSize = 1024);
    virtual ~INetMessageEncodeQPStream_Impl();
};

/** Base64 Encoding */
class INetMessageEncode64Stream_Impl : public INetMessageIStream
{
    SvStream*    pMsgStrm;

    sal_uIntPtr  nMsgBufSiz;
    sal_uInt8*   pMsgBuffer;
    sal_uInt8*   pMsgRead;
    sal_uInt8*   pMsgWrite;

    sal_uIntPtr  nTokBufSiz;
    sal_Char*    pTokBuffer;
    sal_Char*    pTokRead;
    sal_Char*    pTokWrite;

    bool         bDone;

    virtual int GetMsgLine(sal_Char* pData, sal_uIntPtr nSize) SAL_OVERRIDE;

public:
    explicit INetMessageEncode64Stream_Impl(sal_uIntPtr nMsgBufferSize = 2048);
    virtual ~INetMessageEncode64Stream_Impl();
};

// INetMessageIStream

INetMessageIStream::INetMessageIStream(sal_uIntPtr nBufferSize)
    : pSourceMsg(NULL)
    , bHeaderGenerated(false)
    , nBufSiz(nBufferSize)
    , pMsgStrm(NULL)
    , pMsgBuffer(new SvMemoryStream)
    , pMsgRead(NULL)
    , pMsgWrite(NULL)
{
    pMsgBuffer->SetStreamCharSet(RTL_TEXTENCODING_ASCII_US);
    pBuffer = new sal_Char[nBufSiz];
    pRead = pWrite = pBuffer;
}

INetMessageIStream::~INetMessageIStream()
{
    delete [] pBuffer;
    delete pMsgBuffer;
    delete pMsgStrm;
}

int INetMessageIStream::Read(sal_Char* pData, sal_uIntPtr nSize)
{
    if (pSourceMsg == NULL) return INETSTREAM_STATUS_ERROR;

    sal_Char* pWBuf = pData;
    sal_Char* pWEnd = pData + nSize;

    while (pWBuf < pWEnd)
    {
        // Caller's buffer not yet filled.
        sal_uIntPtr n = pRead - pWrite;
        if (n > 0)
        {
            // Bytes still in buffer.
            sal_uIntPtr m = pWEnd - pWBuf;
            if (m < n) n = m;
            for (sal_uIntPtr i = 0; i < n; i++) *pWBuf++ = *pWrite++;
        }
        else
        {
            // Buffer empty. Reset to <Begin-of-Buffer>.
            pRead = pWrite = pBuffer;

            // Read next message line.
            int nRead = GetMsgLine(pBuffer, nBufSiz);
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
                    bHeaderGenerated = true;
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

int INetMessageIStream::GetMsgLine(sal_Char* pData, sal_uIntPtr nSize)
{
    if (pSourceMsg == NULL) return INETSTREAM_STATUS_ERROR;

    sal_Char* pWBuf = pData;
    sal_Char* pWEnd = pData + nSize;

    if (!bHeaderGenerated)
    {
        sal_uIntPtr i, n;

        if (pMsgBuffer->Tell() == 0)
        {
            // Insert formatted header into buffer.
            n = pSourceMsg->GetHeaderCount();
            for (i = 0; i < n; i++)
            {
                INetMessageHeader aHeader (pSourceMsg->GetHeaderField(i));
                if (aHeader.GetValue().getLength())
                {
                    // NYI: Folding long lines.
                    pMsgBuffer->WriteCharPtr( aHeader.GetName().getStr() );
                    pMsgBuffer->WriteCharPtr( ": " );
                    pMsgBuffer->WriteCharPtr( aHeader.GetValue().getStr() );
                    pMsgBuffer->WriteCharPtr( "\r\n" );
                }
            }

            pMsgWrite = const_cast<char *>(static_cast<sal_Char const *>(pMsgBuffer->GetData()));
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
            pMsgBuffer->Seek(STREAM_SEEK_TO_BEGIN);
        }
    }
    else
    {
        if (pSourceMsg->GetDocumentLB())
        {
            if (pMsgStrm == NULL)
                pMsgStrm = new SvStream (pSourceMsg->GetDocumentLB());

            sal_uIntPtr nRead = pMsgStrm->Read(pWBuf, (pWEnd - pWBuf));
            pWBuf += nRead;
        }
    }
    return (pWBuf - pData);
}

// INetMessageEncodeQPStream_Impl

static const sal_Char hex2pr[16] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F'
};

INetMessageEncodeQPStream_Impl::INetMessageEncodeQPStream_Impl( sal_uIntPtr nMsgBufferSize)
    : INetMessageIStream (),
      pMsgStrm   (NULL),
      nMsgBufSiz (nMsgBufferSize),
      nTokBufSiz (80),
      eState     (INETMSG_EOL_SCR),
      bDone      (false)
{
    SetHeaderGenerated();

    pMsgBuffer = new sal_Char[nMsgBufSiz];
    pMsgRead = pMsgWrite = pMsgBuffer;

    pTokBuffer = new sal_Char[nTokBufSiz];
    pTokRead = pTokWrite = pTokBuffer;
}

INetMessageEncodeQPStream_Impl::~INetMessageEncodeQPStream_Impl()
{
    delete pMsgStrm;
    delete [] pMsgBuffer;
    delete [] pTokBuffer;
}

int INetMessageEncodeQPStream_Impl::GetMsgLine(sal_Char* pData, sal_uIntPtr nSize)
{
    INetMIMEMessage* pMsg = GetSourceMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    if (pMsg->GetDocumentLB() == NULL) return 0;
    if (pMsgStrm == NULL) pMsgStrm = new SvStream(pMsg->GetDocumentLB());

    sal_Char* pWBuf = pData;
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
            sal_uIntPtr nRead = pMsgStrm->Read(pMsgBuffer, nMsgBufSiz);
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

                    bDone = true;
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

// INetMessageEncode64Stream_Impl

static const sal_Char six2pr[64] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
};

INetMessageEncode64Stream_Impl::INetMessageEncode64Stream_Impl(
    sal_uIntPtr nMsgBufferSize)
    : INetMessageIStream(),
      pMsgStrm   (NULL),
      nMsgBufSiz (nMsgBufferSize),
      nTokBufSiz (80),
      bDone      (false)
{
    SetHeaderGenerated();

    pMsgBuffer = new sal_uInt8[nMsgBufSiz];
    pMsgRead = pMsgWrite = pMsgBuffer;

    pTokBuffer = new sal_Char[nTokBufSiz];
    pTokRead = pTokWrite = pTokBuffer;
}

INetMessageEncode64Stream_Impl::~INetMessageEncode64Stream_Impl()
{
    delete pMsgStrm;
    delete [] pMsgBuffer;
    delete [] pTokBuffer;
}

int INetMessageEncode64Stream_Impl::GetMsgLine(sal_Char* pData, sal_uIntPtr nSize)
{
    INetMIMEMessage* pMsg = GetSourceMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    if (pMsg->GetDocumentLB() == NULL) return 0;
    if (pMsgStrm == NULL) pMsgStrm = new SvStream(pMsg->GetDocumentLB());

    sal_Char* pWBuf = pData;
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
                    *pTokRead++ = six2pr[ (int)(((*pMsgWrite << 4) & 060) |
                                                (((*(pMsgWrite + 1)) >> 4) & 017))];
                    pMsgWrite++;
                    break;

                case 2:
                    *pTokRead++ = six2pr[ (int)(((*pMsgWrite << 2) & 074) |
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
            sal_uIntPtr nRead = pMsgStrm->Read(pMsgBuffer, nMsgBufSiz);
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
                    bDone = true;
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
                        // if (pWBuf == pData) bDone = false;

                        // Return.
                        return (pWBuf - pData);
                    }
                }
            }
        }
    } // while (pWBuf < (pData + nSize))
    return (pWBuf - pData);
}

// INetMIMEMessageStream

INetMIMEMessageStream::INetMIMEMessageStream(sal_uIntPtr nBufferSize)
    : INetMessageIStream(nBufferSize),
      eState      (INETMSG_EOL_BEGIN),
      nChildIndex (0),
      pChildStrm  (NULL),
      eEncoding   (INETMSG_ENCODING_BINARY),
      pEncodeStrm (NULL),
      pMsgBuffer  (NULL)
{
}

INetMIMEMessageStream::~INetMIMEMessageStream()
{
    delete pChildStrm;
    delete pEncodeStrm;
    delete pMsgBuffer;
}

INetMessageEncoding
INetMIMEMessageStream::GetMsgEncoding(const OUString& rContentType)
{
    if (rContentType.startsWithIgnoreAsciiCase("message") ||
        rContentType.startsWithIgnoreAsciiCase("multipart"))
    {
        return INETMSG_ENCODING_7BIT;
    }
    if (rContentType.startsWithIgnoreAsciiCase("text"))
    {
        if (rContentType.startsWithIgnoreAsciiCase("text/plain"))
        {
            if (comphelper::string::getTokenCount(rContentType, '=') > 1)
            {
                OUString aCharset(rContentType.getToken(1, '='));
                aCharset = comphelper::string::stripStart(aCharset, ' ');
                aCharset = comphelper::string::stripStart(aCharset, '"');

                if (aCharset.startsWithIgnoreAsciiCase("us-ascii"))
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

/// Message Generator
int INetMIMEMessageStream::GetMsgLine(sal_Char* pData, sal_uIntPtr nSize)
{
    // Check for message container.
    INetMIMEMessage* pMsg = GetSourceMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    // Check for header or body.
    if (!IsHeaderGenerated())
    {
        if (eState == INETMSG_EOL_BEGIN)
        {
            // Prepare special header fields.
            if (pMsg->GetParent())
            {
                OUString aPCT(pMsg->GetParent()->GetContentType());
                if (aPCT.startsWithIgnoreAsciiCase("message/rfc822"))
                    pMsg->SetMIMEVersion("1.0");
                else
                    pMsg->SetMIMEVersion(OUString());
            }
            else
            {
                pMsg->SetMIMEVersion("1.0");
            }

            // Check ContentType.
            OUString aContentType(pMsg->GetContentType());
            if (!aContentType.isEmpty())
            {
                // Determine default Content-Type.
                OUString aDefaultType = pMsg->GetDefaultContentType();

                if (aDefaultType.equalsIgnoreAsciiCase(aContentType))
                {
                    // No need to specify default.
                    pMsg->SetContentType(OUString());
                }
            }

            // Check Encoding.
            OUString aEncoding(pMsg->GetContentTransferEncoding());
            if (!aEncoding.isEmpty())
            {
                // Use given Encoding.
                if (aEncoding.startsWithIgnoreAsciiCase("base64"))
                {
                    eEncoding = INETMSG_ENCODING_BASE64;
                }
                else if (aEncoding.startsWithIgnoreAsciiCase("quoted-printable"))
                {
                    eEncoding = INETMSG_ENCODING_QUOTED;
                }
                else
                {
                    eEncoding = INETMSG_ENCODING_7BIT;
                }
            }
            else
            {
                // Use default Encoding for (given|default) Content-Type.
                if (aContentType.isEmpty())
                {
                    // Determine default Content-Type.
                    aContentType = pMsg->GetDefaultContentType();
                }
                eEncoding = GetMsgEncoding(aContentType);
            }

            // Set Content-Transfer-Encoding header.
            if (eEncoding == INETMSG_ENCODING_BASE64)
            {
                // Base64.
                pMsg->SetContentTransferEncoding("base64");
            }
            else if (eEncoding == INETMSG_ENCODING_QUOTED)
            {
                // Quoted-Printable.
                pMsg->SetContentTransferEncoding("quoted-printable");
            }
            else
            {
                // No need to specify default.
                pMsg->SetContentTransferEncoding(OUString());
            }

            // Mark we're done.
            eState = INETMSG_EOL_DONE;
        }

        // Generate the message header.
        int nRead = INetMessageIStream::GetMsgLine(pData, nSize);
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
                    INetMIMEMessage *pChild = pMsg->GetChild(nChildIndex);
                    if (pChild)
                    {
                        // Increment child index.
                        nChildIndex++;

                        // Create child stream.
                        pChildStrm = new INetMIMEMessageStream;
                        pChildStrm->SetSourceMessage(pChild);

                        if (pMsg->IsMultipart())
                        {
                            // Insert multipart delimiter.
                            OStringBuffer aDelim("--");
                            aDelim.append(pMsg->GetMultipartBoundary());
                            aDelim.append("\r\n");

                            memcpy(pData, aDelim.getStr(),
                                aDelim.getLength());
                            return aDelim.getLength();
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
                            OStringBuffer aDelim("--");
                            aDelim.append(pMsg->GetMultipartBoundary());
                            aDelim.append("--\r\n");

                            memcpy(pData, aDelim.getStr(),
                                aDelim.getLength());
                            return aDelim.getLength();
                        }
                    }
                }
                else
                {
                    // Read current child stream.
                    int nRead = pChildStrm->Read(pData, nSize);
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

            // Check whether message body needs to be encoded.
            if (eEncoding == INETMSG_ENCODING_7BIT)
            {
                // No Encoding.
                return INetMessageIStream::GetMsgLine(pData, nSize);
            }

            // Apply appropriate Encoding.
            while (eState == INETMSG_EOL_BEGIN)
            {
                if (pEncodeStrm == NULL)
                {
                    // Create encoder stream.
                    if (eEncoding == INETMSG_ENCODING_QUOTED)
                    {
                        // Quoted-Printable Encoding.
                        pEncodeStrm = new INetMessageEncodeQPStream_Impl;
                    }
                    else
                    {
                        // Base64 Encoding.
                        pEncodeStrm = new INetMessageEncode64Stream_Impl;
                    }
                    pEncodeStrm->SetSourceMessage(pMsg);
                }

                // Read encoded message.
                int nRead = pEncodeStrm->Read(pData, nSize);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
