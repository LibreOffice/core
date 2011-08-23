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
#ifndef _SFX_ITEMPROP_HXX
#define _SFX_ITEMPROP_HXX

#include <tools/solar.h>
#include <tools/string.hxx>
#include <bf_svtools/itemset.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>

namespace binfilter
{

/* -----------------------------21.02.00 11:03--------------------------------
    UNO III - Implementation
 ---------------------------------------------------------------------------*/
#define MAP_CHAR_LEN(cchar) cchar, sizeof(cchar) - 1
struct SfxItemPropertyMap
{
    const char* 						pName;
    USHORT 								nNameLen;
    USHORT								nWID;
    const ::com::sun::star::uno::Type*	pType;
    long								nFlags;
    BYTE								nMemberId;

     static const SfxItemPropertyMap*	GetByName(
        const SfxItemPropertyMap *pMap,
        const ::rtl::OUString &rName );
};
/* -----------------------------21.02.00 11:19--------------------------------

 ---------------------------------------------------------------------------*/
class  SfxItemPropertySet
{
    const SfxItemPropertyMap* 	_pMap;
protected:
    virtual BOOL			FillItem(SfxItemSet& rSet, USHORT nWhich, BOOL bGetProperty) const;

public:
                            SfxItemPropertySet( const SfxItemPropertyMap *pMap ) :
                                _pMap(pMap) {}

    void getPropertyValue( const SfxItemPropertyMap& rMap,
                                            const SfxItemSet& rSet,
                                            ::com::sun::star::uno::Any& rAny) const
                                            throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any
        getPropertyValue( const SfxItemPropertyMap& rMap,
                                            const SfxItemSet& rSet ) const
                                            throw(::com::sun::star::uno::RuntimeException);
    void getPropertyValue( const ::rtl::OUString &rName,
                                            const SfxItemSet& rSet,
                                            ::com::sun::star::uno::Any& rAny) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                                    ::com::sun::star::beans::UnknownPropertyException);
    ::com::sun::star::uno::Any
        getPropertyValue( const ::rtl::OUString &rName,
                                            const SfxItemSet& rSet ) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                                    ::com::sun::star::beans::UnknownPropertyException);
    void					setPropertyValue( const SfxItemPropertyMap& rMap,
        const ::com::sun::star::uno::Any& aVal,
                                            SfxItemSet& rSet ) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                            ::com::sun::star::lang::IllegalArgumentException);
    void					setPropertyValue( const ::rtl::OUString& rPropertyName,
        const ::com::sun::star::uno::Any& aVal,
                                            SfxItemSet& rSet ) const
                                            throw(::com::sun::star::uno::RuntimeException,
                                            ::com::sun::star::lang::IllegalArgumentException,
                                                    ::com::sun::star::beans::UnknownPropertyException);

    ::com::sun::star::beans::PropertyState
        getPropertyState(const ::rtl::OUString& rName, const SfxItemSet& rSet)
        throw(::com::sun::star::beans::UnknownPropertyException);
    ::com::sun::star::beans::PropertyState
        getPropertyState(const SfxItemPropertyMap& rMap, const SfxItemSet& rSet)
                                    throw();

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo >
        getPropertySetInfo() const;
    const SfxItemPropertyMap*
        getPropertyMap() const {return _pMap;}

};

class  SfxItemPropertySetInfo : public cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo>
{
    const SfxItemPropertyMap* 	_pMap;

public:
    SfxItemPropertySetInfo(const SfxItemPropertyMap *pMap ) :
                                _pMap(pMap) {}

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

    const SfxItemPropertyMap* getMap() const { return _pMap; }

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();
};
/* -----------------------------21.02.00 12:01--------------------------------

 ---------------------------------------------------------------------------*/
class  SfxExtItemPropertySetInfo: public cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertySetInfo >
{
    const SfxItemPropertyMap* 											_pExtMap;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property>		aPropertySeq;
public:
                            SfxExtItemPropertySetInfo(
                                const SfxItemPropertyMap *pMap,
                                const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property>& rPropSeq );

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > SAL_CALL
        getProperties(  )
            throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::Property SAL_CALL
        getPropertyByName( const ::rtl::OUString& aName )
            throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        hasPropertyByName( const ::rtl::OUString& Name )
            throw(::com::sun::star::uno::RuntimeException);

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) throw ();
    virtual void SAL_CALL release(  ) throw ();
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
