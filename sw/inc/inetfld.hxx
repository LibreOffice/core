/*************************************************************************
 *
 *  $RCSfile: inetfld.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:26 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
