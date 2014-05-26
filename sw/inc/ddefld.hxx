/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_INC_DDEFLD_HXX
#define INCLUDED_SW_INC_DDEFLD_HXX

#include <sfx2/lnkbase.hxx>
#include "swdllapi.h"
#include "fldbas.hxx"

class SwDoc;

// FieldType for DDE
class SW_DLLPUBLIC SwDDEFieldType : public SwFieldType
{
    OUString aName;
    OUString aExpansion;

    ::sfx2::SvBaseLinkRef refLink;
    SwDoc* pDoc;

    sal_uInt16 nRefCnt;
    bool bCRLFFlag : 1;
    bool bDeleted : 1;

    SAL_DLLPRIVATE void _RefCntChgd();

public:
    SwDDEFieldType( const OUString& rName, const OUString& rCmd,
                    sal_uInt16 = sfx2::LINKUPDATE_ONCALL );
    virtual ~SwDDEFieldType();

    OUString GetExpansion() const               { return aExpansion; }
    void SetExpansion( const OUString& rStr )   { aExpansion = rStr;
                                                  bCRLFFlag = false; }

    virtual SwFieldType* Copy() const SAL_OVERRIDE;
    virtual OUString GetName() const SAL_OVERRIDE;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) const SAL_OVERRIDE;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt16 nWhich ) SAL_OVERRIDE;

    OUString GetCmd() const;
    void SetCmd( const OUString& aStr );

    sal_uInt16 GetType() const          { return refLink->GetUpdateMode();  }
    void SetType( sal_uInt16 nType )    { refLink->SetUpdateMode( nType );  }

    bool IsDeleted() const          { return bDeleted; }
    void SetDeleted( bool b )       { bDeleted = b; }

    void UpdateNow()                { refLink->Update(); }
    void Disconnect()               { refLink->Disconnect(); }

    const ::sfx2::SvBaseLink& GetBaseLink() const    { return *refLink; }
          ::sfx2::SvBaseLink& GetBaseLink()          { return *refLink; }

    const SwDoc* GetDoc() const     { return pDoc; }
          SwDoc* GetDoc()           { return pDoc; }
    void SetDoc( SwDoc* pDoc );

    void IncRefCnt() {  if( !nRefCnt++ && pDoc ) _RefCntChgd(); }
    void DecRefCnt() {  if( !--nRefCnt && pDoc ) _RefCntChgd(); }

    void SetCRLFDelFlag( bool bFlag = true )    { bCRLFFlag = bFlag; }
};

// DDE-field
class SwDDEField : public SwField
{
private:
    virtual OUString Expand() const SAL_OVERRIDE;
    virtual SwField* Copy() const SAL_OVERRIDE;

public:
    SwDDEField(SwDDEFieldType*);
    virtual ~SwDDEField();

    /** Get parameter via types.
     Name cannot be changed. */
    virtual OUString  GetPar1() const SAL_OVERRIDE;

    // Command
    virtual OUString  GetPar2() const SAL_OVERRIDE;
    virtual void    SetPar2(const OUString& rStr) SAL_OVERRIDE;
};

#endif // INCLUDED_SW_INC_DDEFLD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
