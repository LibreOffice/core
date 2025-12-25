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

#include <sal/config.h>

#include <array>
#include <string_view>

#include <o3tl/string_view.hxx>
#include <osl/diagnose.h>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>

#include "ftransl.hxx"
#include <com/sun/star/container/NoSuchElementException.hpp>
#include <com/sun/star/datatransfer/XMimeContentType.hpp>
#include <com/sun/star/datatransfer/MimeContentTypeFactory.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include "ImplHelper.hxx"

#include <shlobj.h>

#define CPPUTYPE_SEQSALINT8       cppu::UnoType<Sequence< sal_Int8 >>::get()

constexpr OUString Windows_FormatName = u"windows_formatname"_ustr;

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;

namespace
{
css::datatransfer::DataFlavor getDataFlavor(SotClipboardFormatId sotFormatId)
{
    css::datatransfer::DataFlavor aDataFlavor;
    SotExchange::GetFormatDataFlavor(sotFormatId, aDataFlavor);
    return aDataFlavor;
}

OUString getNativeFormatName(const css::datatransfer::DataFlavor& rFlavor,
                             const Reference<XMimeContentTypeFactory>& xFactory)
{
    auto xMimeContentType(xFactory->createMimeContentType(rFlavor.MimeType));
    if (xMimeContentType->hasParameter(Windows_FormatName))
        return xMimeContentType->getParameterValue(Windows_FormatName);
    return rFlavor.HumanPresentableName;
}

struct FormatEntry
{
    FormatEntry(const OUString& mime_content_type, const OUString& human_presentable_name,
                CLIPFORMAT std_clipboard_format_id,
                css::uno::Type const& cppu_type = CPPUTYPE_SEQSALINT8)
    : aDataFlavor(mime_content_type, human_presentable_name, cppu_type)
    , aNativeFormatName(human_presentable_name)
    , aStandardFormatId(std_clipboard_format_id)
    {
    }

    FormatEntry(const Reference<XMimeContentTypeFactory>& xFactory,
                SotClipboardFormatId sotFormatId, CLIPFORMAT std_clipboard_format_id = CF_INVALID)
        : aDataFlavor(getDataFlavor(sotFormatId))
        , aNativeFormatName(getNativeFormatName(aDataFlavor, xFactory))
        , aStandardFormatId(std_clipboard_format_id)
    {
    }

