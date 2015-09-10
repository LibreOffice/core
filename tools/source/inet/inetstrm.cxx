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

int INetMIMEMessageStream::GetInnerMsgLine(sal_Char* pData, sal_uIntPtr nSize)
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

int INetMIMEMessageStream::GetOuterMsgLine(sal_Char* pData, sal_uIntPtr nSize)
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

            // No need to specify default.
            pMsg->SetContentTransferEncoding(OUString());

            // Mark we're done.
            eState = INETMSG_EOL_DONE;
        }

        // Generate the message header.
        int nRead = GetInnerMsgLine(pData, nSize);
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

            // No Encoding.
            return GetInnerMsgLine(pData, nSize);
        }
    }
}

INetMIMEMessageStream::INetMIMEMessageStream(sal_uIntPtr nBufferSize):
    pSourceMsg(NULL),
    bHeaderGenerated(false),
    nBufSiz(nBufferSize),
    pMsgStrm(NULL),
    pMsgBuffer(new SvMemoryStream),
    pMsgRead(NULL),
    pMsgWrite(NULL),
    eState(INETMSG_EOL_BEGIN),
    nChildIndex(0),
    pChildStrm(NULL)
{
    pMsgBuffer->SetStreamCharSet(RTL_TEXTENCODING_ASCII_US);
    pBuffer = new sal_Char[nBufSiz];
    pRead = pWrite = pBuffer;
}

INetMIMEMessageStream::~INetMIMEMessageStream()
{
    delete pChildStrm;
    delete [] pBuffer;
    delete pMsgBuffer;
    delete pMsgStrm;
}

int INetMIMEMessageStream::Read(sal_Char* pData, sal_uIntPtr nSize)
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
            int nRead = GetOuterMsgLine(pBuffer, nBufSiz);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
