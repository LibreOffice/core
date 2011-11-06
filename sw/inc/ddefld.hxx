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


#ifndef SW_DDEFLD_HXX
#define SW_DDEFLD_HXX

#include <sfx2/lnkbase.hxx>
#include "swdllapi.h"
#include "fldbas.hxx"

class SwDoc;

/*--------------------------------------------------------------------
    Beschreibung: FieldType fuer DDE
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDDEFieldType : public SwFieldType
{
    String aName;
    String aExpansion;

    ::sfx2::SvBaseLinkRef refLink;
    SwDoc* pDoc;

    sal_uInt16 nRefCnt;
    sal_Bool bCRLFFlag : 1;
    sal_Bool bDeleted : 1;

    SW_DLLPRIVATE void _RefCntChgd();

public:
    SwDDEFieldType( const String& rName, const String& rCmd,
                    sal_uInt16 = sfx2::LINKUPDATE_ONCALL );
    ~SwDDEFieldType();

    const String& GetExpansion() const          { return aExpansion; }
    void SetExpansion( const String& rStr )     { aExpansion = rStr,
                                                  bCRLFFlag = sal_False; }

    virtual SwFieldType* Copy() const;
    virtual const String& GetName() const;

    virtual sal_Bool        QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual sal_Bool        PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );

    String GetCmd() const;
    void SetCmd( const String& rStr );

    sal_uInt16 GetType() const          { return refLink->GetUpdateMode();  }
    void SetType( sal_uInt16 nType )    { refLink->SetUpdateMode( nType );  }

    sal_Bool IsDeleted() const          { return bDeleted; }
    void SetDeleted( sal_Bool b )       { bDeleted = b; }

    void UpdateNow()                { refLink->Update(); }
    void Disconnect()               { refLink->Disconnect(); }

    const ::sfx2::SvBaseLink& GetBaseLink() const    { return *refLink; }
          ::sfx2::SvBaseLink& GetBaseLink()          { return *refLink; }

    const SwDoc* GetDoc() const     { return pDoc; }
          SwDoc* GetDoc()           { return pDoc; }
    void SetDoc( SwDoc* pDoc );

    void IncRefCnt() {  if( !nRefCnt++ && pDoc ) _RefCntChgd(); }
    void DecRefCnt() {  if( !--nRefCnt && pDoc ) _RefCntChgd(); }

    void SetCRLFDelFlag( sal_Bool bFlag = sal_True )    { bCRLFFlag = bFlag; }
};

/*--------------------------------------------------------------------
    Beschreibung: DDE-Feld
 --------------------------------------------------------------------*/

class SwDDEField : public SwField
{
private:
    virtual String   Expand() const;
    virtual SwField* Copy() const;

public:
    SwDDEField(SwDDEFieldType*);
    ~SwDDEField();

    // ueber Typen Parameter ermitteln
    // Name kann nicht geaendert werden
    virtual const String& GetPar1() const;

    // Commando
    virtual String  GetPar2() const;
    virtual void    SetPar2(const String& rStr);
};


#endif // SW_DDEFLD_HXX
