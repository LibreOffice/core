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

#include "typedetection.hxx"
#include "constant.hxx"

#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <tools/wldcrd.hxx>
#include <rtl/ustrbuf.hxx>
#include <framework/interaction.hxx>
#include <tools/urlobj.hxx>
#include <comphelper/fileurl.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>

#define DEBUG_TYPE_DETECTION 0

#if DEBUG_TYPE_DETECTION
#include <iostream>
using std::cout;
using std::endl;
#endif

using namespace com::sun::star;

namespace filter{
    namespace config{

TypeDetection::TypeDetection(const css::uno::Reference< css::uno::XComponentContext >& rxContext)
   : m_xContext(rxContext)
{
    BaseContainer::init(rxContext                                     ,
                        TypeDetection::impl_getImplementationName()   ,
                        TypeDetection::impl_getSupportedServiceNames(),
                        FilterCache::E_TYPE                           );
}


TypeDetection::~TypeDetection()
{
}


OUString SAL_CALL TypeDetection::queryTypeByURL(const OUString& sURL)
    throw (css::uno::RuntimeException, std::exception)
{
    OUString sType;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    css::util::URL  aURL;
    aURL.Complete = sURL;
    css::uno::Reference< css::util::XURLTransformer > xParser( css::util::URLTransformer::create(m_xContext) );
    xParser->parseStrict(aURL);

    // set std types as minimum requirement first!
    // Only in case no type was found for given URL,
    // use optional types too ...
    FlatDetection lFlatTypes;
    m_rCache->detectFlatForURL(aURL, lFlatTypes);

    if (
        (lFlatTypes.size() < 1                                ) &&
        (!m_rCache->isFillState(FilterCache::E_CONTAINS_TYPES))
       )
    {
        m_rCache->load(FilterCache::E_CONTAINS_TYPES);
        m_rCache->detectFlatForURL(aURL, lFlatTypes);
    }

    // first item is guaranteed as "preferred" one!
    if (lFlatTypes.size() > 0)
    {
        const FlatDetectionInfo& aMatch = *(lFlatTypes.begin());
        sType = aMatch.sType;
    }

    return sType;
    // <- SAFE
}

namespace {

/**
 * Rank format types in order of complexity.  More complex formats are
 * ranked higher so that they get tested sooner over simpler formats.
 *
 * Guidelines to determine how complex a format is (subject to change):
 *
 * 1) compressed text (XML, HTML, etc)
 * 2) binary
 * 3) non-compressed text
 *   3.1) structured text
 *     3.1.1) dialect of a structured text (e.g. docbook XML)
 *     3.1.2) generic structured text (e.g. generic XML)
 *   3.2) non-structured text
 *
 * In each category, rank them from strictly-structured to
 * loosely-structured.
 */
int getFlatTypeRank(const OUString& rType)
{
    // List formats from more complex to less complex.
    // TODO: Add more.
    static const char* ranks[] = {

        // Compressed XML (ODF XML zip formats)
        "writer8_template",
        "writer8",
        "calc8_template",
        "calc8",
        "impress8_template",
        "impress8",
        "draw8_template",
        "draw8",
        "chart8",
        "math8",
        "writerglobal8_template",
        "writerglobal8",
        "writerweb8_writer_template",
        "StarBase",

        // Compressed XML (OOXML)
        "writer_OOXML_Text_Template",
        "writer_OOXML",
        "writer_MS_Word_2007_Template",
        "writer_MS_Word_2007",
        "Office Open XML Spreadsheet Template",
        "Office Open XML Spreadsheet",
        "MS Excel 2007 XML Template",
        "MS Excel 2007 XML",
        "MS PowerPoint 2007 XML Template",
        "MS PowerPoint 2007 XML AutoPlay",
        "MS PowerPoint 2007 XML",

        // Compressed XML (Uniform/Unified Office Format)
        "Unified_Office_Format_text",
        "Unified_Office_Format_spreadsheet",
        "Unified_Office_Format_presentation",

        // Compressed XML (StarOffice XML zip formats)
        "calc_StarOffice_XML_Calc",
        "calc_StarOffice_XML_Calc_Template",
        "chart_StarOffice_XML_Chart",
        "draw_StarOffice_XML_Draw",
        "draw_StarOffice_XML_Draw_Template",
        "impress_StarOffice_XML_Impress",
        "impress_StarOffice_XML_Impress_Template",
        "math_StarOffice_XML_Math",
        "writer_StarOffice_XML_Writer",
        "writer_StarOffice_XML_Writer_Template",
        "writer_globaldocument_StarOffice_XML_Writer_GlobalDocument",
        "writer_web_StarOffice_XML_Writer_Web_Template",

        // Compressed text
        "pdf_Portable_Document_Format",

        // Binary
        "writer_T602_Document",
        "writer_WordPerfect_Document",
        "writer_MS_Works_Document",
        "writer_MS_Word_97_Vorlage",
        "writer_MS_Word_97",
        "writer_MS_Word_95_Vorlage",
        "writer_MS_Word_95",
        "writer_MS_WinWord_60",
        "writer_MS_WinWord_5",
        "MS Excel 2007 Binary",
        "calc_MS_Excel_97_VorlageTemplate",
        "calc_MS_Excel_97",
        "calc_MS_Excel_95_VorlageTemplate",
        "calc_MS_Excel_95",
        "calc_MS_Excel_5095_VorlageTemplate",
        "calc_MS_Excel_5095",
        "calc_MS_Excel_40_VorlageTemplate",
        "calc_MS_Excel_40",
        "calc_Pocket_Excel_File",
        "impress_MS_PowerPoint_97_Vorlage",
        "impress_MS_PowerPoint_97_AutoPlay",
        "impress_MS_PowerPoint_97",
        "calc_Lotus",
        "calc_QPro",
        "calc_SYLK",
        "calc_DIF",
        "calc_dBase",

        // Binary (raster and vector image files)
        "emf_MS_Windows_Metafile",
        "wmf_MS_Windows_Metafile",
        "met_OS2_Metafile",
        "svm_StarView_Metafile",
        "sgv_StarDraw_20",
        "tif_Tag_Image_File",
        "tga_Truevision_TARGA",
        "sgf_StarOffice_Writer_SGF",
        "ras_Sun_Rasterfile",
        "psd_Adobe_Photoshop",
        "png_Portable_Network_Graphic",
        "jpg_JPEG",
        "mov_MOV",
        "gif_Graphics_Interchange",
        "bmp_MS_Windows",
        "pcx_Zsoft_Paintbrush",
        "pct_Mac_Pict",
        "pcd_Photo_CD_Base",
        "pcd_Photo_CD_Base4",
        "pcd_Photo_CD_Base16",
        "impress_CGM_Computer_Graphics_Metafile", // There is binary and ascii variants ?
        "draw_WordPerfect_Graphics",
        "draw_Visio_Document",
        "draw_Publisher_Document",
        "draw_Corel_Presentation_Exchange",
        "draw_CorelDraw_Document",
        "writer_LotusWordPro_Document",
        "writer_MIZI_Hwp_97", // Hanword (Hancom Office)

        // Non-compressed XML
        "writer_ODT_FlatXML",
        "calc_ODS_FlatXML",
        "impress_ODP_FlatXML",
        "draw_ODG_FlatXML",
        "calc_MS_Excel_2003_XML",
        "writer_MS_Word_2003_XML",
        "writer_DocBook_File",
        "XHTML_File",
        "svg_Scalable_Vector_Graphics",
        "math_MathML_XML_Math",

        // Non-compressed text
        "dxf_AutoCAD_Interchange",
        "eps_Encapsulated_PostScript",
        "pbm_Portable_Bitmap",   // There is 'raw' and 'ascii' variants.
        "ppm_Portable_Pixelmap", // There is 'raw' and 'ascii' variants.
        "pgm_Portable_Graymap",  // There is 'raw' and 'ascii' variants.
        "xpm_XPM",
        "xbm_X_Consortium",
        "writer_Rich_Text_Format",
        "writer_web_HTML_help",
        "generic_HTML",

        "generic_Text", // Plain text (catch all)

        // Anything ranked lower than generic_Text will never be used during
        // type detection (since generic_Text catches all).

        // Export only
        "writer_layout_dump_xml",
        "pwp_PlaceWare",
        "graphic_SWF",
        "graphic_HTML",

        // Internal use only
        "StarBaseReportChart",
        "StarBaseReport",
        "math_MathType_3x", // MathType equation embedded in Word doc.
    };

    size_t n = SAL_N_ELEMENTS(ranks);

    for (size_t i = 0; i < n; ++i)
    {
        if (rType.equalsAscii(ranks[i]))
            return n - i - 1;
    }

    // Not ranked.  Treat them equally.  Unranked formats have higher priority
    // than the ranked internal ones since they may be defined externally.
    return n;
}

/**
 * Types with matching pattern first, then extension, then custom ranks by
 * types, then types that are supported by the document service come next.
 * Lastly, sort them alphabetically.
 */
struct SortByPriority : public std::binary_function<FlatDetectionInfo, FlatDetectionInfo, bool>
{
    bool operator() (const FlatDetectionInfo& r1, const FlatDetectionInfo& r2) const
    {
        if (r1.bMatchByPattern != r2.bMatchByPattern)
            return r1.bMatchByPattern;

        if (r1.bMatchByExtension != r2.bMatchByExtension)
            return r1.bMatchByExtension;

        int rank1 = getFlatTypeRank(r1.sType);
        int rank2 = getFlatTypeRank(r2.sType);

        if (rank1 != rank2)
            return rank1 > rank2;

        if (r1.bPreselectedByDocumentService != r2.bPreselectedByDocumentService)
            return r1.bPreselectedByDocumentService;

        // All things being equal, sort them alphabetically.
        return r1.sType > r2.sType;
    }
};

struct SortByType : public std::binary_function<FlatDetectionInfo, FlatDetectionInfo, bool>

{
    bool operator() (const FlatDetectionInfo& r1, const FlatDetectionInfo& r2) const
    {
        return r1.sType > r2.sType;
    }
};

struct EqualByType : public std::binary_function<FlatDetectionInfo, FlatDetectionInfo, bool>
{
    bool operator() (const FlatDetectionInfo& r1, const FlatDetectionInfo& r2) const
    {
        return r1.sType == r2.sType;
    }
};

class FindByType : public std::unary_function<FlatDetectionInfo, bool>
{
    OUString maType;
public:
    explicit FindByType(const OUString& rType) : maType(rType) {}
    bool operator() (const FlatDetectionInfo& rInfo) const
    {
        return rInfo.sType == maType;
    }
};

#if DEBUG_TYPE_DETECTION
void printFlatDetectionList(const char* caption, const FlatDetection& types)
{
    cout << "-- " << caption << " (size=" << types.size() << ")" << endl;
    FlatDetection::const_iterator it = types.begin(), itEnd = types.end();
    for (; it != itEnd; ++it)
    {
        const FlatDetectionInfo& item = *it;
        cout << "  type='" << item.sType << "'; match by extension (" << item.bMatchByExtension
            << "); match by pattern (" << item.bMatchByPattern << "); pre-selected by doc service ("
            << item.bPreselectedByDocumentService << ")" << endl;
    }
    cout << "--" << endl;
}
#endif

}

OUString SAL_CALL TypeDetection::queryTypeByDescriptor(css::uno::Sequence< css::beans::PropertyValue >& lDescriptor,
                                                              sal_Bool                                         bAllowDeep )
    throw (css::uno::RuntimeException, std::exception)
{
    // make the descriptor more useable :-)
    utl::MediaDescriptor stlDescriptor(lDescriptor);
    OUString sType, sURL;

    try
    {
        // SAFE -> ----------------------------------
        ::osl::ResettableMutexGuard aLock(m_aLock);

        // parse given URL to split it into e.g. main and jump marks ...
        sURL = stlDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_URL(), OUString());

#if OSL_DEBUG_LEVEL > 0
        if (stlDescriptor.find( "FileName" ) != stlDescriptor.end())
            OSL_FAIL("Detect using of deprecated and already unsupported MediaDescriptor property \"FileName\"!");
#endif

        css::util::URL  aURL;
        aURL.Complete = sURL;
        css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(m_xContext));
        xParser->parseStrict(aURL);

