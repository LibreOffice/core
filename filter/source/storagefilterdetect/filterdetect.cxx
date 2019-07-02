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

#include "filterdetect.hxx"

#include <comphelper/documentconstants.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/mediadescriptor.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/brokenpackageint.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>

using namespace ::com::sun::star;
using utl::MediaDescriptor;

namespace {

OUString getInternalFromMediaType(const OUString& aMediaType)
{
    // OpenDocument types
    if (      aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII )                  return OUString("writer8");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII )         return OUString("writer8_template");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII )              return OUString("writerweb8_writer_template");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII )           return OUString("writerglobal8");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_TEMPLATE_ASCII )  return OUString("writerglobal8_template");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII )               return OUString("draw8");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII )      return OUString("draw8_template");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII )          return OUString("impress8");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII ) return OUString("impress8_template");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII )           return OUString("calc8");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII )  return OUString("calc8_template");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII )                 return OUString("chart8");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII )               return OUString("math8");
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_REPORT_CHART_ASCII )          return OUString("StarBaseReportChart");

    // OOo legacy types
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_WRITER_ASCII )           return OUString("writer_StarOffice_XML_Writer");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_WRITER_TEMPLATE_ASCII )  return OUString("writer_StarOffice_XML_Writer_Template");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII )       return OUString("writer_web_StarOffice_XML_Writer_Web_Template");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII )    return OUString("writer_globaldocument_StarOffice_XML_Writer_GlobalDocument");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_DRAW_ASCII )             return OUString("draw_StarOffice_XML_Draw");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_DRAW_TEMPLATE_ASCII )    return OUString("draw_StarOffice_XML_Draw_Template");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_IMPRESS_ASCII )          return OUString("impress_StarOffice_XML_Impress");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_IMPRESS_TEMPLATE_ASCII ) return OUString("impress_StarOffice_XML_Impress_Template");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_CALC_ASCII )             return OUString("calc_StarOffice_XML_Calc");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_CALC_TEMPLATE_ASCII )    return OUString("calc_StarOffice_XML_Calc_Template");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_CHART_ASCII )            return OUString("chart_StarOffice_XML_Chart");
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_MATH_ASCII )             return OUString("math_StarOffice_XML_Math");

    // Unknown type
    return OUString();
}

}

StorageFilterDetect::StorageFilterDetect(const uno::Reference<uno::XComponentContext>& xCxt) :
    mxCxt(xCxt) {}

StorageFilterDetect::~StorageFilterDetect() {}

OUString SAL_CALL StorageFilterDetect::detect(uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    MediaDescriptor aMediaDesc( rDescriptor );
    OUString aTypeName;

    try
    {
        uno::Reference< io::XInputStream > xInStream( aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM()], uno::UNO_QUERY );
        if ( !xInStream.is() )
            return OUString();

        uno::Reference< embed::XStorage > xStorage = ::comphelper::OStorageHelper::GetStorageFromInputStream( xInStream, mxCxt );
        if ( !xStorage.is() )
            return OUString();

        uno::Reference< beans::XPropertySet > xStorageProperties( xStorage, uno::UNO_QUERY );
        if ( !xStorageProperties.is() )
            return OUString();

        OUString aMediaType;
        xStorageProperties->getPropertyValue( "MediaType" ) >>= aMediaType;
        aTypeName = getInternalFromMediaType( aMediaType );
    }

    catch( const lang::WrappedTargetException& aWrap )
    {
        packages::zip::ZipIOException aZipException;
        // We don't do any type detection on broken packages (f.e. because it might be impossible),
        // so for repairing we'll use the requested type, which was detected by the flat detection.
        OUString aRequestedTypeName = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_TYPENAME(), OUString() );
        if ( ( aWrap.TargetException >>= aZipException ) && !aRequestedTypeName.isEmpty() )
        {
            // The package is a broken one.
            uno::Reference< task::XInteractionHandler > xInteraction =
                aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_INTERACTIONHANDLER(), uno::Reference< task::XInteractionHandler >() );

            if ( xInteraction.is() )
            {
                INetURLObject aParser( aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_URL(), OUString() ) );
                OUString aDocumentTitle = aParser.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
                bool bRepairPackage = aMediaDesc.getUnpackedValueOrDefault( "RepairPackage", false );
                // fdo#46310 Don't try to repair if the user rejected it once.
                bool bRepairAllowed = aMediaDesc.getUnpackedValueOrDefault( "RepairAllowed", true );

                if ( !bRepairPackage && bRepairAllowed )
                {
                    // Ask the user whether he wants to try to repair.
                    RequestPackageReparation aRequest( aDocumentTitle );
                    xInteraction->handle( aRequest.GetRequest() );

                    if ( aRequest.isApproved() )
                    {
                        aTypeName = aRequestedTypeName;
                        aMediaDesc[MediaDescriptor::PROP_DOCUMENTTITLE()] <<= aDocumentTitle;
                        aMediaDesc[MediaDescriptor::PROP_ASTEMPLATE()] <<= true;
                        aMediaDesc["RepairPackage"] <<= true;
                    }
                    else
                    {
                        // Repair either not allowed or not successful.
                        NotifyBrokenPackage aNotifyRequest( aDocumentTitle );
                        xInteraction->handle( aNotifyRequest.GetRequest() );
                        aMediaDesc["RepairAllowed"] <<= false;
                    }

                    // Write the changes back.
                    aMediaDesc >> rDescriptor;
                }
            }
        }
    }
    catch( uno::RuntimeException& )
    {
        throw;
    }
    catch( uno::Exception& )
    {}

    return aTypeName;
}

// XInitialization
void SAL_CALL StorageFilterDetect::initialize(const uno::Sequence<uno::Any>& /*aArguments*/) {}

OUString StorageFilterDetect_getImplementationName()
{
    return OUString("com.sun.star.comp.filters.StorageFilterDetect");
}

uno::Sequence<OUString> StorageFilterDetect_getSupportedServiceNames()
{
    uno::Sequence<OUString> aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = "com.sun.star.document.ExtendedTypeDetection";
    pArray[1] = "com.sun.star.comp.filters.StorageFilterDetect";
    return aRet;
}

uno::Reference<uno::XInterface> StorageFilterDetect_createInstance(
    const uno::Reference<uno::XComponentContext> & rCxt)
{
    return static_cast<cppu::OWeakObject*>(new StorageFilterDetect(rCxt));
}

// XServiceInfo
OUString SAL_CALL StorageFilterDetect::getImplementationName()
{
    return StorageFilterDetect_getImplementationName();
}

sal_Bool SAL_CALL StorageFilterDetect::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL StorageFilterDetect::getSupportedServiceNames()
{
    return StorageFilterDetect_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
