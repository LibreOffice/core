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
#ifndef SW_DDEFLD_HXX
#define SW_DDEFLD_HXX

#include <sfx2/lnkbase.hxx>
#include "swdllapi.h"
#include "fldbas.hxx"

class SwDoc;

/*--------------------------------------------------------------------
    FieldType for DDE
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwDDEFieldType : public SwFieldType
{
    rtl::OUString aName;
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
    virtual const rtl::OUString& GetName() const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich );

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
    DDE-field
 --------------------------------------------------------------------*/

class SwDDEField : public SwField
{
private:
    virtual String   Expand() const;
    virtual SwField* Copy() const;

public:
    SwDDEField(SwDDEFieldType*);
    ~SwDDEField();

    /** Get parameter via types.
     Name cannot be changed. */
    virtual const rtl::OUString& GetPar1() const;

    // Command
    virtual rtl::OUString  GetPar2() const;
    virtual void    SetPar2(const rtl::OUString& rStr);
};


#endif // SW_DDEFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