        OUString aSelectedFilter = stlDescriptor.getUnpackedValueOrDefault(
            utl::MediaDescriptor::PROP_FILTERNAME(), OUString());
        if (!aSelectedFilter.isEmpty())
        {
            // Caller specified the filter type.  Honor it.  Just get the default
            // type for that filter, and bail out.
            if (impl_validateAndSetFilterOnDescriptor(stlDescriptor, aSelectedFilter))
                return stlDescriptor[utl::MediaDescriptor::PROP_TYPENAME()].get<OUString>();
        }

        FlatDetection lFlatTypes;
        impl_getAllFormatTypes(aURL, stlDescriptor, lFlatTypes);

        aLock.clear();
        // <- SAFE ----------------------------------

        // Properly prioritize all candidate types.
        lFlatTypes.sort(SortByPriority());
        lFlatTypes.unique(EqualByType());

        OUString sLastChance;

        // verify every flat detected (or preselected!) type
        // by calling its registered deep detection service.
        // But break this loop if a type match to the given descriptor
        // by an URL pattern(!) or if deep detection isn't allowed from
        // outside (bAllowDeep=sal_False) or break the whole detection by
        // throwing an exception if creation of the might needed input
        // stream failed by e.g. an IO exception ...
        OUStringList lUsedDetectors;
        if (lFlatTypes.size()>0)
            sType = impl_detectTypeFlatAndDeep(stlDescriptor, lFlatTypes, bAllowDeep, lUsedDetectors, sLastChance);


