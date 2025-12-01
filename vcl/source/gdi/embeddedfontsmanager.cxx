/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <memory>
#include <set>
#include <unordered_map>
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_set.h>
#include <config_folders.h>
#include <config_eot.h>

#include <o3tl/temporary.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/uri.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/embeddedfontsmanager.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/diagnose_ex.hxx>
#include <comphelper/interaction.hxx>
#include <comphelper/lok.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/storagehelper.hxx>
#include <unotools/tempfile.hxx>

#include <font/PhysicalFontFaceCollection.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <salgdi.hxx>
#include <sft.hxx>
#include <tools/stream.hxx>

#include <com/sun/star/beans/StringPair.hpp>
#include <com/sun/star/document/FontsDisallowEditingRequest.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/frame/XModel2.hpp>

#if ENABLE_EOT
extern "C"
{
namespace libeot
{
#include <libeot/libeot.h>
} // namespace libeot
} // extern "C"
#endif

using namespace com::sun::star;
using namespace vcl;

namespace
{
OUString GetStandardEmbeddedFontsRoot()
{
    OUString p = u"${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") "::UserInstallation}"_ustr;
    rtl::Bootstrap::expandMacros(p);
    return p + "/user/temp/embeddedfonts";
}

OUString GetEmbeddedFontsRootURL(bool bLOK)
{
    OUString p = bLOK ? utl::CreateTempURL(nullptr, true) : GetStandardEmbeddedFontsRoot();
    // Drop things like "//" and "/../"; ensure trailing slash
    (void)osl::FileBase::getAbsoluteFileURL({}, p + "/", p);
    return p;
}

const OUString& GetEmbeddedFontsRoot()
{
    static const OUString path = GetEmbeddedFontsRootURL(comphelper::LibreOfficeKit::isActive());
    return path;
}

struct EmbeddedFontData
{
    OUString familyName;
    int refcount = 0;
    bool isRestricted = true; // Has restricted permissions, *and* isn't installed locally
    bool isActivated = false;
};

// file URL -> EmbeddedFontData
std::unordered_map<OUString, EmbeddedFontData> s_EmbeddedFonts;

void clearDir( const OUString& path )
{
    osl::Directory dir( path );
    if( dir.reset() == osl::Directory::E_None )
    {
        for (osl::DirectoryItem item; dir.getNextItem(item) == osl::Directory::E_None;)
        {
            osl::FileStatus status( osl_FileStatus_Mask_FileURL );
            if( item.getFileStatus( status ) == osl::File::E_None )
                osl::File::remove( status.getFileURL());
        }
    }
}
}

// Returns a URL for a file where to store contents of a given temporary font.
// The file may or not may not exist yet, and will be cleaned up automatically as appropriate.
// Use activateFonts() to actually enable usage of the font.
// static
OUString EmbeddedFontsManager::getFileUrlForTemporaryFont(std::u16string_view name, std::u16string_view suffix)
{
    OUString filename = OUString::Concat(name) + suffix;

    if (!comphelper::OStorageHelper::IsValidZipEntryFileName(filename, false))
    {
        SAL_WARN("vcl.fonts", "Cannot use filename: " << filename << " for temporary font");
        static int uniqueCounter = 0;
        filename = "font" + OUString::number(uniqueCounter++) + ".ttf";
    }

    OUString path = GetEmbeddedFontsRoot() + "fromdocs/";
    osl::Directory::createPath(path);
    return path
           + rtl::Uri::encode(filename, rtl_UriCharClassPchar, rtl_UriEncodeIgnoreEscapes,
                              RTL_TEXTENCODING_UTF8);
}

