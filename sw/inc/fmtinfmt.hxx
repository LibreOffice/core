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


#ifndef _FMTINFMT_HXX
#define _FMTINFMT_HXX

#include <tools/string.hxx>
#include <svl/poolitem.hxx>
#include "swdllapi.h"

class SvxMacro;
class SvxMacroTableDtor;
class SwTxtINetFmt;
class IntlWrapper;

// ATT_INETFMT *********************************************

class SW_DLLPUBLIC SwFmtINetFmt: public SfxPoolItem
{
    friend class SwTxtINetFmt;

    String aURL;                    // die URL
    String aTargetFrame;            // in diesen Frame soll die URL
    String aINetFmt;
    String aVisitedFmt;
    String aName;                   // Name des Links
    SvxMacroTableDtor* pMacroTbl;
    SwTxtINetFmt* pTxtAttr;         // mein TextAttribut
    sal_uInt16 nINetId;
    sal_uInt16 nVisitedId;
public:
    POOLITEM_FACTORY()
    SwFmtINetFmt( const String& rURL, const String& rTarget );
    SwFmtINetFmt( const SwFmtINetFmt& rAttr );
    SwFmtINetFmt();                     // for TypeInfo
    virtual ~SwFmtINetFmt();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 );


    const SwTxtINetFmt* GetTxtINetFmt() const   { return pTxtAttr; }
    SwTxtINetFmt* GetTxtINetFmt()               { return pTxtAttr; }

    const String& GetValue() const          { return aURL; }

    const String& GetName() const           { return aName; }
    void SetName( const String& rNm )       { aName = rNm; }

    const String& GetTargetFrame() const    { return aTargetFrame; }

    const String& GetINetFmt() const        { return aINetFmt; }
    void SetINetFmt( const String& rNm )    { aINetFmt = rNm; }

    const String& GetVisitedFmt() const     { return aVisitedFmt; }
    void SetVisitedFmt( const String& rNm ) { aVisitedFmt = rNm; }

    sal_uInt16 GetINetFmtId() const             { return nINetId; }
    void SetINetFmtId( sal_uInt16 nNew )        { nINetId = nNew; }

    sal_uInt16 GetVisitedFmtId() const          { return nVisitedId; }
    void SetVisitedFmtId( sal_uInt16 nNew )     { nVisitedId = nNew; }

    // setze eine neue oder loesche die akt. MakroTabelle
    void SetMacroTbl( const SvxMacroTableDtor* pTbl = 0 );
    const SvxMacroTableDtor* GetMacroTbl() const    { return pMacroTbl; }

    // setze / erfrage ein Makro
    void SetMacro( sal_uInt16 nEvent, const SvxMacro& rMacro );
    const SvxMacro* GetMacro( sal_uInt16 nEvent ) const;
};


#endif