        // flat detection failed
        // pure deep detection failed
        // => ask might existing InteractionHandler
        // means: ask user for its decision
        if (sType.isEmpty())
            sType = impl_askUserForTypeAndFilterIfAllowed(stlDescriptor);


        // no real detected type - but a might valid one.
        // update descriptor and set last chance for return.
        if (sType.isEmpty() && !sLastChance.isEmpty())
        {
            OSL_FAIL("set first flat detected type without a registered deep detection service as \"last chance\" ... nevertheless some other deep detections said \"NO\". I TRY IT!");
            sType = sLastChance;
        }
    }
    catch(const css::uno::RuntimeException&)
    {
        throw;
    }
    catch(const css::uno::Exception& e)
    {
        SAL_WARN(
            "filter.config",
            "caught Exception \"" << e.Message
                << "\" while querying type of <" << sURL << ">");
        sType.clear();
    }

    // adapt media descriptor, so it contains the right values
    // for type/filter name/document service/ etcpp.
    impl_checkResultsAndAddBestFilter(stlDescriptor, sType); // Attention: sType is used as IN/OUT param here and will might be changed inside this method !!!
    impl_validateAndSetTypeOnDescriptor(stlDescriptor, sType);

    stlDescriptor >> lDescriptor;
    return sType;
}


