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

#ifndef INCLUDED_SVTOOLS_PARRTF_HXX
#define INCLUDED_SVTOOLS_PARRTF_HXX

#include <svtools/svtdllapi.h>
#include <svtools/svparser.hxx>
#include <stack>

struct RtfParserState_Impl
{
    rtl_TextEncoding eCodeSet;
    sal_uInt8 nUCharOverread;

    RtfParserState_Impl( sal_uInt8 nUOverread, rtl_TextEncoding eCdSt )
        : eCodeSet( eCdSt ), nUCharOverread( nUOverread )
    {}
};

class SVT_DLLPUBLIC SvRTFParser : public SvParser
{
    std::stack< RtfParserState_Impl > aParserStates;
    int              nOpenBrakets;
    rtl_TextEncoding eCodeSet,
                     eUNICodeSet;
    sal_uInt8        nUCharOverread;

private:
    static short _inSkipGroup;

protected:
    sal_Unicode GetHexValue();
    void ScanText( const sal_Unicode = 0 );
    void SkipGroup();

    // scanne das naechste Token,
    virtual int _GetNextToken() override;

    void ReadUnknownData();
    virtual void ReadBitmapData();
    virtual void ReadOLEData();

    virtual ~SvRTFParser();

    rtl_TextEncoding GetCodeSet() const             { return eCodeSet; }
    void SetEncoding( rtl_TextEncoding eEnc );

public:
    SvRTFParser( SvStream& rIn, sal_uInt8 nStackSize = 3 );

    virtual SvParserState CallParser() override;   // Aufruf des Parsers

    // fuers asynchrone lesen aus dem SvStream
    virtual void Continue( int nToken ) override;
};

#endif // INCLUDED_SVTOOLS_PARRTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