    css::datatransfer::DataFlavor aDataFlavor;
    OUString                      aNativeFormatName;
    sal_Int32                     aStandardFormatId;
};

auto initTranslTable()
{
    auto xFactory = MimeContentTypeFactory::create(comphelper::getProcessComponentContext());

    // to optimize searching we add all entries with a
    // standard clipboard format number first, in the
    // table before the entries with CF_INVALID
    // if we are searching for a standard clipboard
    // format number we can stop if we find the first
    // CF_INVALID
    return std::array
    {
        FormatEntry(xFactory, SotClipboardFormatId::DIF, CF_DIF),

    // #i124085# CF_DIBV5 disabled, leads to problems at export. To fully support, using
    // an own mime-type may be necessary. I have tried that, but saw no real advantages
    // with different apps when exchanging bitmap-based data. So, disabled for now. At
    // the same time increased png format exchange for better interoperability
    //    FormatEntry(SotClipboardFormatId::BITMAP, CF_DIBV5),

        FormatEntry(xFactory, SotClipboardFormatId::BITMAP, CF_DIB),
        FormatEntry(xFactory, SotClipboardFormatId::BITMAP, CF_BITMAP),
        FormatEntry(xFactory, SotClipboardFormatId::STRING, CF_UNICODETEXT),
    // Format Locale - for internal use
        FormatEntry("application/x-openoffice-locale;windows_formatname=\"Locale\"", "Locale", CF_LOCALE),
        FormatEntry(xFactory, SotClipboardFormatId::WMF, CF_METAFILEPICT),
        FormatEntry(xFactory, SotClipboardFormatId::EMF, CF_ENHMETAFILE),
        FormatEntry(xFactory, SotClipboardFormatId::FILE_LIST, CF_HDROP),
        FormatEntry(xFactory, SotClipboardFormatId::SYLK, CF_SYLK),

    // End of standard clipboard format numbers

        FormatEntry(xFactory, SotClipboardFormatId::GDIMETAFILE),
        FormatEntry(xFactory, SotClipboardFormatId::PRIVATE),
        FormatEntry(xFactory, SotClipboardFormatId::SIMPLE_FILE),
        FormatEntry(xFactory, SotClipboardFormatId::RTF),
        FormatEntry(xFactory, SotClipboardFormatId::DRAWING),
        FormatEntry(xFactory, SotClipboardFormatId::SVXB),
        FormatEntry(xFactory, SotClipboardFormatId::SVIM),
        FormatEntry(xFactory, SotClipboardFormatId::XFA),
        FormatEntry(xFactory, SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT),
        FormatEntry(xFactory, SotClipboardFormatId::INTERNALLINK_STATE),
        FormatEntry(xFactory, SotClipboardFormatId::SOLK),
        FormatEntry(xFactory, SotClipboardFormatId::NETSCAPE_BOOKMARK),
        FormatEntry(xFactory, SotClipboardFormatId::TREELISTBOX),
        FormatEntry(xFactory, SotClipboardFormatId::NATIVE),
        FormatEntry(xFactory, SotClipboardFormatId::OWNERLINK),
        FormatEntry(xFactory, SotClipboardFormatId::STARSERVER),
        FormatEntry(xFactory, SotClipboardFormatId::STAROBJECT),
        FormatEntry(xFactory, SotClipboardFormatId::APPLETOBJECT),
        FormatEntry(xFactory, SotClipboardFormatId::PLUGIN_OBJECT),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITER_30),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITER_40),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITER_50),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITERWEB_40),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITERWEB_50),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITERGLOB_40),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITERGLOB_50),
        FormatEntry(xFactory, SotClipboardFormatId::STARDRAW),
        FormatEntry(xFactory, SotClipboardFormatId::STARDRAW_40),
        FormatEntry(xFactory, SotClipboardFormatId::STARIMPRESS_50),
        FormatEntry(xFactory, SotClipboardFormatId::STARDRAW_50),
        FormatEntry(xFactory, SotClipboardFormatId::STARCALC),
        FormatEntry(xFactory, SotClipboardFormatId::STARCALC_40),
        FormatEntry(xFactory, SotClipboardFormatId::STARCALC_50),
        FormatEntry(xFactory, SotClipboardFormatId::STARCHART),
        FormatEntry(xFactory, SotClipboardFormatId::STARCHART_40),
        FormatEntry(xFactory, SotClipboardFormatId::STARCHART_50),
        FormatEntry(xFactory, SotClipboardFormatId::STARIMAGE),
        FormatEntry(xFactory, SotClipboardFormatId::STARIMAGE_40),
        FormatEntry(xFactory, SotClipboardFormatId::STARIMAGE_50),
        FormatEntry(xFactory, SotClipboardFormatId::STARMATH),
        FormatEntry(xFactory, SotClipboardFormatId::STARMATH_40),
        FormatEntry(xFactory, SotClipboardFormatId::STARMATH_50),
        FormatEntry(xFactory, SotClipboardFormatId::STAROBJECT_PAINTDOC),
        FormatEntry(xFactory, SotClipboardFormatId::FILLED_AREA),
        FormatEntry(xFactory, SotClipboardFormatId::HTML),
        FormatEntry(xFactory, SotClipboardFormatId::HTML_SIMPLE),
        FormatEntry(xFactory, SotClipboardFormatId::CHAOS),
        FormatEntry(xFactory, SotClipboardFormatId::CNT_MSGATTACHFILE),
        FormatEntry(xFactory, SotClipboardFormatId::BIFF_5),
        FormatEntry(xFactory, SotClipboardFormatId::BIFF__5),
        FormatEntry(xFactory, SotClipboardFormatId::BIFF_8),
        FormatEntry(xFactory, SotClipboardFormatId::BIFF_12),
        FormatEntry(xFactory, SotClipboardFormatId::SYLK_BIGCAPS),
        FormatEntry(xFactory, SotClipboardFormatId::LINK),
        FormatEntry(xFactory, SotClipboardFormatId::STARDRAW_TABBAR),
        FormatEntry(xFactory, SotClipboardFormatId::SONLK),
        FormatEntry(xFactory, SotClipboardFormatId::MSWORD_DOC),
        FormatEntry(xFactory, SotClipboardFormatId::STAR_FRAMESET_DOC),
        FormatEntry(xFactory, SotClipboardFormatId::OFFICE_DOC),
        FormatEntry(xFactory, SotClipboardFormatId::NOTES_DOCINFO),
        FormatEntry(xFactory, SotClipboardFormatId::NOTES_HNOTE),
        FormatEntry(xFactory, SotClipboardFormatId::NOTES_NATIVE),
        FormatEntry(xFactory, SotClipboardFormatId::SFX_DOC),
        FormatEntry(xFactory, SotClipboardFormatId::EVDF),
        FormatEntry(xFactory, SotClipboardFormatId::ESDF),
        FormatEntry(xFactory, SotClipboardFormatId::IDF),
        FormatEntry(xFactory, SotClipboardFormatId::EFTP),
        FormatEntry(xFactory, SotClipboardFormatId::EFD),
        FormatEntry(xFactory, SotClipboardFormatId::SVX_FORMFIELDEXCH),
        FormatEntry(xFactory, SotClipboardFormatId::EXTENDED_TABBAR),
        FormatEntry(xFactory, SotClipboardFormatId::SBA_DATAEXCHANGE),
        FormatEntry(xFactory, SotClipboardFormatId::SBA_FIELDDATAEXCHANGE),
        FormatEntry(xFactory, SotClipboardFormatId::SBA_PRIVATE_URL),
        FormatEntry(xFactory, SotClipboardFormatId::SBA_TABED),
        FormatEntry(xFactory, SotClipboardFormatId::SBA_TABID),
        FormatEntry(xFactory, SotClipboardFormatId::SBA_JOIN),
        FormatEntry(xFactory, SotClipboardFormatId::OBJECTDESCRIPTOR),
        FormatEntry(xFactory, SotClipboardFormatId::LINKSRCDESCRIPTOR),
        FormatEntry(xFactory, SotClipboardFormatId::EMBED_SOURCE),
        FormatEntry(xFactory, SotClipboardFormatId::LINK_SOURCE),
        FormatEntry(xFactory, SotClipboardFormatId::EMBEDDED_OBJ),
        FormatEntry(xFactory, SotClipboardFormatId::FILECONTENT),
        FormatEntry(xFactory, SotClipboardFormatId::FILEGRPDESCRIPTOR),
        FormatEntry(xFactory, SotClipboardFormatId::FILENAME),
        FormatEntry(xFactory, SotClipboardFormatId::SD_OLE),
        FormatEntry(xFactory, SotClipboardFormatId::EMBEDDED_OBJ_OLE),
        FormatEntry(xFactory, SotClipboardFormatId::EMBED_SOURCE_OLE),
        FormatEntry(xFactory, SotClipboardFormatId::OBJECTDESCRIPTOR_OLE),
        FormatEntry(xFactory, SotClipboardFormatId::LINKSRCDESCRIPTOR_OLE),
        FormatEntry(xFactory, SotClipboardFormatId::LINK_SOURCE_OLE),
        FormatEntry(xFactory, SotClipboardFormatId::SBA_CTRLDATAEXCHANGE),
        FormatEntry(xFactory, SotClipboardFormatId::OUTPLACE_OBJ),
        FormatEntry(xFactory, SotClipboardFormatId::CNT_OWN_CLIP),
        FormatEntry(xFactory, SotClipboardFormatId::INET_IMAGE),
        FormatEntry(xFactory, SotClipboardFormatId::NETSCAPE_IMAGE),
        FormatEntry(xFactory, SotClipboardFormatId::SBA_FORMEXCHANGE),
        FormatEntry(xFactory, SotClipboardFormatId::SBA_REPORTEXCHANGE),
        FormatEntry(xFactory, SotClipboardFormatId::UNIFORMRESOURCELOCATOR),
        FormatEntry(xFactory, SotClipboardFormatId::STARCHARTDOCUMENT_50),
        FormatEntry(xFactory, SotClipboardFormatId::GRAPHOBJ),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITER_60),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITERWEB_60),
        FormatEntry(xFactory, SotClipboardFormatId::STARWRITERGLOB_60),
        FormatEntry(xFactory, SotClipboardFormatId::STARDRAW_60),
        FormatEntry(xFactory, SotClipboardFormatId::STARIMPRESS_60),
        FormatEntry(xFactory, SotClipboardFormatId::STARCALC_60),
        FormatEntry(xFactory, SotClipboardFormatId::STARCHART_60),
        FormatEntry(xFactory, SotClipboardFormatId::STARMATH_60),
        FormatEntry(xFactory, SotClipboardFormatId::DIALOG_60),
        FormatEntry(xFactory, SotClipboardFormatId::BMP),
        FormatEntry(xFactory, SotClipboardFormatId::PNG),
        FormatEntry(xFactory, SotClipboardFormatId::SVG),
        FormatEntry(xFactory, SotClipboardFormatId::MATHML),
        FormatEntry(xFactory, SotClipboardFormatId::RICHTEXT),
        FormatEntry(xFactory, SotClipboardFormatId::MARKDOWN),
    };
}

