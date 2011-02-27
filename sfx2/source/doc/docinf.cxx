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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sfx2.hxx"

#include <sfx2/docinf.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyContainer.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XCompatWriterDocProperties.hpp>
#include <com/sun/star/uno/Exception.hpp>

#include <rtl/ustring.hxx>
#include <tools/debug.hxx>
#include <comphelper/string.hxx>
#include <sot/storage.hxx>
#include <vcl/gdimtf.hxx>

#include "oleprops.hxx"
// ============================================================================

// stream names
#define STREAM_SUMMARYINFO      "\005SummaryInformation"
#define STREAM_DOCSUMMARYINFO   "\005DocumentSummaryInformation"

// usings
using namespace ::com::sun::star;


namespace sfx2 {

sal_uInt32 SFX2_DLLPUBLIC LoadOlePropertySet(
    uno::Reference< document::XDocumentProperties> i_xDocProps,
    SotStorage* i_pStorage )
{
    // *** global properties from stream "005SummaryInformation" ***

    // load the property set
    SfxOlePropertySet aGlobSet;
    ErrCode nGlobError = aGlobSet.LoadPropertySet(i_pStorage,
        String( RTL_CONSTASCII_USTRINGPARAM( STREAM_SUMMARYINFO ) ) );

    // global section
    SfxOleSectionRef xGlobSect = aGlobSet.GetSection( SECTION_GLOBAL );
    if( xGlobSect.get() )
    {
        // set supported properties
        String aStrValue;
        util::DateTime aDateTime;

        if( xGlobSect->GetStringValue( aStrValue, PROPID_TITLE ) )
            i_xDocProps->setTitle( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_SUBJECT ) )
            i_xDocProps->setSubject( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_KEYWORDS ) ) {
            i_xDocProps->setKeywords(
                ::comphelper::string::convertCommaSeparated(aStrValue) );
        }
        if( xGlobSect->GetStringValue( aStrValue, PROPID_TEMPLATE ) )
            i_xDocProps->setTemplateName( aStrValue );
        if( xGlobSect->GetStringValue( aStrValue, PROPID_COMMENTS ) )
            i_xDocProps->setDescription( aStrValue );

