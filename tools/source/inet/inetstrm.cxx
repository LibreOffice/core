/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <comphelper/string.hxx>
#include <sal/types.h>
#include <rtl/strbuf.hxx>
#include <tools/inetmsg.hxx>
#include <tools/inetstrm.hxx>

#include <ctype.h>

inline bool SAL_CALL ascii_isWhitespace( sal_Unicode ch )
{
    return ((ch <= 0x20) && ch);
}

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

    virtual int GetMsgLine(sal_Char* pData, sal_uIntPtr nSize);

public:
    INetMessageEncodeQPStream_Impl(sal_uIntPtr nMsgBufferSize = 1024);
    virtual ~INetMessageEncodeQPStream_Impl(void);
};

/** Quoted-Printable Decoding */
class INetMessageDecodeQPStream_Impl : public INetMessageOStream
{
    INetMessageStreamState  eState;
    SvMemoryStream         *pMsgBuffer;

    sal_uIntPtr             nTokBufLen;
    sal_Char                pTokBuffer[4];

    virtual int PutMsgLine(const sal_Char* pData, sal_uIntPtr nSize);

public:
    INetMessageDecodeQPStream_Impl(void);
    virtual ~INetMessageDecodeQPStream_Impl(void);
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

    virtual int GetMsgLine(sal_Char* pData, sal_uIntPtr nSize);

public:
    INetMessageEncode64Stream_Impl(sal_uIntPtr nMsgBufferSize = 2048);
    virtual ~INetMessageEncode64Stream_Impl(void);
};

/** Base64 Decoding */
class INetMessageDecode64Stream_Impl : public INetMessageOStream
{
    INetMessageStreamState  eState;

    sal_uIntPtr             nMsgBufSiz;
    sal_Char*               pMsgBuffer;
    sal_Char*               pMsgRead;
    sal_Char*               pMsgWrite;

    virtual int PutMsgLine(const sal_Char* pData, sal_uIntPtr nSize);

public:
    INetMessageDecode64Stream_Impl(sal_uIntPtr nMsgBufferSize = 128);
    virtual ~INetMessageDecode64Stream_Impl(void);
};



INetIStream::INetIStream()
{
}

INetIStream::~INetIStream(void)
{
}

int INetIStream::Read(sal_Char* pData, sal_uIntPtr nSize)
{
    return GetData(pData, nSize);
}



INetOStream::INetOStream()
{
}

INetOStream::~INetOStream(void)
{
}

int INetOStream::Write(const sal_Char* pData, sal_uIntPtr nSize)
{
    return PutData(pData, nSize);
}



INetMessageIStream::INetMessageIStream(sal_uIntPtr nBufferSize)
    : pSourceMsg       (NULL),
      bHeaderGenerated (false),
      nBufSiz          (nBufferSize),
      pMsgStrm         (NULL),
      pMsgBuffer       (new SvMemoryStream)
{
    pMsgBuffer->SetStreamCharSet(RTL_TEXTENCODING_ASCII_US);
    pBuffer = new sal_Char[nBufSiz];
    pRead = pWrite = pBuffer;
}

INetMessageIStream::~INetMessageIStream(void)
{
    delete [] pBuffer;
    delete pMsgBuffer;
    delete pMsgStrm;
}

