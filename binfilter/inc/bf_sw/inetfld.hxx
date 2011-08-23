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
#ifndef _INETFLD_HXX
#define _INETFLD_HXX


#include "fldbas.hxx"

namespace binfilter {
class SvxMacro; 
class SvxMacroTableDtor; 

class SwINetField;
class SwCharFmt;
class SwDoc;

/*--------------------------------------------------------------------
    Beschreibung: InterNet-FieldType -> Lade Document mit der URL
 --------------------------------------------------------------------*/

class SwINetFieldType : public SwFieldType
{
    SwDepend aNormalFmt;
    SwDepend aVisitFmt;
    SwDoc* pDoc;

public:
    SwINetFieldType( SwDoc* pDoc );

    virtual SwFieldType*    Copy() const;

    SwCharFmt* 				GetCharFmt( const SwINetField& rFld );

    SwDoc* GetDoc() const	{ return pDoc; }
};

/*--------------------------------------------------------------------
    Beschreibung: InterNet-Field -> Lade Document mit der URL
 --------------------------------------------------------------------*/

class SwINetField : public SwField
{
    friend class SwINetFieldType;

    String	sTargetFrameName;	// in diesen Frame soll die URL
    String  sURL;
    String  sText;
    SvxMacroTableDtor* pMacroTbl;

public:
    // Direkte Eingabe alten Wert loeschen
    SwINetField( SwINetFieldType* pTyp, USHORT nFmt,
                  const String& rURL, const String& rText );
    virtual	~SwINetField();

    virtual	String	 GetCntnt(BOOL bName = FALSE) const;
    virtual String	 Expand() const;
    virtual SwField* Copy() const;

    // URL
    virtual const String& GetPar1() const;
    virtual void	SetPar1(const String& rStr);

    // HinweisText
    virtual String	GetPar2() const;
    virtual void	SetPar2(const String& rStr);

    // das ist das akt. Zeichenformat
          SwCharFmt* GetCharFmt();
    const SwCharFmt* GetCharFmt() const
            { return ((SwINetField*)this)->GetCharFmt(); }

    const String& GetTargetFrameName() const 		{ return sTargetFrameName; }
    void SetTargetFrameName( const String& rNm )	{ sTargetFrameName = rNm; }

    // setze eine neue oder loesche die akt. MakroTabelle
    void SetMacroTbl( const SvxMacroTableDtor* pTbl = 0 );
    const SvxMacroTableDtor* GetMacroTbl() const	{ return pMacroTbl; }

    // setze / erfrage ein Makro
    void SetMacro( USHORT nEvent, const SvxMacro& rMacro );
    const SvxMacro* GetMacro( USHORT nEvent ) const;
};


} //namespace binfilter
#endif // _INETFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
