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
#if 1

#include <tools/stream.hxx>

// -----------------------------------------------------------------------

        //  "Automatischer" Record-Header mit Groessenangabe

/*                      wird fuer SvNumberFormatter nicht gebraucht
class SvNumReadHeader
{
private:
    SvStream&   rStream;
    sal_uLong       nDataEnd;

public:
    SvNumReadHeader(SvStream& rNewStream);
    ~SvNumReadHeader();

    sal_uLong   BytesLeft() const;
};

class SvNumWriteHeader
{
private:
    SvStream&   rStream;
    sal_uLong       nDataPos;
    sal_uLong       nDataSize;

public:
    SvNumWriteHeader(SvStream& rNewStream, sal_uLong nDefault = 0);
    ~SvNumWriteHeader();
};

*/

        //  Header mit Groessenangaben fuer mehrere Objekte

class ImpSvNumMultipleReadHeader
{
private:
    SvStream&       rStream;
    char*           pBuf;
    SvMemoryStream* pMemStream;
    sal_uLong           nEndPos;
    sal_uLong           nEntryEnd;

public:
    ImpSvNumMultipleReadHeader(SvStream& rNewStream);
    ~ImpSvNumMultipleReadHeader();

    void    StartEntry();
    void    EndEntry();
    sal_uLong   BytesLeft() const;
};

class ImpSvNumMultipleWriteHeader
{
private:
    SvStream&       rStream;
    SvMemoryStream  aMemStream;
    sal_uLong           nDataPos;
    sal_uInt32      nDataSize;
    sal_uLong           nEntryStart;

public:
    ImpSvNumMultipleWriteHeader(SvStream& rNewStream, sal_uLong nDefault = 0);
    ~ImpSvNumMultipleWriteHeader();

    void    StartEntry();
    void    EndEntry();
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
