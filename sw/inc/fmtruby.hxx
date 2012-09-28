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
#ifndef _FMTRUBY_HXX
#define _FMTRUBY_HXX


#include <tools/string.hxx>
#include <svl/poolitem.hxx>

class SwTxtRuby;

class SW_DLLPUBLIC SwFmtRuby : public SfxPoolItem
{
    friend class SwTxtRuby;

    String sRubyTxt;                        ///< The ruby txt.
    String sCharFmtName;                    ///< Name of the charformat.
    SwTxtRuby* pTxtAttr;                    ///< The TextAttribute.
    sal_uInt16 nCharFmtId;                  ///< PoolId of the charformat.
    sal_uInt16 nPosition;                   ///< Position of the Ruby-character.
    sal_uInt16 nAdjustment;                 ///< Specific adjustment of the Ruby-ch.

public:
    SwFmtRuby( const String& rRubyTxt );
    SwFmtRuby( const SwFmtRuby& rAttr );
    virtual ~SwFmtRuby();

    SwFmtRuby& operator=( const SwFmtRuby& rAttr );

    // "Pure virtual methods" of SfxPoolItem.
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper* pIntl = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal,
                                        sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal,
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
