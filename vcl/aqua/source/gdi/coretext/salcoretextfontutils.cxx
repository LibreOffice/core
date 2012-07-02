/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "aqua/common.h"

#include "aqua/coretext/salcoretextfontutils.hxx"
#include "aqua/coretext/salgdi.h"

#include "sft.hxx"
#include "aqua/salinst.h"


static bool GetDevFontAttributes( CTFontDescriptorRef font_descriptor, ImplDevFontAttributes& rDFA  )
{
    int value = 0;

    // reset the attributes
    rDFA.meFamily     = FAMILY_DONTKNOW;
    rDFA.mePitch      = PITCH_VARIABLE;
    rDFA.meWidthType  = WIDTH_NORMAL;
    rDFA.meWeight     = WEIGHT_NORMAL;
    rDFA.meItalic     = ITALIC_NONE;
    rDFA.mbSymbolFlag = false;
    rDFA.mbOrientation = true;
    rDFA.mbDevice      = true;
    rDFA.mnQuality     = 0;

#if MAC_OS_X_VERSION_MIN_REQUIRED < 1060
    CTFontRef font = CTFontCreateWithFontDescriptor(font_descriptor, 0.0, NULL);
    CFDataRef rHeadTable = CTFontCopyTable(font, kCTFontTableHead, kCTFontTableOptionNoOptions);
    CFRelease(font);
    if(!rHeadTable || CFDataGetLength(rHeadTable) == 0)
    {
        SafeCFRelease(rHeadTable);
        return false;
    }
    CFRelease(rHeadTable);
#else
    CFNumberRef format = (CFNumberRef)CTFontDescriptorCopyAttribute(font_descriptor, kCTFontFormatAttribute);
    CFNumberGetValue(format, kCFNumberIntType, &value);
    CFRelease(format);

    if(value == kCTFontFormatBitmap)
    {
        /* we don't want bitmap fonts */
        return false;
    }
#endif
    rDFA.mbSubsettable  = true;
    rDFA.mbEmbeddable   = false;

    CFStringRef family_name = (CFStringRef)CTFontDescriptorCopyAttribute(font_descriptor, kCTFontFamilyNameAttribute);
    rDFA.maName = GetOUString(family_name);
    CFRelease(family_name);

    CFDictionaryRef traits = (CFDictionaryRef)CTFontDescriptorCopyAttribute(font_descriptor, kCTFontTraitsAttribute);
    CFNumberRef symbolics = (CFNumberRef)CFDictionaryGetValue(traits, kCTFontSymbolicTrait);
    CFNumberGetValue(symbolics, kCFNumberIntType, &value);
    CFRelease(symbolics);

    if(value & kCTFontMonoSpaceTrait)
    {
        rDFA.mePitch = PITCH_FIXED;
    }

    if(value & kCTFontItalicTrait)
    {
        rDFA.meItalic = ITALIC_NORMAL;
    }

    if(value & kCTFontBoldTrait)
    {
        rDFA.meWeight = WEIGHT_BOLD;
    }

    if(value & kCTFontCondensedTrait)
    {
        rDFA.meWidthType = WIDTH_CONDENSED;
    }
    else if(value & kCTFontExpandedTrait)
    {
        rDFA.meWidthType = WIDTH_EXPANDED;
    }
    switch(value & kCTFontClassMaskTrait)
    {
    case kCTFontOldStyleSerifsClass:
        rDFA.meFamily = FAMILY_ROMAN;
        break;
    case kCTFontTransitionalSerifsClass:
    case kCTFontModernSerifsClass:
    case kCTFontClarendonSerifsClass:
    case kCTFontSlabSerifsClass:
    case kCTFontFreeformSerifsClass:
        break;
    case kCTFontSansSerifClass:
        rDFA.meFamily = FAMILY_SWISS;
    case kCTFontOrnamentalsClass:
        rDFA.meFamily = FAMILY_DECORATIVE;
        break;
    case kCTFontScriptsClass:
        rDFA.meFamily = FAMILY_SCRIPT;
        break;
    case kCTFontSymbolicClass:
        rDFA.mbSymbolFlag = true;
        break;
    }

    CFNumberRef weight = (CFNumberRef)CFDictionaryGetValue(traits, kCTFontWeightTrait);
    float fdval = 0.0;
    CFNumberGetValue(weight, kCFNumberFloatType, &fdval);
    if(fdval > 0.6)
    {
        rDFA.meWeight = WEIGHT_BLACK;
    }
    else if(fdval > 0.4)
    {
        rDFA.meWeight = WEIGHT_ULTRABOLD;
    }
    else if (fdval > 0.3)
    {
        rDFA.meWeight = WEIGHT_BOLD;
    }
    else if (fdval > 0.0)
    {
        rDFA.meWeight = WEIGHT_SEMIBOLD;
    }
    else if (fdval <= -0.8)
    {
        rDFA.meWeight = WEIGHT_ULTRALIGHT;
    }
    else if (fdval <= -0.4)
    {
        rDFA.meWeight = WEIGHT_LIGHT;
    }
    else if (fdval <= -0.3)
    {
        rDFA.meWeight = WEIGHT_SEMILIGHT;
    }
    else if (fdval <= -0.2)
    {
        rDFA.meWeight = WEIGHT_THIN;
    }
    else
    {
        rDFA.meWeight = WEIGHT_NORMAL;
    }

    CFStringRef string_ref = (CFStringRef)CTFontDescriptorCopyAttribute(font_descriptor, kCTFontStyleNameAttribute);
    rtl::OUString font_name = GetOUString(string_ref);
    rtl::OUString font_name_lc(font_name.toAsciiLowerCase());
    CFRelease(string_ref);

    // heuristics to adjust font slant
    if( (font_name_lc.indexOf("oblique") != -1) ||
        (font_name_lc.indexOf("inclined") != -1) ||
        (font_name_lc.indexOf("slanted") != -1) )
    {
        rDFA.meItalic = ITALIC_OBLIQUE;
    }

    // heuristics to adjust font width
    if (font_name_lc.indexOf("narrow") != -1)
    {
        rDFA.meWidthType = WIDTH_SEMI_CONDENSED;
    }

    // heuristics for font family type
    if( (font_name_lc.indexOf("script") != -1) ||
        (font_name_lc.indexOf("chancery") != -1) ||
        (font_name_lc.indexOf("zapfino") != -1))
    {
        rDFA.meFamily = FAMILY_SCRIPT;
    }
    else if( (font_name_lc.indexOf("comic") != -1) ||
             (font_name_lc.indexOf("outline") != -1) ||
             (font_name_lc.indexOf("pinpoint") != -1) )
    {
        rDFA.meFamily = FAMILY_DECORATIVE;
    }
    else if( (font_name_lc.indexOf("sans") != -1) ||
             (font_name_lc.indexOf("arial") != -1) )
    {
        rDFA.meFamily = FAMILY_SWISS;
    }
    else if( (font_name_lc.indexOf("roman") != -1) ||
             (font_name_lc.indexOf("times") != -1) )
    {
        rDFA.meFamily = FAMILY_ROMAN;
    }
    return true;
}

