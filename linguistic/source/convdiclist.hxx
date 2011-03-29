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

#ifndef _LINGUISTIC_CONVDICLIST_HXX_
#define _LINGUISTIC_CONVDICLIST_HXX_

#include <com/sun/star/linguistic2/XConversionDictionaryList.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <svl/svarray.hxx>
#include <tools/debug.hxx>

#include "linguistic/misc.hxx"
#include "lngopt.hxx"


class ConvDicNameContainer;

///////////////////////////////////////////////////////////////////////////

class ConvDicList :
    public cppu::WeakImplHelper3
    <
        ::com::sun::star::linguistic2::XConversionDictionaryList,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{

    class MyAppExitListener : public linguistic::AppExitListener
    {
        ConvDicList & rMyDicList;

    public:
        MyAppExitListener( ConvDicList &rDicList ) : rMyDicList( rDicList ) {}
        virtual void    AtExit();
    };


    ::cppu::OInterfaceContainerHelper       aEvtListeners;

    ConvDicNameContainer                   *pNameContainer;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameContainer >   xNameContainer;

    MyAppExitListener                      *pExitListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::
                XTerminateListener >        xExitListener;

    sal_Bool                                    bDisposing;

    // disallow copy-constructor and assignment-operator for now
    ConvDicList( const ConvDicList & );
    ConvDicList & operator = (const ConvDicList &);

    ConvDicNameContainer &  GetNameContainer();

public:
    ConvDicList();
    virtual ~ConvDicList();

    // XConversionDictionaryList
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > SAL_CALL getDictionaryContainer(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XConversionDictionary > SAL_CALL addNewDictionary( const ::rtl::OUString& aName, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL queryConversions( const ::rtl::OUString& aText, sal_Int32 nStartPos, sal_Int32 nLength, const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, ::com::sun::star::linguistic2::ConversionDirection eDirection, sal_Int32 nTextConversionOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL queryMaxCharCount( const ::com::sun::star::lang::Locale& aLocale, sal_Int16 nConversionDictionaryType, ::com::sun::star::linguistic2::ConversionDirection eDirection ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);


    static inline ::rtl::OUString
        getImplementationName_Static() throw();
    static com::sun::star::uno::Sequence< ::rtl::OUString >
        getSupportedServiceNames_Static() throw();

    // non UNO-specific
    void                    FlushDics();
};

inline ::rtl::OUString ConvDicList::getImplementationName_Static() throw()
{
    return A2OU( "com.sun.star.lingu2.ConvDicList" );
}

///////////////////////////////////////////////////////////////////////////

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
