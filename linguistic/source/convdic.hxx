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
#include <comphelper/interfacecontainer2.hxx>
#include <i18nlangtag/lang.h>

#include <memory>
#include <set>
#include <unordered_map>

// text conversion dictionary extension
#define CONV_DIC_EXT            "tcd"
#define CONV_DIC_DOT_EXT        ".tcd"

#define SN_CONV_DICTIONARY      "com.sun.star.linguistic2.ConversionDictionary"


bool    IsConvDic( const OUString &rFileURL, LanguageType &nLang, sal_Int16 &nConvType );

typedef std::unordered_multimap<OUString, OUString> ConvMap;
typedef std::unordered_multimap<OUString, sal_Int16> PropTypeMap;

class ConvDic :
    public ::cppu::WeakImplHelper
    <
        css::linguistic2::XConversionDictionary,
        css::linguistic2::XConversionPropertyType,
        css::util::XFlushable,
        css::lang::XServiceInfo
    >
{
    friend class ConvDicXMLExport;

protected:

    ::comphelper::OInterfaceContainerHelper2       aFlushListeners;

    ConvMap                         aFromLeft;
    std::unique_ptr< ConvMap >        pFromRight;     // only available for bidirectional conversion dictionaries

    std::unique_ptr< PropTypeMap >    pConvPropType;

    OUString        aMainURL;   // URL to file
    OUString        aName;
    LanguageType    nLanguage;
    sal_Int16       nConversionType;
    sal_Int16       nMaxLeftCharCount;
    sal_Int16       nMaxRightCharCount;
    bool            bMaxCharCountIsValid;
    bool            bNeedEntries;
    bool            bIsModified;
    bool            bIsActive;

    // disallow copy-constructor and assignment-operator for now
    ConvDic(const ConvDic &);
    ConvDic & operator = (const ConvDic &);

    static ConvMap::iterator GetEntry( ConvMap &rMap, const OUString &rFirstText, const OUString &rSecondText );
    void    Load();
    void    Save();

public:
    ConvDic( const OUString &rName,
             LanguageType nLanguage,
             sal_Int16 nConversionType,
             bool bBiDirectional,
             const OUString &rMainURL);
    virtual ~ConvDic() override;

    // XConversionDictionary
    virtual OUString SAL_CALL getName(  ) override;
    virtual css::lang::Locale SAL_CALL getLocale(  ) override;
    virtual sal_Int16 SAL_CALL getConversionType(  ) override;
    virtual void SAL_CALL setActive( sal_Bool bActivate ) override;
    virtual sal_Bool SAL_CALL isActive(  ) override;
    virtual void SAL_CALL clear(  ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getConversions( const OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, css::linguistic2::ConversionDirection eDirection, sal_Int32 nTextConversionOptions ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getConversionEntries( css::linguistic2::ConversionDirection eDirection ) override;
    virtual void SAL_CALL addEntry( const OUString& aLeftText, const OUString& aRightText ) override;
    virtual void SAL_CALL removeEntry( const OUString& aLeftText, const OUString& aRightText ) override;
    virtual sal_Int16 SAL_CALL getMaxCharCount( css::linguistic2::ConversionDirection eDirection ) override;

    // XConversionPropertyType
    virtual void SAL_CALL setPropertyType( const OUString& aLeftText, const OUString& aRightText, ::sal_Int16 nPropertyType ) override;
    virtual ::sal_Int16 SAL_CALL getPropertyType( const OUString& aLeftText, const OUString& aRightText ) override;

    // XFlushable
    virtual void SAL_CALL flush(  ) override;
    virtual void SAL_CALL addFlushListener( const css::uno::Reference< css::util::XFlushListener >& l ) override;
    virtual void SAL_CALL removeFlushListener( const css::uno::Reference< css::util::XFlushListener >& l ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    bool    HasEntry( const OUString &rLeftText, const OUString &rRightText );
    void    AddEntry( const OUString &rLeftText, const OUString &rRightText );
    void    RemoveEntry( const OUString &rLeftText, const OUString &rRightText );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