void TypeDetection::impl_checkResultsAndAddBestFilter(utl::MediaDescriptor& rDescriptor,
                                                      OUString&               sType      )
{
    // a)
    // Don't overwrite a might preselected filter!
    OUString sFilter = rDescriptor.getUnpackedValueOrDefault(
                                utl::MediaDescriptor::PROP_FILTERNAME(),
                                OUString());
    if (!sFilter.isEmpty())
        return;

    // b)
    // check a preselected document service too.
    // Then we have to search a suitable filter within this module.
    OUString sDocumentService = rDescriptor.getUnpackedValueOrDefault(
                                            utl::MediaDescriptor::PROP_DOCUMENTSERVICE(),
                                            OUString());
    if (!sDocumentService.isEmpty())
    {
        try
        {
            OUString sRealType = sType;

            // SAFE ->
            ::osl::ResettableMutexGuard aLock(m_aLock);

            // Attention: For executing next lines of code, We must be sure that
            // all filters already loaded :-(
            // That can disturb our "load on demand feature". But we have no other chance!
            m_rCache->load(FilterCache::E_CONTAINS_FILTERS);

            CacheItem lIProps;
            lIProps[PROPNAME_DOCUMENTSERVICE] <<= sDocumentService;
            lIProps[PROPNAME_TYPE           ] <<= sRealType;
            OUStringList lFilters = m_rCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps);

            aLock.clear();
            // <- SAFE

            for (OUStringList::const_iterator pIt  = lFilters.begin();
                  pIt != lFilters.end(); ++pIt)
            {
                // SAFE ->
                aLock.reset();
                try
                {
                    CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, *pIt);
                    sal_Int32 nFlags  = 0;
                    aFilter[PROPNAME_FLAGS] >>= nFlags;

                    if (static_cast<SfxFilterFlags>(nFlags) & SfxFilterFlags::IMPORT)
                        sFilter = *pIt;
                    if (static_cast<SfxFilterFlags>(nFlags) & SfxFilterFlags::PREFERED)
                        break;
                }
                catch(const css::uno::Exception&) {}
                aLock.clear();
                // <- SAFE
            }

            if (!sFilter.isEmpty())
            {
                rDescriptor[utl::MediaDescriptor::PROP_TYPENAME()  ] <<= sRealType;
                rDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
                sType = sRealType;
                return;
            }
        }
        catch(const css::uno::Exception&)
            {}
    }

    // c)
    // We can use the preferred filter for the specified type.
    // Such preferred filter points:
    // - to the default filter of the preferred application
    // - or to any other filter if no preferred filter was set.
    // Note: It's an optimization only!
    // It's not guaranteed, that such preferred filter exists.
    sFilter.clear();
    try
    {
        // SAFE ->
        ::osl::ResettableMutexGuard aLock(m_aLock);

        CacheItem aType = m_rCache->getItem(FilterCache::E_TYPE, sType);
        aType[PROPNAME_PREFERREDFILTER] >>= sFilter;
        CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, sFilter);

        aLock.clear();
        // <- SAFE

        // no exception => found valid type and filter => set it on the given descriptor
        rDescriptor[utl::MediaDescriptor::PROP_TYPENAME()  ] <<= sType  ;
        rDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
        return;
    }
    catch(const css::uno::Exception&)
        {}

    // d)
    // Search for any import(!) filter, which is registered for this type.
    sFilter.clear();
    try
    {
        // SAFE ->
        ::osl::ResettableMutexGuard aLock(m_aLock);

        // Attention: For executing next lines of code, We must be sure that
        // all filters already loaded :-(
        // That can disturb our "load on demand feature". But we have no other chance!
        m_rCache->load(FilterCache::E_CONTAINS_FILTERS);

        CacheItem lIProps;
        lIProps[PROPNAME_TYPE] <<= sType;
        OUStringList lFilters = m_rCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps);

        aLock.clear();
        // <- SAFE

        OUStringList::const_iterator pIt;
        for (  pIt  = lFilters.begin();
               pIt != lFilters.end()  ;
             ++pIt                    )
        {
            sFilter = *pIt;

            // SAFE ->
            aLock.reset();
            try
            {
                CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, sFilter);
                sal_Int32 nFlags  = 0;
                aFilter[PROPNAME_FLAGS] >>= nFlags;

                if (static_cast<SfxFilterFlags>(nFlags) & SfxFilterFlags::IMPORT)
                    break;
            }
            catch(const css::uno::Exception&)
                { continue; }
            aLock.clear();
            // <- SAFE

            sFilter.clear();
        }

        if (!sFilter.isEmpty())
        {
            rDescriptor[utl::MediaDescriptor::PROP_TYPENAME()  ] <<= sType  ;
            rDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
            return;
        }
    }
    catch(const css::uno::Exception&)
        {}
}


