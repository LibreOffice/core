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

#include <sal/types.h>
#include <tools/inetmsg.hxx>
#include <tools/inetstrm.hxx>

int INetMIMEMessageStream::GetHeaderLine(char* pData, sal_uInt32 nSize)
{
    char* pWBuf = pData;

    sal_uInt32 i, n;

    if (maMsgBuffer.Tell() == 0)
    {
        // Insert formatted header into buffer.
        n = pSourceMsg->GetHeaderCount();
        for (i = 0; i < n; i++)
        {
            INetMessageHeader aHeader (pSourceMsg->GetHeaderField(i));
            if (aHeader.GetValue().getLength())
            {
                // NYI: Folding long lines.
                maMsgBuffer.WriteOString( aHeader.GetName() );
                maMsgBuffer.WriteOString( ": " );
                maMsgBuffer.WriteOString( aHeader.GetValue() );
                maMsgBuffer.WriteOString( "\r\n" );
            }
        }

        pMsgWrite = const_cast<char *>(static_cast<char const *>(maMsgBuffer.GetData()));
        pMsgRead  = pMsgWrite + maMsgBuffer.Tell();
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
        maMsgBuffer.Seek(STREAM_SEEK_TO_BEGIN);
    }

    return (pWBuf - pData);
}

int INetMIMEMessageStream::GetBodyLine(char* pData, sal_uInt32 nSize)
{
    char* pWBuf = pData;
    char* pWEnd = pData + nSize;

    if (pSourceMsg->GetDocumentLB())
    {
        sal_uInt32 nRead = pSourceMsg->GetDocumentLB()->ReadBytes(pWBuf, (pWEnd - pWBuf));
        pWBuf += nRead;
    }

    return (pWBuf - pData);
}

int INetMIMEMessageStream::GetMsgLine(char* pData, sal_uInt32 nSize)
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
                    pSourceMsg->SetMIMEVersion(u"1.0"_ustr);
                else
                    pSourceMsg->SetMIMEVersion(OUString());
            }
            else
            {
                pSourceMsg->SetMIMEVersion(u"1.0"_ustr);
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
                        pChildStrm.reset(new INetMIMEMessageStream(pChild, false));

                        if (pSourceMsg->IsMultipart())
                        {
                            // Insert multipart delimiter.
                            OString aDelim = "--" +
                                pSourceMsg->GetMultipartBoundary() +
                                "\r\n";

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
                            OString aDelim = "--" +
                                pSourceMsg->GetMultipartBoundary() +
                                "--\r\n";

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
                        pChildStrm.reset();
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

namespace
{

const int BUFFER_SIZE = 2048;

}

INetMIMEMessageStream::INetMIMEMessageStream(
    INetMIMEMessage *pMsg, bool headerGenerated):
    pSourceMsg(pMsg),
    bHeaderGenerated(headerGenerated),
    mvBuffer(BUFFER_SIZE),
    pMsgRead(nullptr),
    pMsgWrite(nullptr),
    done(false),
    nChildIndex(0)
{
    assert(pMsg != nullptr);
    maMsgBuffer.SetStreamCharSet(RTL_TEXTENCODING_ASCII_US);
    pRead = pWrite = mvBuffer.data();
}

INetMIMEMessageStream::~INetMIMEMessageStream()
{
    pChildStrm.reset();
}

int INetMIMEMessageStream::Read(char* pData, sal_uInt32 nSize)
{
    char* pWBuf = pData;
    char* pWEnd = pData + nSize;

    while (pWBuf < pWEnd)
    {
        // Caller's buffer not yet filled.
        sal_uInt32 n = pRead - pWrite;
        if (n > 0)
        {
            // Bytes still in buffer.
            sal_uInt32 m = pWEnd - pWBuf;
            if (m < n) n = m;
            for (sal_uInt32 i = 0; i < n; i++) *pWBuf++ = *pWrite++;
        }
        else
        {
            // Buffer empty. Reset to <Begin-of-Buffer>.
            pRead = pWrite = mvBuffer.data();

            // Read next message line.
            int nRead = GetMsgLine(mvBuffer.data(), mvBuffer.size());
            if (nRead > 0)
            {
                // Set read pointer.
                pRead = mvBuffer.data() + nRead;
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