SystemFontList::SystemFontList()
{
	CTFontCollectionRef font_collection = CTFontCollectionCreateFromAvailableFonts(NULL);
    if(font_collection)
    {
        CFArrayRef font_descriptors = CTFontCollectionCreateMatchingFontDescriptors(font_collection);

        for(int i = 0; i < CFArrayGetCount(font_descriptors); i++)
        {
            CTFontDescriptorRef font_descriptor = (CTFontDescriptorRef)CFArrayGetValueAtIndex(font_descriptors, i);
            CTFontRef font = CTFontCreateWithFontDescriptor(font_descriptor, 0, NULL);
            ImplDevFontAttributes devfont_attr;
            if(GetDevFontAttributes( font_descriptor, devfont_attr ) )
            {
                CoreTextPhysicalFontFace* font_data = new CoreTextPhysicalFontFace(devfont_attr, font);
                if(font_data && font_data->GetCTFont())
                {
                    m_aFontContainer [ font_data->GetCTFont() ] = font_data;
                }
            }
            CFRelease(font);
        }
        CFRelease(font_descriptors);
    }
    CFRelease(font_collection);
}

SystemFontList::~SystemFontList()
{
    CoreTextFontContainer::const_iterator it = m_aFontContainer.begin();
    for(; it != m_aFontContainer.end(); ++it )
        delete (*it).second;
    m_aFontContainer.clear();
}

