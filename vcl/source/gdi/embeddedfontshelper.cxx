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
#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/unordered_set.h>
#include <config_folders.h>
#include <config_eot.h>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <vcl/embeddedfontshelper.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <comphelper/storagehelper.hxx>

#include <font/PhysicalFontFaceCollection.hxx>
#include <font/PhysicalFontCollection.hxx>
#include <salgdi.hxx>
#include <sft.hxx>

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
OUString GetEmbeddedFontsRoot()
{
    OUString path = u"${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE( "bootstrap") "::UserInstallation}"_ustr;
    rtl::Bootstrap::expandMacros( path );
    return path + "/user/temp/embeddedfonts/";
}
}

static void clearDir( const OUString& path )
{
    osl::Directory dir( path );
    if( dir.reset() == osl::Directory::E_None )
    {
        for(;;)
        {
            osl::DirectoryItem item;
            if( dir.getNextItem( item ) != osl::Directory::E_None )
                break;
            osl::FileStatus status( osl_FileStatus_Mask_FileURL );
            if( item.getFileStatus( status ) == osl::File::E_None )
                osl::File::remove( status.getFileURL());
        }
    }
}

void EmbeddedFontsHelper::clearTemporaryFontFiles()
{
    OUString path = GetEmbeddedFontsRoot();
    clearDir( path + "fromdocs/" );
    clearDir( path + "fromsystem/" );
}

bool EmbeddedFontsHelper::addEmbeddedFont( const uno::Reference< io::XInputStream >& stream, const OUString& fontName,
    std::u16string_view extra, std::vector< unsigned char > const & key, bool eot,
    bool bSubsetted )
{
    OUString fileUrl = EmbeddedFontsHelper::fileUrlForTemporaryFont( fontName, extra );
    osl::File file( fileUrl );
    switch( file.open( osl_File_OpenFlag_Create | osl_File_OpenFlag_Write ))
    {
        case osl::File::E_None:
            break; // ok
        case osl::File::E_EXIST:
            return true; // Assume it's already been added correctly.
        default:
            SAL_WARN( "vcl.fonts", "Cannot open file for temporary font" );
            return false;
    }
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
        // if eot, don't write the file out yet, since we need to unpack it first.
        if( !eot && read > 0 )
        {
            sal_uInt64 writtenTotal = 0;
            while( writtenTotal < read )
            {
                sal_uInt64 written;
                file.write( buffer.getConstArray(), read, written );
                writtenTotal += written;
            }
        }
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
            osl::File::remove( fileUrl );
            return false;
        }
        sal_uInt64 writtenTotal = 0;
        while( writtenTotal < uncompressedFontSize )
        {
            sal_uInt64 written;
            if( file.write( uncompressedFont.get() + writtenTotal, uncompressedFontSize - writtenTotal, written ) != osl::File::E_None )
            {
                SAL_WARN( "vcl.fonts", "Error writing temporary font file" );
                osl::File::remove( fileUrl );
                return false;
            }
            writtenTotal += written;
        }
        sufficientFontRights = libeot::EOTcanLegallyEdit( &eotMetadata );
        libeot::EOTfreeMetadata( &eotMetadata );
    }
#endif

    if( file.close() != osl::File::E_None )
    {
        SAL_WARN( "vcl.fonts", "Writing temporary font file failed" );
        osl::File::remove( fileUrl );
        return false;
    }
    if( !eot )
    {
        sufficientFontRights = sufficientTTFRights(fontData.data(), fontData.size(), FontRights::EditingAllowed);
    }
    if( !sufficientFontRights )
    {
        // It would be actually better to open the document in read-only mode in this case,
        // warn the user about this, and provide a button to drop the font(s) in order
        // to switch to editing.
        SAL_INFO( "vcl.fonts", "Ignoring embedded font that is not usable for editing" );
        osl::File::remove( fileUrl );
        return false;
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
            osl::File::remove(fileUrl);
            return false;
        }
    }

    m_aAccumulatedFonts.emplace_back(std::make_pair(fontName, fileUrl));
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

void EmbeddedFontsHelper::activateFonts()
{
    if (m_aAccumulatedFonts.empty())
        return;
    UpdateFontsGuard aUpdateFontsGuard;
    OutputDevice *pDevice = Application::GetDefaultDevice();
    for (const auto& rEntry : m_aAccumulatedFonts)
        pDevice->AddTempDevFont(rEntry.second, rEntry.first);
    m_aAccumulatedFonts.clear();
}

