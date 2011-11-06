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


#ifndef _TXATRITR_HXX
#define _TXATRITR_HXX

#include <tools/solar.h>
#include <sal/types.h>
#include <svl/svarray.hxx>
#include <editeng/langitem.hxx>
#include <hintids.hxx>

class String;
class SwTxtNode;
class SwTxtAttr;
class SfxPoolItem;


class SwScriptIterator
{
    const String& rText;
    xub_StrLen nChgPos;
    sal_uInt16 nCurScript;
    sal_Bool bForward;

public:
    SwScriptIterator( const String& rStr, xub_StrLen nStart = 0,
                      sal_Bool bFrwrd = sal_True );

    sal_Bool Next();

    sal_uInt16 GetCurrScript() const        { return nCurScript; }
    xub_StrLen GetScriptChgPos() const      { return nChgPos; }
    const String& GetText() const           { return rText; }
};


class SwTxtAttrIterator
{
    SwScriptIterator aSIter;
    SvPtrarr aStack;
    const SwTxtNode& rTxtNd;
    const SfxPoolItem *pParaItem, *pCurItem;
    xub_StrLen nChgPos;
    sal_uInt16 nAttrPos, nWhichId;
    sal_Bool bIsUseGetWhichOfScript;

    void AddToStack( const SwTxtAttr& rAttr );
    void SearchNextChg();

public:
    SwTxtAttrIterator( const SwTxtNode& rTxtNd, sal_uInt16 nWhichId,
                        xub_StrLen nStart = 0, sal_Bool bUseGetWhichOfScript = sal_True );

    sal_Bool Next();

    const SfxPoolItem& GetAttr() const  { return *pCurItem; }
    xub_StrLen GetChgPos() const        { return nChgPos; }
};


class SwLanguageIterator : public SwTxtAttrIterator
{
public:
    SwLanguageIterator( const SwTxtNode& rTxtNode, xub_StrLen nStart = 0,
                        sal_uInt16 nWhich = RES_CHRATR_LANGUAGE,
                        sal_Bool bUseGetWhichOfScript = sal_True )
        : SwTxtAttrIterator( rTxtNode, nWhich, nStart, bUseGetWhichOfScript )
    {}

    sal_uInt16 GetLanguage() const
        { return ((SvxLanguageItem&)GetAttr()).GetValue(); }
};


#endif
