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
#ifndef _SFX_ITEMPROP_HXX
#define _SFX_ITEMPROP_HXX

#include "svl/svldllapi.h"
#include <tools/solar.h>
#include <tools/string.hxx>
#include <svl/itemset.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <vector>

// UNO III - Implementation
#define MAP_CHAR_LEN(cchar) cchar, sizeof(cchar) - 1

struct SfxItemPropertyMapEntry
{
    const char*                         pName;
    sal_uInt16                              nNameLen;
    sal_uInt16                              nWID;
    const com::sun::star::uno::Type*    pType;
    long                                nFlags;
    sal_uInt8                                nMemberId;

};

struct SfxItemPropertySimpleEntry
{
    sal_uInt16                              nWID;
    const com::sun::star::uno::Type*    pType;
    long                                nFlags;
    sal_uInt8                                nMemberId;

    SfxItemPropertySimpleEntry() :
        nWID( 0 ),
        pType( 0 ),
        nFlags( 0 ),
        nMemberId( 0 ){}

    SfxItemPropertySimpleEntry(sal_uInt16 _nWID, const com::sun::star::uno::Type* _pType,
                               long _nFlags, sal_uInt8 _nMemberId) :
        nWID(      _nWID ),
        pType(     _pType ),
        nFlags(    _nFlags ),
        nMemberId( _nMemberId ){}

    SfxItemPropertySimpleEntry( const SfxItemPropertyMapEntry* pMapEntry ) :
        nWID(      pMapEntry->nWID ),
        pType(     pMapEntry->pType ),
        nFlags(    pMapEntry->nFlags ),
        nMemberId( pMapEntry->nMemberId ){}

};
struct SfxItemPropertyNamedEntry : public SfxItemPropertySimpleEntry
{
    OUString sName;
    SfxItemPropertyNamedEntry( const String& rName, const SfxItemPropertySimpleEntry& rSimpleEntry) :
        SfxItemPropertySimpleEntry( rSimpleEntry ),
        sName( rName ){}

};
typedef std::vector< SfxItemPropertyNamedEntry > PropertyEntryVector_t;
class SfxItemPropertyMap_Impl;
class SVL_DLLPUBLIC SfxItemPropertyMap
{
    SfxItemPropertyMap_Impl* m_pImpl;
public:
    SfxItemPropertyMap( const SfxItemPropertyMapEntry* pEntries );
    SfxItemPropertyMap( const SfxItemPropertyMap& rSource );
    ~SfxItemPropertyMap();

    const SfxItemPropertySimpleEntry*  getByName( const OUString &rName ) const;
    com::sun::star::uno::Sequence< com::sun::star::beans::Property > getProperties() const;
    com::sun::star::beans::Property getPropertyByName( const OUString rName ) const
        throw( ::com::sun::star::beans::UnknownPropertyException );
    sal_Bool hasPropertyByName( const OUString& rName ) const;

    void mergeProperties( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& rPropSeq );
    PropertyEntryVector_t getPropertyEntries() const;
    sal_uInt32 getSize() const;

};

class SVL_DLLPUBLIC SfxItemPropertySet
{
    SfxItemPropertyMap                                                              m_aMap;
    mutable com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo> m_xInfo;
protected:
    virtual sal_Bool            FillItem(SfxItemSet& rSet, sal_uInt16 nWhich, sal_Bool bGetProperty) const;

public:
                            SfxItemPropertySet( const SfxItemPropertyMapEntry *pMap ) :
                                m_aMap(pMap) {}
                            virtual ~SfxItemPropertySet();

    void getPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
                                          const SfxItemSet& rSet,
                                          com::sun::star::uno::Any& rAny) const
                                          throw(::com::sun::star::uno::RuntimeException);
    void getPropertyValue( const OUString &rName,
                                            const SfxItemSet& rSet,
                                            com::sun::star::uno::Any& rAny) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                                    ::com::sun::star::beans::UnknownPropertyException);
    com::sun::star::uno::Any
        getPropertyValue( const OUString &rName,
                                            const SfxItemSet& rSet ) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                                    ::com::sun::star::beans::UnknownPropertyException);
    void                    setPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
                                          const com::sun::star::uno::Any& aVal,
                                          SfxItemSet& rSet ) const
                                          throw(::com::sun::star::uno::RuntimeException,
                                                  com::sun::star::lang::IllegalArgumentException);
    void                    setPropertyValue( const OUString& rPropertyName,
                                            const com::sun::star::uno::Any& aVal,
                                            SfxItemSet& rSet ) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                                    com::sun::star::lang::IllegalArgumentException,
                                                    ::com::sun::star::beans::UnknownPropertyException);

    com::sun::star::beans::PropertyState
        getPropertyState(const OUString& rName, const SfxItemSet& rSet)const
                                    throw(com::sun::star::beans::UnknownPropertyException);
    com::sun::star::beans::PropertyState
        getPropertyState(const SfxItemPropertySimpleEntry& rEntry, const SfxItemSet& rSet) const
                                    throw();

    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>
        getPropertySetInfo() const;
    const SfxItemPropertyMap& getPropertyMap() const {return m_aMap;}
};

struct SfxItemPropertySetInfo_Impl;
class SVL_DLLPUBLIC SfxItemPropertySetInfo : public
    cppu::WeakImplHelper1<com::sun::star::beans::XPropertySetInfo>
{
    SfxItemPropertySetInfo_Impl* m_pImpl;

public:
    SfxItemPropertySetInfo(const SfxItemPropertyMap &rMap );
    SfxItemPropertySetInfo(const SfxItemPropertyMapEntry *pEntries );
    virtual ~SfxItemPropertySetInfo();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL
        getProperties(  )
            throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::Property SAL_CALL
        getPropertyByName( const OUString& aName )
            throw(::com::sun::star::beans::UnknownPropertyException,
                    ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const OUString& Name )
            throw(::com::sun::star::uno::RuntimeException);

};

class SVL_DLLPUBLIC SfxExtItemPropertySetInfo: public cppu::WeakImplHelper1<com::sun::star::beans::XPropertySetInfo >
{
    SfxItemPropertyMap aExtMap;
public:
                            SfxExtItemPropertySetInfo(
                                const SfxItemPropertyMapEntry *pMap,
                                const com::sun::star::uno::Sequence<com::sun::star::beans::Property>& rPropSeq );
                            virtual ~SfxExtItemPropertySetInfo();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL
        getProperties(  )
            throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::Property SAL_CALL
        getPropertyByName( const OUString& aName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const OUString& Name )
            throw(::com::sun::star::uno::RuntimeException);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