const auto& g_TranslTable()
{
    static const auto table = initTranslTable();
    return table;
}

bool isStandardFormat(sal_Int32 fmtid) { return fmtid != CF_INVALID; }

const auto& EndOfStandardFormats()
{
    /*
        we stop search if we find the first CF_INVALID
        because in the translation table the entries with a
        standard clipboard format id appear before the other
        entries with CF_INVALID
    */
    static const auto end
        = std::find_if(g_TranslTable().begin(), g_TranslTable().end(), [](const FormatEntry& rEntry)
                       { return !isStandardFormat(rEntry.aStandardFormatId); });
    return end;
}

void findDataFlavorForStandardFormatId( sal_Int32 aStandardFormatId, DataFlavor& aDataFlavor )
{
    const auto& end = EndOfStandardFormats();
    auto citer = std::find_if(g_TranslTable().begin(), end,
        [&aStandardFormatId](const FormatEntry& rEntry) {
            return rEntry.aStandardFormatId == aStandardFormatId;
        });
    if (citer != end)
        aDataFlavor = citer->aDataFlavor;
}

void findDataFlavorForNativeFormatName( const OUString& aNativeFormatName, DataFlavor& aDataFlavor )
{
    const auto& end = g_TranslTable().end();
    auto citer = std::find_if(g_TranslTable().begin(), end,
        [&aNativeFormatName](const FormatEntry& rEntry) {
            return aNativeFormatName.equalsIgnoreAsciiCase(rEntry.aNativeFormatName); });
    if (citer != end)
        aDataFlavor = citer->aDataFlavor;
}

