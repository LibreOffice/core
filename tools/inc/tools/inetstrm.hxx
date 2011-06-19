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
#ifndef _TOOLS_INETSTRM_HXX
#define _TOOLS_INETSTRM_HXX

#include "tools/toolsdllapi.h"
#include <sal/types.h>
#include <tools/string.hxx>

class INetMessage;
class INetMIMEMessage;
class INetHTTPMessage;
class SvMemoryStream;
class SvStream;

/*=========================================================================
 *
 * INetStream Interface.
 *
 *=======================================================================*/
enum INetStreamStatus
{
    INETSTREAM_STATUS_LOADED     = -4,
    INETSTREAM_STATUS_WOULDBLOCK = -3,
    INETSTREAM_STATUS_OK         = -2,
    INETSTREAM_STATUS_ERROR      = -1
};

/*
 * INetIStream.
 */
class TOOLS_DLLPUBLIC INetIStream
{
    // Not implemented.
    INetIStream (const INetIStream& rStrm);
    INetIStream& operator= (const INetIStream& rStrm);

protected:
    virtual int GetData (sal_Char *pData, sal_uIntPtr nSize) = 0;

public:
    INetIStream ();
    virtual ~INetIStream (void);

    int Read (sal_Char *pData, sal_uIntPtr nSize);

    static void Decode64 (SvStream& rIn, SvStream& rOut);
    static void Encode64 (SvStream& rIn, SvStream& rOut);
};

/*
 * INetOStream.
 */
class INetOStream
{
    // Not implemented.
    INetOStream (const INetOStream& rStrm);
    INetOStream& operator= (const INetOStream& rStrm);

protected:
    virtual int PutData (
        const sal_Char *pData, sal_uIntPtr nSize) = 0;

public:
    INetOStream ();
    virtual ~INetOStream (void);

    int Write (const sal_Char *pData, sal_uIntPtr nSize);
};

/*
 * INetIOStream.
 */
class INetIOStream : public INetIStream, public INetOStream
{
    // Not implemented.
    INetIOStream (const INetIOStream& rStrm);
    INetIOStream& operator= (const INetIOStream& rStrm);

public:
    INetIOStream (sal_uIntPtr nIBufferSize = 0, sal_uIntPtr nOBufferSize = 0);
    virtual ~INetIOStream (void);
};

/*=========================================================================
 *
 * INetMessageStream Interface.
 *
 *=======================================================================*/
enum INetMessageStreamState
{
    INETMSG_EOL_BEGIN,
    INETMSG_EOL_DONE,
    INETMSG_EOL_SCR,
    INETMSG_EOL_FCR,
    INETMSG_EOL_FLF,
    INETMSG_EOL_FSP,
    INETMSG_EOL_FESC
};

/*
 * INetMessageIStream (Message Generator) Interface.
 */
class INetMessageIStream : public INetIStream
{
    INetMessage    *pSourceMsg;
    sal_Bool            bHeaderGenerated;

    sal_uIntPtr           nBufSiz;
    sal_Char       *pBuffer;
    sal_Char       *pRead;
    sal_Char       *pWrite;

    SvStream       *pMsgStrm;
    SvMemoryStream *pMsgBuffer;
    sal_Char       *pMsgRead;
    sal_Char       *pMsgWrite;

    virtual int GetData (sal_Char *pData, sal_uIntPtr nSize);

    // Not implemented.
    INetMessageIStream (const INetMessageIStream& rStrm);
    INetMessageIStream& operator= (const INetMessageIStream& rStrm);

protected:
    virtual int GetMsgLine (sal_Char *pData, sal_uIntPtr nSize);

public:
    INetMessageIStream (sal_uIntPtr nBufferSize = 2048);
    virtual ~INetMessageIStream (void);

    INetMessage *GetSourceMessage (void) const { return pSourceMsg; }
    void SetSourceMessage (INetMessage *pMsg) { pSourceMsg = pMsg; }

