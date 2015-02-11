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
#ifndef INCLUDED_TOOLS_INETSTRM_HXX
#define INCLUDED_TOOLS_INETSTRM_HXX

#include <tools/toolsdllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>

class INetRFC822Message;
class INetMIMEMessage;
class SvMemoryStream;
class SvStream;

enum INetStreamStatus
{
    INETSTREAM_STATUS_LOADED     = -4,
    INETSTREAM_STATUS_WOULDBLOCK = -3,
    INETSTREAM_STATUS_OK         = -2,
    INETSTREAM_STATUS_ERROR      = -1
};

class TOOLS_DLLPUBLIC INetIStream
{
    INetIStream (const INetIStream& rStrm) SAL_DELETED_FUNCTION;
    INetIStream& operator= (const INetIStream& rStrm) SAL_DELETED_FUNCTION;

protected:
    virtual int GetData (sal_Char *pData, sal_uIntPtr nSize) = 0;

public:
    INetIStream ();
    virtual ~INetIStream (void);

    int Read (sal_Char *pData, sal_uIntPtr nSize);
};

class INetOStream
{
    INetOStream (const INetOStream& rStrm) SAL_DELETED_FUNCTION;
    INetOStream& operator= (const INetOStream& rStrm) SAL_DELETED_FUNCTION;

protected:
    virtual int PutData (
        const sal_Char *pData, sal_uIntPtr nSize) = 0;

public:
    INetOStream ();
    virtual ~INetOStream (void);

    int Write (const sal_Char *pData, sal_uIntPtr nSize);
};

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

/// Message Generator Interface.
class INetMessageIStream : public INetIStream
{
    INetRFC822Message *pSourceMsg;
    bool            bHeaderGenerated;

    sal_uIntPtr           nBufSiz;
    sal_Char       *pBuffer;
    sal_Char       *pRead;
    sal_Char       *pWrite;

    SvStream       *pMsgStrm;
    SvMemoryStream *pMsgBuffer;
    sal_Char       *pMsgRead;
    sal_Char       *pMsgWrite;

    virtual int GetData (sal_Char *pData, sal_uIntPtr nSize) SAL_OVERRIDE;

    INetMessageIStream (const INetMessageIStream& rStrm) SAL_DELETED_FUNCTION;
    INetMessageIStream& operator= (const INetMessageIStream& rStrm) SAL_DELETED_FUNCTION;

protected:
    virtual int GetMsgLine (sal_Char *pData, sal_uIntPtr nSize);

public:
    INetMessageIStream (sal_uIntPtr nBufferSize = 2048);
    virtual ~INetMessageIStream (void);

    INetRFC822Message *GetSourceMessage (void) const { return pSourceMsg; }
    void SetSourceMessage (INetRFC822Message *pMsg) { pSourceMsg = pMsg; }

    void GenerateHeader (bool bGen = true) { bHeaderGenerated = !bGen; }
    bool IsHeaderGenerated (void) const { return bHeaderGenerated; }
};

/// Message Parser Interface.
class INetMessageOStream : public INetOStream
{
    INetRFC822Message      *pTargetMsg;
    bool                    bHeaderParsed;

    INetMessageStreamState  eOState;

    SvMemoryStream         *pMsgBuffer;

    virtual int PutData (const sal_Char *pData, sal_uIntPtr nSize) SAL_OVERRIDE;

    INetMessageOStream (const INetMessageOStream& rStrm) SAL_DELETED_FUNCTION;
    INetMessageOStream& operator= (const INetMessageOStream& rStrm) SAL_DELETED_FUNCTION;

protected:
    virtual int PutMsgLine (const sal_Char *pData, sal_uIntPtr nSize);

public:
    INetMessageOStream (void);
    virtual ~INetMessageOStream (void);

    INetRFC822Message *GetTargetMessage (void) const { return pTargetMsg; }
    void SetTargetMessage (INetRFC822Message *pMsg) { pTargetMsg = pMsg; }

    void ParseHeader (bool bParse = true) { bHeaderParsed = !bParse; }
    bool IsHeaderParsed (void) const { return bHeaderParsed; }
};

class INetMessageIOStream
    : public INetMessageIStream,
      public INetMessageOStream
{
    INetMessageIOStream (const INetMessageIOStream& rStrm) SAL_DELETED_FUNCTION;
    INetMessageIOStream& operator= (const INetMessageIOStream& rStrm) SAL_DELETED_FUNCTION;

public:
    INetMessageIOStream (sal_uIntPtr nBufferSize = 2048);
    virtual ~INetMessageIOStream (void);
};

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
        const OUString& rContentType);

    INetMIMEMessageStream (const INetMIMEMessageStream& rStrm) SAL_DELETED_FUNCTION;
    INetMIMEMessageStream& operator= (const INetMIMEMessageStream& rStrm) SAL_DELETED_FUNCTION;

protected:
    virtual int GetMsgLine (sal_Char *pData, sal_uIntPtr nSize) SAL_OVERRIDE;
    virtual int PutMsgLine (const sal_Char *pData, sal_uIntPtr nSize) SAL_OVERRIDE;

public:
    INetMIMEMessageStream (sal_uIntPtr nBufferSize = 2048);
    virtual ~INetMIMEMessageStream (void);

    using INetMessageIStream::SetSourceMessage;
    void SetSourceMessage (INetMIMEMessage *pMsg)
    {
        INetMessageIStream::SetSourceMessage ((INetRFC822Message *)pMsg);
    }
    INetMIMEMessage *GetSourceMessage (void) const
    {
        return static_cast<INetMIMEMessage *>(INetMessageIStream::GetSourceMessage());
    }

    using INetMessageOStream::SetTargetMessage;
    void SetTargetMessage (INetMIMEMessage *pMsg)
    {
        INetMessageOStream::SetTargetMessage ((INetRFC822Message *)pMsg);
    }
    INetMIMEMessage *GetTargetMessage (void) const
    {
        return static_cast<INetMIMEMessage *>(INetMessageOStream::GetTargetMessage());
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
