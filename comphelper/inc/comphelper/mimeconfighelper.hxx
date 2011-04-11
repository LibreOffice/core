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

#ifndef _COMPHELPER_MIMECONFIGHELPER_HXX_
#define _COMPHELPER_MIMECONFIGHELPER_HXX_

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/embed/VerbDescriptor.hpp>
#include "comphelper/comphelperdllapi.h"


namespace comphelper {

class COMPHELPER_DLLPUBLIC MimeConfigurationHelper
{
    ::osl::Mutex m_aMutex;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xConfigProvider;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xObjectConfig;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xVerbsConfig;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xMediaTypeConfig;

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > m_xFilterFactory;

public:

    MimeConfigurationHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );


    static ::rtl::OUString GetStringClassIDRepresentation( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID );

    static ::com::sun::star::uno::Sequence< sal_Int8 > GetSequenceClassIDRepresentation( const ::rtl::OUString& aClassID );


    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                                            GetConfigurationByPath( const ::rtl::OUString& aPath );

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetObjConfiguration();

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetVerbsConfiguration();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetMediaTypeConfiguration();


    ::rtl::OUString GetDocServiceNameFromFilter( const ::rtl::OUString& aFilterName );

    ::rtl::OUString GetDocServiceNameFromMediaType( const ::rtl::OUString& aMediaType );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjPropsFromConfigEntry(
                        const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& xObjectProps );

    sal_Bool GetVerbByShortcut( const ::rtl::OUString& aVerbShortcut,
                                ::com::sun::star::embed::VerbDescriptor& aDescriptor );

    ::rtl::OUString GetExplicitlyRegisteredObjClassID( const ::rtl::OUString& aMediaType );


    // retrieving object description from configuration
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByStringClassID(
                                                                const ::rtl::OUString& aStringClassID );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByClassID(
                                                const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByMediaType(
                                                const ::rtl::OUString& aMediaType );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByFilter(
                                                const ::rtl::OUString& aFilterName );

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > GetObjectPropsByDocumentName(
                                                const ::rtl::OUString& aDocumentName );

    // retrieving object factory from configuration
    ::rtl::OUString GetFactoryNameByStringClassID( const ::rtl::OUString& aStringClassID );
    ::rtl::OUString GetFactoryNameByClassID( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID );
    ::rtl::OUString GetFactoryNameByDocumentName( const ::rtl::OUString& aDocName );
    ::rtl::OUString GetFactoryNameByMediaType( const ::rtl::OUString& aMediaType );

    // typedetection related
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > GetFilterFactory();

    sal_Int32 GetFilterFlags( const ::rtl::OUString& aFilterName );

    ::rtl::OUString UpdateMediaDescriptorWithFilterName(
                        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr,
                        sal_Bool bIgnoreType );
    ::rtl::OUString UpdateMediaDescriptorWithFilterName(
                        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr,
                        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aObject );
    sal_Bool AddFilterNameCheckOwnFile(
                        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aMediaDescr );

    ::rtl::OUString GetDefaultFilterFromServiceName( const ::rtl::OUString& aServName, sal_Int32 nVersion );

    ::rtl::OUString GetExportFilterFromImportFilter( const ::rtl::OUString& aImportFilterName );

    static ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SearchForFilter(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerQuery >& xFilterQuery,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& aSearchRequest,
                        sal_Int32 nMustFlags,
                        sal_Int32 nDontFlags );

    static sal_Bool ClassIDsEqual( const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID1,
                        const ::com::sun::star::uno::Sequence< sal_Int8 >& aClassID2 );
    static ::com::sun::star::uno::Sequence< sal_Int8 > GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 );

    ::com::sun::star::uno::Sequence<sal_Int8> GetSequenceClassIDFromObjectName(const ::rtl::OUString& _sObjectName) ;

};

}

#endif // _COMPHELPER_MIMECONFIGHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
