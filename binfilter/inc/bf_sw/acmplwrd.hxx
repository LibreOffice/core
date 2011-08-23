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
#ifndef _ACMPLWRD_HXX
#define _ACMPLWRD_HXX


#define _SVSTDARR_STRINGSISORTDTOR
#include <bf_svtools/svstdarr.hxx>
namespace binfilter {

class SwDoc;
class SwAutoCompleteWord_Impl;
class SwAutoCompleteClient;

class SwAutoCompleteWord
{
    friend class SwAutoCompleteClient;

    SvStringsISortDtor aWordLst; // contains extended strings carrying source information
    SvPtrarr aLRULst;

    SwAutoCompleteWord_Impl* pImpl;
    USHORT nMaxCount, nMinWrdLen;
    BOOL bLockWordLst;

    void DocumentDying(const SwDoc& rDoc);
public:
    SwAutoCompleteWord( USHORT nWords = 500, USHORT nMWrdLen = 10 );
    ~SwAutoCompleteWord();

    BOOL InsertWord( const String& rWord, SwDoc& rDoc );



    USHORT Count() const { return aWordLst.Count(); }

    const String& operator[]( USHORT n ) const { return *aWordLst[ n ]; }

    BOOL IsLockWordLstLocked() const 		{ return bLockWordLst; }
    void SetLockWordLstLocked( BOOL bFlag ) { bLockWordLst = bFlag; }

    USHORT GetMaxCount() const 				{ return nMaxCount; }

    USHORT GetMinWordLen() const 				{ return nMinWrdLen; }

    const SvStringsISortDtor& GetWordList() const { return aWordLst; }
};


} //namespace binfilter
#endif
