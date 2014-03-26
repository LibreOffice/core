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

#ifndef INCLUDED_BASIC_SBXOBJ_HXX
#define INCLUDED_BASIC_SBXOBJ_HXX

#include <svl/lstner.hxx>
#include <basic/sbxvar.hxx>
#include <basic/basicdllapi.h>


class SbxProperty;

class BASIC_DLLPUBLIC SbxObject : public SbxVariable, public SfxListener
{
    BASIC_DLLPRIVATE SbxArray* FindVar( SbxVariable*, sal_uInt16& );
protected:
    SbxArrayRef  pMethods;                  // Methods
    SbxArrayRef  pProps;                    // Properties
    SbxArrayRef  pObjs;                     // Objects
    SbxProperty* pDfltProp;                 // Default-Property
    OUString     aClassName;                // Classname
    OUString     aDfltPropName;
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 ) SAL_OVERRIDE;
    virtual sal_Bool StoreData( SvStream& ) const SAL_OVERRIDE;
    virtual ~SbxObject();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType ) SAL_OVERRIDE;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_OBJECT,1);
    TYPEINFO_OVERRIDE();
    SbxObject( const OUString& rClassname );
    SbxObject( const SbxObject& );
    SbxObject& operator=( const SbxObject& );
    virtual SbxDataType GetType() const SAL_OVERRIDE;
    virtual SbxClassType GetClass() const SAL_OVERRIDE;
    virtual void Clear() SAL_OVERRIDE;

    virtual sal_Bool  IsClass( const OUString& ) const;
    const OUString& GetClassName() const { return aClassName; }
    void SetClassName( const OUString &rNew ) { aClassName = rNew; }
    // Default-Property
    SbxProperty* GetDfltProperty();
    void SetDfltProperty( const OUString& r );
    // Search for an element
    virtual SbxVariable* FindUserData( sal_uInt32 nUserData );
    virtual SbxVariable* Find( const OUString&, SbxClassType );
    SbxVariable* FindQualified( const OUString&, SbxClassType );
    // Quick-Call-Interface for Methods
    virtual sal_Bool Call( const OUString&, SbxArray* = NULL );
    // Execution of DDE-Commands
    SbxVariable* Execute( const OUString& );
    // Manage elements
    virtual sal_Bool GetAll( SbxClassType ) { return sal_True; }
    SbxVariable* Make( const OUString&, SbxClassType, SbxDataType );
    virtual SbxObject* MakeObject( const OUString&, const OUString& );
    virtual void Insert( SbxVariable* );
    // AB 23.4.1997, Optimization, Insertion without check for duplicate Entries and
    // without Broadcasts, only used in SO2/auto.cxx
    void QuickInsert( SbxVariable* );
    virtual void Remove( const OUString&, SbxClassType );
    virtual void Remove( SbxVariable* );

    // Macro-Recording
    virtual OUString GenerateSource( const OUString &rLinePrefix,
                                     const SbxObject *pRelativeTo );
    // Direct access on arrays
    SbxArray* GetMethods()      { return pMethods;  }
    SbxArray* GetProperties()   { return pProps;    }
    SbxArray* GetObjects()      { return pObjs;     }
    // Debugging
    void Dump( SvStream&, sal_Bool bDumpAll=sal_False );
};

SV_DECL_REF(SbxObject)
SV_IMPL_REF(SbxObject)

#endif // INCLUDED_BASIC_SBXOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