bool TypeDetection::impl_getPreselectionForType(
    const OUString& sPreSelType, const util::URL& aParsedURL, FlatDetection& rFlatTypes, bool bDocService)
{
    // Can be used to suppress execution of some parts of this method
    // if its already clear that detected type is valid or not.
    // Its necessary to use shared code at the end, which update
    // all return parameters constistency!
    bool bBreakDetection = false;

    // Further we must know if it matches by pattern
    // Every flat detected type by pattern won't be detected deep!
    bool bMatchByPattern = false;

    // And we must know if a preselection must be preferred, because
    // it matches by its extension too.
    bool bMatchByExtension = false;

    // validate type
    OUString sType(sPreSelType);
    CacheItem       aType;
    try
    {
        // SAFE -> --------------------------
        ::osl::ResettableMutexGuard aLock(m_aLock);
        aType = m_rCache->getItem(FilterCache::E_TYPE, sType);
        aLock.clear();
        // <- SAFE --------------------------
    }
    catch(const css::container::NoSuchElementException&)
    {
        sType.clear();
        bBreakDetection = true;
    }

    if (!bBreakDetection)
    {
        // We can't check a preselected type for a given stream!
        // So we must believe, that it can work ...
        if ( aParsedURL.Complete == "private:stream" )
            bBreakDetection = true;
    }

    if (!bBreakDetection)
    {
        // extract extension from URL .. to check it case-insensitive !
        INetURLObject   aParser    (aParsedURL.Main);
        OUString sExtension = aParser.getExtension(INetURLObject::LAST_SEGMENT       ,
                                                          true                          ,
                                                          INetURLObject::DECODE_WITH_CHARSET);
        sExtension = sExtension.toAsciiLowerCase();

        // otherwise we must know, if it matches to the given URL really.
        // especially if it matches by its extension or pattern registration.
        OUStringList lExtensions(comphelper::sequenceToContainer<OUStringList>(aType[PROPNAME_EXTENSIONS].get<css::uno::Sequence<OUString> >() ));
        OUStringList lURLPattern(comphelper::sequenceToContainer<OUStringList>(aType[PROPNAME_URLPATTERN].get<css::uno::Sequence<OUString> >() ));

        for (OUStringList::const_iterator pIt  = lExtensions.begin();
                                          pIt != lExtensions.end()  ;
                                        ++pIt                       )
        {
            OUString sCheckExtension(pIt->toAsciiLowerCase());
            if (sCheckExtension.equals(sExtension))
            {
                bBreakDetection        = true;
                bMatchByExtension      = true;
                break;
            }
        }

        if (!bBreakDetection)
        {
            for (OUStringList::const_iterator pIt  = lURLPattern.begin();
                                              pIt != lURLPattern.end()  ;
                                            ++pIt                       )
            {
                WildCard aCheck(*pIt);
                if (aCheck.Matches(aParsedURL.Main))
                {
                    bMatchByPattern        = true;
                    break;
                }
            }
        }
    }

    // if it's a valid type - set it on all return values!
    if (!sType.isEmpty())
    {
        FlatDetection::iterator it = std::find_if(rFlatTypes.begin(), rFlatTypes.end(), FindByType(sType));
        if (it != rFlatTypes.end())
        {
            if (bMatchByExtension)
                it->bMatchByExtension = true;
            if (bMatchByPattern)
                it->bMatchByPattern = true;
            if (bDocService)
                it->bPreselectedByDocumentService = true;
        }

        return true;
    }

    // not valid!
    return false;
}

bool TypeDetection::impl_getPreselectionForDocumentService(
    const OUString& sPreSelDocumentService, const util::URL& aParsedURL, FlatDetection& rFlatTypes)
{
    // get all filters, which match to this doc service
    OUStringList lFilters;
    try
    {
        // SAFE -> --------------------------
        ::osl::ResettableMutexGuard aLock(m_aLock);

        // Attention: For executing next lines of code, We must be sure that
        // all filters already loaded :-(
        // That can disturb our "load on demand feature". But we have no other chance!
        m_rCache->load(FilterCache::E_CONTAINS_FILTERS);

        CacheItem lIProps;
        lIProps[PROPNAME_DOCUMENTSERVICE] <<= sPreSelDocumentService;
        lFilters = m_rCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps);

        aLock.clear();
        // <- SAFE --------------------------
    }
    catch (const css::container::NoSuchElementException&)
    {
        lFilters.clear();
    }

    // step over all filters, and check if its registered type
    // match the given URL.
    // But use temp. list of "preselected types" instead of incoming rFlatTypes list!
    // The reason behind: we must filter the obtained results. And copying stl entries
    // is an easier job than removing them .-)
    for (OUStringList::const_iterator pFilter  = lFilters.begin();
         pFilter != lFilters.end();
         ++pFilter)
    {
        OUString aType = impl_getTypeFromFilter(*pFilter);
        if (aType.isEmpty())
            continue;

        impl_getPreselectionForType(aType, aParsedURL, rFlatTypes, true);
    }

    return true;
}

OUString TypeDetection::impl_getTypeFromFilter(const OUString& rFilterName)
{
    CacheItem aFilter;
    try
    {
        osl::MutexGuard aLock(m_aLock);
        aFilter = m_rCache->getItem(FilterCache::E_FILTER, rFilterName);
    }
    catch (const container::NoSuchElementException&)
    {
        return OUString();
    }

    OUString aType;
    aFilter[PROPNAME_TYPE] >>= aType;
    return aType;
}

