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
#ifndef _FMTINFMT_HXX
#define _FMTINFMT_HXX


#include <bf_svtools/poolitem.hxx>

class IntlWrapper;

namespace binfilter {

class SvxMacro;
class SvxMacroTableDtor;
class SwTxtINetFmt;
// ATT_INETFMT *********************************************

class SwFmtINetFmt: public SfxPoolItem
{
    friend class SwTxtINetFmt;

    String aURL;					// die URL
    String aTargetFrame;			// in diesen Frame soll die URL
    String aINetFmt;
    String aVisitedFmt;
    String aName;					// Name des Links
    SvxMacroTableDtor* pMacroTbl;
    SwTxtINetFmt* pTxtAttr;			// mein TextAttribut
    USHORT nINetId;
    USHORT nVisitedId;
public:
    SwFmtINetFmt( const String& rURL, const String& rTarget );
    SwFmtINetFmt( const SwFmtINetFmt& rAttr );
    SwFmtINetFmt();                     // for TypeInfo
    virtual ~SwFmtINetFmt();

    TYPEINFO();

    // "pure virtual Methoden" vom SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*	Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxPoolItem*	Create(SvStream &, USHORT nVer) const;
    virtual SvStream&		Store(SvStream &, USHORT nIVer) const;

    virtual	bool            QueryValue( ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 ) const;
    virtual	bool            PutValue( const ::com::sun::star::uno::Any& rVal,
                                        BYTE nMemberId = 0 );

    virtual USHORT			GetVersion( USHORT nFFVer ) const;

    const SwTxtINetFmt* GetTxtINetFmt() const	{ return pTxtAttr; }
    SwTxtINetFmt* GetTxtINetFmt()				{ return pTxtAttr; }

    const String& GetValue() const 			{ return aURL; }
    void SetValue( const String& rS ) 		{ aURL = rS; }

    const String& GetName() const 			{ return aName; }
    void SetName( const String& rNm )		{ aName = rNm; }

    const String& GetTargetFrame() const 	{ return aTargetFrame; }
    void SetTargetFrame( const String& rNm ){ aTargetFrame = rNm; }

    const String& GetINetFmt() const		{ return aINetFmt; }
    void SetINetFmt( const String& rNm )	{ aINetFmt = rNm; }

    const String& GetVisitedFmt() const		{ return aVisitedFmt; }
    void SetVisitedFmt( const String& rNm )	{ aVisitedFmt = rNm; }

    USHORT GetINetFmtId() const				{ return nINetId; }
    void SetINetFmtId( USHORT nNew )		{ nINetId = nNew; }

    USHORT GetVisitedFmtId() const			{ return nVisitedId; }
    void SetVisitedFmtId( USHORT nNew )		{ nVisitedId = nNew; }

    // setze eine neue oder loesche die akt. MakroTabelle
    const SvxMacroTableDtor* GetMacroTbl() const	{ return pMacroTbl; }

    // setze / erfrage ein Makro
/*N #i27164#*/	void SetMacro( USHORT nEvent, const SvxMacro& rMacro );
    const SvxMacro* GetMacro( USHORT nEvent ) const;
};


} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
