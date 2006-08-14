/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inetfld.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-08-14 15:25:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _INETFLD_HXX
#define _INETFLD_HXX


#include "fldbas.hxx"

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

    SwCharFmt*              GetCharFmt( const SwINetField& rFld );

    SwDoc* GetDoc() const   { return pDoc; }
};

/*--------------------------------------------------------------------
    Beschreibung: InterNet-Field -> Lade Document mit der URL
 --------------------------------------------------------------------*/

class SwINetField : public SwField
{
    friend class SwINetFieldType;

    String  sTargetFrameName;   // in diesen Frame soll die URL
    String  sURL;
    String  sText;
    SvxMacroTableDtor* pMacroTbl;

public:
    // Direkte Eingabe alten Wert loeschen
    SwINetField( SwINetFieldType* pTyp, USHORT nFmt,
                  const String& rURL, const String& rText );
    virtual ~SwINetField();

    virtual String   GetCntnt(BOOL bName = FALSE) const;
    virtual String   Expand() const;
    virtual SwField* Copy() const;

    // URL
    virtual const String& GetPar1() const;
    virtual void    SetPar1(const String& rStr);

    // HinweisText
    virtual String  GetPar2() const;
    virtual void    SetPar2(const String& rStr);

    // das ist das akt. Zeichenformat
          SwCharFmt* GetCharFmt();
    const SwCharFmt* GetCharFmt() const
            { return ((SwINetField*)this)->GetCharFmt(); }

    const String& GetTargetFrameName() const        { return sTargetFrameName; }
    void SetTargetFrameName( const String& rNm )    { sTargetFrameName = rNm; }

    // setze eine neue oder loesche die akt. MakroTabelle
    void SetMacroTbl( const SvxMacroTableDtor* pTbl = 0 );
    const SvxMacroTableDtor* GetMacroTbl() const    { return pMacroTbl; }

    // setze / erfrage ein Makro
    void SetMacro( USHORT nEvent, const SvxMacro& rMacro );
    const SvxMacro* GetMacro( USHORT nEvent ) const;
};


#endif // _INETFLD_HXX

