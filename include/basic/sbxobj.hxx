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
    virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    virtual bool StoreData( SvStream& ) const override;
    virtual ~SbxObject();
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
public:
    SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_OBJECT,1);
    SbxObject( const OUString& rClassname );
    SbxObject( const SbxObject& );
    SbxObject& operator=( const SbxObject& );
    virtual SbxDataType GetType() const override;
    virtual SbxClassType GetClass() const override;
    virtual void Clear() override;

    virtual bool IsClass( const OUString& ) const;
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
    virtual bool Call( const OUString&, SbxArray* = nullptr );
    // Execution of DDE-Commands
    SbxVariable* Execute( const OUString& );
    // Manage elements
    SbxVariable* Make( const OUString&, SbxClassType, SbxDataType, bool bIsRuntimeFunction = false );
    virtual void Insert( SbxVariable* );
    // AB 23.4.1997, Optimization, Insertion without check for duplicate Entries and
    // without Broadcasts, only used in SO2/auto.cxx
    void QuickInsert( SbxVariable* );
    void Remove( const OUString&, SbxClassType );
    virtual void Remove( SbxVariable* );

    // Direct access on arrays
    SbxArray* GetMethods()      { return pMethods;  }
    SbxArray* GetProperties()   { return pProps;    }
    SbxArray* GetObjects()      { return pObjs;     }
    // Debugging
    void Dump( SvStream&, bool bDumpAll=false );
};

typedef tools::SvRef<SbxObject> SbxObjectRef;

#endif // INCLUDED_BASIC_SBXOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
