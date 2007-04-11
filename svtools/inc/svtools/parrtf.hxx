/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parrtf.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:30:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _PARRTF_HXX
#define _PARRTF_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SVPARSER_HXX
#include <svtools/svparser.hxx>
#endif
#ifndef _SVARRAY_HXX
#include <svtools/svarray.hxx>
#endif

struct RtfParserState_Impl
{
    rtl_TextEncoding eCodeSet;
    BYTE nUCharOverread;

    RtfParserState_Impl( BYTE nUOverread, rtl_TextEncoding eCdSt )
        : eCodeSet( eCdSt ), nUCharOverread( nUOverread )
    {}
};

SV_DECL_VARARR( RtfParserStates_Impl, RtfParserState_Impl, 16, 16 )

class SVT_DLLPUBLIC SvRTFParser : public SvParser
{
    RtfParserStates_Impl aParserStates;

    int nOpenBrakets;
    rtl_TextEncoding eCodeSet, eUNICodeSet;
    BYTE nUCharOverread;

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
    SvRTFParser( SvStream& rIn, BYTE nStackSize = 3 );

    virtual SvParserState CallParser();   // Aufruf des Parsers

    int GetOpenBrakets() const { return nOpenBrakets; }

    // fuers asynchrone lesen aus dem SvStream
//  virtual void SaveState( int nToken );
//  virtual void RestoreState();
    virtual void Continue( int nToken );
};

#endif //_PARRTF_HXX
