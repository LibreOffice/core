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


#ifndef SW_INETFLD_HXX
#define SW_INETFLD_HXX


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

    virtual String   Expand() const;
    virtual SwField* Copy() const;

public:
    // Direkte Eingabe alten Wert loeschen
    SwINetField( SwINetFieldType* pTyp, sal_uInt16 nFmt,
                  const String& rURL, const String& rText );
    virtual ~SwINetField();

    virtual String   GetFieldName() const;

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
    void SetMacro( sal_uInt16 nEvent, const SvxMacro& rMacro );
    const SvxMacro* GetMacro( sal_uInt16 nEvent ) const;
};


#endif // SW_INETFLD_HXX