namespace
{
bool writeFontBytesToFile(osl::File& file, const void* data, sal_uInt64 size)
{
    auto bytes = static_cast<const char*>(data);
    for (sal_uInt64 writtenTotal = 0; writtenTotal < size;)
    {
        sal_uInt64 written = 0;
        switch (file.write(bytes + writtenTotal, size - writtenTotal, written))
        {
            case osl::File::E_None:
                writtenTotal += written;
                break;
            case osl::File::E_AGAIN:
            case osl::File::E_INTR:
                break;
            default:
                SAL_WARN("vcl.fonts", "Writing temporary font file failed");
                file.close();
                osl::File::remove(file.getURL());
                return false;
        }
    }
    if (file.close() != osl::File::E_None)
    {
        // Something failed in delayed writing?
        SAL_WARN("vcl.fonts", "Writing temporary font file failed");
        osl::File::remove(file.getURL());
        return false;
    }

    return true;
}

// Returns actual URL (maybe of an already existing file), or empty string on failure.
//
// @param name name of the font file
OUString writeFontBytesToFile(const std::vector<char>& bytes, std::u16string_view name)
{
    OUString url = EmbeddedFontsManager::getFileUrlForTemporaryFont(name, u".ttf");
    std::optional<osl::File> file(url);
    auto rc = file->open(osl_File_OpenFlag_Create | osl_File_OpenFlag_Write);

    // Check if existing file is the same
    for (int counter = 0; rc == osl::File::E_EXIST;)
    {
        if (file->open(osl_File_OpenFlag_Read | osl_File_OpenFlag_NoLock) == osl::File::E_None)
        {
            if (sal_uInt64 size; file->getSize(size) == osl::File::E_None && size == bytes.size())
            {
                std::vector<char> bytes2(bytes.size());
                sal_uInt64 readTotal = 0;
                while (readTotal < bytes.size())
                {
                    sal_uInt64 read = 0;
                    rc = file->read(bytes2.data() + readTotal, bytes.size() - readTotal, read);
                    if (read == 0)
                        break;
                    readTotal += read;
                }
                if (rc == osl::File::E_None && bytes2 == bytes)
                {
                    return url; // OK, it's the same bytes
                }
            }
        }
        url = EmbeddedFontsManager::getFileUrlForTemporaryFont(Concat2View(name + OUString::number(counter++)), u".ttf");
        file.emplace(url);
        rc = file->open(osl_File_OpenFlag_Create | osl_File_OpenFlag_Write);
    }

    if (rc != osl::File::E_None)
        return {};

    if (!writeFontBytesToFile(*file, bytes.data(), bytes.size()))
        return {};

    return url;
}

OUString getFilenameForExport(std::u16string_view familyName, FontFamily family, FontItalic italic,
                              FontWeight weight, FontPitch pitch)
{
    OUString filename = OUString::Concat(familyName) + "_" + OUString::number(family) + "_"
                        + OUString::number(italic) + "_" + OUString::number(weight) + "_"
                        + OUString::number(pitch) + ".ttf";
    return rtl::Uri::encode(filename, rtl_UriCharClassPchar, rtl_UriEncodeIgnoreEscapes,
                            RTL_TEXTENCODING_UTF8);
}

// Check if it's (legally) allowed to embed the font file into a document
// (ttf has a flag allowing this). PhysicalFontFace::IsEmbeddable() appears
// to have a different meaning (guessing from code, IsSubsettable() might
// possibly mean it's ttf, while IsEmbeddable() might mean it's type1).
// So just try to open the data as ttf and see.
bool sufficientTTFRights(const void* data, tools::Long size,
                         EmbeddedFontsManager::FontRights rights)
{
    TrueTypeFont* font;
    if (OpenTTFontBuffer(data, size, 0 /*TODO*/, &font) == SFErrCodes::Ok)
    {
        TTGlobalFontInfo info;
        GetTTGlobalFontInfo(font, &info);
        CloseTTFont(font);
        // https://www.microsoft.com/typography/otspec/os2.htm#fst
        int copyright = info.typeFlags;
        switch (rights)
        {
            case EmbeddedFontsManager::FontRights::ViewingAllowed:
                // Embedding not restricted completely.
                return (copyright & 0x02) != 0x02;
            case EmbeddedFontsManager::FontRights::EditingAllowed:
                // Font is installable or editable.
                return copyright == 0 || (copyright & 0x08);
        }
    }
    return true; // no known restriction
}

}

EmbeddedFontsManager::EmbeddedFontsManager(const uno::Reference<frame::XModel>& xModel)
    : m_xDocumentModel(xModel)
{
}

