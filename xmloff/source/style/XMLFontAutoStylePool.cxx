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
#include <utility>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmluconv.hxx>
#include "fonthdl.hxx"
#include <xmloff/xmlexp.hxx>
#include <xmloff/XMLFontAutoStylePool.hxx>
#include <vcl/embeddedfontshelper.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/style/XStyle.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include <XMLBase64Export.hxx>
#include <AutoStyleEntry.hxx>
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
            OUString aName,
            OUString aFamilyName,
            OUString aStyleName,
            FontFamily nFamily,
            FontPitch nPitch,
            rtl_TextEncoding eEnc );

    inline XMLFontAutoStylePoolEntry_Impl(
            OUString aFamilyName,
            OUString aStyleName,
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
        OUString aName,
        OUString aFamilyName,
        OUString aStyleName,
        FontFamily nFam,
        FontPitch nP,
        rtl_TextEncoding eE ) :
    sName(std::move( aName )),
    sFamilyName(std::move( aFamilyName )),
    sStyleName(std::move( aStyleName )),
    nFamily( nFam ),
    nPitch( nP ),
    eEnc( eE )
{
}

inline XMLFontAutoStylePoolEntry_Impl::XMLFontAutoStylePoolEntry_Impl(
        OUString  rFamilyName,
        OUString  rStyleName,
        FontFamily nFam,
        FontPitch nP,
        rtl_TextEncoding eE ) :
    sFamilyName(std::move( rFamilyName )),
    sStyleName(std::move( rStyleName )),
    nFamily( nFam ),
    nPitch( nP ),
    eEnc( eE )
{
}

namespace {

struct XMLFontAutoStylePoolEntryCmp_Impl {
    template <typename T1, typename T2>
        requires o3tl::is_reference_to<T1, XMLFontAutoStylePoolEntry_Impl>
                 && o3tl::is_reference_to<T2, XMLFontAutoStylePoolEntry_Impl>
    bool operator()(T1 const& r1, T2 const& r2) const
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

}

class XMLFontAutoStylePool_Impl : public o3tl::sorted_vector<std::unique_ptr<XMLFontAutoStylePoolEntry_Impl>, XMLFontAutoStylePoolEntryCmp_Impl>
{
};

XMLFontAutoStylePool::XMLFontAutoStylePool(SvXMLExport& rExp) :
    m_rExport( rExp ),
    m_pFontAutoStylePool( new XMLFontAutoStylePool_Impl )
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
            sName = sPrefix + OUString::number( nCount );
            while( m_aNames.find(sName) != m_aNames.end() )
            {
                sName = sPrefix + OUString::number( ++nCount );
            }
        }

        std::unique_ptr<XMLFontAutoStylePoolEntry_Impl> pEntry(
            new XMLFontAutoStylePoolEntry_Impl( sName, rFamilyName, rStyleName,
                                                nFamily, nPitch, eEnc ));
        m_pFontAutoStylePool->insert( std::move(pEntry) );
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

