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
#include <cppuhelper/supportsservice.hxx>
#include <unotools/mediadescriptor.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/brokenpackageint.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <utility>

#include <comphelper/lok.hxx>

using namespace ::com::sun::star;
using utl::MediaDescriptor;

namespace {

OUString getInternalFromMediaType(std::u16string_view aMediaType)
{
    // OpenDocument types
    if (      aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_ASCII )                  return u"writer8"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_TEMPLATE_ASCII )         return u"writer8_template"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_WEB_ASCII )              return u"writerweb8_writer_template"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_ASCII )           return u"writerglobal8"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_TEXT_GLOBAL_TEMPLATE_ASCII )  return u"writerglobal8_template"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_ASCII )               return u"draw8"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_DRAWING_TEMPLATE_ASCII )      return u"draw8_template"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_ASCII )          return u"impress8"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_PRESENTATION_TEMPLATE_ASCII ) return u"impress8_template"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_ASCII )           return u"calc8"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET_TEMPLATE_ASCII )  return u"calc8_template"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_CHART_ASCII )                 return u"chart8"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_FORMULA_ASCII )               return u"math8"_ustr;
    else if ( aMediaType == MIMETYPE_OASIS_OPENDOCUMENT_REPORT_CHART_ASCII )          return u"StarBaseReportChart"_ustr;

    // OOo legacy types
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_WRITER_ASCII )           return u"writer_StarOffice_XML_Writer"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_WRITER_TEMPLATE_ASCII )  return u"writer_StarOffice_XML_Writer_Template"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_WRITER_WEB_ASCII )       return u"writer_web_StarOffice_XML_Writer_Web_Template"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_WRITER_GLOBAL_ASCII )    return u"writer_globaldocument_StarOffice_XML_Writer_GlobalDocument"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_DRAW_ASCII )             return u"draw_StarOffice_XML_Draw"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_DRAW_TEMPLATE_ASCII )    return u"draw_StarOffice_XML_Draw_Template"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_IMPRESS_ASCII )          return u"impress_StarOffice_XML_Impress"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_IMPRESS_TEMPLATE_ASCII ) return u"impress_StarOffice_XML_Impress_Template"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_CALC_ASCII )             return u"calc_StarOffice_XML_Calc"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_CALC_TEMPLATE_ASCII )    return u"calc_StarOffice_XML_Calc_Template"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_CHART_ASCII )            return u"chart_StarOffice_XML_Chart"_ustr;
    else if ( aMediaType == MIMETYPE_VND_SUN_XML_MATH_ASCII )             return u"math_StarOffice_XML_Math"_ustr;

    // Unknown type
    return OUString();
}

}

StorageFilterDetect::StorageFilterDetect(uno::Reference<uno::XComponentContext> xCxt) :
    mxCxt(std::move(xCxt)) {}

StorageFilterDetect::~StorageFilterDetect() {}

OUString SAL_CALL StorageFilterDetect::detect(uno::Sequence<beans::PropertyValue>& rDescriptor)
{
    MediaDescriptor aMediaDesc( rDescriptor );
    OUString aTypeName;

    try
    {
        uno::Reference< io::XInputStream > xInStream( aMediaDesc[MediaDescriptor::PROP_INPUTSTREAM], uno::UNO_QUERY );
        if ( !xInStream.is() )
            return OUString();

        uno::Reference< embed::XStorage > xStorage = ::comphelper::OStorageHelper::GetStorageFromInputStream( xInStream, mxCxt );
        if ( !xStorage.is() )
            return OUString();

        uno::Reference< beans::XPropertySet > xStorageProperties( xStorage, uno::UNO_QUERY );
        if ( !xStorageProperties.is() )
            return OUString();

        OUString aMediaType;
        xStorageProperties->getPropertyValue( u"MediaType"_ustr ) >>= aMediaType;
        aTypeName = getInternalFromMediaType( aMediaType );
        if (comphelper::LibreOfficeKit::isActive() && aTypeName == "draw8_template")
        {
            // save it as draw8 instead of template format
            aTypeName = "draw8";
        }
    }

    catch( const lang::WrappedTargetException& aWrap )
    {
        packages::zip::ZipIOException aZipException;
        // We don't do any type detection on broken packages (f.e. because it might be impossible),
        // so for repairing we'll use the requested type, which was detected by the flat detection.
        OUString aRequestedTypeName = aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_TYPENAME, OUString() );
        if ( ( aWrap.TargetException >>= aZipException ) && !aRequestedTypeName.isEmpty() )
        {
            // The package is a broken one.
            INetURLObject aParser(
                aMediaDesc.getUnpackedValueOrDefault(MediaDescriptor::PROP_URL, OUString()));
            OUString aDocumentTitle = aParser.getName(INetURLObject::LAST_SEGMENT, true,
                                                      INetURLObject::DecodeMechanism::WithCharset);
            bool bRepairPackage = aMediaDesc.getUnpackedValueOrDefault(u"RepairPackage"_ustr, false);
            // fdo#46310 Don't ask to repair if the user rejected it once.
            if (!bRepairPackage && aMediaDesc.getUnpackedValueOrDefault(u"RepairAllowed"_ustr, true))
            {
                uno::Reference< task::XInteractionHandler > xInteraction =
                    aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_INTERACTIONHANDLER, uno::Reference< task::XInteractionHandler >() );

                if ( xInteraction.is() )
                {
                    // Ask the user whether he wants to try to repair.
                    RequestPackageReparation aRequest(aDocumentTitle);
                    xInteraction->handle(aRequest.GetRequest());
                    bRepairPackage = aRequest.isApproved();
                    if (!bRepairPackage)
                    {
                        // Repair either not allowed or not successful.
                        NotifyBrokenPackage aNotifyRequest( aDocumentTitle );
                        xInteraction->handle( aNotifyRequest.GetRequest() );
                        aMediaDesc[u"RepairAllowed"_ustr] <<= false;
                    }
                }
            }
            if (bRepairPackage)
            {
                aTypeName = aRequestedTypeName;
                // lok: we want to overwrite file in jail, so don't use template flag
                const bool bIsLOK = comphelper::LibreOfficeKit::isActive();
                aMediaDesc[MediaDescriptor::PROP_DOCUMENTTITLE] <<= aDocumentTitle;
                aMediaDesc[MediaDescriptor::PROP_ASTEMPLATE] <<= !bIsLOK;
                aMediaDesc[u"RepairPackage"_ustr] <<= true;
            }

            // Write the changes back.
            aMediaDesc >> rDescriptor;
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

// XServiceInfo
OUString SAL_CALL StorageFilterDetect::getImplementationName()
{
    return u"com.sun.star.comp.filters.StorageFilterDetect"_ustr;
}

sal_Bool SAL_CALL StorageFilterDetect::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence<OUString> SAL_CALL StorageFilterDetect::getSupportedServiceNames()
{
    return { u"com.sun.star.document.ExtendedTypeDetection"_ustr, u"com.sun.star.comp.filters.StorageFilterDetect"_ustr };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_StorageFilterDetect_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new StorageFilterDetect(context));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
