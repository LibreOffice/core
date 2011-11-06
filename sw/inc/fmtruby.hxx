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


#ifndef _FMTRUBY_HXX
#define _FMTRUBY_HXX


#include <tools/string.hxx>
#include <svl/poolitem.hxx>

class SwTxtRuby;

class SW_DLLPUBLIC SwFmtRuby : public SfxPoolItem
{
    friend class SwTxtRuby;

    String sRubyTxt;                    // the ruby txt
    String sCharFmtName;                // name of the charformat
    SwTxtRuby* pTxtAttr;                // the TextAttribut
    sal_uInt16 nCharFmtId;                  // PoolId of the charformat
    sal_uInt16 nPosition;                   // Position of the Ruby-Character
    sal_uInt16 nAdjustment;                 // specific adjustment of the Ruby-Ch.

public:
    SwFmtRuby( const String& rRubyTxt );
    SwFmtRuby( const SwFmtRuby& rAttr );
    virtual ~SwFmtRuby();

    SwFmtRuby& operator=( const SwFmtRuby& rAttr );

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


    const SwTxtRuby* GetTxtRuby() const         { return pTxtAttr; }
    SwTxtRuby* GetTxtRuby()                     { return pTxtAttr; }

    const String& GetText() const               { return sRubyTxt; }
    void SetText( const String& rTxt )          { sRubyTxt = rTxt; }

    const String& GetCharFmtName() const        { return sCharFmtName; }
    void SetCharFmtName( const String& rNm )    { sCharFmtName = rNm; }

    sal_uInt16 GetCharFmtId() const                 { return nCharFmtId; }
    void SetCharFmtId( sal_uInt16 nNew )            { nCharFmtId = nNew; }

    sal_uInt16 GetPosition() const                  { return nPosition; }
    void SetPosition( sal_uInt16 nNew )             { nPosition = nNew; }

    sal_uInt16 GetAdjustment() const                { return nAdjustment; }
    void SetAdjustment( sal_uInt16 nNew )           { nAdjustment = nNew; }
};


#endif

