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

    String msURL;                   // die URL
    String msTargetFrame;           // in diesen Frame soll die URL
    String msINetFmtName;
    String msVisitedFmtName;
    String msHyperlinkName;                 // Name des Links
    SvxMacroTableDtor* mpMacroTbl;
    SwTxtINetFmt* mpTxtAttr;            // mein TextAttribut
    sal_uInt16 mnINetFmtId;
    sal_uInt16 mnVisitedFmtId;
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


    const SwTxtINetFmt* GetTxtINetFmt() const
    {
        return mpTxtAttr;
    }
    SwTxtINetFmt* GetTxtINetFmt()
    {
        return mpTxtAttr;
    }

    const String& GetValue() const
    {
        return msURL;
    }

    const String& GetName() const
    {
        return msHyperlinkName;
    }
    void SetName( const String& rNm )
    {
        msHyperlinkName = rNm;
    }

    const String& GetTargetFrame() const
    {
        return msTargetFrame;
    }

    void SetINetFmtAndId(
            const String& rNm,
            const sal_uInt16 nId )
    {
        msINetFmtName = rNm;
        mnINetFmtId = nId;
    }

    const String& GetINetFmt() const
    {
        return msINetFmtName;
    }

    sal_uInt16 GetINetFmtId() const
    {
        return mnINetFmtId;
    }

    void SetVisitedFmtAndId(
            const String& rNm,
            const sal_uInt16 nId )
    {
        msVisitedFmtName = rNm;
        mnVisitedFmtId = nId;
    }

    const String& GetVisitedFmt() const
    {
        return msVisitedFmtName;
    }

    sal_uInt16 GetVisitedFmtId() const
    {
        return mnVisitedFmtId;
    }

    // setze eine neue oder loesche die akt. MakroTabelle
    void SetMacroTbl( const SvxMacroTableDtor* pTbl = 0 );
    const SvxMacroTableDtor* GetMacroTbl() const
    {
        return mpMacroTbl;
    }

    // setze / erfrage ein Makro
    void SetMacro(
            sal_uInt16 nEvent,
            const SvxMacro& rMacro );
    const SvxMacro* GetMacro( sal_uInt16 nEvent ) const;
};


#endif

