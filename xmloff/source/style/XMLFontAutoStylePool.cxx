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

#include <o3tl/sorted_vector.hxx>
#include <tools/fontenum.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "fonthdl.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/XMLFontAutoStylePool.hxx>
#include <vcl/embeddedfontshelper.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <XMLBase64Export.hxx>
#include <xmloff/AutoStyleEntry.hxx>
#include <comphelper/hash.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::xmloff::token;

class XMLFontAutoStylePoolEntry_Impl
{
    OUString    sName;
    OUString    sFamilyName;
    OUString    sStyleName;
    FontFamily  nFamily;
    FontPitch   nPitch;
    rtl_TextEncoding eEnc;

public:

    inline XMLFontAutoStylePoolEntry_Impl(
            const OUString& rName,
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
            rtl_TextEncoding eEnc );

    inline XMLFontAutoStylePoolEntry_Impl(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
            rtl_TextEncoding eEnc );

    const OUString& GetName() const { return sName; }
    const OUString& GetFamilyName() const { return sFamilyName; }
    const OUString& GetStyleName() const { return sStyleName; }
    FontFamily GetFamily() const {   return nFamily; }
    FontPitch GetPitch() const { return nPitch; }
    rtl_TextEncoding GetEncoding() const { return eEnc; }
};


inline XMLFontAutoStylePoolEntry_Impl::XMLFontAutoStylePoolEntry_Impl(
        const OUString& rName,
        const OUString& rFamilyName,
        const OUString& rStyleName,
        FontFamily nFam,
        FontPitch nP,
        rtl_TextEncoding eE ) :
    sName( rName ),
    sFamilyName( rFamilyName ),
    sStyleName( rStyleName ),
    nFamily( nFam ),
    nPitch( nP ),
    eEnc( eE )
{
}

inline XMLFontAutoStylePoolEntry_Impl::XMLFontAutoStylePoolEntry_Impl(
        const OUString& rFamilyName,
        const OUString& rStyleName,
        FontFamily nFam,
        FontPitch nP,
        rtl_TextEncoding eE ) :
    sFamilyName( rFamilyName ),
    sStyleName( rStyleName ),
    nFamily( nFam ),
    nPitch( nP ),
    eEnc( eE )
{
}

struct XMLFontAutoStylePoolEntryCmp_Impl {
    bool operator()(
        XMLFontAutoStylePoolEntry_Impl* const& r1,
        XMLFontAutoStylePoolEntry_Impl* const& r2 ) const
    {
        bool bEnc1(r1->GetEncoding() != RTL_TEXTENCODING_SYMBOL);
        bool bEnc2(r2->GetEncoding() != RTL_TEXTENCODING_SYMBOL);
        if( bEnc1 != bEnc2 )
            return bEnc1 < bEnc2;
        else if( r1->GetPitch() != r2->GetPitch() )
            return r1->GetPitch() < r2->GetPitch();
        else if( r1->GetFamily() != r2->GetFamily() )
            return r1->GetFamily() < r2->GetFamily();
        else
        {
            sal_Int32 nCmp = r1->GetFamilyName().compareTo( r2->GetFamilyName() );
            if( 0 == nCmp )
                return r1->GetStyleName().compareTo( r2->GetStyleName() ) < 0;
            else
                return nCmp < 0;
        }
    }
};

class XMLFontAutoStylePool_Impl : public o3tl::sorted_vector<XMLFontAutoStylePoolEntry_Impl*, XMLFontAutoStylePoolEntryCmp_Impl>
{
public:
    ~XMLFontAutoStylePool_Impl()
    {
        DeleteAndDestroyAll();
    }
};

XMLFontAutoStylePool::XMLFontAutoStylePool(SvXMLExport& rExp, bool bTryToEmbedFonts) :
    rExport( rExp ),
    m_pFontAutoStylePool( new XMLFontAutoStylePool_Impl ),
    m_bTryToEmbedFonts( bTryToEmbedFonts ),
    m_bEmbedUsedOnly(false),
    m_bEmbedLatinScript(true),
    m_bEmbedAsianScript(true),
    m_bEmbedComplexScript(true)
{
}

XMLFontAutoStylePool::~XMLFontAutoStylePool()
{
}