EmbeddedFontsManager::~EmbeddedFontsManager()
{
    if (m_aAccumulatedFonts.empty())
        return;

    if (auto xModel2 = m_xDocumentModel.query<frame::XModel2>())
    {
        // Send embedded fonts list into the document's media descriptor.
        // This helps to decide if the document is allowed to switch to edit mode.
        try
        {
            uno::Sequence<beans::StringPair> fonts(m_aAccumulatedFonts.size());
            std::transform(m_aAccumulatedFonts.begin(), m_aAccumulatedFonts.end(), fonts.getArray(),
                           [](const auto& el) { return beans::StringPair(el.first, el.second); });
            xModel2->setArgs({ comphelper::makePropertyValue(u"EmbeddedFonts"_ustr, fonts) });
            // The following will not get executed, if setArgs throws
            return;
        }
        catch (const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.core");
        }
    }

    // Failed to transfer the fonts to the document. Activate them here, discarding restricted.
    // They won't be released, so will stay until the application shutdown.
    activateFonts(m_aAccumulatedFonts, false, {}, o3tl::temporary(bool()));
}

void EmbeddedFontsManager::clearTemporaryFontFiles()
{
    const OUString& path = GetEmbeddedFontsRoot();
    clearDir( path + "fromdocs/" );
    clearDir( path + "fromsystem/" );
}

bool EmbeddedFontsManager::addEmbeddedFont( const uno::Reference< io::XInputStream >& stream, const OUString& fontName,
    std::u16string_view extra, std::vector< unsigned char > const & key, bool eot,
    bool bSubsetted )
{
    size_t keyPos = 0;
    std::vector< char > fontData;
    fontData.reserve( 1000000 );
    for(;;)
    {
        uno::Sequence< sal_Int8 > buffer;
        sal_uInt64 read = stream->readBytes( buffer, 1024 );
        auto bufferRange = asNonConstRange(buffer);
        for( sal_uInt64 pos = 0;
             pos < read && keyPos < key.size();
             ++pos )
            bufferRange[ pos ] ^= key[ keyPos++ ];
        fontData.insert( fontData.end(), buffer.getConstArray(), buffer.getConstArray() + read );
        if( read <= 0 )
            break;
    }
    bool sufficientFontRights(false);
#if ENABLE_EOT
    if( eot )
    {
        unsigned uncompressedFontSize = 0;
        unsigned char *nakedPointerToUncompressedFont = nullptr;
        libeot::EOTMetadata eotMetadata;
        libeot::EOTError uncompressError =
            libeot::EOT2ttf_buffer( reinterpret_cast<unsigned char *>(fontData.data()), fontData.size(), &eotMetadata, &nakedPointerToUncompressedFont, &uncompressedFontSize );
        std::shared_ptr<unsigned char> uncompressedFont( nakedPointerToUncompressedFont, libeot::EOTfreeBuffer );
        if( uncompressError != libeot::EOT_SUCCESS )
        {
            SAL_WARN( "vcl.fonts", "Failed to uncompress font" );
            return false;
        }
        fontData.clear();
        fontData.insert(fontData.end(), reinterpret_cast<char*>(uncompressedFont.get()),
                        reinterpret_cast<char*>(uncompressedFont.get() + uncompressedFontSize));
        sufficientFontRights = libeot::EOTcanLegallyEdit( &eotMetadata );
        libeot::EOTfreeMetadata( &eotMetadata );
    }
#endif

    if( !eot )
    {
        sufficientFontRights = sufficientTTFRights(fontData.data(), fontData.size(), FontRights::EditingAllowed);
    }

    if (bSubsetted)
    {
        TrueTypeFont* font;
        sal_uInt32 nGlyphs = 0;
        if (OpenTTFontBuffer(fontData.data(), fontData.size(), 0, &font) == SFErrCodes::Ok)
        {
            sal_uInt32 nGlyphCount = font->glyphCount();
            for (sal_uInt32 i = 0; i < nGlyphCount; ++i)
            {
                sal_uInt32 nOffset = font->glyphOffset(i);
                sal_uInt32 nNextOffset = font->glyphOffset(i + 1);
                if (nOffset == nNextOffset)
                {
                    // GetTTGlyphComponents() says this is an empty glyph, ignore it.
                    continue;
                }
                ++nGlyphs;
            }
            CloseTTFont(font);
        }
        // Check if it has reasonable amount of glyphs, set the limit to the number of glyphs in the
        // English alphabet (not differentiating lowercase and uppercase).
        if (nGlyphs < 26)
        {
            SAL_INFO("vcl.fonts", "Ignoring embedded font that only provides " << nGlyphs << " non-empty glyphs");
            return false;
        }
    }

    OUString fileUrl = writeFontBytesToFile(fontData, Concat2View(fontName + extra));
    if (fileUrl.isEmpty())
        return false;

    return addEmbeddedFont(fileUrl, fontName, sufficientFontRights);
}