std::unordered_set<OUString> SvXMLExport::getUsedFontList()
{
    std::unordered_set<OUString> aReturnSet;

    uno::Reference<style::XStyleFamiliesSupplier> xFamiliesSupp(GetModel(), UNO_QUERY);
    if (!xFamiliesSupp.is())
        return aReturnSet;

    // Check styles first
    uno::Reference<container::XNameAccess> xFamilies(xFamiliesSupp->getStyleFamilies());
    if (xFamilies.is())
    {
        const bool bEmbedLatinScript = getEmbedLatinScript();
        const bool bEmbedAsianScript = getEmbedAsianScript();
        const bool bEmbedComplexScript = getEmbedComplexScript();
        const uno::Sequence<OUString> aFamilyNames = xFamilies->getElementNames();
        for (OUString const & sFamilyName : aFamilyNames)
        {
            uno::Reference<container::XNameAccess> xStyleContainer;
            xFamilies->getByName(sFamilyName) >>= xStyleContainer;

            if (xStyleContainer.is())
            {
                const uno::Sequence<OUString> aStyleNames = xStyleContainer->getElementNames();
                for (OUString const & rName : aStyleNames)
                {
                    uno::Reference<style::XStyle> xStyle;
                    xStyleContainer->getByName(rName) >>= xStyle;
                    if (xStyle->isInUse())
                    {
                        uno::Reference<beans::XPropertySet> xPropertySet(xStyle, UNO_QUERY);
                        uno::Reference<beans::XPropertySetInfo> xInfo(xPropertySet ? xPropertySet->getPropertySetInfo() : nullptr);
                        if (xInfo)
                        {
                            if (bEmbedLatinScript && xInfo->hasPropertyByName(u"CharFontName"_ustr))
                            {
                                OUString sCharFontName;
                                Any aFontAny = xPropertySet->getPropertyValue(u"CharFontName"_ustr);
                                aFontAny >>= sCharFontName;
                                if (!sCharFontName.isEmpty())
                                    aReturnSet.insert(sCharFontName);
                            }
                            if (bEmbedAsianScript && xInfo->hasPropertyByName(u"CharFontNameAsian"_ustr))
                            {
                                OUString sCharFontNameAsian;
                                Any aFontAny = xPropertySet->getPropertyValue(u"CharFontNameAsian"_ustr);
                                aFontAny >>= sCharFontNameAsian;
                                if (!sCharFontNameAsian.isEmpty())
                                    aReturnSet.insert(sCharFontNameAsian);
                            }
                            if (bEmbedComplexScript && xInfo->hasPropertyByName(u"CharFontNameComplex"_ustr))
                            {
                                OUString sCharFontNameComplex;
                                Any aFontAny = xPropertySet->getPropertyValue(u"CharFontNameComplex"_ustr);
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
    collectAutoStyles();

    // Check auto-styles for fonts
    std::vector<xmloff::AutoStyleEntry> aAutoStyleEntries = GetAutoStylePool()->GetAutoStyleEntries();
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

void SvXMLExport::exportFonts(const std::vector<XMLFontAutoStylePoolEntry_Impl*>& rFonts)
{
    SvXMLElementExport aElem(*this, XML_NAMESPACE_OFFICE, XML_FONT_FACE_DECLS, true, true);

    Any aAny;
    OUString sTmp;
    XMLFontFamilyNamePropHdl aFamilyNameHdl;
    XMLFontFamilyPropHdl aFamilyHdl;
    XMLFontPitchPropHdl aPitchHdl;
    XMLFontEncodingPropHdl aEncHdl;
    const bool bEmbedFonts = getEmbedFonts();
    const bool bEmbedUsedOnly = bEmbedFonts && getEmbedOnlyUsedFonts();

    std::map<OUString, OUString> fontFilesMap; // our url to document url

    std::unordered_set<OUString> aUsedFontNames;
    if (bEmbedUsedOnly)
        aUsedFontNames = getUsedFontList();

    for (const auto& pEntry : rFonts)
    {
        AddAttribute(XML_NAMESPACE_STYLE, XML_NAME, pEntry->GetName());

        aAny <<= pEntry->GetFamilyName();
        if (aFamilyNameHdl.exportXML(sTmp, aAny, GetMM100UnitConverter()))
            AddAttribute(XML_NAMESPACE_SVG, XML_FONT_FAMILY, sTmp);

        const OUString& rStyleName = pEntry->GetStyleName();
        if (!rStyleName.isEmpty())
            AddAttribute(XML_NAMESPACE_STYLE, XML_FONT_ADORNMENTS, rStyleName);

        aAny <<= static_cast<sal_Int16>(pEntry->GetFamily());
        if (aFamilyHdl.exportXML(sTmp, aAny, GetMM100UnitConverter()))
        {
            AddAttribute(XML_NAMESPACE_STYLE, XML_FONT_FAMILY_GENERIC, sTmp);
        }
        aAny <<= static_cast<sal_Int16>(pEntry->GetPitch());
        if (aPitchHdl.exportXML(sTmp, aAny, GetMM100UnitConverter()))
        {
            AddAttribute(XML_NAMESPACE_STYLE, XML_FONT_PITCH, sTmp);
        }

        aAny <<= static_cast<sal_Int16>(pEntry->GetEncoding());
        if (aEncHdl.exportXML(sTmp, aAny, GetMM100UnitConverter()))
        {
            AddAttribute(XML_NAMESPACE_STYLE, XML_FONT_CHARSET, sTmp);
        }

        SvXMLElementExport aElement(*this, XML_NAMESPACE_STYLE, XML_FONT_FACE, true, true);

        // When embedding is requested, and embedded only is not set or font is used
        if (bEmbedFonts && (!bEmbedUsedOnly || aUsedFontNames.contains(pEntry->GetFamilyName())))
        {
            const bool bExportFlat(getExportFlags() & SvXMLExportFlags::EMBEDDED);

            /// Contains information about a single variant of an embedded font.
            struct EmbeddedFontInfo
            {
                OUString aURL;
                FontWeight eWeight;
                FontItalic eItalic;
            };
            std::vector<EmbeddedFontInfo> aEmbeddedFonts;

            for (FontItalic fontItalic : { ITALIC_NONE, ITALIC_NORMAL })
            {
                for (FontWeight fontWeight : { WEIGHT_NORMAL, WEIGHT_BOLD })
                {
                    // Embed font if at least viewing is allowed (in which case the opening app must check
                    // the font license rights too and open either read-only or not use the font for editing).
                    OUString sFileUrl = EmbeddedFontsHelper::fontFileUrl(
                        pEntry->GetFamilyName(), pEntry->GetFamily(), fontItalic, fontWeight,
                        pEntry->GetPitch(), EmbeddedFontsHelper::FontRights::ViewingAllowed);
                    if (sFileUrl.isEmpty())
                        continue;

                    if (!fontFilesMap.contains(sFileUrl))
                    {
                        const OUString docUrl = bExportFlat ?
                                                    lcl_checkFontFile(sFileUrl) : embedFontFile(sFileUrl, pEntry->GetFamilyName());
                        if (!docUrl.isEmpty())
                            fontFilesMap[sFileUrl] = docUrl;
                        else
                            continue; // --> failed to embed
                    }
                    aEmbeddedFonts.push_back({ sFileUrl, fontWeight, fontItalic });
                }
            }
            if (!aEmbeddedFonts.empty())
            {
                SvXMLElementExport fontFaceSrc(*this, XML_NAMESPACE_SVG, XML_FONT_FACE_SRC, true, true);
                for (EmbeddedFontInfo const & rEmbeddedFont : aEmbeddedFonts)
                {
                    if (fontFilesMap.contains(rEmbeddedFont.aURL))
                    {
                        if (!bExportFlat)
                        {
                            AddAttribute(XML_NAMESPACE_XLINK, XML_HREF,
                                         fontFilesMap[rEmbeddedFont.aURL]);
                            AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, u"simple"_ustr);
                        }

                        // Help consumers of our output by telling them which
                        // font file is which one.
                        AddAttribute(XML_NAMESPACE_LO_EXT, XML_FONT_STYLE,
                                     FontItalicToString(rEmbeddedFont.eItalic));
                        AddAttribute(XML_NAMESPACE_LO_EXT, XML_FONT_WEIGHT,
                                     FontWeightToString(rEmbeddedFont.eWeight));

                        SvXMLElementExport fontFaceUri(*this, XML_NAMESPACE_SVG,
                                                       XML_FONT_FACE_URI, true, true);

                        if (bExportFlat)
                        {
                            const uno::Reference<ucb::XSimpleFileAccess> xFileAccess(
                                ucb::SimpleFileAccess::create(getComponentContext()));
                            try
                            {
                                const uno::Reference<io::XInputStream> xInput(xFileAccess->openFileRead(fontFilesMap[rEmbeddedFont.aURL]));
                                XMLBase64Export aBase64Exp(*this);
                                aBase64Exp.exportOfficeBinaryDataElement(xInput);
                            }
                            catch (const uno::Exception &)
                            {
                                // opening the file failed, ignore
                            }
                        }

                        AddAttribute(XML_NAMESPACE_SVG, XML_STRING, u"truetype"_ustr);
                        SvXMLElementExport fontFaceFormat(*this, XML_NAMESPACE_SVG,
                                                          XML_FONT_FACE_FORMAT, true, true);
                    }
                }
            }
        }
    }
}

std::unordered_map<OString, OUString> SvXMLExport::getEmbeddedFontFiles() const
{
    return m_aEmbeddedFontFiles;
}

void SvXMLExport::setEmbeddedFontFiles(const std::unordered_map<OString, OUString>& value)
{
    m_aEmbeddedFontFiles = value;
}

void XMLFontAutoStylePool::exportXML()
{
    // Sort <style:font-face> elements based on their style:name attribute.
    std::vector<XMLFontAutoStylePoolEntry_Impl*> aFontAutoStyles;
    for (const auto& pEntry : *m_pFontAutoStylePool)
    {
        aFontAutoStyles.push_back(pEntry.get());
    }
    std::sort(
        aFontAutoStyles.begin(), aFontAutoStyles.end(),
        [](const XMLFontAutoStylePoolEntry_Impl* pA, XMLFontAutoStylePoolEntry_Impl* pB) -> bool {
            return pA->GetName() < pB->GetName();
        });
    GetExport().exportFonts(aFontAutoStyles);
}

static OUString getFreeFontName(uno::Reference<embed::XStorage> const & rxStorage, OUString const & rFamilyName)
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

static OString convertToHashString(std::vector<unsigned char> const & rHash)
{
    std::stringstream aStringStream;
    for (auto const & rByte : rHash)
    {
        aStringStream << std::setw(2) << std::setfill('0') << std::hex << int(rByte);
    }

    return OString(aStringStream.str());
}

static OString getFileHash(OUString const & rFileUrl)
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
        aHashEngine.update(aBuffer, nReadSize);
    }
    return convertToHashString(aHashEngine.finalize());
}

OUString SvXMLExport::embedFontFile(OUString const& fileUrl, OUString const& rFamilyName)
{
    try
    {
        OString sHashString = getFileHash(fileUrl);
        if (m_aEmbeddedFontFiles.find(sHashString) != m_aEmbeddedFontFiles.end())
            return m_aEmbeddedFontFiles.at(sHashString);

        osl::File file( fileUrl );
        if( file.open( osl_File_OpenFlag_Read ) != osl::File::E_None )
            return OUString();

        if ( !GetTargetStorage().is() )
            return OUString();

        uno::Reference< embed::XStorage > storage;
        storage.set( GetTargetStorage()->openStorageElement( u"Fonts"_ustr,
            ::embed::ElementModes::WRITE ), uno::UNO_SET_THROW );

        OUString name = getFreeFontName(storage, rFamilyName);

        uno::Reference< io::XOutputStream > outputStream;
        outputStream.set( storage->openStreamElement( name, ::embed::ElementModes::WRITE ), UNO_QUERY_THROW );
        uno::Reference < beans::XPropertySet > propertySet( outputStream, uno::UNO_QUERY );
        assert( propertySet.is());
        propertySet->setPropertyValue( u"MediaType"_ustr, uno::Any( u"application/x-font-ttf"_ustr)); // TODO
        for(;;)
        {
            sal_Int8 buffer[ 4096 ];
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
            // coverity[overrun-buffer-arg : FALSE] - coverity has difficulty with css::uno::Sequence
            outputStream->writeBytes(uno::Sequence<sal_Int8>(buffer, readSize));
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
    } catch( const Exception& )
    {
        TOOLS_WARN_EXCEPTION( "xmloff", "Exception when embedding a font file" );
    }
    return OUString();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