void findStandardFormatIdForCharset( const OUString& aCharset, Any& aAny )
{
    if ( aCharset.equalsIgnoreAsciiCase( "utf-16" ) )
        aAny <<= static_cast< sal_Int32 >( CF_UNICODETEXT );
    else
    {
        sal_Int32 wincp = getWinCPFromMimeCharset( aCharset );
        if ( IsOEMCP ( wincp ) )
            aAny <<= static_cast< sal_Int32 >( CF_OEMTEXT );
    }
}

void setStandardFormatIdForNativeFormatName( const OUString& aNativeFormatName, Any& aAny )
{
    const auto& end = EndOfStandardFormats();
    auto citer = std::find_if(g_TranslTable().begin(), end,
        [&aNativeFormatName](const FormatEntry& rEntry) {
            return aNativeFormatName.equalsIgnoreAsciiCase(rEntry.aNativeFormatName);
        });
    if (citer != end)
        aAny <<= citer->aStandardFormatId;
}

void findStdFormatIdOrNativeFormatNameForFullMediaType(
    const Reference< XMimeContentTypeFactory >& aRefXMimeFactory,
    const OUString& aFullMediaType,
    Any& aAny )
{
    const auto& end = g_TranslTable().end();
    auto citer = std::find_if(g_TranslTable().begin(), end,
        [&aRefXMimeFactory, &aFullMediaType](const FormatEntry& rEntry) {
            Reference<XMimeContentType> refXMime( aRefXMimeFactory->createMimeContentType(rEntry.aDataFlavor.MimeType) );
            return aFullMediaType.equalsIgnoreAsciiCase(refXMime->getFullMediaType());
        });
    if (citer != end)
    {
        if (sal_Int32 cf = citer->aStandardFormatId; isStandardFormat(cf))
            aAny <<= cf;
        else
        {
            OSL_ENSURE( citer->aNativeFormatName.getLength( ),
                "Invalid standard format id and empty native format name in translation table" );
            aAny <<= citer->aNativeFormatName;
        }
    }
}
}

