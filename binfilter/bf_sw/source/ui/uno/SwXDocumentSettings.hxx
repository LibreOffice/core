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

#ifndef _SW_XDOCUMENT_SETTINGS_HXX
#define _SW_XDOCUMENT_SETTINGS_HXX

#ifdef PRECOMPILED
#endif
#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _COMPHELPER_MASTERPROPERTYSET_HXX_ 
#include <comphelper/MasterPropertySet.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include  <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
namespace binfilter {


class SwXTextDocument;
class SwDocShell;
class SwDoc;
class SfxPrinter;

class SwXDocumentSettings : 
        public comphelper::MasterPropertySet,
        public ::com::sun::star::lang::XServiceInfo,
        public ::com::sun::star::lang::XTypeProvider,
        public cppu::OWeakObject
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument >		mxModel;
    SwXTextDocument*		mpModel;
    SwDocShell* 			mpDocSh;
    SwDoc* 					mpDoc;

    /** the printer should be set only once; since there are several
     * printer-related properties, remember the last printer and set it in
     * _postSetValues */
    SfxPrinter*             mpPrinter;


    virtual void _preSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _setSingleValue( const comphelper::PropertyInfo & rInfo, const ::com::sun::star::uno::Any &rValue ) 
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postSetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );

    virtual void _preGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _getSingleValue( const comphelper::PropertyInfo & rInfo, ::com::sun::star::uno::Any & rValue ) 
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException );
    virtual void _postGetValues ()
        throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException );
protected:
    virtual ~SwXDocumentSettings()
        throw();
public:
    SwXDocumentSettings( SwXTextDocument* pModel );
    

    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) 
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  ) 
        throw ();
    virtual void SAL_CALL release(  ) 
        throw ();

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) 
        throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) 
        throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) 
        throw(::com::sun::star::uno::RuntimeException);

    // XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw (::com::sun::star::uno::RuntimeException);
};
} //namespace binfilter
#endif
