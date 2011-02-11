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
#ifndef _LINGUISTIC_CONVDIC_HXX_
#define _LINGUISTIC_CONVDIC_HXX_

#include <com/sun/star/linguistic2/XConversionDictionary.hpp>
#include <com/sun/star/linguistic2/XConversionPropertyType.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <tools/string.hxx>

#include <hash_map>
#include <set>
#include <memory>
#include "linguistic/misc.hxx"
#include "defs.hxx"

// text conversion dictionary extension
#define CONV_DIC_EXT            "tcd"
#define CONV_DIC_DOT_EXT        ".tcd"

#define SN_CONV_DICTIONARY      "com.sun.star.linguistic2.ConversionDictionary"


class SvStream;

///////////////////////////////////////////////////////////////////////////

sal_Bool    IsConvDic( const String &rFileURL, sal_Int16 &nLang, sal_Int16 &nConvType );

///////////////////////////////////////////////////////////////////////////

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

typedef std::hash_multimap< const rtl::OUString, rtl::OUString,
                       const rtl::OUStringHash, StrEQ > ConvMap;

typedef std::set< rtl::OUString, StrLT > ConvMapKeySet;

typedef std::hash_multimap< const rtl::OUString, sal_Int16,
                       rtl::OUStringHash, StrEQ > PropTypeMap;

///////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////

#endif