OUString XMLFontAutoStylePool::Add(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
            rtl_TextEncoding eEnc )
{
    OUString sPoolName;
    XMLFontAutoStylePoolEntry_Impl aTmp( rFamilyName, rStyleName, nFamily,
                                          nPitch, eEnc );
    XMLFontAutoStylePool_Impl::const_iterator it = m_pFontAutoStylePool->find( &aTmp );
    if( it != m_pFontAutoStylePool->end() )
    {
        sPoolName = (*it)->GetName();
    }
    else
    {
        OUString sName;
        sal_Int32 nLen = rFamilyName.indexOf( ';' );
        if( -1 == nLen )
        {
            sName = rFamilyName;
        }
        else if( nLen > 0 )
        {
            sName = rFamilyName.copy( 0, nLen );
            sName = sName.trim();
        }

        if( sName.isEmpty() )
            sName = "F";

        if( m_aNames.find(sName) != m_aNames.end() )
        {
            sal_Int32 nCount = 1;
            OUString sPrefix( sName );
            sName += OUString::number( nCount );
            while( m_aNames.find(sName) != m_aNames.end() )
            {
                sName = sPrefix;
                sName += OUString::number( ++nCount );
            }
        }

        XMLFontAutoStylePoolEntry_Impl *pEntry =
            new XMLFontAutoStylePoolEntry_Impl( sName, rFamilyName, rStyleName,
                                                nFamily, nPitch, eEnc );
        m_pFontAutoStylePool->insert( pEntry );
        m_aNames.insert(sName);
    }

    return sPoolName;
}

OUString XMLFontAutoStylePool::Find(
            const OUString& rFamilyName,
            const OUString& rStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
            rtl_TextEncoding eEnc ) const
{
    OUString sName;
    XMLFontAutoStylePoolEntry_Impl aTmp( rFamilyName, rStyleName, nFamily,
                                          nPitch, eEnc );
    XMLFontAutoStylePool_Impl::const_iterator it = m_pFontAutoStylePool->find( &aTmp );
    if( it != m_pFontAutoStylePool->end() )
    {
        sName = (*it)->GetName();
    }

    return sName;
}

namespace
{

OUString lcl_checkFontFile( const OUString &fileUrl )
{
    osl::DirectoryItem aDirItem;
    if( osl::DirectoryItem::get( fileUrl, aDirItem ) == osl::File::E_None )
    {
        osl::FileStatus aStatus( osl_FileStatus_Mask_Type );
        if( aDirItem.getFileStatus( aStatus ) == osl::File::E_None )
        {
            if( !aStatus.isDirectory() )
                return fileUrl;
        }
    }
    return OUString();
}

/// Contains information about a single variant of an embedded font.
struct EmbeddedFontInfo
{
    OUString aURL;
    FontWeight eWeight = WEIGHT_NORMAL;
    FontItalic eItalic = ITALIC_NONE;
};

/// Converts FontWeight to CSS-compatible string representation.
OUString FontWeightToString(FontWeight eWeight)
{
    OUString aRet;

    switch (eWeight)
    {
    case WEIGHT_BOLD:
        aRet = "bold";
        break;
    default:
        aRet = "normal";
        break;
    }

    return aRet;
}

/// Converts FontItalic to CSS-compatible string representation.
OUString FontItalicToString(FontItalic eWeight)
{
    OUString aRet;

    switch (eWeight)
    {
    case ITALIC_NORMAL:
        aRet = "italic";
        break;
    default:
        aRet = "normal";
        break;
    }

    return aRet;
}

}