    void GenerateHeader (sal_Bool bGen = sal_True) { bHeaderGenerated = !bGen; }
    sal_Bool IsHeaderGenerated (void) const { return bHeaderGenerated; }
};

/*
 * INetMessageOStream (Message Parser) Interface.
 */
class INetMessageOStream : public INetOStream
{
    INetMessage            *pTargetMsg;
    sal_Bool                    bHeaderParsed;

    INetMessageStreamState  eOState;

    SvMemoryStream         *pMsgBuffer;

    virtual int PutData (const sal_Char *pData, sal_uIntPtr nSize);

    // Not implemented.
    INetMessageOStream (const INetMessageOStream& rStrm);
    INetMessageOStream& operator= (const INetMessageOStream& rStrm);

protected:
    virtual int PutMsgLine (const sal_Char *pData, sal_uIntPtr nSize);

public:
    INetMessageOStream (void);
    virtual ~INetMessageOStream (void);

    INetMessage *GetTargetMessage (void) const { return pTargetMsg; }
    void SetTargetMessage (INetMessage *pMsg) { pTargetMsg = pMsg; }

    void ParseHeader (sal_Bool bParse = sal_True) { bHeaderParsed = !bParse; }
    sal_Bool IsHeaderParsed (void) const { return bHeaderParsed; }
};

/*
 * INetMessageIOStream Interface.
 */
class INetMessageIOStream
    : public INetMessageIStream,
      public INetMessageOStream
{
    // Not implemented.
    INetMessageIOStream (const INetMessageIOStream& rStrm);
    INetMessageIOStream& operator= (const INetMessageIOStream& rStrm);

public:
    INetMessageIOStream (sal_uIntPtr nBufferSize = 2048);
    virtual ~INetMessageIOStream (void);
};

/*=========================================================================
 *
 * INetMIMEMessageStream Interface.
 *
 *=======================================================================*/
enum INetMessageEncoding
{
    INETMSG_ENCODING_7BIT,
    INETMSG_ENCODING_8BIT,
    INETMSG_ENCODING_BINARY,
    INETMSG_ENCODING_QUOTED,
    INETMSG_ENCODING_BASE64
};

class TOOLS_DLLPUBLIC INetMIMEMessageStream : public INetMessageIOStream
{
    int                    eState;

    sal_uIntPtr                  nChildIndex;
    INetMIMEMessageStream *pChildStrm;

    INetMessageEncoding    eEncoding;
    INetMessageIStream    *pEncodeStrm;
    INetMessageOStream    *pDecodeStrm;

    SvMemoryStream        *pMsgBuffer;

    static INetMessageEncoding GetMsgEncoding (
        const String& rContentType);

    // Not implemented.
    INetMIMEMessageStream (const INetMIMEMessageStream& rStrm);
    INetMIMEMessageStream& operator= (const INetMIMEMessageStream& rStrm);

protected:
    virtual int GetMsgLine (sal_Char *pData, sal_uIntPtr nSize);
    virtual int PutMsgLine (const sal_Char *pData, sal_uIntPtr nSize);

public:
    INetMIMEMessageStream (sal_uIntPtr nBufferSize = 2048);
    virtual ~INetMIMEMessageStream (void);

    using INetMessageIStream::SetSourceMessage;
    void SetSourceMessage (INetMIMEMessage *pMsg)
    {
        INetMessageIStream::SetSourceMessage ((INetMessage *)pMsg);
    }
    INetMIMEMessage *GetSourceMessage (void) const
    {
        return ((INetMIMEMessage *)INetMessageIStream::GetSourceMessage());
    }

    using INetMessageOStream::SetTargetMessage;
    void SetTargetMessage (INetMIMEMessage *pMsg)
    {
        INetMessageOStream::SetTargetMessage ((INetMessage *)pMsg);
    }
    INetMIMEMessage *GetTargetMessage (void) const
    {
        return ((INetMIMEMessage *)INetMessageOStream::GetTargetMessage());
    }
};

#endif /* !_TOOLS_INETSTRM_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
