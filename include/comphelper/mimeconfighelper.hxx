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

#ifndef INCLUDED_COMPHELPER_MIMECONFIGHELPER_HXX
#define INCLUDED_COMPHELPER_MIMECONFIGHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <comphelper/comphelperdllapi.h>
#include <mutex>

namespace com::sun::star::beans { struct NamedValue; }
namespace com::sun::star::beans { struct PropertyValue; }
namespace com::sun::star::container { class XContainerQuery; }
namespace com::sun::star::container { class XNameAccess; }
namespace com::sun::star::embed { struct VerbDescriptor; }
namespace com::sun::star::lang { class XMultiServiceFactory; }
namespace com::sun::star::uno { class XComponentContext; }

enum class SfxFilterFlags;

namespace comphelper {

class COMPHELPER_DLLPUBLIC MimeConfigurationHelper
{
    std::mutex                                             m_aMutex;
    css::uno::Reference< css::uno::XComponentContext >     m_xContext;
    css::uno::Reference< css::lang::XMultiServiceFactory > m_xConfigProvider;

    css::uno::Reference< css::container::XNameAccess > m_xObjectConfig;
    css::uno::Reference< css::container::XNameAccess > m_xVerbsConfig;
    css::uno::Reference< css::container::XNameAccess > m_xMediaTypeConfig;

    css::uno::Reference< css::container::XNameAccess > m_xFilterFactory;

public:

    MimeConfigurationHelper( css::uno::Reference< css::uno::XComponentContext > xContext );


    static OUString GetStringClassIDRepresentation( const css::uno::Sequence< sal_Int8 >& aClassID );

    static css::uno::Sequence< sal_Int8 > GetSequenceClassIDRepresentation( std::u16string_view aClassID );


    css::uno::Reference< css::container::XNameAccess > GetObjConfiguration();
    css::uno::Reference< css::container::XNameAccess > GetVerbsConfiguration();
    css::uno::Reference< css::container::XNameAccess > GetMediaTypeConfiguration();


    OUString GetDocServiceNameFromFilter( const OUString& aFilterName );

    OUString GetDocServiceNameFromMediaType( const OUString& aMediaType );

    css::uno::Sequence< css::beans::NamedValue > GetObjPropsFromConfigEntry(
                        const css::uno::Sequence< sal_Int8 >& aClassID,
                        const css::uno::Reference< css::container::XNameAccess >& xObjectProps );

    bool GetVerbByShortcut( const OUString& aVerbShortcut,
                                css::embed::VerbDescriptor& aDescriptor );

    OUString GetExplicitlyRegisteredObjClassID( const OUString& aMediaType );


    // retrieving object description from configuration
    css::uno::Sequence< css::beans::NamedValue > GetObjectPropsByStringClassID(
                                                                const OUString& aStringClassID );

    css::uno::Sequence< css::beans::NamedValue > GetObjectPropsByClassID(
                                                const css::uno::Sequence< sal_Int8 >& aClassID );

    css::uno::Sequence< css::beans::NamedValue > GetObjectPropsByMediaType(
                                                const OUString& aMediaType );

    css::uno::Sequence< css::beans::NamedValue > GetObjectPropsByFilter(
                                                const OUString& aFilterName );

    css::uno::Sequence< css::beans::NamedValue > GetObjectPropsByDocumentName(
                                                std::u16string_view aDocumentName );

    // retrieving object factory from configuration
    OUString GetFactoryNameByStringClassID( const OUString& aStringClassID );
    OUString GetFactoryNameByClassID( const css::uno::Sequence< sal_Int8 >& aClassID );
    OUString GetFactoryNameByDocumentName( std::u16string_view aDocName );
    OUString GetFactoryNameByMediaType( const OUString& aMediaType );

    // typedetection related
    css::uno::Reference< css::container::XNameAccess > GetFilterFactory();

    OUString UpdateMediaDescriptorWithFilterName(
                        css::uno::Sequence< css::beans::PropertyValue >& aMediaDescr,
                        bool bIgnoreType );
    OUString UpdateMediaDescriptorWithFilterName(
                        css::uno::Sequence< css::beans::PropertyValue >& aMediaDescr,
                        css::uno::Sequence< css::beans::NamedValue >& aObject );
#ifdef _WIN32
    SfxFilterFlags GetFilterFlags( const OUString& aFilterName );

    bool AddFilterNameCheckOwnFile(
                        css::uno::Sequence< css::beans::PropertyValue >& aMediaDescr );
#endif

    OUString GetDefaultFilterFromServiceName( const OUString& aServName, sal_Int32 nVersion );

    OUString GetExportFilterFromImportFilter( const OUString& aImportFilterName );

    static css::uno::Sequence< css::beans::PropertyValue > SearchForFilter(
                        const css::uno::Reference< css::container::XContainerQuery >& xFilterQuery,
                        const css::uno::Sequence< css::beans::NamedValue >& aSearchRequest,
                        SfxFilterFlags nMustFlags,
                        SfxFilterFlags nDontFlags );

    static bool ClassIDsEqual( const css::uno::Sequence< sal_Int8 >& aClassID1,
                        const css::uno::Sequence< sal_Int8 >& aClassID2 );
    static css::uno::Sequence< sal_Int8 > GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 );
private:
    css::uno::Reference< css::container::XNameAccess >
                                            GetConfigurationByPathImpl( const OUString& aPath );
};

}

#endif // INCLUDED_COMPHELPER_MIMECONFIGHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
