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
#ifndef _DDEFLD_HXX
#define _DDEFLD_HXX

#ifndef _LNKBASE_HXX //autogen
#include <bf_so3/lnkbase.hxx>
#endif
#include "fldbas.hxx"
namespace binfilter {

class SwDoc;

/*--------------------------------------------------------------------
    Beschreibung: FieldType fuer DDE
 --------------------------------------------------------------------*/

class SwDDEFieldType : public SwFieldType
{
    String aName;
    String aExpansion;

    ::binfilter::SvBaseLinkRef refLink;
    SwDoc* pDoc;

    USHORT nRefCnt;
    BOOL bCRLFFlag : 1;
    BOOL bDeleted : 1;

    void _RefCntChgd();
public:
    SwDDEFieldType( const String& rName, const String& rCmd,
                    USHORT = ::binfilter::LINKUPDATE_ONCALL );
    ~SwDDEFieldType();

    const String& GetExpansion() const			{ return aExpansion; }
    void SetExpansion( const String& rStr )		{ aExpansion = rStr,
                                                  bCRLFFlag = FALSE; }

    virtual SwFieldType* Copy() const;
    virtual const String& GetName() const;

    virtual BOOL        QueryValue( ::com::sun::star::uno::Any& rVal, BYTE nMId ) const;
    virtual BOOL        PutValue( const ::com::sun::star::uno::Any& rVal, BYTE nMId );

    String GetCmd() const;
    void SetCmd( const String& rStr );

    USHORT GetType() const 			{ return refLink->GetUpdateMode();	}
    void SetType( USHORT nType )	{ refLink->SetUpdateMode( nType );	}

    BOOL IsDeleted() const 			{ return bDeleted; }
    void SetDeleted( BOOL b )		{ bDeleted = b; }

    BOOL IsConnected() const		{ return 0 != refLink->GetObj(); }
    void UpdateNow()				{ refLink->Update(); }
    void Disconnect()				{ refLink->Disconnect(); }

    const ::binfilter::SvBaseLink& GetBaseLink() const 	{ return *refLink; }
          ::binfilter::SvBaseLink& GetBaseLink()			{ return *refLink; }

    const SwDoc* GetDoc() const 	{ return pDoc; }
          SwDoc* GetDoc() 			{ return pDoc; }
    void SetDoc( SwDoc* pDoc );

    void IncRefCnt() {	if( !nRefCnt++ && pDoc ) _RefCntChgd();	}
    void DecRefCnt() {	if( !--nRefCnt && pDoc ) _RefCntChgd(); }

    void SetCRLFDelFlag( BOOL bFlag = TRUE )	{ bCRLFFlag = bFlag; }
    BOOL IsCRLFDelFlag() const 					{ return bCRLFFlag; }
};

/*--------------------------------------------------------------------
    Beschreibung: DDE-Feld
 --------------------------------------------------------------------*/

class SwDDEField : public SwField
{
public:
    SwDDEField(SwDDEFieldType*);
    ~SwDDEField();

    virtual String	 Expand() const;
    virtual SwField* Copy() const;

    // ueber Typen Parameter ermitteln
    // Name kann nicht geaendert werden
    virtual const String& GetPar1() const;

    // Commando
    virtual String  GetPar2() const;
    virtual void    SetPar2(const String& rStr);
};


} //namespace binfilter
#endif // _DDEFLD_HXX