int INetMessageIStream::GetData(sal_Char* pData, sal_uIntPtr nSize)
{
    if (pSourceMsg == NULL) return INETSTREAM_STATUS_ERROR;

    sal_Char* pWBuf = pData;
    sal_Char* pWEnd = pData + nSize;

    while (pWBuf < pWEnd)
    {
        
        sal_uIntPtr n = pRead - pWrite;
        if (n > 0)
        {
            
            sal_uIntPtr m = pWEnd - pWBuf;
            if (m < n) n = m;
            for (sal_uIntPtr i = 0; i < n; i++) *pWBuf++ = *pWrite++;
        }
        else
        {
            
            pRead = pWrite = pBuffer;

            
            int nRead = GetMsgLine(pBuffer, nBufSiz);
            if (nRead > 0)
            {
                
                pRead = pBuffer + nRead;
            }
            else
            {
                if (!bHeaderGenerated)
                {
                    
                    bHeaderGenerated = true;
                    *pRead++ = '\r';
                    *pRead++ = '\n';
                }
                else
                {
                    
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
            
            n = pSourceMsg->GetHeaderCount();
            for (i = 0; i < n; i++)
            {
                INetMessageHeader aHeader (pSourceMsg->GetHeaderField(i));
                if (aHeader.GetValue().getLength())
                {
                    
                    pMsgBuffer->WriteCharPtr( aHeader.GetName().getStr() );
                    pMsgBuffer->WriteCharPtr( ": " );
                    pMsgBuffer->WriteCharPtr( aHeader.GetValue().getStr() );
                    pMsgBuffer->WriteCharPtr( "\r\n" );
                }
            }

            pMsgWrite = (sal_Char*)(pMsgBuffer->GetData());
            pMsgRead  = pMsgWrite + pMsgBuffer->Tell();
        }

        n = pMsgRead - pMsgWrite;
        if (n > 0)
        {
            
            if (nSize < n) n = nSize;
            for (i = 0; i < n; i++) *pWBuf++ = *pMsgWrite++;
        }
        else
        {
            
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



INetMessageOStream::INetMessageOStream(void)
    : pTargetMsg    (NULL),
      bHeaderParsed (false),
      eOState       (INETMSG_EOL_BEGIN),
      pMsgBuffer    (new SvMemoryStream)
{
}

INetMessageOStream::~INetMessageOStream(void)
{
    if (pMsgBuffer->Tell() > 0)
        PutMsgLine((const sal_Char*) pMsgBuffer->GetData(), pMsgBuffer->Tell());
    delete pMsgBuffer;

    if (pTargetMsg)
    {
        SvOpenLockBytes* pLB = PTR_CAST(SvOpenLockBytes, pTargetMsg->GetDocumentLB());
        if (pLB)
        {
            pLB->Flush();
            pLB->Terminate();
        }
    }
}


int INetMessageOStream::PutData(const sal_Char* pData, sal_uIntPtr nSize)
{
    if (pTargetMsg == NULL) return INETSTREAM_STATUS_ERROR;

    const sal_Char* pStop = (pData + nSize);

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

                
                if (pMsgBuffer->Tell() > 0)
                {
                    pMsgBuffer->WriteChar( '\0' );
                    int status = PutMsgLine( (const sal_Char*) pMsgBuffer->GetData(),
                                             pMsgBuffer->Tell());
                    if (status != INETSTREAM_STATUS_OK) return status;
                }

                
                eOState = INETMSG_EOL_BEGIN;
                pMsgBuffer->Seek(STREAM_SEEK_TO_BEGIN);

                
                bHeaderParsed = true;
            }
            else if ((*pData == ' ') || (*pData == '\t'))
            {
                
                pMsgBuffer->WriteChar( ' ' );
                pData++;
            }
            else
            {
                
                if (pMsgBuffer->Tell() > 0)
                {
                    
                    pMsgBuffer->WriteChar( '\0' );
                    int status = PutMsgLine((const sal_Char*) pMsgBuffer->GetData(),
                                             pMsgBuffer->Tell());
                    if (status != INETSTREAM_STATUS_OK) return status;
                }

                
                pMsgBuffer->Seek(STREAM_SEEK_TO_BEGIN);

                
                pMsgBuffer->WriteChar( *pData++ );
            }

            
            if (!bHeaderParsed) eOState = INETMSG_EOL_SCR;
        }
        else if (eOState == INETMSG_EOL_FCR)
        {
            
            pData++;

            
            eOState = INETMSG_EOL_BEGIN;
        }
        else if ((*pData == '\r') || (*pData == '\n'))
        {
            if (*pData == '\r') pData++;
            eOState = INETMSG_EOL_FCR;
        }
        else if (ascii_isWhitespace(*pData & 0x7f))
        {
            
            sal_Char c = *((const sal_Char*) pMsgBuffer->GetData() + pMsgBuffer->Tell() - 1);
            if (!ascii_isWhitespace(c & 0x7f)) pMsgBuffer->WriteChar( ' ' );

            
            pData++;
        }
        else
        {
            
            pMsgBuffer->WriteChar( *pData++ );
        }
    }

    if (bHeaderParsed && (pData < pStop))
    {
        
        return PutMsgLine(pData, (pStop - pData));
    }

    return INETSTREAM_STATUS_OK;
}

int INetMessageOStream::PutMsgLine(const sal_Char* pData, sal_uIntPtr nSize)
{
    
    if (pTargetMsg == NULL) return INETSTREAM_STATUS_ERROR;

    
    if (!IsHeaderParsed())
    {
        OString aField(pData);
        sal_Int32 nPos = aField.indexOf(':');
        if (nPos != -1)
        {
            OString aName( aField.copy(0, nPos));
            OString aValue( aField.copy(nPos + 1, aField.getLength() - nPos + 1));
            aValue = comphelper::string::stripStart(aValue, ' ');

            pTargetMsg->SetHeaderField( INetMessageHeader (aName, aValue));
        }
    }
    else
    {
        SvOpenLockBytes *pLB = PTR_CAST(SvOpenLockBytes, pTargetMsg->GetDocumentLB());
        if (pLB == NULL)
            return INETSTREAM_STATUS_WOULDBLOCK;

        sal_Size nDocSiz = pTargetMsg->GetDocumentSize();
        sal_Size nWrite  = 0;

        pLB->FillAppend((sal_Char*)pData, nSize, &nWrite);
        pTargetMsg->SetDocumentSize(nDocSiz + nWrite);

        if (nWrite < nSize) return INETSTREAM_STATUS_ERROR;
    }
    return INETSTREAM_STATUS_OK;
}



INetMessageIOStream::INetMessageIOStream(sal_uIntPtr nBufferSize)
    : INetMessageIStream (nBufferSize),
      INetMessageOStream ()
{
}

INetMessageIOStream::~INetMessageIOStream(void)
{
}



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
    GenerateHeader (false);

    pMsgBuffer = new sal_Char[nMsgBufSiz];
    pMsgRead = pMsgWrite = pMsgBuffer;

    pTokBuffer = new sal_Char[nTokBufSiz];
    pTokRead = pTokWrite = pTokBuffer;
}

INetMessageEncodeQPStream_Impl::~INetMessageEncodeQPStream_Impl(void)
{
    delete pMsgStrm;
    delete [] pMsgBuffer;
    delete [] pTokBuffer;
}

int INetMessageEncodeQPStream_Impl::GetMsgLine(sal_Char* pData, sal_uIntPtr nSize)
{
    INetMessage* pMsg = GetSourceMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    if (pMsg->GetDocumentLB() == NULL) return 0;
    if (pMsgStrm == NULL) pMsgStrm = new SvStream(pMsg->GetDocumentLB());

    sal_Char* pWBuf = pData;
    while (pWBuf < (pData + nSize))
    {
        
        if ((pMsgRead - pMsgWrite) > 0)
        {
            
            if ((eState != INETMSG_EOL_BEGIN) &&
                ((pTokRead - pTokBuffer) < 72))
            {
                
                if (eState == INETMSG_EOL_FCR)
                {
                    eState = INETMSG_EOL_BEGIN;
                    if (*pMsgWrite != '\n')
                    {
                        
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
                    
                    if (eState == INETMSG_EOL_FSP)
                    {
                        
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
                    
                    if (eState == INETMSG_EOL_FSP)
                    {
                        
                        sal_uInt8 c = (sal_uInt8)(*(--pTokRead));
                        *pTokRead++ = '=';
                        *pTokRead++ = hex2pr[((c & 0xf0) >> 4)];
                        *pTokRead++ = hex2pr[((c & 0x0f)     )];
                    }
                    eState = INETMSG_EOL_BEGIN;

                    
                    *pTokRead++ = '\r';
                    *pTokRead++ = *pMsgWrite++;
                }
                else if (*pMsgWrite == '=')
                {
                    
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
                    
                    sal_uInt8 c = (sal_uInt8)(*pMsgWrite++);
                    *pTokRead++ = '=';
                    *pTokRead++ = hex2pr[((c & 0xf0) >> 4)];
                    *pTokRead++ = hex2pr[((c & 0x0f)     )];

                    eState = INETMSG_EOL_SCR;
                }
            }
            else
            {
                
                if (eState != INETMSG_EOL_BEGIN)
                {
                    
                    *pTokRead++ = '=';
                    *pTokRead++ = '\r';
                    *pTokRead++ = '\n';

                    eState = INETMSG_EOL_BEGIN;
                }

                
                if ((pTokRead - pTokWrite) > 0)
                {
                    
                    *pWBuf++ = *pTokWrite++;
                }
                else
                {
                    
                    pTokRead = pTokWrite = pTokBuffer;
                    eState = INETMSG_EOL_SCR;
                }
            }
        }
        else
        {
            
            pMsgRead = pMsgWrite = pMsgBuffer;

            
            sal_uIntPtr nRead = pMsgStrm->Read(pMsgBuffer, nMsgBufSiz);
            if (nRead > 0)
            {
                
                pMsgRead = (pMsgBuffer + nRead);
            }
            else
            {
                
                if (!bDone)
                {
                    
                    *pTokRead++ = '\r';
                    *pTokRead++ = '\n';

                    bDone = true;
                }
                else
                {
                    
                    if ((pTokRead - pTokWrite) > 0)
                    {
                        
                        *pWBuf++ = *pTokWrite++;
                    }
                    else
                    {
                        
                        pTokRead = pTokWrite = pTokBuffer;

                        
                        return (pWBuf - pData);
                    }
                }
            }
        }
    }
    return (pWBuf - pData);
}



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

INetMessageDecodeQPStream_Impl::INetMessageDecodeQPStream_Impl(void)
    : INetMessageOStream(),
      eState     (INETMSG_EOL_BEGIN),
      pMsgBuffer (new SvMemoryStream),
      nTokBufLen (0)
{
    ParseHeader(false);
}

INetMessageDecodeQPStream_Impl::~INetMessageDecodeQPStream_Impl(void)
{
    delete pMsgBuffer;
}

int INetMessageDecodeQPStream_Impl::PutMsgLine( const sal_Char* pData,
                                                sal_uIntPtr nSize)
{
    INetMessage* pMsg = GetTargetMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    SvOpenLockBytes* pLB = PTR_CAST(SvOpenLockBytes, pMsg->GetDocumentLB());
    if (pLB == NULL) return INETSTREAM_STATUS_WOULDBLOCK;

    const sal_Char* pStop = pData + nSize;
    while (pData < pStop)
    {
        if (eState == INETMSG_EOL_FESC)
        {
            *(pTokBuffer + nTokBufLen++) = static_cast< char >(toupper(*pData));
            pData++;
            if (nTokBufLen == 2)
            {
                if ((*pTokBuffer == '\r') || (*pTokBuffer == '\n'))
                {
                    
                    eState = INETMSG_EOL_BEGIN;
                }
                else
                {
                    
                    pMsgBuffer->WriteUChar( sal_uInt8 (
                        (pr2hex[(int)(pTokBuffer[0] & 0x7f)] << 4) |
                        (pr2hex[(int)(pTokBuffer[1] & 0x7f)] & 15)   ) );

                    
                    eState = INETMSG_EOL_SCR;
                }

                
                nTokBufLen = 0;
            }
        }
        else if (*pData == '=')
        {
            
            pData++;
            eState = INETMSG_EOL_FESC;
        }
        else if (eState == INETMSG_EOL_FCR)
        {
            pMsgBuffer->WriteChar( *pData++ );
            eState = INETMSG_EOL_BEGIN;
        }
        else if (*pData == '\r')
        {
            pMsgBuffer->WriteChar( *pData++ );
            eState = INETMSG_EOL_FCR;
        }
        else
        {
            pMsgBuffer->WriteChar( *pData++ );
        }

        if (eState == INETMSG_EOL_BEGIN)
        {
            sal_Size nRead = pMsgBuffer->Tell();
            if (nRead > 0)
            {
                
                sal_Size nDocSiz = pMsg->GetDocumentSize();
                sal_Size nWrite  = 0;

                pLB->FillAppend((sal_Char*)(pMsgBuffer->GetData()), nRead, &nWrite);
                pMsg->SetDocumentSize(nDocSiz + nWrite);

                if (nWrite < nRead) return INETSTREAM_STATUS_ERROR;

                pMsgBuffer->Seek(STREAM_SEEK_TO_BEGIN);
            }
            eState = INETMSG_EOL_SCR;
        }
    }
    return INETSTREAM_STATUS_OK;
}



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
    GenerateHeader(false);

    pMsgBuffer = new sal_uInt8[nMsgBufSiz];
    pMsgRead = pMsgWrite = pMsgBuffer;

    pTokBuffer = new sal_Char[nTokBufSiz];
    pTokRead = pTokWrite = pTokBuffer;
}

INetMessageEncode64Stream_Impl::~INetMessageEncode64Stream_Impl(void)
{
    delete pMsgStrm;
    delete [] pMsgBuffer;
    delete [] pTokBuffer;
}

int INetMessageEncode64Stream_Impl::GetMsgLine(sal_Char* pData, sal_uIntPtr nSize)
{
    INetMessage* pMsg = GetSourceMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    if (pMsg->GetDocumentLB() == NULL) return 0;
    if (pMsgStrm == NULL) pMsgStrm = new SvStream(pMsg->GetDocumentLB());

    sal_Char* pWBuf = pData;
    while (pWBuf < (pData + nSize))
    {
        
        if ((pMsgRead - pMsgWrite) > 0)
        {
            
            if ((pTokRead - pTokBuffer) < 72)
            {
                
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

                default: 
                    *pTokRead++ = six2pr[(int)(*pMsgWrite & 077)];
                    pMsgWrite++;
                    break;
                }
            }
            else if ((pTokRead - pTokBuffer) == 72)
            {
                
                *pTokRead++ = '\r';
                *pTokRead++ = '\n';
            }
            else
            {
                if ((pTokRead - pTokWrite) > 0)
                {
                    
                    *pWBuf++ = *pTokWrite++;
                }
                else
                {
                    
                    pTokRead = pTokWrite = pTokBuffer;
                }
            }
        }
        else
        {
            
            pMsgRead = pMsgWrite = pMsgBuffer;

            
            sal_uIntPtr nRead = pMsgStrm->Read(pMsgBuffer, nMsgBufSiz);
            if (nRead > 0)
            {
                
                pMsgRead = (pMsgBuffer + nRead);
            }
            else
            {
                
                if (!bDone)
                {
                    
                    switch ((pTokRead - pTokBuffer) % 4)
                    {
                    case 2:
                        *pTokRead++ = '=';
                        
                    case 3:
                        *pTokRead++ = '=';
                        break;

                    default:
                        break;
                    }
                    *pTokRead++ = '\r';
                    *pTokRead++ = '\n';

                    
                    bDone = true;
                }
                else
                {
                    
                    if ((pTokRead - pTokWrite) > 0)
                    {
                        
                        *pWBuf++ = *pTokWrite++;
                    }
                    else
                    {
                        
                        pTokRead = pTokWrite = pTokBuffer;

                        
                        

                        
                        return (pWBuf - pData);
                    }
                }
            }
        }
    } 
    return (pWBuf - pData);
}



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

INetMessageDecode64Stream_Impl::INetMessageDecode64Stream_Impl(
    sal_uIntPtr nMsgBufferSize)
    : INetMessageOStream(),
      eState     (INETMSG_EOL_SCR),
      nMsgBufSiz (nMsgBufferSize)
{
    ParseHeader(false);

    pMsgBuffer = new sal_Char[nMsgBufSiz];
    pMsgRead = pMsgWrite = pMsgBuffer;
}

INetMessageDecode64Stream_Impl::~INetMessageDecode64Stream_Impl(void)
{
    delete [] pMsgBuffer;
}

int INetMessageDecode64Stream_Impl::PutMsgLine(const sal_Char* pData,
                                               sal_uIntPtr nSize)
{
    INetMessage* pMsg = GetTargetMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    SvOpenLockBytes* pLB = PTR_CAST(SvOpenLockBytes, pMsg->GetDocumentLB());
    if (pLB == NULL) return INETSTREAM_STATUS_WOULDBLOCK;

    const sal_Char* pStop = (pData + nSize);
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
                
                sal_Size nDocSiz = pMsg->GetDocumentSize();
                sal_Size nRead   = pMsgWrite - pMsgBuffer;
                sal_Size nWrite  = 0;

                pLB->FillAppend(pMsgBuffer, nRead, &nWrite);
                pMsg->SetDocumentSize(nDocSiz + nWrite);

                if (nWrite < nRead)
                    return INETSTREAM_STATUS_ERROR;
                else
                    return INETSTREAM_STATUS_LOADED;
            }
            else if (eState == INETMSG_EOL_FCR)
            {
                
                if ((*pData == '\r') || (*pData == '\n')) pData++;

                
                sal_Size nDocSiz = pMsg->GetDocumentSize();
                sal_Size nRead   = pMsgWrite - pMsgBuffer;
                sal_Size nWrite  = 0;

                pLB->FillAppend(pMsgBuffer, nRead, &nWrite);
                pMsg->SetDocumentSize(nDocSiz + nWrite);

                if (nWrite < nRead) return INETSTREAM_STATUS_ERROR;

                
                pMsgWrite = pMsgBuffer;
                eState = INETMSG_EOL_SCR;
            }
            else if ((*pData == '\r') || (*pData == '\n'))
            {
                
                pData++;
                eState = INETMSG_EOL_FCR;
            }
            else
            {
                
                pData++;
            }
        }
        else
        {
            
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

            default: 
                *pMsgWrite++ |= (pr2six[(int)(*pData++)]);
                pMsgRead = pMsgBuffer;
                break;
            } 
        }
    } 
    return INETSTREAM_STATUS_OK;
}



