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
#pragma once

#include <tools/toolsdllapi.h>
#include <tools/stream.hxx>
#include <sal/types.h>
#include <vector>
#include <memory>
#include <config_options.h>

class INetMIMEMessage;

class UNLESS_MERGELIBS(TOOLS_DLLPUBLIC) INetMIMEMessageStream
{
    INetMIMEMessage *pSourceMsg;
    bool            bHeaderGenerated;

    std::vector<char> mvBuffer;
    char           *pRead;
    char           *pWrite;

    SvMemoryStream  maMsgBuffer;
    char           *pMsgRead;
    char           *pMsgWrite;

    bool done;

    sal_uInt32             nChildIndex;
    std::unique_ptr<INetMIMEMessageStream> pChildStrm;

    INetMIMEMessageStream (const INetMIMEMessageStream& rStrm) = delete;
    INetMIMEMessageStream& operator= (const INetMIMEMessageStream& rStrm) = delete;

    int GetHeaderLine(char *pData, sal_uInt32 nSize);
    int GetBodyLine(char *pData, sal_uInt32 nSize);
    int GetMsgLine(char *pData, sal_uInt32 nSize);

public:
    explicit INetMIMEMessageStream(INetMIMEMessage *pMsg, bool headerGenerated);
    ~INetMIMEMessageStream();

    int Read (char *pData, sal_uInt32 nSize);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