bool EmbeddedFontsManager::addEmbeddedFont( const OUString& fileUrl, const OUString& fontName, bool sufficientFontRights )
{

    // Register  it / increase its refcount in s_EmbeddedFonts
    {
        DBG_TESTSOLARMUTEX();
        auto& rData = s_EmbeddedFonts[fileUrl];
        if (rData.refcount == 0)
        {
            rData.familyName = fontName;
            // Check if the font is already installed on system. At this point, we know that this
            // restricted font hasn't yet been activated as embedded (rData.refcount == 0); if it
            // is already available, it's pre-installed, meaning that there are sufficient rights
            // to use the font. You can always edit with the restricted fonts taken from your own
            // system.
            rData.isRestricted = !sufficientFontRights
                                 && !Application::GetDefaultDevice()->IsFontAvailable(fontName);
        }
        assert(rData.familyName == fontName);
        ++rData.refcount;
    }

    m_aAccumulatedFonts.emplace_back(fontName, fileUrl);

    return true;
}

namespace
{
    struct UpdateFontsGuard
    {
        UpdateFontsGuard()
        {
            OutputDevice::ImplClearAllFontData(true);
        }

        ~UpdateFontsGuard()
        {
            OutputDevice::ImplRefreshAllFontData(true);
        }
    };
}

void EmbeddedFontsManager::activateFonts(std::vector<std::pair<OUString, OUString>>& fonts,
                                        bool silentlyAllowRestrictedFonts,
                                        const uno::Reference<task::XInteractionHandler>& xHandler,
                                        bool& activatedRestrictedFonts)
{
    activatedRestrictedFonts = false;
    if (fonts.empty())
        return;
    std::vector<std::pair<OUString, OUString>> temp;

    {
        DBG_TESTSOLARMUTEX();
        // Handle restricted fonts
        for (auto it1 = fonts.begin(); it1 != fonts.end();)
        {
            auto it2 = s_EmbeddedFonts.find(it1->second);
            if (it2 == s_EmbeddedFonts.end())
            {
                SAL_WARN("vcl.fonts", "Trying to activate a font not in s_EmbeddedFonts");
                it1 = fonts.erase(it1);
                continue;
            }
            assert(it2->second.familyName == it1->first);

            if (!silentlyAllowRestrictedFonts && it2->second.isRestricted)
            {
                temp.push_back(*it1);
                it1 = fonts.erase(it1);
                continue;
            }

            ++it1;
        }
    }

    if (!temp.empty())
    {
        bool allowRestrictedFonts = false;
        if (xHandler)
        {
            std::set<OUString> filteredFamilies; // families can repeat, e.g. for bold/italic
            for (const auto& pair : temp)
                filteredFamilies.insert(pair.first);
            OUStringBuffer fontlist;
            for (const auto& family : filteredFamilies)
                fontlist.append("\n" + family);
            rtl::Reference pRequest(new comphelper::OInteractionRequest(
                uno::Any(document::FontsDisallowEditingRequest({}, {}, fontlist.toString()))));
            rtl::Reference pApprove(new comphelper::OInteractionApprove);
            pRequest->addContinuation(pApprove);
            pRequest->addContinuation(new comphelper::OInteractionDisapprove);
            xHandler->handle(pRequest);
            allowRestrictedFonts = pApprove->wasSelected();
        }
        if (allowRestrictedFonts)
        {
            activatedRestrictedFonts = true;
            fonts.insert(fonts.end(), temp.begin(), temp.end());
        }
        else
        {
            releaseFonts(temp);
        }
        temp.clear();
    }

    {
        DBG_TESTSOLARMUTEX();
        // Only activate fonts that need activation. It goes after restricted fonts handling,
        // because we must ask user about a second document embedding the same restricted font.
        // We do not remove from fonts: the unlocking must happen only when the document is closed,
        // so that necessary fonts are not unregistered.
        for (const auto& pair : fonts)
        {
            auto it = s_EmbeddedFonts.find(pair.second);
            // At this point, we must find a match: neither releaseFonts above, nor other possible
            // intermediate changes of s_EmbeddedFonts must not remove our locked entries
            assert(it != s_EmbeddedFonts.end());
            if (!it->second.isActivated)
            {
                it->second.isActivated = true;
                temp.push_back(pair);
            }
        }
    }

    if (temp.empty())
        return;

    UpdateFontsGuard aUpdateFontsGuard;
    OutputDevice *pDevice = Application::GetDefaultDevice();
    for (const auto& [ fontName, fileUrl ] : temp)
        pDevice->AddTempDevFont(fileUrl, fontName);
}

