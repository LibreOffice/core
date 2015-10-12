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
#ifndef INCLUDED_LINGUISTIC_SOURCE_CONVDIC_HXX
#define INCLUDED_LINGUISTIC_SOURCE_CONVDIC_HXX

#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/XConversionPropertyType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.h>

#include <memory>
#include <set>
#include <unordered_map>
#include "linguistic/misc.hxx"
#include "defs.hxx"

// text conversion dictionary extension
#define CONV_DIC_EXT            "tcd"
#define CONV_DIC_DOT_EXT        ".tcd"

#define SN_CONV_DICTIONARY      "com.sun.star.linguistic2.ConversionDictionary"


bool    IsConvDic( const OUString &rFileURL, sal_Int16 &nLang, sal_Int16 &nConvType );

struct StrLT
{
    bool operator()( const OUString &rTxt1, const OUString &rTxt2 ) const
    {
        return rTxt1 < rTxt2;
    }
};

struct StrEQ
{
    bool operator()( const OUString &rTxt1, const OUString &rTxt2 ) const
    {
        return rTxt1 == rTxt2;
    }
};

typedef std::unordered_multimap< OUString, OUString,
                       const OUStringHash, StrEQ > ConvMap;

typedef std::set< OUString, StrLT > ConvMapKeySet;

typedef std::unordered_multimap< OUString, sal_Int16,
                       OUStringHash, StrEQ > PropTypeMap;


class ConvDic :
    public ::cppu::WeakImplHelper
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
    std::unique_ptr< ConvMap >        pFromRight;     // only available for bidirectional conversion dictionaries

    std::unique_ptr< PropTypeMap >    pConvPropType;

    OUString        aMainURL;   // URL to file
    OUString        aName;
    sal_Int16       nLanguage;
    sal_Int16       nConversionType;
    sal_Int16       nMaxLeftCharCount;
    sal_Int16       nMaxRightCharCount;
    bool            bMaxCharCountIsValid;
    bool            bNeedEntries;
    bool            bIsModified;
    bool            bIsActive;
    bool            bIsReadonly;

    // disallow copy-constructor and assignment-operator for now
    ConvDic(const ConvDic &);
    ConvDic & operator = (const ConvDic &);

    static ConvMap::iterator GetEntry( ConvMap &rMap, const OUString &rFirstText, const OUString &rSecondText );
    void    Load();
    void    Save();

public:
    ConvDic( const OUString &rName,
             sal_Int16 nLanguage,
             sal_Int16 nConversionType,
             bool bBiDirectional,
             const OUString &rMainURL);
    virtual ~ConvDic();

    // XConversionDictionary
    virtual OUString SAL_CALL getName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::lang::Locale SAL_CALL getLocale(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getConversionType(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setActive( sal_Bool bActivate ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isActive(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL clear(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, ::com::sun::star::linguistic2::ConversionDirection eDirection, sal_Int32 nTextConversionOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getConversionEntries( ::com::sun::star::linguistic2::ConversionDirection eDirection ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEntry( const OUString& aLeftText, const OUString& aRightText ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEntry( const OUString& aLeftText, const OUString& aRightText ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getMaxCharCount( ::com::sun::star::linguistic2::ConversionDirection eDirection ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XConversionPropertyType
    virtual void SAL_CALL setPropertyType( const OUString& aLeftText, const OUString& aRightText, ::sal_Int16 nPropertyType ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getPropertyType( const OUString& aLeftText, const OUString& aRightText ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XFlushable
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;


    static inline OUString
        getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< OUString >
        getSupportedServiceNames_Static() throw();

    bool    HasEntry( const OUString &rLeftText, const OUString &rRightText );
    void    AddEntry( const OUString &rLeftText, const OUString &rRightText );
    void    RemoveEntry( const OUString &rLeftText, const OUString &rRightText );
};

inline OUString ConvDic::getImplementationName_Static() throw()
{
    return OUString( "com.sun.star.lingu2.ConvDic" );
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