std::unordered_set<OUString> XMLFontAutoStylePool::getUsedFontList()
{
    std::unordered_set<OUString> aReturnSet;

    uno::Reference<style::XStyleFamiliesSupplier> xFamiliesSupp(GetExport().GetModel(), UNO_QUERY);
    if (!xFamiliesSupp.is())
        return aReturnSet;

    // Check styles first
    uno::Reference<container::XNameAccess> xFamilies(xFamiliesSupp->getStyleFamilies());
    if (xFamilies.is())
    {
        for (OUString const & sFamilyName : xFamilies->getElementNames())
        {
            uno::Reference<container::XNameAccess> xStyleContainer;
            xFamilies->getByName(sFamilyName) >>= xStyleContainer;

            if (xStyleContainer.is())
            {
                for (OUString const & rName : xStyleContainer->getElementNames())
                {
                    uno::Reference<style::XStyle> xStyle;
                    xStyleContainer->getByName(rName) >>= xStyle;
                    if (xStyle->isInUse())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(xStyle, UNO_QUERY);
                        if (xPropertySet.is())
                        {
                            uno::Reference<beans::XPropertySetInfo> xInfo(xPropertySet->getPropertySetInfo());
                            if (m_bEmbedLatinScript && xInfo->hasPropertyByName("CharFontName"))
                            {
                                OUString sCharFontName;
                                Any aFontAny = xPropertySet->getPropertyValue("CharFontName");
                                aFontAny >>= sCharFontName;
                                if (!sCharFontName.isEmpty())
                                    aReturnSet.insert(sCharFontName);
                            }
                            if (m_bEmbedAsianScript && xInfo->hasPropertyByName("CharFontNameAsian"))
                            {
                                OUString sCharFontNameAsian;
                                Any aFontAny = xPropertySet->getPropertyValue("CharFontNameAsian");
                                aFontAny >>= sCharFontNameAsian;
                                if (!sCharFontNameAsian.isEmpty())
                                    aReturnSet.insert(sCharFontNameAsian);
                            }
                            if (m_bEmbedComplexScript && xInfo->hasPropertyByName("CharFontNameComplex"))
                            {
                                OUString sCharFontNameComplex;
                                Any aFontAny = xPropertySet->getPropertyValue("CharFontNameComplex");
                                aFontAny >>= sCharFontNameComplex;
                                if (!sCharFontNameComplex.isEmpty())
                                    aReturnSet.insert(sCharFontNameComplex);
                            }
                        }
                    }
                }
            }
        }
    }

    // make sure auto-styles are collected
    GetExport().collectAutoStyles();

    // Check auto-styles for fonts
    std::vector<xmloff::AutoStyleEntry> aAutoStyleEntries;
    aAutoStyleEntries = GetExport().GetAutoStylePool()->GetAutoStyleEntries();
    for (auto const & rAutoStyleEntry : aAutoStyleEntries)
    {
        for (auto const & rPair : rAutoStyleEntry.m_aXmlProperties)
        {
            if (rPair.first == "font-name" ||
                rPair.first == "font-weight-asian" ||
                rPair.first == "font-weight-complex")
            {
                if (rPair.second.has<OUString>())
                {
                    OUString sFontName = rPair.second.get<OUString>();
                    if (!sFontName.isEmpty())
                        aReturnSet.insert(sFontName);
                }
            }
        }
    }

    return aReturnSet;
}