CoreTextPhysicalFontFace* SystemFontList::GetFontDataFromRef( CTFontRef font ) const
{
    CoreTextFontContainer::const_iterator it = m_aFontContainer.find( font );
    return it == m_aFontContainer.end() ? NULL : (*it).second;
}


void SystemFontList::AnnounceFonts( ImplDevFontList& rFontList ) const
{
    CoreTextFontContainer::const_iterator it = m_aFontContainer.begin();
    for(; it != m_aFontContainer.end(); ++it )
    {
        rFontList.Add( (*it).second->Clone() );
    }
}

CoreTextPhysicalFontFace::CoreTextPhysicalFontFace( const ImplDevFontAttributes& rDFA, CTFontRef font )
:   PhysicalFontFace( rDFA, 0 )
,   m_CTFontRef((CTFontRef)CFRetain(font))
,   m_pCharMap( NULL )
,   m_bHasOs2Table( false )
,   m_bOs2TableRead( false )
,   m_bCmapTableRead( false )
,   m_bHasCJKSupport( false )
,   m_bFontCapabilitiesRead( false )
{
}

CoreTextPhysicalFontFace::~CoreTextPhysicalFontFace()
{
    if( m_pCharMap )
    {
        m_pCharMap->DeReference();
    }
    if( m_CTFontRef )
    {
        CFRelease(m_CTFontRef);
    }
}

PhysicalFontFace* CoreTextPhysicalFontFace::Clone() const
{
    CoreTextPhysicalFontFace* pClone = new CoreTextPhysicalFontFace(*this);
    if( m_pCharMap )
    {
        m_pCharMap->AddReference();
    }
    if( m_CTFontRef )
    {
        pClone->m_CTFontRef = (CTFontRef)CFRetain(m_CTFontRef);
    }
    return pClone;
}

ImplFontEntry* CoreTextPhysicalFontFace::CreateFontInstance(FontSelectPattern& rFSD) const
{
    return new ImplFontEntry(rFSD);
}

const ImplFontCharMap* CoreTextPhysicalFontFace::GetImplFontCharMap()
{
    // return the cached charmap
    if( m_pCharMap )
    {
        return m_pCharMap;
    }
    // set the default charmap
    m_pCharMap = ImplFontCharMap::GetDefaultMap();
    m_pCharMap->AddReference();

    // get the CMAP byte size
    CFDataRef rCmapTable = CTFontCopyTable( m_CTFontRef, kCTFontTableCmap, kCTFontTableOptionNoOptions);
    if(!rCmapTable)
    {
        return m_pCharMap;
    }
    if(!m_bCmapTableRead)
    {
        m_bCmapTableRead = true;
        DetermineCJKSupport_cmap(rCmapTable);
    }
    // parse the CMAP
    CmapResult aCmapResult;
    if(ParseCMAP( CFDataGetBytePtr(rCmapTable), CFDataGetLength(rCmapTable), aCmapResult ) )
    {
        m_pCharMap = new ImplFontCharMap( aCmapResult );
        m_pCharMap->AddReference();
    }
    CFRelease(rCmapTable);
    return m_pCharMap;
}