        util::DateTime aInvalid;
        if( xGlobSect->GetStringValue( aStrValue, PROPID_AUTHOR) )
            i_xDocProps->setAuthor( aStrValue );
        else
            i_xDocProps->setAuthor( ::rtl::OUString() );
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_CREATED ) )
            i_xDocProps->setCreationDate( aDateTime );
        else
            i_xDocProps->setCreationDate( aInvalid );

        if( xGlobSect->GetStringValue( aStrValue, PROPID_LASTAUTHOR) )
            i_xDocProps->setModifiedBy( aStrValue );
        else
            i_xDocProps->setModifiedBy( ::rtl::OUString() );
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_LASTSAVED ) )
            i_xDocProps->setModificationDate( aDateTime );
        else
            i_xDocProps->setModificationDate( aInvalid );

        i_xDocProps->setPrintedBy( ::rtl::OUString() );
        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_LASTPRINTED ) )
            i_xDocProps->setPrintDate( aDateTime );
        else
            i_xDocProps->setPrintDate( aInvalid );

        if( xGlobSect->GetStringValue( aStrValue, PROPID_REVNUMBER ) )
        {
            sal_Int16 nRevision = static_cast< sal_Int16 >( aStrValue.ToInt32() );
            if ( nRevision > 0 )
                i_xDocProps->setEditingCycles( nRevision );
        }

        if( xGlobSect->GetFileTimeValue( aDateTime, PROPID_EDITTIME ) )
        {
            // subtract offset 1601-01-01
            aDateTime.Year  -= 1601;
            aDateTime.Month -= 1;
            aDateTime.Day   -= 1;
            try
            {
                i_xDocProps->setEditingDuration(
                    aDateTime.Day     * 60*60*24 +
                    aDateTime.Hours   * 60*60    +
                    aDateTime.Minutes * 60       +
                    aDateTime.Seconds            );
            }
            catch (lang::IllegalArgumentException &)
            {
                // ignore
            }
        }
    }

    // *** custom properties from stream "005DocumentSummaryInformation" ***

    // load the property set
    SfxOlePropertySet aDocSet;
    ErrCode nDocError = aDocSet.LoadPropertySet(i_pStorage,
        String( RTL_CONSTASCII_USTRINGPARAM( STREAM_DOCSUMMARYINFO ) ) );

    // custom properties
    SfxOleSectionRef xCustomSect = aDocSet.GetSection( SECTION_CUSTOM );
    if( xCustomSect.get() )
    {
        uno::Reference < beans::XPropertyContainer > xUserDefined(
            i_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
        ::std::vector< sal_Int32 > aPropIds;
        xCustomSect->GetPropertyIds( aPropIds );
        for( ::std::vector< sal_Int32 >::const_iterator aIt = aPropIds.begin(),
             aEnd = aPropIds.end(); aIt != aEnd; ++aIt )
        {
            ::rtl::OUString aPropName = xCustomSect->GetPropertyName( *aIt );
            uno::Any aPropValue = xCustomSect->GetAnyValue( *aIt );
            if( (aPropName.getLength() > 0) && aPropValue.hasValue() ) {
                try {
                    xUserDefined->addProperty( aPropName,
                        beans::PropertyAttribute::REMOVEABLE, aPropValue );
                } catch ( uno::Exception& ) {
                    //ignore
                }
            }
        }
    }

    uno::Reference< document::XCompatWriterDocProperties > xWriterProps( i_xDocProps, uno::UNO_QUERY );
    if ( xWriterProps.is() )
    {
        SfxOleSectionRef xBuiltin = aDocSet.GetSection( SECTION_BUILTIN );
        if ( xBuiltin.get() )
        {
            try
            {
                String aStrValue;
                if ( xBuiltin->GetStringValue( aStrValue, PROPID_MANAGER ) )
                    xWriterProps->setManager( aStrValue );
                if ( xBuiltin->GetStringValue( aStrValue, PROPID_CATEGORY ) )
                    xWriterProps->setCategory( aStrValue );
                if ( xBuiltin->GetStringValue( aStrValue, PROPID_COMPANY ) )
                    xWriterProps->setCompany( aStrValue );
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    // return code
    return (nGlobError != ERRCODE_NONE) ? nGlobError : nDocError;
}

bool SFX2_DLLPUBLIC SaveOlePropertySet(
    uno::Reference< document::XDocumentProperties> i_xDocProps,
    SotStorage* i_pStorage,
    const uno::Sequence<sal_uInt8> * i_pThumb,
    const uno::Sequence<sal_uInt8> * i_pGuid,
    const uno::Sequence<sal_uInt8> * i_pHyperlinks)
{
    // *** global properties into stream "005SummaryInformation" ***

    SfxOlePropertySet aGlobSet;

    // set supported properties
    SfxOleSection& rGlobSect = aGlobSet.AddSection( SECTION_GLOBAL );
    rGlobSect.SetStringValue( PROPID_TITLE,    i_xDocProps->getTitle() );
    rGlobSect.SetStringValue( PROPID_SUBJECT,  i_xDocProps->getSubject() );
    String aStr = ::comphelper::string::convertCommaSeparated(
        i_xDocProps->getKeywords() );
    rGlobSect.SetStringValue( PROPID_KEYWORDS, aStr );
    rGlobSect.SetStringValue( PROPID_TEMPLATE, i_xDocProps->getTemplateName() );
    rGlobSect.SetStringValue( PROPID_COMMENTS, i_xDocProps->getDescription() );
    rGlobSect.SetStringValue( PROPID_AUTHOR,   i_xDocProps->getAuthor() );
    rGlobSect.SetFileTimeValue(PROPID_CREATED, i_xDocProps->getCreationDate());
    rGlobSect.SetStringValue( PROPID_LASTAUTHOR, i_xDocProps->getModifiedBy() );
    rGlobSect.SetFileTimeValue(PROPID_LASTSAVED,
                                i_xDocProps->getModificationDate() );
    // note: apparently PrintedBy is not supported in file format
    rGlobSect.SetFileTimeValue(PROPID_LASTPRINTED, i_xDocProps->getPrintDate());

    sal_Int32 dur = i_xDocProps->getEditingDuration();
    util::DateTime aEditTime;
    // add offset 1601-01-01
    aEditTime.Year    = 1601;
    aEditTime.Month   = 1;
    aEditTime.Day     = 1;
    aEditTime.Hours   = static_cast<sal_Int16>(dur / 3600);
    aEditTime.Minutes = static_cast<sal_Int16>((dur % 3600) / 60);
    aEditTime.Seconds = static_cast<sal_Int16>(dur % 60);
    rGlobSect.SetFileTimeValue( PROPID_EDITTIME, aEditTime );

    rGlobSect.SetStringValue( PROPID_REVNUMBER,
                String::CreateFromInt32( i_xDocProps->getEditingCycles() ) );
    if ( i_pThumb && i_pThumb->getLength() )
        rGlobSect.SetThumbnailValue( PROPID_THUMBNAIL, *i_pThumb );

    // save the property set
    ErrCode nGlobError = aGlobSet.SavePropertySet(i_pStorage,
        String( RTL_CONSTASCII_USTRINGPARAM( STREAM_SUMMARYINFO ) ) );

    // *** custom properties into stream "005DocumentSummaryInformation" ***

    SfxOlePropertySet aDocSet;

    // set builtin properties
    aDocSet.AddSection( SECTION_BUILTIN );

    // set custom properties
    SfxOleSection& rCustomSect = aDocSet.AddSection( SECTION_CUSTOM );

    // write GUID
    if (i_pGuid) {
        const sal_Int32 nPropId = rCustomSect.GetFreePropertyId();
        rCustomSect.SetBlobValue( nPropId, *i_pGuid );
        rCustomSect.SetPropertyName( nPropId,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_PID_GUID")) );
    }

    // write hyperlinks
    if (i_pHyperlinks) {
        const sal_Int32 nPropId = rCustomSect.GetFreePropertyId();
        rCustomSect.SetBlobValue( nPropId, *i_pHyperlinks );
        rCustomSect.SetPropertyName( nPropId,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_PID_HLINKS")) );
    }

    uno::Reference<beans::XPropertySet> xUserDefinedProps(
        i_xDocProps->getUserDefinedProperties(), uno::UNO_QUERY_THROW);
    DBG_ASSERT(xUserDefinedProps.is(), "UserDefinedProperties is null");
    uno::Reference<beans::XPropertySetInfo> xPropInfo =
        xUserDefinedProps->getPropertySetInfo();
    DBG_ASSERT(xPropInfo.is(), "UserDefinedProperties Info is null");
    uno::Sequence<beans::Property> props = xPropInfo->getProperties();
    for (sal_Int32 i = 0; i < props.getLength(); ++i) {
        try {
            // skip transient properties
            if (~props[i].Attributes & beans::PropertyAttribute::TRANSIENT)
            {
                const ::rtl::OUString name = props[i].Name;
                const sal_Int32 nPropId = rCustomSect.GetFreePropertyId();
                if (rCustomSect.SetAnyValue( nPropId,
                            xUserDefinedProps->getPropertyValue(name))) {
                    rCustomSect.SetPropertyName( nPropId, name );
                }
            }
        } catch (uno::Exception &) {
            // may happen with concurrent modification...
            DBG_WARNING("SavePropertySet: exception");
        }
    }

    // save the property set
    ErrCode nDocError = aDocSet.SavePropertySet(i_pStorage,
        String( RTL_CONSTASCII_USTRINGPARAM( STREAM_DOCSUMMARYINFO ) ) );

    // return code
    return (nGlobError == ERRCODE_NONE) && (nDocError == ERRCODE_NONE);
}

uno::Sequence<sal_uInt8> SFX2_DLLPUBLIC convertMetaFile(GDIMetaFile* i_pThumb)
{
    if (i_pThumb) {
        BitmapEx aBitmap;
        SvMemoryStream aStream;
// magic value 160 taken from GraphicHelper::getThumbnailFormatFromGDI_Impl()
        if( i_pThumb->CreateThumbnail( 160, aBitmap ) ) {
            aBitmap.GetBitmap().Write( aStream, FALSE, FALSE );
            aStream.Seek(STREAM_SEEK_TO_END);
            uno::Sequence<sal_uInt8> aSeq(aStream.Tell());
            const sal_uInt8* pBlob(
                static_cast<const sal_uInt8*>(aStream.GetData()));
            for (sal_Int32 j = 0; j < aSeq.getLength(); ++j) {
                aSeq[j] = pBlob[j];
            }
            return aSeq;
        }
    }
    return uno::Sequence<sal_uInt8>();
}

} // namespace sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
