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

#ifndef _SBX_SBXOBJECT_HXX
#define _SBX_SBXOBJECT_HXX

#include <svl/lstner.hxx>
#include <basic/sbxvar.hxx>
#include "basicdllapi.h"


class SbxProperty;

class BASIC_DLLPUBLIC SbxObject : public SbxVariable, public SfxListener
{
    BASIC_DLLPRIVATE SbxArray* FindVar( SbxVariable*, sal_uInt16& );
protected:
    SbxArrayRef  pMethods;                  // Methods
    SbxArrayRef  pProps;                    // Properties
    SbxArrayRef  pObjs;                     // Objects
    SbxProperty* pDfltProp;                 // Default-Property
    String       aClassName;                // Classname
    String       aDfltPropName;
    virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
    virtual sal_Bool StoreData( SvStream& ) const;
    virtual ~SbxObject();
    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                             const SfxHint& rHint, const TypeId& rHintType );
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_OBJECT,1);
    TYPEINFO();
    SbxObject( const String& rClassname );
    SbxObject( const SbxObject& );
    SbxObject& operator=( const SbxObject& );
    virtual SbxDataType GetType() const;
    virtual SbxClassType GetClass() const;
    virtual void Clear();

    virtual sal_Bool  IsClass( const rtl::OUString& ) const;
    const String& GetClassName() const { return aClassName; }
    void          SetClassName( const String &rNew ) { aClassName = rNew; }
    // Default-Property
    SbxProperty* GetDfltProperty();
    void SetDfltProperty( const String& r );
    // Search for an element
    virtual SbxVariable* FindUserData( sal_uInt32 nUserData );
    virtual SbxVariable* Find( const rtl::OUString&, SbxClassType );
    SbxVariable* FindQualified( const String&, SbxClassType );
    // Quick-Call-Interface for Methods
    virtual sal_Bool Call( const String&, SbxArray* = NULL );
    // Execution of DDE-Commands
    SbxVariable* Execute( const String& );
    // Manage elements
    virtual sal_Bool GetAll( SbxClassType ) { return sal_True; }
    SbxVariable* Make( const String&, SbxClassType, SbxDataType );
    virtual SbxObject* MakeObject( const String&, const String& );
    virtual void Insert( SbxVariable* );
    // AB 23.4.1997, Optimization, Insertion without check for duplicate Entries and
    // without Broadcasts, only used in SO2/auto.cxx
    void QuickInsert( SbxVariable* );
    virtual void Remove( const String&, SbxClassType );
    virtual void Remove( SbxVariable* );

    // Macro-Recording
    virtual String GenerateSource( const String &rLinePrefix,
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

#endif /* _SBX_SBXOBJECT_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
