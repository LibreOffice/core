/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