CDataFormatTranslatorUNO::CDataFormatTranslatorUNO( const Reference< XComponentContext >& rxContext ) :
    m_xContext( rxContext )
{
}

Any SAL_CALL CDataFormatTranslatorUNO::getSystemDataTypeFromDataFlavor( const DataFlavor& aDataFlavor )
{
    Any aAny;

    try
    {
        Reference< XMimeContentTypeFactory > refXMimeCntFactory = MimeContentTypeFactory::create( m_xContext );

        Reference< XMimeContentType >
            refXMimeCntType( refXMimeCntFactory->createMimeContentType( aDataFlavor.MimeType ) );

        OUString fullMediaType = refXMimeCntType->getFullMediaType( );
        if (fullMediaType.equalsIgnoreAsciiCase("text/plain"))
        {
            // default is CF_TEXT
            aAny <<= static_cast< sal_Int32 >( CF_TEXT );

            if ( refXMimeCntType->hasParameter( "charset" ) )
            {
                // but maybe it is unicode text or oem text
                OUString charset = refXMimeCntType->getParameterValue( "charset" );
                findStandardFormatIdForCharset( charset, aAny );
            }
        }
        else
        {
            if (refXMimeCntType->hasParameter(Windows_FormatName))
            {
                OUString winfmtname = refXMimeCntType->getParameterValue(Windows_FormatName);
                aAny <<= winfmtname;

                setStandardFormatIdForNativeFormatName( winfmtname, aAny );
            }
            else
                findStdFormatIdOrNativeFormatNameForFullMediaType( refXMimeCntFactory, fullMediaType, aAny );
        }
    }
    catch (css::lang::IllegalArgumentException&)
    {
        OSL_FAIL( "Invalid content-type detected!" );
    }
    catch (css::container::NoSuchElementException&)
    {
        OSL_FAIL( "Illegal content-type parameter" );
    }
    catch( ... )
    {
        OSL_FAIL( "Unexpected error" );
        throw;
    }

    return aAny;
}

DataFlavor SAL_CALL CDataFormatTranslatorUNO::getDataFlavorFromSystemDataType( const Any& aSysDataType )
{
    OSL_PRECOND( aSysDataType.hasValue( ), "Empty system data type delivered" );

    DataFlavor aFlavor({}, {}, CPPUTYPE_SEQSALINT8);

    if (sal_Int32 clipformat; aSysDataType >>= clipformat)
    {
        findDataFlavorForStandardFormatId(clipformat, aFlavor);
    }
    else if (OUString nativeFormatName; aSysDataType >>= nativeFormatName)
    {
        findDataFlavorForNativeFormatName( nativeFormatName, aFlavor );
    }
    else
        OSL_FAIL( "Invalid data type received" );

    return aFlavor;
}

// XServiceInfo

OUString SAL_CALL CDataFormatTranslatorUNO::getImplementationName(  )
{
    return "com.sun.star.datatransfer.DataFormatTranslator";
}

sal_Bool SAL_CALL CDataFormatTranslatorUNO::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL CDataFormatTranslatorUNO::getSupportedServiceNames( )
{
    return { "com.sun.star.datatransfer.DataFormatTranslator" };
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
dtrans_CDataFormatTranslatorUNO_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new CDataFormatTranslatorUNO(context));
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