bool CoreTextPhysicalFontFace::GetImplFontCapabilities(vcl::FontCapabilities &rFontCapabilities)
{
    // read this only once per font
    if( m_bFontCapabilitiesRead )
    {
        rFontCapabilities = m_aFontCapabilities;
        return !rFontCapabilities.maUnicodeRange.empty() || !rFontCapabilities.maCodePageRange.empty();
    }
    m_bFontCapabilitiesRead = true;

    // get the GSUB table raw data
    CFDataRef rGSUBTable = CTFontCopyTable( m_CTFontRef, kCTFontTableGSUB, kCTFontTableOptionNoOptions);
    if(rGSUBTable)
    {

        vcl::getTTScripts(m_aFontCapabilities.maGSUBScriptTags,
                          CFDataGetBytePtr(rGSUBTable), CFDataGetLength(rGSUBTable));
        CFRelease(rGSUBTable);
    }
    CFDataRef OS2_Table = CTFontCopyTable( m_CTFontRef, kCTFontTableOS2, kCTFontTableOptionNoOptions);
    if(OS2_Table)
    {
        vcl::getTTCoverage(
                m_aFontCapabilities.maUnicodeRange,
                m_aFontCapabilities.maCodePageRange,
                CFDataGetBytePtr(OS2_Table), CFDataGetLength(OS2_Table));
        /* while we are at it let's solve HasCJK for the same price */
        if(!m_bOs2TableRead )
        {
            m_bOs2TableRead = true;
            m_bHasOs2Table = true;
            DetermineCJKSupport_OS2(OS2_Table);
        }
        CFRelease(OS2_Table);
    }
    rFontCapabilities = m_aFontCapabilities;
    return !rFontCapabilities.maUnicodeRange.empty() || !rFontCapabilities.maCodePageRange.empty();
}

struct font_table
{
    unsigned char* table;
    unsigned char* dir_entry;
    unsigned char* cursor;
};

void addTable(struct font_table* table, CTFontTableTag tag, CFDataRef data)
{
    if(data && CFDataGetLength(data) > 0)
    {
        *(uint32_t*)table->dir_entry = CFSwapInt32HostToBig(tag);
        table->dir_entry += 4;
        *(uint32_t*)table->dir_entry = 0; /* TODO: checksum */
        table->dir_entry += 4;
        *(uint32_t*)table->dir_entry = CFSwapInt32HostToBig((uint32_t)((uintptr_t)table->cursor - (uintptr_t)table));
        table->dir_entry += 4;
        *(uint32_t*)table->dir_entry = CFSwapInt32HostToBig(CFDataGetLength(data));
        table->dir_entry += 4;

        memcpy(table->cursor, CFDataGetBytePtr(data), CFDataGetLength(data));
        table->cursor += CFDataGetLength(data);
    }
}