void TypeDetection::impl_getAllFormatTypes(
    const util::URL& aParsedURL, utl::MediaDescriptor& rDescriptor, FlatDetection& rFlatTypes)
{
    rFlatTypes.clear();

    // Get all filters that we have.
    OUStringList aFilterNames;
    try
    {
        osl::MutexGuard aLock(m_aLock);
        m_rCache->load(FilterCache::E_CONTAINS_FILTERS);
        aFilterNames = m_rCache->getItemNames(FilterCache::E_FILTER);
    }
    catch (const container::NoSuchElementException&)
    {
        return;
    }

    // Retrieve the default type for each of these filters, and store them.
    for (OUStringList::const_iterator it = aFilterNames.begin(); it != aFilterNames.end(); ++it)
    {
        OUString aType = impl_getTypeFromFilter(*it);

        if (aType.isEmpty())
            continue;

        FlatDetectionInfo aInfo; // all flags set to false by default.
        aInfo.sType = aType;
        rFlatTypes.push_back(aInfo);
    }

    {
        // Get all types that match the URL alone.
        FlatDetection aFlatByURL;
        m_rCache->detectFlatForURL(aParsedURL, aFlatByURL);
        FlatDetection::const_iterator it = aFlatByURL.begin(), itEnd = aFlatByURL.end();
        for (; it != itEnd; ++it)
        {
            FlatDetection::iterator itPos = std::find_if(rFlatTypes.begin(), rFlatTypes.end(), FindByType(it->sType));
            if (itPos == rFlatTypes.end())
                // Not in the list yet.
                rFlatTypes.push_back(*it);
            else
            {
                // Already in the list. Update the flags.
                FlatDetectionInfo& rInfo = *itPos;
                const FlatDetectionInfo& rThisInfo = *it;
                if (rThisInfo.bMatchByExtension)
                    rInfo.bMatchByExtension = true;
                if (rThisInfo.bMatchByPattern)
                    rInfo.bMatchByPattern = true;
                if (rThisInfo.bPreselectedByDocumentService)
                    rInfo.bPreselectedByDocumentService = true;
            }
        }
    }

    // Remove duplicates.
    rFlatTypes.sort(SortByType());
    rFlatTypes.unique(EqualByType());

    // Mark pre-selected type (if any) to have it prioritized.
    OUString sSelectedType = rDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_TYPENAME(), OUString());
    if (!sSelectedType.isEmpty())
        impl_getPreselectionForType(sSelectedType, aParsedURL, rFlatTypes, false);

    // Mark all types preferred by the current document service, to have it prioritized.
    OUString sSelectedDoc = rDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_DOCUMENTSERVICE(), OUString());
    if (!sSelectedDoc.isEmpty())
        impl_getPreselectionForDocumentService(sSelectedDoc, aParsedURL, rFlatTypes);
}


OUString TypeDetection::impl_detectTypeFlatAndDeep(      utl::MediaDescriptor& rDescriptor   ,
                                                          const FlatDetection&                 lFlatTypes    ,
                                                                bool                       bAllowDeep    ,
                                                                OUStringList&                  rUsedDetectors,
                                                                OUString&               rLastChance   )
{
    // reset it everytimes, so the outside code can distinguish between
    // a set and a not set value.
    rLastChance.clear();
    rUsedDetectors.clear();

    // step over all possible types for this URL.
    // solutions:
    // a) no types                                => no detection
    // b) deep detection not allowed              => return first valid type of list (because its the preferred or the first valid one)
    //    or(!) match by URLPattern               => in such case a deep detection will be suppressed!
    // c) type has no detect service              => safe the first occurred type without a detect service
    //                                               as "last chance"(!). It will be used outside of this method
    //                                               if no further type could be detected.
    //                                               It must be the first one, because it can be a preferred type.
    //                                               Our types list was sorted by such criteria!
    // d) detect service return a valid result    => return its decision
    // e) detect service return an invalid result
    //    or any needed information could not be
    //    obtained from the cache                 => ignore it, and continue with search

    for (FlatDetection::const_iterator pFlatIt  = lFlatTypes.begin();
                                       pFlatIt != lFlatTypes.end()  ;
                                     ++pFlatIt                      )
    {
        const FlatDetectionInfo& aFlatTypeInfo = *pFlatIt;
        OUString sFlatType = aFlatTypeInfo.sType;

        if (!impl_validateAndSetTypeOnDescriptor(rDescriptor, sFlatType))
            continue;

        // b)
        if (
            (!bAllowDeep                  ) ||
            (aFlatTypeInfo.bMatchByPattern)
           )
        {
            return sFlatType;
        }

        try
        {
            // SAFE -> ----------------------------------
            ::osl::ResettableMutexGuard aLock(m_aLock);
            CacheItem aType = m_rCache->getItem(FilterCache::E_TYPE, sFlatType);
            aLock.clear();

            OUString sDetectService;
            aType[PROPNAME_DETECTSERVICE] >>= sDetectService;

            // c)
            if (sDetectService.isEmpty())
            {
                // flat detected types without any registered deep detection service and not
                // preselected by the user can be used as LAST CHANCE in case no other type could
                // be detected. Of course only the first type without deep detector can be used.
                // Further ones has to be ignored.
                if (rLastChance.isEmpty())
                    rLastChance = sFlatType;

                continue;
            }

            // don't forget to add every real asked deep detection service here.
            // Such detectors will be ignored if may be "impl_detectTypeDeepOnly()"
            // must be called later!
            rUsedDetectors.push_back(sDetectService);
            OUString sDeepType = impl_askDetectService(sDetectService, rDescriptor);

            // d)
            if (!sDeepType.isEmpty())
                return sDeepType;
        }
        catch(const css::container::NoSuchElementException&)
            {}
        // e)
    }

    return OUString();
    // <- SAFE ----------------------------------
}

