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


#ifndef _GUESS_HXX
#define _GUESS_HXX
#include <com/sun/star/linguistic2/XHyphenatedWord.hpp>

#include "txttypes.hxx"
#include "breakit.hxx"
#include "porrst.hxx"   // SwHangingPortion

class SwTxtFormatInfo;

/*************************************************************************
 *                      class SwTxtGuess
 *************************************************************************/

class SwTxtGuess
{
    ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord >  xHyphWord;
    SwHangingPortion *pHanging; // for hanging punctuation
    xub_StrLen nCutPos;         // this character doesn't fit
    xub_StrLen nBreakStart;     // start index of word containing line break
    xub_StrLen nBreakPos;       // start index of break position
    xub_StrLen nFieldDiff;      // absolut positions can be wrong if we
                                // a field in the text has been expanded
    KSHORT nBreakWidth;         // width of the broken portion
public:
    inline SwTxtGuess(): pHanging( NULL ), nCutPos(0), nBreakStart(0),
                        nBreakPos(0), nFieldDiff(0), nBreakWidth(0)
        { }
    ~SwTxtGuess() { delete pHanging; }

    // true, if current portion still fits to current line
    sal_Bool Guess( const SwTxtPortion& rPor, SwTxtFormatInfo &rInf,
                    const KSHORT nHeight );
    sal_Bool AlternativeSpelling( const SwTxtFormatInfo &rInf, const xub_StrLen nPos );

    inline SwHangingPortion* GetHangingPortion() const { return pHanging; }
    inline void ClearHangingPortion() { pHanging = NULL; }
    inline KSHORT BreakWidth() const { return nBreakWidth; }
    inline xub_StrLen CutPos() const { return nCutPos; }
    inline xub_StrLen BreakStart() const { return nBreakStart; }
    inline xub_StrLen BreakPos() const {return nBreakPos; }
    inline xub_StrLen FieldDiff() const {return nFieldDiff; }
    inline ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XHyphenatedWord > HyphWord() const
        { return xHyphWord; }
};

#endif
