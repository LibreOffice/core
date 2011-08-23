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
#ifndef _SWXFILTEROPTIONS_HXX
#define _SWXFILTEROPTIONS_HXX

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYACCESS_HPP_
#include <com/sun/star/beans/XPropertyAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_DIALOGS_XEXECUTABLEDIALOG_HPP_
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XIMPORTER_HPP_
#include <com/sun/star/document/XImporter.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEXPORTER_HPP_
#include <com/sun/star/document/XExporter.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx>
#endif
namespace com { namespace sun { namespace star { namespace io {
    class XInputStream;
} } } }

namespace binfilter {



/* -----------------------------2002/06/21 12:07------------------------------

 ---------------------------------------------------------------------------*/
class SwXFilterOptions : public ::cppu::WeakImplHelper5<
                            ::com::sun::star::beans::XPropertyAccess,
                            ::com::sun::star::ui::dialogs::XExecutableDialog,
                            ::com::sun::star::document::XImporter,
                            ::com::sun::star::document::XExporter,
                            ::com::sun::star::lang::XServiceInfo >
{
    ::rtl::OUString     sFilterName;
    ::rtl::OUString     sFilterOptions;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::io::XInputStream > xInputStream;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::lang::XComponent > xModel;
    sal_Bool            bExport;

public:
                            SwXFilterOptions();
    virtual                 ~SwXFilterOptions();

    static ::rtl::OUString  getImplementationName_Static();
    static ::com::sun::star::uno::Sequence< ::rtl::OUString> getSupportedServiceNames_Static();

                            // XPropertyAccess
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                            SAL_CALL getPropertyValues() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL	setPropertyValues( const ::com::sun::star::uno::Sequence<
                                    ::com::sun::star::beans::PropertyValue >& aProps )
                                throw (::com::sun::star::beans::UnknownPropertyException,
                                        ::com::sun::star::beans::PropertyVetoException,
                                        ::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::lang::WrappedTargetException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XExecutableDialog
    virtual void SAL_CALL	setTitle( const ::rtl::OUString& aTitle )
                                throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int16 SAL_CALL execute() throw (::com::sun::star::uno::RuntimeException);

                            // XImporter
    virtual void SAL_CALL	setTargetDocument( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::lang::XComponent >& xDoc )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XExporter
    virtual void SAL_CALL	setSourceDocument( const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::lang::XComponent >& xDoc )
                                throw (::com::sun::star::lang::IllegalArgumentException,
                                        ::com::sun::star::uno::RuntimeException);

                            // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName()
                                throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                                throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                                throw(::com::sun::star::uno::RuntimeException);

};

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
    SwXFilterOptions_createInstance(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XMultiServiceFactory >& );

} //namespace binfilter
#endif


