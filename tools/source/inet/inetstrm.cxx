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

#include <sal/config.h>

#include <cassert>

#include <comphelper/string.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <rtl/strbuf.hxx>
#include <tools/inetmsg.hxx>
#include <tools/inetstrm.hxx>

int INetMIMEMessageStream::GetHeaderLine(sal_Char* pData, sal_uIntPtr nSize)
{
    sal_Char* pWBuf = pData;

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

    return (pWBuf - pData);
}

int INetMIMEMessageStream::GetBodyLine(sal_Char* pData, sal_uIntPtr nSize)
{
    sal_Char* pWBuf = pData;
    sal_Char* pWEnd = pData + nSize;

    if (pSourceMsg->GetDocumentLB())
    {
        if (pMsgStrm == nullptr)
            pMsgStrm = new SvStream (pSourceMsg->GetDocumentLB());

        sal_uIntPtr nRead = pMsgStrm->Read(pWBuf, (pWEnd - pWBuf));
        pWBuf += nRead;
    }

    return (pWBuf - pData);
}

int INetMIMEMessageStream::GetMsgLine(sal_Char* pData, sal_uIntPtr nSize)
{
    // Check for header or body.
    if (!bHeaderGenerated)
    {
        if (!done)
        {
            // Prepare special header fields.
            if (pSourceMsg->GetParent())
            {
                OUString aPCT(pSourceMsg->GetParent()->GetContentType());
                if (aPCT.startsWithIgnoreAsciiCase("message/rfc822"))
                    pSourceMsg->SetMIMEVersion("1.0");
                else
                    pSourceMsg->SetMIMEVersion(OUString());
            }
            else
            {
                pSourceMsg->SetMIMEVersion("1.0");
            }

            // Check ContentType.
            OUString aContentType(pSourceMsg->GetContentType());
            if (!aContentType.isEmpty())
            {
                // Determine default Content-Type.
                OUString aDefaultType = pSourceMsg->GetDefaultContentType();

                if (aDefaultType.equalsIgnoreAsciiCase(aContentType))
                {
                    // No need to specify default.
                    pSourceMsg->SetContentType(OUString());
                }
            }

            // No need to specify default.
            pSourceMsg->SetContentTransferEncoding(OUString());

            // Mark we're done.
            done = true;
        }

        // Generate the message header.
        int nRead = GetHeaderLine(pData, nSize);
        if (nRead <= 0)
        {
            // Reset state.
            done = false;
        }
        return nRead;
    }
    else
    {
        // Generate the message body.
        if (pSourceMsg->IsContainer())
        {
            // Encapsulated message body.
            while (!done)
            {
                if (pChildStrm == nullptr)
                {
                    INetMIMEMessage *pChild = pSourceMsg->GetChild(nChildIndex);
                    if (pChild)
                    {
                        // Increment child index.
                        nChildIndex++;

                        // Create child stream.
                        pChildStrm = new INetMIMEMessageStream(pChild, false);

                        if (pSourceMsg->IsMultipart())
                        {
                            // Insert multipart delimiter.
                            OStringBuffer aDelim("--");
                            aDelim.append(pSourceMsg->GetMultipartBoundary());
                            aDelim.append("\r\n");

                            memcpy(pData, aDelim.getStr(),
                                aDelim.getLength());
                            return aDelim.getLength();
                        }
                    }
                    else
                    {
                        // No more parts. Mark we're done.
                        done = true;
                        nChildIndex = 0;

                        if (pSourceMsg->IsMultipart())
                        {
                            // Insert close delimiter.
                            OStringBuffer aDelim("--");
                            aDelim.append(pSourceMsg->GetMultipartBoundary());
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
                        pChildStrm = nullptr;
                    }
                }
            }
            return 0;
        }
        else
        {
            // Single part message body.
            if (pSourceMsg->GetDocumentLB() == nullptr)
            {
                // Empty message body.
                return 0;
            }

            // No Encoding.
            return GetBodyLine(pData, nSize);
        }
    }
}

INetMIMEMessageStream::INetMIMEMessageStream(
    INetMIMEMessage *pMsg, bool headerGenerated):
    pSourceMsg(pMsg),
    bHeaderGenerated(headerGenerated),
    nBufSiz(2048),
    pMsgStrm(nullptr),
    pMsgBuffer(new SvMemoryStream),
    pMsgRead(nullptr),
    pMsgWrite(nullptr),
    done(false),
    nChildIndex(0),
    pChildStrm(nullptr)
{
    assert(pMsg != nullptr);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