void XMLFontAutoStylePool::exportXML()
{
    SvXMLElementExport aElem(GetExport(), XML_NAMESPACE_OFFICE,
                             XML_FONT_FACE_DECLS,
                             true, true);
    Any aAny;
    OUString sTmp;
    XMLFontFamilyNamePropHdl aFamilyNameHdl;
    XMLFontFamilyPropHdl aFamilyHdl;
    XMLFontPitchPropHdl aPitchHdl;
    XMLFontEncodingPropHdl aEncHdl;
    const SvXMLUnitConverter& rUnitConv = GetExport().GetMM100UnitConverter();

    std::map<OUString, OUString> fontFilesMap; // our url to document url
    sal_uInt32 nCount = m_pFontAutoStylePool->size();

    std::unordered_set<OUString> aUsedFontNames;
    if (m_bEmbedUsedOnly)
        aUsedFontNames = getUsedFontList();

    for (sal_uInt32 i = 0; i < nCount; i++)
    {
        const XMLFontAutoStylePoolEntry_Impl* pEntry = (*m_pFontAutoStylePool)[i];

        GetExport().AddAttribute(XML_NAMESPACE_STYLE, XML_NAME, pEntry->GetName());

        aAny <<= pEntry->GetFamilyName();
        if (aFamilyNameHdl.exportXML(sTmp, aAny, rUnitConv))
            GetExport().AddAttribute(XML_NAMESPACE_SVG,
                                     XML_FONT_FAMILY, sTmp);

        const OUString& rStyleName = pEntry->GetStyleName();
        if (!rStyleName.isEmpty())
            GetExport().AddAttribute(XML_NAMESPACE_STYLE,
                                     XML_FONT_ADORNMENTS,
                                     rStyleName);

        aAny <<= static_cast<sal_Int16>(pEntry->GetFamily());
        if (aFamilyHdl.exportXML(sTmp, aAny, rUnitConv))
        {
            GetExport().AddAttribute(XML_NAMESPACE_STYLE,
                                     XML_FONT_FAMILY_GENERIC, sTmp);
        }
        aAny <<= static_cast<sal_Int16>(pEntry->GetPitch());
        if (aPitchHdl.exportXML(sTmp, aAny, rUnitConv))
        {
            GetExport().AddAttribute(XML_NAMESPACE_STYLE,
                                     XML_FONT_PITCH, sTmp);
        }

        aAny <<= static_cast<sal_Int16>(pEntry->GetEncoding());
        if (aEncHdl.exportXML( sTmp, aAny, rUnitConv))
        {
            GetExport().AddAttribute(XML_NAMESPACE_STYLE,
                                     XML_FONT_CHARSET, sTmp);
        }

        SvXMLElementExport aElement(GetExport(), XML_NAMESPACE_STYLE,
                                    XML_FONT_FACE, true, true);

        if (m_bTryToEmbedFonts)
        {
            const bool bExportFlat(GetExport().getExportFlags() & SvXMLExportFlags::EMBEDDED);
            std::vector<EmbeddedFontInfo> aEmbeddedFonts;
            static const std::vector<std::pair<FontWeight, FontItalic>> aCombinations =
            {
                { WEIGHT_NORMAL, ITALIC_NONE },
                { WEIGHT_BOLD,   ITALIC_NONE },
                { WEIGHT_NORMAL, ITALIC_NORMAL },
                { WEIGHT_BOLD,   ITALIC_NORMAL },
            };

            for (auto const & aCombinationPair : aCombinations)
            {
                // Embed font if at least viewing is allowed (in which case the opening app must check
                // the font license rights too and open either read-only or not use the font for editing).
                OUString sFileUrl = EmbeddedFontsHelper::fontFileUrl(
                                        pEntry->GetFamilyName(), pEntry->GetFamily(),
                                        aCombinationPair.second, aCombinationPair.first, pEntry->GetPitch(),
                                        EmbeddedFontsHelper::FontRights::ViewingAllowed);
                if (sFileUrl.isEmpty())
                    continue;

                // When embedded only is not set or font is used
                if (!m_bEmbedUsedOnly ||
                    aUsedFontNames.find(pEntry->GetFamilyName()) != aUsedFontNames.end())
                {
                    if (!fontFilesMap.count(sFileUrl))
                    {
                        const OUString docUrl = bExportFlat ?
                                                    lcl_checkFontFile(sFileUrl) : embedFontFile(sFileUrl, pEntry->GetFamilyName());
                        if (!docUrl.isEmpty())
                            fontFilesMap[sFileUrl] = docUrl;
                        else
                            continue; // --> failed to embed
                    }
                    EmbeddedFontInfo aEmbeddedFont;
                    aEmbeddedFont.aURL = sFileUrl;
                    aEmbeddedFont.eWeight = aCombinationPair.first;
                    aEmbeddedFont.eItalic = aCombinationPair.second;
                    aEmbeddedFonts.push_back(aEmbeddedFont);
                }
            }
            if (!aEmbeddedFonts.empty())
            {
                SvXMLElementExport fontFaceSrc(GetExport(), XML_NAMESPACE_SVG, XML_FONT_FACE_SRC, true, true);
                for (EmbeddedFontInfo const & rEmbeddedFont : aEmbeddedFonts)
                {
                    if (fontFilesMap.count(rEmbeddedFont.aURL))
                    {
                        if (!bExportFlat)
                        {
                            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_HREF,
                                                     fontFilesMap[rEmbeddedFont.aURL]);
                            GetExport().AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, "simple");
                        }

                        // Help consumers of our output by telling them which
                        // font file is which one.
                        GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, XML_FONT_STYLE,
                                                 FontItalicToString(rEmbeddedFont.eItalic));
                        GetExport().AddAttribute(XML_NAMESPACE_LO_EXT, XML_FONT_WEIGHT,
                                                 FontWeightToString(rEmbeddedFont.eWeight));

                        SvXMLElementExport fontFaceUri(GetExport(), XML_NAMESPACE_SVG,
                                                       XML_FONT_FACE_URI, true, true);

                        if (bExportFlat)
                        {
                            const uno::Reference<ucb::XSimpleFileAccess> xFileAccess(
                                ucb::SimpleFileAccess::create(GetExport().getComponentContext()));
                            try
                            {
                                const uno::Reference<io::XInputStream> xInput(xFileAccess->openFileRead(fontFilesMap[rEmbeddedFont.aURL]));
                                XMLBase64Export aBase64Exp(GetExport());
                                aBase64Exp.exportOfficeBinaryDataElement(xInput);
                            }
                            catch (const uno::Exception &)
                            {
                                // opening the file failed, ignore
                            }
                        }

                        GetExport().AddAttribute(XML_NAMESPACE_SVG, XML_STRING, "truetype");
                        SvXMLElementExport fontFaceFormat(GetExport(), XML_NAMESPACE_SVG,
                                                          XML_FONT_FACE_FORMAT, true, true);
                    }
                }
            }
        }
    }
}

