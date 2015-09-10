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

class INetMIMEMessage;
class SvMemoryStream;
class SvStream;

enum INetStreamStatus
{
    INETSTREAM_STATUS_ERROR      = -1
};

enum INetMessageStreamState
{
    INETMSG_EOL_BEGIN,
    INETMSG_EOL_DONE
};

class TOOLS_DLLPUBLIC INetMIMEMessageStream
{
    INetMIMEMessage *pSourceMsg;
    bool            bHeaderGenerated;

    sal_uIntPtr           nBufSiz;
    sal_Char       *pBuffer;
    sal_Char       *pRead;
    sal_Char       *pWrite;

    SvStream       *pMsgStrm;
    SvMemoryStream *pMsgBuffer;
    sal_Char       *pMsgRead;
    sal_Char       *pMsgWrite;

    int                    eState;

    sal_uIntPtr                  nChildIndex;
    INetMIMEMessageStream *pChildStrm;

    INetMIMEMessageStream (const INetMIMEMessageStream& rStrm) SAL_DELETED_FUNCTION;
    INetMIMEMessageStream& operator= (const INetMIMEMessageStream& rStrm) SAL_DELETED_FUNCTION;

    int GetInnerMsgLine(sal_Char *pData, sal_uIntPtr nSize);
    int GetOuterMsgLine(sal_Char *pData, sal_uIntPtr nSize);

public:
    INetMIMEMessageStream (sal_uIntPtr nBufferSize = 2048);
    ~INetMIMEMessageStream();

    int Read (sal_Char *pData, sal_uIntPtr nSize);

    INetMIMEMessage *GetSourceMessage() const { return pSourceMsg; }
    void SetSourceMessage (INetMIMEMessage *pMsg) { pSourceMsg = pMsg; }

    void SetHeaderGenerated() { bHeaderGenerated = true; }
    bool IsHeaderGenerated() const { return bHeaderGenerated; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