void TypeDetection::impl_seekStreamToZero(utl::MediaDescriptor& rDescriptor)
{
    // try to seek to 0 ...
    // But because XSeekable is an optional interface ... try it only .-)
    css::uno::Reference< css::io::XInputStream > xStream = rDescriptor.getUnpackedValueOrDefault(
                                                            utl::MediaDescriptor::PROP_INPUTSTREAM(),
                                                            css::uno::Reference< css::io::XInputStream >());
    css::uno::Reference< css::io::XSeekable > xSeek(xStream, css::uno::UNO_QUERY);
    if (xSeek.is())
    {
        try
        {
            xSeek->seek(0);
        }
        catch(const css::uno::RuntimeException&)
        {
            throw;
        }
        catch(const css::uno::Exception&)
        {
        }
    }
}

OUString TypeDetection::impl_askDetectService(const OUString&               sDetectService,
                                                           utl::MediaDescriptor& rDescriptor   )
{
    // Open the stream and add it to the media descriptor if this method is called for the first time.
    // All following requests to this method will detect, that there already exists a stream .-)
    // Attention: This method throws an exception if the stream could not be opened.
    // It's important to break any further detection in such case.
    // Catch it on the highest detection level only !!!
    impl_openStream(rDescriptor);

    // seek to 0 is an optional feature to be more robust against
    // "simple implemented detect services" .-)
    impl_seekStreamToZero(rDescriptor);

    css::uno::Reference< css::document::XExtendedFilterDetection > xDetector;
    css::uno::Reference< css::uno::XComponentContext >         xContext;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    xContext = m_xContext;
    aLock.clear();
    // <- SAFE

    try
    {
        // Attention! If e.g. an office module was not installed sometimes we
        // find a registered detect service, which is referred inside the
        // configuration ... but not really installed. On the other side we use
        // third party components here, which can make trouble anyway.  So we
        // should handle errors during creation of such services more
        // gracefully .-)
        xDetector.set(
                xContext->getServiceManager()->createInstanceWithContext(sDetectService, xContext),
                css::uno::UNO_QUERY_THROW);
    }
    catch (...)
    {
    }

    if ( ! xDetector.is())
        return OUString();

    OUString sDeepType;
    try
    {
        // start deep detection
        // Don't forget to convert stl descriptor to its uno representation.

        /* Attention!
                You have to use an explicit instance of this uno sequence ...
                Because its used as an in out parameter. And in case of a temp. used object
                we will run into memory corruptions!
        */
        css::uno::Sequence< css::beans::PropertyValue > lDescriptor;
        rDescriptor >> lDescriptor;
        sDeepType = xDetector->detect(lDescriptor);
        rDescriptor << lDescriptor;
    }
    catch (...)
    {
        // We should ignore errors here.
        // Thrown exceptions mostly will end in crash recovery ...
        // But might be we find another deep detection service which can detect the same
        // document without a problem .-)
        sDeepType.clear();
    }

    // seek to 0 is an optional feature to be more robust against
    // "simple implemented detect services" .-)
    impl_seekStreamToZero(rDescriptor);

    // analyze the results
    // a) detect service returns "" => return "" too and remove TYPE/FILTER prop from descriptor
    // b) returned type is unknown  => return "" too and remove TYPE/FILTER prop from descriptor
    // c) returned type is valid    => check TYPE/FILTER props inside descriptor and return the type

    // this special helper checks for a valid type
    // and set right values on the descriptor!
    bool bValidType = impl_validateAndSetTypeOnDescriptor(rDescriptor, sDeepType);
    if (bValidType)
        return sDeepType;

    return OUString();
}