OUString getFreeFontName(uno::Reference<embed::XStorage> const & rxStorage, OUString const & rFamilyName)
{
    OUString sName;
    int nIndex = 1;
    do
    {
        sName = "Font_" +
                rFamilyName.replaceAll(" ", "_") + "_" +
                OUString::number(nIndex) + ".ttf";
        nIndex++;
    } while (rxStorage->hasByName(sName));

    return sName;
}

OString convertToHashString(std::vector<unsigned char> const & rHash)
{
    std::stringstream aStringStream;
    for (auto const & rByte : rHash)
    {
        aStringStream << std::setw(2) << std::setfill('0') << std::hex << int(rByte);
    }

    return OString(aStringStream.str().c_str());
}

OString getFileHash(OUString const & rFileUrl)
{
    OString aHash;
    osl::File aFile(rFileUrl);
    if (aFile.open(osl_File_OpenFlag_Read) != osl::File::E_None)
        return aHash;

    comphelper::Hash aHashEngine(comphelper::HashType::SHA512);
    for (;;)
    {
        sal_Int8 aBuffer[4096];
        sal_uInt64 nReadSize;
        sal_Bool bEof;
        if (aFile.isEndOfFile(&bEof) != osl::File::E_None)
        {
            SAL_WARN("xmloff", "Error reading font file " << rFileUrl);
            return aHash;
        }
        if (bEof)
            break;
        if (aFile.read(aBuffer, 4096, nReadSize) != osl::File::E_None)
        {
            SAL_WARN("xmloff", "Error reading font file " << rFileUrl);
            return aHash;
        }
        if (nReadSize == 0)
            break;
        aHashEngine.update(reinterpret_cast<unsigned char*>(aBuffer), nReadSize);
    }
    return convertToHashString(aHashEngine.finalize());
}

OUString XMLFontAutoStylePool::embedFontFile(OUString const & fileUrl, OUString const & rFamilyName)
{
    try
    {
        OString sHashString = getFileHash(fileUrl);
        if (m_aEmbeddedFontFiles.find(sHashString) != m_aEmbeddedFontFiles.end())
            return m_aEmbeddedFontFiles.at(sHashString);

        osl::File file( fileUrl );
        if( file.open( osl_File_OpenFlag_Read ) != osl::File::E_None )
            return OUString();

        if ( !GetExport().GetTargetStorage().is() )
            return OUString();

        uno::Reference< embed::XStorage > storage;
        storage.set( GetExport().GetTargetStorage()->openStorageElement( "Fonts",
            ::embed::ElementModes::WRITE ), uno::UNO_QUERY_THROW );

        OUString name = getFreeFontName(storage, rFamilyName);

        uno::Reference< io::XOutputStream > outputStream;
        outputStream.set( storage->openStreamElement( name, ::embed::ElementModes::WRITE ), UNO_QUERY_THROW );
        uno::Reference < beans::XPropertySet > propertySet( outputStream, uno::UNO_QUERY );
        assert( propertySet.is());
        propertySet->setPropertyValue( "MediaType", uno::makeAny( OUString( "application/x-font-ttf" ))); // TODO
        for(;;)
        {
            char buffer[ 4096 ];
            sal_uInt64 readSize;
            sal_Bool eof;
            if( file.isEndOfFile( &eof ) != osl::File::E_None )
            {
                SAL_WARN( "xmloff", "Error reading font file " << fileUrl );
                outputStream->closeOutput();
                return OUString();
            }
            if( eof )
                break;
            if( file.read( buffer, 4096, readSize ) != osl::File::E_None )
            {
                SAL_WARN( "xmloff", "Error reading font file " << fileUrl );
                outputStream->closeOutput();
                return OUString();
            }
            if( readSize == 0 )
                break;
            outputStream->writeBytes( uno::Sequence< sal_Int8 >( reinterpret_cast< const sal_Int8* >( buffer ), readSize ));
        }
        outputStream->closeOutput();
        if( storage.is() )
        {
            Reference< embed::XTransactedObject > transaction( storage, UNO_QUERY );
            if( transaction.is())
            {
                transaction->commit();
                OUString sInternalName = "Fonts/" + name;
                m_aEmbeddedFontFiles.emplace(sHashString, sInternalName);
                return sInternalName;
            }
        }
    } catch( const Exception& e )
    {
        SAL_WARN( "xmloff", "Exception when embedding a font file:" << e );
    }
    return OUString();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
