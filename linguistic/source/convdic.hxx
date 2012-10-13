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
#ifndef _LINGUISTIC_CONVDIC_HXX_
#define _LINGUISTIC_CONVDIC_HXX_

#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/XConversionPropertyType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <tools/string.hxx>

#include <boost/unordered_map.hpp>
#include <set>
#include <memory>
#include "linguistic/misc.hxx"
#include "defs.hxx"

// text conversion dictionary extension
#define CONV_DIC_EXT            "tcd"
#define CONV_DIC_DOT_EXT        ".tcd"

#define SN_CONV_DICTIONARY      "com.sun.star.linguistic2.ConversionDictionary"


sal_Bool    IsConvDic( const String &rFileURL, sal_Int16 &nLang, sal_Int16 &nConvType );

struct StrLT
{
    bool operator()( const rtl::OUString &rTxt1, const rtl::OUString &rTxt2 ) const
    {
        return rTxt1 < rTxt2;
    }
};

struct StrEQ
{
    bool operator()( const rtl::OUString &rTxt1, const rtl::OUString &rTxt2 ) const
    {
        return rTxt1 == rTxt2;
    }
};

typedef boost::unordered_multimap< const rtl::OUString, rtl::OUString,
                       const rtl::OUStringHash, StrEQ > ConvMap;

typedef std::set< rtl::OUString, StrLT > ConvMapKeySet;

typedef boost::unordered_multimap< const rtl::OUString, sal_Int16,
                       rtl::OUStringHash, StrEQ > PropTypeMap;


class ConvDic :
    public ::cppu::WeakImplHelper4
    <
        ::com::sun::star::linguistic2::XConversionDictionary,
        ::com::sun::star::linguistic2::XConversionPropertyType,
        ::com::sun::star::util::XFlushable,
        ::com::sun::star::lang::XServiceInfo
    >
{
    friend class ConvDicXMLExport;

protected:

    ::cppu::OInterfaceContainerHelper       aFlushListeners;

    ConvMap                         aFromLeft;
    std::auto_ptr< ConvMap >        pFromRight;     // only available for bidirectional conversion dictionaries

    std::auto_ptr< PropTypeMap >    pConvPropType;

    String          aMainURL;   // URL to file
    rtl::OUString   aName;
    sal_Int16           nLanguage;
    sal_Int16       nConversionType;
    sal_Int16       nMaxLeftCharCount;
    sal_Int16       nMaxRightCharCount;
    sal_Bool            bMaxCharCountIsValid;
    sal_Bool            bNeedEntries;
    sal_Bool            bIsModified;
    sal_Bool            bIsActive;
    sal_Bool            bIsReadonly;

    // disallow copy-constructor and assignment-operator for now
    ConvDic(const ConvDic &);
    ConvDic & operator = (const ConvDic &);

    ConvMap::iterator   GetEntry( ConvMap &rMap, const rtl::OUString &rFirstText, const rtl::OUString &rSecondText );
    void    Load();
    void    Save();

public:
    ConvDic( const String &rName,
             sal_Int16 nLanguage,
             sal_Int16 nConversionType,
             sal_Bool bBiDirectional,
             const String &rMainURL);
    virtual ~ConvDic();

    // XConversionDictionary
    virtual ::rtl::OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getConversionType(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActive( sal_Bool bActivate ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isActive(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL clear(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, ::com::sun::star::linguistic2::ConversionDirection eDirection, sal_Int32 nTextConversionOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getConversionEntries( ::com::sun::star::linguistic2::ConversionDirection eDirection ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEntry( const ::rtl::OUString& aLeftText, const ::rtl::OUString& aRightText ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEntry( const ::rtl::OUString& aLeftText, const ::rtl::OUString& aRightText ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL getMaxCharCount( ::com::sun::star::linguistic2::ConversionDirection eDirection ) throw (::com::sun::star::uno::RuntimeException);

    // XConversionPropertyType
    virtual void SAL_CALL setPropertyType( const ::rtl::OUString& aLeftText, const ::rtl::OUString& aRightText, ::sal_Int16 nPropertyType ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getPropertyType( const ::rtl::OUString& aLeftText, const ::rtl::OUString& aRightText ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);

    // XFlushable
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);


    static inline ::rtl::OUString
        getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< ::rtl::OUString >
        getSupportedServiceNames_Static() throw();

    sal_Bool    HasEntry( const rtl::OUString &rLeftText, const rtl::OUString &rRightText );
    void    AddEntry( const rtl::OUString &rLeftText, const rtl::OUString &rRightText );
    void    RemoveEntry( const rtl::OUString &rLeftText, const rtl::OUString &rRightText );
};

inline ::rtl::OUString ConvDic::getImplementationName_Static() throw()
{
    return A2OU( "com.sun.star.lingu2.ConvDic" );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