INetMIMEMessageStream::INetMIMEMessageStream(sal_uIntPtr nBufferSize)
    : INetMessageIOStream(nBufferSize),
      eState      (INETMSG_EOL_BEGIN),
      nChildIndex (0),
      pChildStrm  (NULL),
      eEncoding   (INETMSG_ENCODING_BINARY),
      pEncodeStrm (NULL),
      pDecodeStrm (NULL),
      pMsgBuffer  (NULL)
{
}

INetMIMEMessageStream::~INetMIMEMessageStream(void)
{
    delete pChildStrm;
    delete pEncodeStrm;
    delete pDecodeStrm;
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


int INetMIMEMessageStream::GetMsgLine(sal_Char* pData, sal_uIntPtr nSize)
{
    
    INetMIMEMessage* pMsg = GetSourceMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    
    if (!IsHeaderGenerated())
    {
        if (eState == INETMSG_EOL_BEGIN)
        {
            
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

            
            OUString aContentType(pMsg->GetContentType());
            if (!aContentType.isEmpty())
            {
                
                OUString aDefaultType = pMsg->GetDefaultContentType();

                if (aDefaultType.equalsIgnoreAsciiCase(aContentType))
                {
                    
                    pMsg->SetContentType(OUString());
                }
            }

            
            OUString aEncoding(pMsg->GetContentTransferEncoding());
            if (!aEncoding.isEmpty())
            {
                
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
                
                if (aContentType.isEmpty())
                {
                    
                    aContentType = pMsg->GetDefaultContentType();
                }
                eEncoding = GetMsgEncoding(aContentType);
            }

            
            if (eEncoding == INETMSG_ENCODING_BASE64)
            {
                
                pMsg->SetContentTransferEncoding("base64");
            }
            else if (eEncoding == INETMSG_ENCODING_QUOTED)
            {
                
                pMsg->SetContentTransferEncoding("quoted-printable");
            }
            else
            {
                
                pMsg->SetContentTransferEncoding(OUString());
            }

            
            eState = INETMSG_EOL_DONE;
        }

        
        int nRead = INetMessageIOStream::GetMsgLine(pData, nSize);
        if (nRead <= 0)
        {
            
            eState = INETMSG_EOL_BEGIN;
        }
        return nRead;
    }
    else
    {
        
        if (pMsg->IsContainer())
        {
            
            while (eState == INETMSG_EOL_BEGIN)
            {
                if (pChildStrm == NULL)
                {
                    INetMIMEMessage *pChild = pMsg->GetChild(nChildIndex);
                    if (pChild)
                    {
                        
                        nChildIndex++;

                        
                        pChildStrm = new INetMIMEMessageStream;
                        pChildStrm->SetSourceMessage(pChild);

                        if (pMsg->IsMultipart())
                        {
                            
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
                        
                        eState = INETMSG_EOL_DONE;
                        nChildIndex = 0;

                        if (pMsg->IsMultipart())
                        {
                            
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
                    
                    int nRead = pChildStrm->Read(pData, nSize);
                    if (nRead > 0)
                    {
                        return nRead;
                    }
                    else
                    {
                        
                        delete pChildStrm;
                        pChildStrm = NULL;
                    }
                }
            }
            return 0;
        }
        else
        {
            
            if (pMsg->GetDocumentLB() == NULL)
            {
                
                return 0;
            }

            
            if (eEncoding == INETMSG_ENCODING_7BIT)
            {
                
                return INetMessageIOStream::GetMsgLine(pData, nSize);
            }

            
            while (eState == INETMSG_EOL_BEGIN)
            {
                if (pEncodeStrm == NULL)
                {
                    
                    if (eEncoding == INETMSG_ENCODING_QUOTED)
                    {
                        
                        pEncodeStrm = new INetMessageEncodeQPStream_Impl;
                    }
                    else
                    {
                        
                        pEncodeStrm = new INetMessageEncode64Stream_Impl;
                    }
                    pEncodeStrm->SetSourceMessage(pMsg);
                }

                
                int nRead = pEncodeStrm->Read(pData, nSize);
                if (nRead > 0)
                {
                    return nRead;
                }
                else
                {
                    
                    delete pEncodeStrm;
                    pEncodeStrm = NULL;

                    
                    eState = INETMSG_EOL_DONE;
                }
            }
            return 0;
        }
    }
}


int INetMIMEMessageStream::PutMsgLine(const sal_Char* pData, sal_uIntPtr nSize)
{
    
    INetMIMEMessage* pMsg = GetTargetMessage();
    if (pMsg == NULL) return INETSTREAM_STATUS_ERROR;

    
    if (!IsHeaderParsed())
    {
        
        int nRet = INetMessageIOStream::PutMsgLine(pData, nSize);
        return nRet;
    }
    else
    {
        pMsg->SetHeaderParsed();
        
        if (pMsg->IsContainer())
        {

            
            if (pMsg->IsMessage())
            {
                if( !pChildStrm )
                {
                    
                    INetMIMEMessage* pNewMessage = new INetMIMEMessage;
                    pNewMessage->SetDocumentLB( new SvAsyncLockBytes(new SvMemoryStream(), false));
                    pMsg->AttachChild( *pNewMessage, true );

                    
                    pChildStrm = new INetMIMEMessageStream;
                    pChildStrm->SetTargetMessage( pNewMessage );

                    
                    eState = INETMSG_EOL_BEGIN;
                }
                if ( nSize > 0)
                {
                    
                    int status = pChildStrm->Write( pData, nSize );
                    if (status != INETSTREAM_STATUS_OK)
                        return status;
                }

                return INetMessageIOStream::PutMsgLine(pData, nSize);
            }
            else
            {

                
                
                if (pMsg->GetMultipartBoundary().getLength() == 0)
                {
                    
                    OString aType(OUStringToOString(
                        pMsg->GetContentType(), RTL_TEXTENCODING_ASCII_US));
                    OString aLowerType(aType.toAsciiLowerCase());

                    sal_Int32 nPos = aLowerType.indexOf("boundary=");
                    OString aBoundary(aType.copy(nPos + 9));

                    aBoundary = comphelper::string::strip(aBoundary, ' ');
                    aBoundary = comphelper::string::strip(aBoundary, '"');

                    
                    pMsg->SetMultipartBoundary(aBoundary);
                }

                OString aPlainDelim(pMsg->GetMultipartBoundary());
                OString aDelim = OStringBuffer("--").
                    append(aPlainDelim).
                    makeStringAndClear();
                OString aPlainClose = OStringBuffer(
                    aPlainDelim).
                    append("--").
                    makeStringAndClear();
                OString aClose = OStringBuffer(
                    aDelim).
                    append("--").
                    makeStringAndClear();

                if (pMsgBuffer == NULL) pMsgBuffer = new SvMemoryStream;
                pMsgBuffer->Write(pData, nSize);
                sal_uIntPtr nBufSize = pMsgBuffer->Tell();

                const sal_Char* pChar;
                const sal_Char* pOldPos;
                int status;
                for( pOldPos = pChar = (const sal_Char*) pMsgBuffer->GetData(); nBufSize--;
                     pChar++ )
                {
                    if( *pChar == '\r' || *pChar == '\n' )
                    {
                        if( aDelim.compareTo(pOldPos, aDelim.getLength())
                            != -1 &&
                            aClose.compareTo(pOldPos, aClose.getLength())
                            != -1 &&
                            aPlainDelim.compareTo(pOldPos, aPlainDelim.getLength())
                            != -1 &&
                            aPlainClose.compareTo(pOldPos, aPlainClose.getLength())
                            != -1 )
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
                            else {
                                SAL_WARN( "tools.stream", "Boundary not found." );
                            }
                            status = INetMessageIOStream::PutMsgLine(
                                pOldPos, pChar - pOldPos + 1 );
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

                            if (aClose.compareTo(pOldPos, aClose.getLength())
                                != -1 &&
                                aPlainClose.compareTo(pOldPos, aClose.getLength())
                                != -1 )
                            {
                                
                                INetMIMEMessage* pNewMessage =
                                    new INetMIMEMessage;
                                pNewMessage->SetDocumentLB(
                                    new SvAsyncLockBytes(
                                        new SvMemoryStream(), false));

                                pMsg->AttachChild( *pNewMessage, true );

                                
                                pChildStrm = new INetMIMEMessageStream;
                                pChildStrm->SetTargetMessage( pNewMessage );

                                
                            }
                            eState = INETMSG_EOL_BEGIN;
                            status = INetMessageIOStream::PutMsgLine(
                                pOldPos, pChar - pOldPos + 1 );
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
            if (pMsg->GetContentType().isEmpty())
            {
                pMsg->SetContentType(pMsg->GetDefaultContentType());
            }

            if (eEncoding == INETMSG_ENCODING_BINARY)
            {
                OUString aEncoding(pMsg->GetContentTransferEncoding());
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

            if (eEncoding == INETMSG_ENCODING_7BIT)
            {
                
                return INetMessageIOStream::PutMsgLine(pData, nSize);
            }
            else
            {
                if (pDecodeStrm == NULL)
                {
                    if (eEncoding == INETMSG_ENCODING_QUOTED)
                    {
                        pDecodeStrm = new INetMessageDecodeQPStream_Impl;
                    }
                    else
                    {
                        pDecodeStrm = new INetMessageDecode64Stream_Impl;
                    }
                    pDecodeStrm->SetTargetMessage(pMsg);
                }
                return pDecodeStrm->Write(pData, nSize);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