bool CoreTextPhysicalFontFace::GetRawFontData( std::vector<unsigned char>& rBuffer, bool* pJustCFF ) const
{
    bool rc;
    int table_count = 0;

    CFDataRef CFF_table = CTFontCopyTable( m_CTFontRef, kCTFontTableCFF, kCTFontTableOptionNoOptions);
    if(pJustCFF)
    {
        if(CFF_table)
        {
            *pJustCFF = CFDataGetLength(CFF_table) ? true : false;
        }
        if(CFF_table)
        {
            CFRelease(CFF_table);
            return true;
        }
        else
        {
            return false;
        }
    }
    size_t total_len = 0;
    CFDataRef head_table = CTFontCopyTable( m_CTFontRef, kCTFontTableHead, kCTFontTableOptionNoOptions);
    CFDataRef maxp_table = CTFontCopyTable( m_CTFontRef, kCTFontTableMaxp, kCTFontTableOptionNoOptions);
    CFDataRef cmap_table = CTFontCopyTable( m_CTFontRef, kCTFontTableHead, kCTFontTableOptionNoOptions);
    CFDataRef name_table = CTFontCopyTable( m_CTFontRef, kCTFontTableName, kCTFontTableOptionNoOptions);
    CFDataRef hhea_table = CTFontCopyTable( m_CTFontRef, kCTFontTableHhea, kCTFontTableOptionNoOptions);
    CFDataRef hmtx_table = CTFontCopyTable( m_CTFontRef, kCTFontTableHmtx, kCTFontTableOptionNoOptions);
    rc = false;
    if(head_table && maxp_table && cmap_table && name_table && hhea_table && hmtx_table)
    {
        if(CFDataGetLength(head_table) &&
           CFDataGetLength(maxp_table) &&
           CFDataGetLength(name_table) &&
           CFDataGetLength(hhea_table) &&
           CFDataGetLength(hmtx_table))
        {
            table_count += 6;
            total_len = CFDataGetLength(head_table) +
                CFDataGetLength(maxp_table) +
                CFDataGetLength(name_table) +
                CFDataGetLength(hhea_table) +
                CFDataGetLength(hmtx_table);
            rc = true;
        }
    }

    CFDataRef loca_table = NULL;
    CFDataRef glyf_table = NULL;
    CFDataRef prep_table = NULL;
    CFDataRef cvt_table = NULL;
    CFDataRef fpgm_table = NULL;
    if(rc)
    {
        if(!CFF_table || CFDataGetLength(CFF_table) == 0)
        {
            loca_table = CTFontCopyTable( m_CTFontRef, kCTFontTableLoca, kCTFontTableOptionNoOptions);
            glyf_table = CTFontCopyTable( m_CTFontRef, kCTFontTableGlyf, kCTFontTableOptionNoOptions);
            if(!loca_table  || !glyf_table || !CFDataGetLength(loca_table) || !CFDataGetLength(glyf_table))
            {
                rc = false;
            }
            else
            {
                table_count += 2;
                total_len += CFDataGetLength(loca_table) + CFDataGetLength(glyf_table);
                prep_table = CTFontCopyTable( m_CTFontRef, kCTFontTablePrep, kCTFontTableOptionNoOptions);
                cvt_table = CTFontCopyTable( m_CTFontRef, kCTFontTableCvt, kCTFontTableOptionNoOptions);
                fpgm_table = CTFontCopyTable( m_CTFontRef, kCTFontTableFpgm, kCTFontTableOptionNoOptions);
                if(prep_table || CFDataGetLength(prep_table) > 0)
                {
                    table_count += 1;
                    total_len += CFDataGetLength(prep_table);
                }
                if(cvt_table || CFDataGetLength(cvt_table) > 0)
                {
                    table_count += 1;
                    total_len += CFDataGetLength(cvt_table);
                }
                if(fpgm_table || CFDataGetLength(fpgm_table) > 0)
                {
                    table_count += 1;
                    total_len += CFDataGetLength(fpgm_table);
                }
            }
        }
        else
        {
            table_count += 1;
            total_len += CFDataGetLength(CFF_table);
        }
    }
    if(rc)
    {
        total_len += 12 + 16 * table_count;
        rBuffer.resize(total_len);
        struct font_table table;
        unsigned char* cursor = &rBuffer[0];
        int nLog2 = 0;

        while( (table_count >> nLog2) > 1 ) ++nLog2;

        table.table = cursor;
        *(uint16_t*)cursor = CFSwapInt16HostToBig(1);
        cursor += 2;
        *(uint16_t*)cursor = 0;
        cursor += 2;
        *(uint16_t*)cursor = CFSwapInt16HostToBig(table_count);
        cursor += 2;
        *(uint16_t*)cursor = CFSwapInt16HostToBig(nLog2 * 16);
        cursor += 2;
        *(uint16_t*)cursor = CFSwapInt16HostToBig(nLog2);
        cursor += 2;
        *(uint16_t*)cursor = CFSwapInt16HostToBig((table_count - nLog2) * 16); // rangeShift
        cursor += 2;
        table.dir_entry = cursor;
        cursor += (16 * table_count);
        table.cursor = cursor;
        addTable(&table, kCTFontTableCmap, cmap_table);
        addTable(&table, kCTFontTableCvt, cvt_table);
        addTable(&table, kCTFontTableFpgm, fpgm_table);
        addTable(&table, kCTFontTableCFF, CFF_table);
        addTable(&table, kCTFontTableGlyf, glyf_table);
        addTable(&table, kCTFontTableLoca, loca_table);
        addTable(&table, kCTFontTableHead, head_table);
        addTable(&table, kCTFontTableHhea, hhea_table);
        addTable(&table, kCTFontTableHmtx, hmtx_table);
        addTable(&table, kCTFontTableMaxp, maxp_table);
        addTable(&table, kCTFontTableName, name_table);
        addTable(&table, kCTFontTablePrep, prep_table);
    }
    SafeCFRelease(cmap_table);
    SafeCFRelease(cvt_table);
    SafeCFRelease(fpgm_table);
    SafeCFRelease(CFF_table);
    SafeCFRelease(glyf_table);
    SafeCFRelease(loca_table);
    SafeCFRelease(head_table);
    SafeCFRelease(hhea_table);
    SafeCFRelease(hmtx_table);
    SafeCFRelease(maxp_table);
    SafeCFRelease(name_table);
    SafeCFRelease(prep_table);

    return rc;
}

