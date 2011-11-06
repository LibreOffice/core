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


#ifndef _ACMPLWRD_HXX
#define _ACMPLWRD_HXX


#define _SVSTDARR_STRINGSISORTDTOR
#include <svl/svstdarr.hxx>

class SwDoc;
class SwAutoCompleteWord_Impl;
class SwAutoCompleteClient;

class SwAutoCompleteWord
{
    friend class SwAutoCompleteClient;

    SvStringsISortDtor aWordLst; // contains extended strings carrying source information
    SvPtrarr aLRULst;

    SwAutoCompleteWord_Impl* pImpl;
    sal_uInt16 nMaxCount, nMinWrdLen;
    sal_Bool bLockWordLst;

    void DocumentDying(const SwDoc& rDoc);
public:
    SwAutoCompleteWord( sal_uInt16 nWords = 500, sal_uInt16 nMWrdLen = 10 );
    ~SwAutoCompleteWord();

    sal_Bool InsertWord( const String& rWord, SwDoc& rDoc );

    sal_Bool GetRange( const String& rWord, sal_uInt16& rStt, sal_uInt16& rEnd ) const;

    const String& operator[]( sal_uInt16 n ) const { return *aWordLst[ n ]; }

    sal_Bool IsLockWordLstLocked() const        { return bLockWordLst; }
    void SetLockWordLstLocked( sal_Bool bFlag ) { bLockWordLst = bFlag; }

    void SetMaxCount( sal_uInt16 n );

    sal_uInt16 GetMinWordLen() const                { return nMinWrdLen; }
    void SetMinWordLen( sal_uInt16 n );

    const SvStringsISortDtor& GetWordList() const { return aWordLst; }
    void CheckChangedList( const SvStringsISortDtor& rNewLst );
};


#endif