void EmbeddedFontsManager::releaseFonts(const std::vector<std::pair<OUString, OUString>>& fonts)
{
    // Background save process: avoid deletion of files created by the parent
    if (comphelper::LibreOfficeKit::isForkedChild())
        return;
    std::vector<std::pair<OUString, OUString>> unregister;
    if (!fonts.empty())
    {
        DBG_TESTSOLARMUTEX();
        for (const auto& pair : fonts)
        {
            auto it = s_EmbeddedFonts.find(pair.second);
            if (it == s_EmbeddedFonts.end())
            {
                SAL_WARN("vcl.fonts", "Trying to release a font that wasn't locked?");
                continue;
            }
            assert(it->second.familyName == pair.first);

            --it->second.refcount;
            if (it->second.refcount == 0)
            {
                unregister.emplace_back(pair);

                s_EmbeddedFonts.erase(it);
            }
        }
    }

    if (unregister.empty())
        return;

    OutputDevice* pDevice = Application::GetDefaultDevice();
    for (const auto& [ family, url ] : unregister)
    {
        if (pDevice->RemoveTempDevFont(url, family))
            osl::File::remove(url);
    }

    OutputDevice::ImplUpdateAllFontData(true);
}

// static
bool EmbeddedFontsManager::isEmbeddedAndRestricted(std::u16string_view familyName)
{
    DBG_TESTSOLARMUTEX();
    for (const auto& pair : s_EmbeddedFonts)
    {
        if (pair.second.familyName == familyName && pair.second.isRestricted)
            return true;
    }
    return false;
}

bool EmbeddedFontsManager::analyzeTTF(const void* data, tools::Long size, FontWeight& weight)
{
    TrueTypeFont* font;
    if (OpenTTFontBuffer( data, size, 0 /*TODO*/, &font ) != SFErrCodes::Ok)
        return false;

    weight = AnalyzeTTFWeight(font);
    CloseTTFont(font);

    return true;
}