void CoreTextPhysicalFontFace::DetermineCJKSupport_OS2(CFDataRef rOS2Table)
{
    if(CFDataGetLength(rOS2Table) >= 48)
    {
        const unsigned short* pOS2buffer = (const unsigned short*)CFDataGetBytePtr(rOS2Table);
        const unsigned short version = CFSwapInt16BigToHost(pOS2buffer[0]);
        if( version >= 1)
        {
            const unsigned short unicode_range = CFSwapInt16BigToHost(pOS2buffer[23]);
            if( unicode_range & 0x2DF0)
            {
                m_bHasCJKSupport = true;
            }
        }
    }
}

void CoreTextPhysicalFontFace::DetermineCJKSupport_cmap(CFDataRef rCmapTable)
{
    int table_len = CFDataGetLength(rCmapTable) / 2;
    if(table_len >= 12)
    {
        const unsigned short* pCmap = (const unsigned short*)CFDataGetBytePtr(rCmapTable);
        if(pCmap[0] == 0)
        {
            short nb_sub_tables = CFSwapInt16BigToHost(pCmap[1]);
            for(int i = 2; --nb_sub_tables >= 0 && i < table_len; i += 4)
            {
                short platform = CFSwapInt16BigToHost(pCmap[i]);
                if( platform == kFontMacintoshPlatform )
                {
                    short encoding = CFSwapInt16BigToHost(pCmap[i+1]);
                    if( encoding == kFontJapaneseScript ||
                        encoding == kFontTraditionalChineseScript ||
                        encoding == kFontKoreanScript ||
                        encoding == kFontSimpleChineseScript )
                    {
                        m_bHasCJKSupport = true;
                        break;
                    }
                }
            }
        }
    }
}

bool CoreTextPhysicalFontFace::HasCJKSupport( void )
{
    // read this only once per font
    if(!m_bOs2TableRead )
    {
        m_bOs2TableRead = true;
        CFDataRef rOS2Table = CTFontCopyTable( m_CTFontRef, kCTFontTableOS2, kCTFontTableOptionNoOptions);
        if(rOS2Table)
        {
            m_bHasOs2Table = true;
            DetermineCJKSupport_OS2(rOS2Table);
            CFRelease(rOS2Table);
        }
    }
    if( !m_bCmapTableRead && !m_bHasOs2Table && !m_bHasCJKSupport )
    {
        m_bCmapTableRead = true;
        CFDataRef rCmapTable = CTFontCopyTable( m_CTFontRef, kCTFontTableCmap, kCTFontTableOptionNoOptions);
        if(rCmapTable)
        {
            DetermineCJKSupport_cmap(rCmapTable);
            CFRelease(rCmapTable);
        }
    }
    return m_bHasCJKSupport;
}