OUString TypeDetection::impl_askUserForTypeAndFilterIfAllowed(utl::MediaDescriptor& rDescriptor)
{
    css::uno::Reference< css::task::XInteractionHandler > xInteraction =
        rDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_INTERACTIONHANDLER(),
        css::uno::Reference< css::task::XInteractionHandler >());

    if (!xInteraction.is())
        return OUString();

    OUString sURL =
        rDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_URL(),
        OUString());

    css::uno::Reference< css::io::XInputStream > xStream =
        rDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_INPUTSTREAM(),
        css::uno::Reference< css::io::XInputStream >());

    // Don't disturb the user for "non existing files - means empty URLs" or
    // if we was forced to detect a stream.
    // Reason behind: we must be sure to ask user for "unknown contents" only...
    // and not for "missing files". Especially if detection is done by a stream only
    // we can't check if the stream points to an "existing content"!
    if (
        (sURL.isEmpty()                                     ) || // "non existing file" ?
        (!xStream.is()                                         ) || // non existing file !
        (sURL.equalsIgnoreAsciiCase("private:stream"))    // not a good idea .-)
       )
        return OUString();

    try
    {
        // create a new request to ask user for its decision about the usable filter
        ::framework::RequestFilterSelect aRequest(sURL);
        xInteraction->handle(aRequest.GetRequest());

        // "Cancel" pressed? => return with error
        if (aRequest.isAbort())
            return OUString();

        // "OK" pressed => verify the selected filter, get its corresponding
        // type and return it. (BTW: We must update the media descriptor here ...)
        // The user selected explicitly a filter ... but normaly we are interested on
        // a type here only. But we must be sure, that the selected filter is used
        // too and no ambigous filter registration disturb us .-)

        OUString sFilter = aRequest.getFilter();
        if (!impl_validateAndSetFilterOnDescriptor(rDescriptor, sFilter))
            return OUString();

        OUString sType;
        rDescriptor[utl::MediaDescriptor::PROP_TYPENAME()] >>= sType;
        return sType;
    }
    catch(const css::uno::Exception&)
        {}

    return OUString();
}


void TypeDetection::impl_openStream(utl::MediaDescriptor& rDescriptor)
    throw (css::uno::Exception)
{
    bool bSuccess = false;
    OUString sURL = rDescriptor.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_URL(), OUString() );
    bool bRequestedReadOnly = rDescriptor.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_READONLY(), false );
    if ( comphelper::isFileUrl( sURL ) )
    {
        // OOo uses own file locking mechanics in case of local file
        bSuccess = rDescriptor.addInputStreamOwnLock();
    }
    else
        bSuccess = rDescriptor.addInputStream();

    if ( !bSuccess )
        throw css::uno::Exception(
            "Could not open stream for <" + sURL + ">",
            static_cast<OWeakObject *>(this));

    if ( !bRequestedReadOnly )
    {
        // The MediaDescriptor implementation adds ReadOnly argument if the file can not be opened for writing
        // this argument should be either removed or an additional argument should be added so that application
        // can separate the case when the user explicitly requests readonly document.
        // The current solution is to remove it here.
        rDescriptor.erase( utl::MediaDescriptor::PROP_READONLY() );
    }
}


void TypeDetection::impl_removeTypeFilterFromDescriptor(utl::MediaDescriptor& rDescriptor)
{
    utl::MediaDescriptor::iterator pItType   = rDescriptor.find(utl::MediaDescriptor::PROP_TYPENAME()  );
    utl::MediaDescriptor::iterator pItFilter = rDescriptor.find(utl::MediaDescriptor::PROP_FILTERNAME());
    if (pItType != rDescriptor.end())
        rDescriptor.erase(pItType);
    if (pItFilter != rDescriptor.end())
        rDescriptor.erase(pItFilter);
}


bool TypeDetection::impl_validateAndSetTypeOnDescriptor(      utl::MediaDescriptor& rDescriptor,
                                                            const OUString&               sType      )
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (m_rCache->hasItem(FilterCache::E_TYPE, sType))
    {
        rDescriptor[utl::MediaDescriptor::PROP_TYPENAME()] <<= sType;
        return true;
    }
    aLock.clear();
    // <- SAFE

    // remove all related information from the descriptor
    impl_removeTypeFilterFromDescriptor(rDescriptor);
    return false;
}


bool TypeDetection::impl_validateAndSetFilterOnDescriptor(      utl::MediaDescriptor& rDescriptor,
                                                              const OUString&               sFilter    )
{
    try
    {
        // SAFE ->
        ::osl::ResettableMutexGuard aLock(m_aLock);

        CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, sFilter);
        OUString sType;
        aFilter[PROPNAME_TYPE] >>= sType;
        CacheItem aType = m_rCache->getItem(FilterCache::E_TYPE, sType);

        aLock.clear();
        // <- SAFE

        // found valid type and filter => set it on the given descriptor
        rDescriptor[utl::MediaDescriptor::PROP_TYPENAME()  ] <<= sType  ;
        rDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
        return true;
    }
    catch(const css::container::NoSuchElementException&){}

    // remove all related information from the descriptor
    impl_removeTypeFilterFromDescriptor(rDescriptor);
    return false;
}


OUString TypeDetection::impl_getImplementationName()
{
    return OUString( "com.sun.star.comp.filter.config.TypeDetection" );
}


css::uno::Sequence< OUString > TypeDetection::impl_getSupportedServiceNames()
{
    css::uno::Sequence< OUString > lServiceNames { "com.sun.star.document.TypeDetection" };
    return lServiceNames;
}


css::uno::Reference< css::uno::XInterface > SAL_CALL TypeDetection::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    TypeDetection* pNew = new TypeDetection( comphelper::getComponentContext(xSMGR) );
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::document::XTypeDetection* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