OUString EmbeddedFontsManager::fontFileUrl( std::u16string_view familyName, FontFamily family, FontItalic italic,
    FontWeight weight, FontPitch pitch, FontRights rights )
{
    // Do not embed restricted fonts not installed locally.
    if (isEmbeddedAndRestricted(familyName))
        return {};

    OUString path = GetEmbeddedFontsRoot() + "fromsystem/";
    osl::Directory::createPath( path );
    OUString url = path + getFilenameForExport(familyName, family, italic, weight, pitch);
    if( osl::File( url ).open( osl_File_OpenFlag_Read ) == osl::File::E_None ) // = exists()
    {
        // File with contents of the font file already exists, assume it's been created by a previous call.
        return url;
    }
    bool ok = false;
    SalGraphics* graphics = Application::GetDefaultDevice()->GetGraphics();
    vcl::font::PhysicalFontCollection fonts;
    graphics->GetDevFontList( &fonts );
    std::unique_ptr< vcl::font::PhysicalFontFaceCollection > fontInfo( fonts.GetFontFaceCollection());
    vcl::font::PhysicalFontFace* selected = nullptr;

    // Maybe we don't find the perfect match for the font. E.G. we have fonts with the same family name
    // but not same bold or italic etc...
    // In this case we add all the fonts having the family name of the used font:
    //  - we store all these fonts in familyNameFonts during loop
    //  - if we haven't found the perfect match we store all fonts in familyNameFonts
    typedef std::vector<vcl::font::PhysicalFontFace*> FontList;
    FontList familyNameFonts;

    for( int i = 0;
         i < fontInfo->Count();
         ++i )
    {
        vcl::font::PhysicalFontFace* f = fontInfo->Get( i );
        if( f->GetFamilyName() == familyName )
        {
            // Ignore comparing text encodings, at least for now. They cannot be trivially compared
            // (e.g. UCS2 and UTF8 are technically the same characters, just have different encoding,
            // and just having a unicode font doesn't say what glyphs it actually contains).
            // It is possible that it still may be needed to do at least some checks here
            // for some encodings (can one font have more font files for more encodings?).
            if(( family == FAMILY_DONTKNOW || f->GetFamilyType() == family )
                && ( italic == ITALIC_DONTKNOW || f->GetItalic() == italic )
                && ( weight == WEIGHT_DONTKNOW || f->GetWeight() == weight )
                && ( pitch == PITCH_DONTKNOW || f->GetPitch() == pitch ))
            { // Exact match, return it immediately.
                selected = f;
                break;
            }
            if(( f->GetFamilyType() == FAMILY_DONTKNOW || family == FAMILY_DONTKNOW || f->GetFamilyType() == family )
                && ( f->GetItalic() == ITALIC_DONTKNOW || italic == ITALIC_DONTKNOW || f->GetItalic() == italic )
                && ( f->GetWeight() == WEIGHT_DONTKNOW || weight == WEIGHT_DONTKNOW || f->GetWeight() == weight )
                && ( f->GetPitch() == PITCH_DONTKNOW || pitch == PITCH_DONTKNOW || f->GetPitch() == pitch ))
            { // Some fonts specify 'DONTKNOW' for some things, still a good match, if we don't find a better one.
                selected = f;
            }
            // adding "not perfect match" to familyNameFonts vector
            familyNameFonts.push_back(f);

        }
    }

    // if we have found a perfect match we will add only "selected", otherwise all familyNameFonts
    FontList fontsToAdd = (selected ? FontList(1, selected) : std::move(familyNameFonts));

    for (vcl::font::PhysicalFontFace* f : fontsToAdd)
    {
        if (!selected) { // recalculate file name for "not perfect match"
            url = path + getFilenameForExport(familyName, f->GetFamilyType(), f->GetItalic(),
                                              f->GetWeight(), f->GetPitch());
            if (osl::File(url).open(osl_File_OpenFlag_Read) == osl::File::E_None) // = exists()
            {
                // File with contents of the font file already exists, assume it's been created by a previous call.
                continue;
            }
        }
        auto aFontData(f->GetRawFontData(0));
        if (!aFontData.empty())
        {
            auto data = aFontData.data();
            auto size = aFontData.size();
            if( sufficientTTFRights( data, size, rights ))
            {
                osl::File file( url );
                if( file.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create ) == osl::File::E_None )
                {
                    ok = writeFontBytesToFile(file, data, size);
                }
            }
        }
    }
    return ok ? url : u""_ustr;
}

bool EmbeddedFontsManager::isCommonFont(std::u16string_view aFontName)
{
    static constexpr auto aCommonFontsList = frozen::make_unordered_set<std::u16string_view>({
        // LO Common
        u"Liberation Sans",
        u"Liberation Serif",
        u"Liberation Sans Narrow",
        u"Liberation Mono",
        u"Caladea",
        u"Carlito",
        u"OpenSymbol",
        // MSO
        u"Times New Roman",
        u"Arial",
        u"Arial Narrow",
        u"Courier New",
        u"Cambria",
        u"Calibri",
        u"Mangal",
        u"Microsoft YaHei",
        u"NSimSun",
        u"Segoe UI",
        u"SimSun",
        u"Symbol",
        u"Tahoma",
    });

    return aCommonFontsList.contains(aFontName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
