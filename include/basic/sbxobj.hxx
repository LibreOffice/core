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
    BASIC_DLLPRIVATE SbxArray* FindVar( SbxVariable const *, sal_uInt32& );
protected:
    SbxArrayRef  pMethods;                  // Methods
    SbxArrayRef  pProps;                    // Properties
    SbxArrayRef  pObjs;                     // Objects
    SbxProperty* pDfltProp;                 // Default-Property
    OUString     aClassName;                // Classname
    OUString     aDfltPropName;
    SAL_DLLPRIVATE virtual bool LoadData( SvStream&, sal_uInt16 ) override;
    SAL_DLLPRIVATE virtual std::pair<bool, sal_uInt32> StoreData( SvStream& ) const override;
    SAL_DLLPRIVATE virtual ~SbxObject() override;
    SAL_DLLPRIVATE virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;
    SAL_DLLPRIVATE virtual bool IsOptionCompatible() const; // Module's Option Compatible

public:
    SBX_DECL_PERSIST_NODATA(SBXID_OBJECT,1);
    SAL_DLLPRIVATE SbxObject( const OUString& rClassname );
    SAL_DLLPRIVATE SbxObject( const SbxObject& );
    SAL_DLLPRIVATE SbxObject& operator=( const SbxObject& );
    SAL_DLLPRIVATE virtual SbxDataType GetType() const override;
    SAL_DLLPRIVATE virtual SbxClassType GetClass() const override;
    SAL_DLLPRIVATE virtual void Clear() override;

    SAL_DLLPRIVATE virtual bool IsClass( const OUString& ) const;
    const OUString& GetClassName() const { return aClassName; }
    void SetClassName( const OUString &rNew ) { aClassName = rNew; }
    // Default-Property
    SAL_DLLPRIVATE SbxProperty* GetDfltProperty();
    SAL_DLLPRIVATE void SetDfltProperty( const OUString& r );
    // Search for an element
    SAL_DLLPRIVATE virtual SbxVariable* Find( const OUString&, SbxClassType );
    SAL_DLLPRIVATE SbxVariable* FindQualified( const OUString&, SbxClassType );
    // Quick-Call-Interface for Methods
    SAL_DLLPRIVATE virtual bool Call( const OUString&, SbxArray* = nullptr );
    // Execution of DDE-Commands
    SbxVariable* Execute( const OUString& );
    // Manage elements
    SAL_DLLPRIVATE SbxVariable* Make( const OUString&, SbxClassType, SbxDataType, bool bIsRuntimeFunction = false );
    SAL_DLLPRIVATE virtual void Insert( SbxVariable* );
    // AB 23.4.1997, Optimization, Insertion without check for duplicate Entries and
    // without Broadcasts, only used in SO2/auto.cxx
    SAL_DLLPRIVATE void QuickInsert( SbxVariable* );
    SAL_DLLPRIVATE void Remove( const OUString&, SbxClassType );
    SAL_DLLPRIVATE virtual void Remove( SbxVariable* );

    // Direct access on arrays
    SbxArray* GetMethods()      { return pMethods.get(); }
    SbxArray* GetProperties()   { return pProps.get(); }
    SbxArray* GetObjects()      { return pObjs.get(); }
    // Debugging
    SAL_DLLPRIVATE void Dump( SvStream&, bool bDumpAll );
};

#endif // INCLUDED_BASIC_SBXOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
