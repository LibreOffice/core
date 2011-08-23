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


#ifndef _SFXPOOLITEM_HXX
#include <bf_svtools/poolitem.hxx>
#endif
namespace binfilter {

class SwTxtRuby;

class SwFmtRuby : public SfxPoolItem
{
    friend class SwTxtRuby;

    String sRubyTxt;					// the ruby txt
    String sCharFmtName;				// name of the charformat
    SwTxtRuby* pTxtAttr;				// the TextAttribut
    USHORT nCharFmtId;					// PoolId of the charformat
    USHORT nPosition;					// Position of the Ruby-Character
    USHORT nAdjustment;					// specific adjustment of the Ruby-Ch.

public:
    SwFmtRuby( const String& rRubyTxt );
    SwFmtRuby( const SwFmtRuby& rAttr );
    virtual ~SwFmtRuby();

    SwFmtRuby& operator=( const SwFmtRuby& rAttr );

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*    Create(SvStream &, USHORT nVer) const;
    virtual SvStream&       Store(SvStream &, USHORT nIVer) const;


    virtual BOOL            QueryValue( ::com::sun::star::uno::Any& rVal,
                                    BYTE nMemberId = 0 ) const;
    virtual	BOOL			PutValue( const ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );

    virtual USHORT			GetVersion( USHORT nFFVer ) const;

    const SwTxtRuby* GetTxtRuby() const			{ return pTxtAttr; }
    SwTxtRuby* GetTxtRuby()						{ return pTxtAttr; }

    const String& GetText() const				{ return sRubyTxt; }
    void SetText( const String& rTxt )			{ sRubyTxt = rTxt; }

    const String& GetCharFmtName() const		{ return sCharFmtName; }
    void SetCharFmtName( const String& rNm )	{ sCharFmtName = rNm; }

    USHORT GetCharFmtId() const					{ return nCharFmtId; }
    void SetCharFmtId( USHORT nNew )			{ nCharFmtId = nNew; }

    USHORT GetPosition() const					{ return nPosition; }
    void SetPosition( USHORT nNew )				{ nPosition = nNew; }

    USHORT GetAdjustment() const				{ return nAdjustment; }
    void SetAdjustment( USHORT nNew )			{ nAdjustment = nNew; }
};


} //namespace binfilter
#endif

