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
/* -----------------------------21.02.00 11:03--------------------------------
    UNO III - Implementation
 ---------------------------------------------------------------------------*/
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
    ::rtl::OUString sName;
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
    SfxItemPropertyMap( const SfxItemPropertyMap* pSource );
    ~SfxItemPropertyMap();

    const SfxItemPropertySimpleEntry*  getByName( const ::rtl::OUString &rName ) const;
    com::sun::star::uno::Sequence< com::sun::star::beans::Property > getProperties() const;
    com::sun::star::beans::Property getPropertyByName( const ::rtl::OUString rName ) const
        throw( ::com::sun::star::beans::UnknownPropertyException );
    sal_Bool hasPropertyByName( const ::rtl::OUString& rName ) const;

    void mergeProperties( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& rPropSeq );
    PropertyEntryVector_t getPropertyEntries() const;
    sal_uInt32 getSize() const;

};
/* -----------------------------21.02.00 11:19--------------------------------

 ---------------------------------------------------------------------------*/
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
    void getPropertyValue( const ::rtl::OUString &rName,
                                            const SfxItemSet& rSet,
                                            com::sun::star::uno::Any& rAny) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                                    ::com::sun::star::beans::UnknownPropertyException);
    com::sun::star::uno::Any
        getPropertyValue( const ::rtl::OUString &rName,
                                            const SfxItemSet& rSet ) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                                    ::com::sun::star::beans::UnknownPropertyException);
    void                    setPropertyValue( const SfxItemPropertySimpleEntry& rEntry,
                                          const com::sun::star::uno::Any& aVal,
                                          SfxItemSet& rSet ) const
                                          throw(::com::sun::star::uno::RuntimeException,
                                                  com::sun::star::lang::IllegalArgumentException);
    void                    setPropertyValue( const ::rtl::OUString& rPropertyName,
                                            const com::sun::star::uno::Any& aVal,
                                            SfxItemSet& rSet ) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                                    com::sun::star::lang::IllegalArgumentException,
                                                    ::com::sun::star::beans::UnknownPropertyException);

    com::sun::star::beans::PropertyState
        getPropertyState(const ::rtl::OUString& rName, const SfxItemSet& rSet)const
                                    throw(com::sun::star::beans::UnknownPropertyException);
    com::sun::star::beans::PropertyState
        getPropertyState(const SfxItemPropertySimpleEntry& rEntry, const SfxItemSet& rSet) const
                                    throw();

    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>
        getPropertySetInfo() const;
    const SfxItemPropertyMap*
        getPropertyMap() const {return &m_aMap;}

};
/* -----------------------------21.02.00 11:09--------------------------------

 ---------------------------------------------------------------------------*/
struct SfxItemPropertySetInfo_Impl;
class SVL_DLLPUBLIC SfxItemPropertySetInfo : public
    cppu::WeakImplHelper1<com::sun::star::beans::XPropertySetInfo>
{
    SfxItemPropertySetInfo_Impl* m_pImpl;

public:
    SfxItemPropertySetInfo(const SfxItemPropertyMap *pMap );
    SfxItemPropertySetInfo(const SfxItemPropertyMapEntry *pEntries );
    virtual ~SfxItemPropertySetInfo();

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL
        getProperties(  )
            throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::Property SAL_CALL
        getPropertyByName( const ::rtl::OUString& aName )
            throw(::com::sun::star::beans::UnknownPropertyException,
                    ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const ::rtl::OUString& Name )
            throw(::com::sun::star::uno::RuntimeException);

    const SfxItemPropertyMap* getMap() const;
};
/* -----------------------------21.02.00 12:01--------------------------------

 ---------------------------------------------------------------------------*/
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
        getPropertyByName( const ::rtl::OUString& aName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const ::rtl::OUString& Name )
            throw(::com::sun::star::uno::RuntimeException);
};

#endif