OUString EmbeddedFontsHelper::fileUrlForTemporaryFont( const OUString& fontName, std::u16string_view extra )
{
    OUString filename = fontName;
    static int uniqueCounter = 0;
    if( extra == u"?" )
        filename += OUString::number( uniqueCounter++ );
    else
        filename += extra;
    filename += ".ttf"; // TODO is it always ttf?

    if (!::comphelper::OStorageHelper::IsValidZipEntryFileName(filename, false))
    {
        SAL_WARN( "vcl.fonts", "Cannot use filename: " << filename << " for temporary font");
        filename = "font" + OUString::number(uniqueCounter++) + ".ttf";
    }

    OUString path = GetEmbeddedFontsRoot() + "fromdocs/";
    osl::Directory::createPath( path );
    return path + filename;
}

// Check if it's (legally) allowed to embed the font file into a document
// (ttf has a flag allowing this). PhysicalFontFace::IsEmbeddable() appears
// to have a different meaning (guessing from code, IsSubsettable() might
// possibly mean it's ttf, while IsEmbeddable() might mean it's type1).
// So just try to open the data as ttf and see.
bool EmbeddedFontsHelper::sufficientTTFRights( const void* data, tools::Long size, FontRights rights )
{
    TrueTypeFont* font;
    if( OpenTTFontBuffer( data, size, 0 /*TODO*/, &font ) == SFErrCodes::Ok )
    {
        TTGlobalFontInfo info;
        GetTTGlobalFontInfo( font, &info );
        CloseTTFont( font );
        // https://www.microsoft.com/typography/otspec/os2.htm#fst
        int copyright = info.typeFlags;
        switch( rights )
        {
            case FontRights::ViewingAllowed:
                // Embedding not restricted completely.
                return ( copyright & 0x02 ) != 0x02;
            case FontRights::EditingAllowed:
                // Font is installable or editable.
                return copyright == 0 || ( copyright & 0x08 );
        }
    }
    return true; // no known restriction
}

OUString EmbeddedFontsHelper::fontFileUrl( std::u16string_view familyName, FontFamily family, FontItalic italic,
    FontWeight weight, FontPitch pitch, FontRights rights )
{
    OUString path = GetEmbeddedFontsRoot() + "fromsystem/";
    osl::Directory::createPath( path );
    OUString filename = OUString::Concat(familyName) + "_" + OUString::number( family ) + "_" + OUString::number( italic )
        + "_" + OUString::number( weight ) + "_" + OUString::number( pitch )
        + ".ttf"; // TODO is it always ttf?
    OUString url = path + filename;
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
        if (!selected) { // recalculate file not for "not perfect match"
            filename = OUString::Concat(familyName) + "_" + OUString::number(f->GetFamilyType()) + "_" +
                OUString::number(f->GetItalic()) + "_" + OUString::number(f->GetWeight()) + "_" +
                OUString::number(f->GetPitch()) + ".ttf"; // TODO is it always ttf?
            url = path + filename;
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
                    sal_uInt64 written = 0;
                    sal_uInt64 totalSize = size;
                    bool error = false;
                    while( written < totalSize && !error)
                    {
                        sal_uInt64 nowWritten;
                        switch( file.write( data + written, size - written, nowWritten ))
                        {
                            case osl::File::E_None:
                                written += nowWritten;
                                break;
                            case osl::File::E_AGAIN:
                            case osl::File::E_INTR:
                                break;
                            default:
                                error = true;
                                break;
                        }
                    }
                    file.close();
                    if( error )
                        osl::File::remove( url );
                    else
                        ok = true;
                }
            }
        }
    }
    return ok ? url : u""_ustr;
}

bool EmbeddedFontsHelper::isCommonFont(std::u16string_view aFontName)
{
    static constexpr auto aCommonFontsList = frozen::make_unordered_set<std::u16string_view>({
        // LO Common
        u"Liberation Sans",
        u"Liberation Serif",
        u"Liberation Sans Narrow",
        u"Liberation Mono",
        u"Caladea",
        u"Carlito",
        // MSO
        u"Times New Roman",
        u"Arial",
        u"Arial Narrow",
        u"Courier New",
        u"Cambria",
        u"Calibri",
    });

    return aCommonFontsList.contains(aFontName);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
