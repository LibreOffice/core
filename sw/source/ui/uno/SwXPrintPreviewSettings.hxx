/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwXPrintPreviewSettings.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:40:49 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SW_XPRINTPREVIEWSETTINGS_HXX_
#define _SW_XPRINTPREVIEWSETTINGS_HXX_

#ifdef PRECOMPILED
#include "ui_pch.hxx"
#endif

#ifndef _COMPHELPER_CHAINABLEPROPERTYSET_HXX_
#include <comphelper/ChainablePropertySet.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTDOCUMENT_HPP
#include <com/sun/star/text/XTextDocument.hpp>
#endif
#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

class SwDoc;
class SwPagePreViewPrtData;

class SwXPrintPreviewSettings :public comphelper::ChainablePropertySet,
                        public cppu::OWeakObject,
                        public com::sun::star::lang::XServiceInfo
{
    friend class SwXDocumentSettings;
protected:
    sal_Bool        mbPreviewDataChanged;
    SwDoc*          mpDoc;
    SwPagePreViewPrtData *mpPreViewData;
    const SwPagePreViewPrtData *mpConstPreViewData;

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
    virtual ~SwXPrintPreviewSettings()
        throw();
public:
    SwXPrintPreviewSettings(SwDoc *pDoc);


    // XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire(  )
        throw ();
    virtual void SAL_CALL release(  )
        throw ();

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
        throw( ::com::sun::star::uno::RuntimeException );
};
#endif
