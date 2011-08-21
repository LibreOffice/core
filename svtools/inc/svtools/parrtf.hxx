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

#ifndef _PARRTF_HXX
#define _PARRTF_HXX

#include "svtools/svtdllapi.h"
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

typedef std::stack< RtfParserState_Impl > RtfParserStates_Impl;

class SVT_DLLPUBLIC SvRTFParser : public SvParser
{
    RtfParserStates_Impl aParserStates;

    int nOpenBrakets;
    rtl_TextEncoding eCodeSet, eUNICodeSet;
    sal_uInt8 nUCharOverread;

private:
    static short _inSkipGroup;

protected:
    sal_Unicode GetHexValue();
    void ScanText( const sal_Unicode = 0 );
    void SkipGroup();

    // scanne das naechste Token,
    virtual int _GetNextToken();

    virtual void ReadUnknownData();
    virtual void ReadBitmapData();
    virtual void ReadOLEData();

    virtual ~SvRTFParser();

    rtl_TextEncoding GetCodeSet() const             { return eCodeSet; }
    void SetEncoding( rtl_TextEncoding eEnc );

    rtl_TextEncoding GetUNICodeSet() const          { return eUNICodeSet; }
    void SetUNICodeSet( rtl_TextEncoding eSet )     { eUNICodeSet = eSet; }

public:
    SvRTFParser( SvStream& rIn, sal_uInt8 nStackSize = 3 );

    virtual SvParserState CallParser();   // Aufruf des Parsers

    int GetOpenBrakets() const { return nOpenBrakets; }

    // fuers asynchrone lesen aus dem SvStream
//  virtual void SaveState( int nToken );
//  virtual void RestoreState();
    virtual void Continue( int nToken );
};

#endif //_PARRTF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
