/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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
#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>

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
    throw (css::uno::RuntimeException)
{
    OUString sType;

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    css::util::URL  aURL;
    aURL.Complete = sURL;
    css::uno::Reference< css::util::XURLTransformer > xParser( css::util::URLTransformer::create(m_xContext) );
    xParser->parseStrict(aURL);

    
    
    
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

    
    if (lFlatTypes.size() > 0)
    {
        const FlatDetectionInfo& aMatch = *(lFlatTypes.begin());
        sType = aMatch.sType;
    }

    return sType;
    
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
    
    
    static const char* ranks[] = {

        
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
        "writerglobal8",
        "writerweb8_writer_template",
        "StarBase",

        
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

        
        "Unified_Office_Format_text",
        "Unified_Office_Format_spreadsheet",
        "Unified_Office_Format_presentation",

        
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

        
        "pdf_Portable_Document_Format",

        
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
        "impress_CGM_Computer_Graphics_Metafile", 
        "draw_WordPerfect_Graphics",
        "draw_Visio_Document",
        "draw_Publisher_Document",
        "draw_Corel_Presentation_Exchange",
        "draw_CorelDraw_Document",
        "writer_LotusWordPro_Document",
        "writer_MIZI_Hwp_97", 

        
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

        
        "dxf_AutoCAD_Interchange",
        "eps_Encapsulated_PostScript",
        "pbm_Portable_Bitmap",   
        "ppm_Portable_Pixelmap", 
        "pgm_Portable_Graymap",  
        "xpm_XPM",
        "xbm_X_Consortium",
        "writer_Rich_Text_Format",
        "writer_web_HTML_help",
        "generic_HTML",

        "generic_Text", 

        
        

        
        "writer_layout_dump_xml",
        "pwp_PlaceWare",
        "graphic_SWF",
        "graphic_HTML",

        
        "StarBaseReportChart",
        "StarBaseReport",
        "math_MathType_3x", 
    };

    size_t n = SAL_N_ELEMENTS(ranks);

    for (size_t i = 0; i < n; ++i)
    {
        if (rType.equalsAscii(ranks[i]))
            return n - i - 1;
    }

    
    
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

class FindByType : std::unary_function<FlatDetectionInfo, bool>
{
    OUString maType;
public:
    FindByType(const OUString& rType) : maType(rType) {}
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
    throw (css::uno::RuntimeException)
{
    
    utl::MediaDescriptor stlDescriptor(lDescriptor);

    
    ::osl::ResettableMutexGuard aLock(m_aLock);

    
    
    OUString sURL = stlDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_URL(), OUString());

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
        
        
        if (impl_validateAndSetFilterOnDescriptor(stlDescriptor, aSelectedFilter))
            return stlDescriptor[utl::MediaDescriptor::PROP_TYPENAME()].get<OUString>();
    }

    FlatDetection lFlatTypes;
    impl_getAllFormatTypes(aURL, stlDescriptor, lFlatTypes);

    aLock.clear();
    

    
    lFlatTypes.sort(SortByPriority());
    lFlatTypes.unique(EqualByType());

    OUString sType      ;
    OUString sLastChance;

    try
    {
        
        
        
        
        
        
        
        
        OUStringList lUsedDetectors;
        if (lFlatTypes.size()>0)
            sType = impl_detectTypeFlatAndDeep(stlDescriptor, lFlatTypes, bAllowDeep, lUsedDetectors, sLastChance);

        
        
        
        
        
        if (sType.isEmpty())
            sType = impl_askUserForTypeAndFilterIfAllowed(stlDescriptor);

        
        
        
        if (sType.isEmpty() && !sLastChance.isEmpty())
        {
            OSL_FAIL("set first flat detected type without a registered deep detection service as \"last chance\" ... nevertheless some other deep detections said \"NO\". I TRY IT!");
            sType = sLastChance;
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception& e)
        {
            SAL_WARN(
                "filter.config",
                "caught Exception \"" << e.Message
                    << "\" while querying type of <" << sURL << ">");
            sType = OUString();
        }

    
    
    
    impl_checkResultsAndAddBestFilter(stlDescriptor, sType); 
    impl_validateAndSetTypeOnDescriptor(stlDescriptor, sType);

    stlDescriptor >> lDescriptor;
    return sType;
}



void TypeDetection::impl_checkResultsAndAddBestFilter(utl::MediaDescriptor& rDescriptor,
                                                      OUString&               sType      )
{
    
    
    OUString sFilter = rDescriptor.getUnpackedValueOrDefault(
                                utl::MediaDescriptor::PROP_FILTERNAME(),
                                OUString());
    if (!sFilter.isEmpty())
        return;

    
    
    
    OUString sDocumentService = rDescriptor.getUnpackedValueOrDefault(
                                            utl::MediaDescriptor::PROP_DOCUMENTSERVICE(),
                                            OUString());
    if (!sDocumentService.isEmpty())
    {
        try
        {
            OUString sRealType = sType;

            
            ::osl::ResettableMutexGuard aLock(m_aLock);

            
            
            
            m_rCache->load(FilterCache::E_CONTAINS_FILTERS);

            CacheItem lIProps;
            lIProps[PROPNAME_DOCUMENTSERVICE] <<= sDocumentService;
            lIProps[PROPNAME_TYPE           ] <<= sRealType;
            OUStringList lFilters = m_rCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps);

            aLock.clear();
            

            for (OUStringList::const_iterator pIt  = lFilters.begin();
                  pIt != lFilters.end(); ++pIt)
            {
                
                aLock.reset();
                try
                {
                    CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, *pIt);
                    sal_Int32 nFlags  = 0;
                    aFilter[PROPNAME_FLAGS] >>= nFlags;

                    if ((nFlags & FLAGVAL_IMPORT) == FLAGVAL_IMPORT)
                        sFilter = *pIt;
                    if ((nFlags & FLAGVAL_PREFERRED) == FLAGVAL_PREFERRED)
                        break;
                }
                catch(const css::uno::Exception&) {}
                aLock.clear();
                
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

    
    
    
    
    
    
    
    sFilter = OUString();
    try
    {
        
        ::osl::ResettableMutexGuard aLock(m_aLock);

        CacheItem aType = m_rCache->getItem(FilterCache::E_TYPE, sType);
        aType[PROPNAME_PREFERREDFILTER] >>= sFilter;
        CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, sFilter);

        aLock.clear();
        

        
        rDescriptor[utl::MediaDescriptor::PROP_TYPENAME()  ] <<= sType  ;
        rDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
        return;
    }
    catch(const css::uno::Exception&)
        {}

    
    
    sFilter = OUString();
    try
    {
        
        ::osl::ResettableMutexGuard aLock(m_aLock);

        
        
        
        m_rCache->load(FilterCache::E_CONTAINS_FILTERS);

        CacheItem lIProps;
        lIProps[PROPNAME_TYPE] <<= sType;
        OUStringList lFilters = m_rCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps);

        aLock.clear();
        

        OUStringList::const_iterator pIt;
        for (  pIt  = lFilters.begin();
               pIt != lFilters.end()  ;
             ++pIt                    )
        {
            sFilter = *pIt;

            
            aLock.reset();
            try
            {
                CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, sFilter);
                sal_Int32 nFlags  = 0;
                aFilter[PROPNAME_FLAGS] >>= nFlags;

                if ((nFlags & FLAGVAL_IMPORT) == FLAGVAL_IMPORT)
                    break;
            }
            catch(const css::uno::Exception&)
                { continue; }
            aLock.clear();
            

            sFilter = OUString();
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
    
    
    
    
    bool bBreakDetection = false;

    
    
    bool bMatchByPattern = false;

    
    
    bool bMatchByExtension = false;

    
    OUString sType(sPreSelType);
    CacheItem       aType;
    try
    {
        
        ::osl::ResettableMutexGuard aLock(m_aLock);
        aType = m_rCache->getItem(FilterCache::E_TYPE, sType);
        aLock.clear();
        
    }
    catch(const css::container::NoSuchElementException&)
    {
        sType = OUString();
        bBreakDetection = true;
    }

    if (!bBreakDetection)
    {
        
        
        if ( aParsedURL.Complete == "private:stream" )
            bBreakDetection = true;
    }

    if (!bBreakDetection)
    {
        
        INetURLObject   aParser    (aParsedURL.Main);
        OUString sExtension = aParser.getExtension(INetURLObject::LAST_SEGMENT       ,
                                                          true                          ,
                                                          INetURLObject::DECODE_WITH_CHARSET);
        sExtension = sExtension.toAsciiLowerCase();

        
        
        OUStringList lExtensions(aType[PROPNAME_EXTENSIONS]);
        OUStringList lURLPattern(aType[PROPNAME_URLPATTERN]);

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
                    bBreakDetection        = true;
                    bMatchByPattern        = true;
                    break;
                }
            }
        }
    }

    
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

    
    return false;
}

bool TypeDetection::impl_getPreselectionForDocumentService(
    const OUString& sPreSelDocumentService, const util::URL& aParsedURL, FlatDetection& rFlatTypes)
{
    
    OUStringList lFilters;
    try
    {
        
        ::osl::ResettableMutexGuard aLock(m_aLock);

        
        
        
        m_rCache->load(FilterCache::E_CONTAINS_FILTERS);

        CacheItem lIProps;
        lIProps[PROPNAME_DOCUMENTSERVICE] <<= sPreSelDocumentService;
        lFilters = m_rCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps);

        aLock.clear();
        
    }
    catch (const css::container::NoSuchElementException&)
    {
        lFilters.clear();
    }

    
    
    
    
    
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

    
    for (OUStringList::const_iterator it = aFilterNames.begin(); it != aFilterNames.end(); ++it)
    {
        OUString aType = impl_getTypeFromFilter(*it);

        if (aType.isEmpty())
            continue;

        FlatDetectionInfo aInfo; 
        aInfo.sType = aType;
        rFlatTypes.push_back(aInfo);
    }

    {
        
        FlatDetection aFlatByURL;
        m_rCache->detectFlatForURL(aParsedURL, aFlatByURL);
        FlatDetection::const_iterator it = aFlatByURL.begin(), itEnd = aFlatByURL.end();
        for (; it != itEnd; ++it)
        {
            FlatDetection::iterator itPos = std::find_if(rFlatTypes.begin(), rFlatTypes.end(), FindByType(it->sType));
            if (itPos == rFlatTypes.end())
                
                rFlatTypes.push_back(*it);
            else
            {
                
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

    
    rFlatTypes.sort(SortByType());
    rFlatTypes.unique(EqualByType());

    
    OUString sSelectedType = rDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_TYPENAME(), OUString());
    if (!sSelectedType.isEmpty())
        impl_getPreselectionForType(sSelectedType, aParsedURL, rFlatTypes, false);

    
    OUString sSelectedDoc = rDescriptor.getUnpackedValueOrDefault(utl::MediaDescriptor::PROP_DOCUMENTSERVICE(), OUString());
    if (!sSelectedDoc.isEmpty())
        impl_getPreselectionForDocumentService(sSelectedDoc, aParsedURL, rFlatTypes);
}



OUString TypeDetection::impl_detectTypeFlatAndDeep(      utl::MediaDescriptor& rDescriptor   ,
                                                          const FlatDetection&                 lFlatTypes    ,
                                                                sal_Bool                       bAllowDeep    ,
                                                                OUStringList&                  rUsedDetectors,
                                                                OUString&               rLastChance   )
{
    
    
    rLastChance = OUString();
    rUsedDetectors.clear();

    
    
    
    
    
    
    
    
    
    
    
    
    
    

    for (FlatDetection::const_iterator pFlatIt  = lFlatTypes.begin();
                                       pFlatIt != lFlatTypes.end()  ;
                                     ++pFlatIt                      )
    {
        const FlatDetectionInfo& aFlatTypeInfo = *pFlatIt;
        OUString sFlatType = aFlatTypeInfo.sType;

        if (!impl_validateAndSetTypeOnDescriptor(rDescriptor, sFlatType))
            continue;

        
        if (
            (!bAllowDeep                  ) ||
            (aFlatTypeInfo.bMatchByPattern)
           )
        {
            return sFlatType;
        }

        try
        {
            
            ::osl::ResettableMutexGuard aLock(m_aLock);
            CacheItem aType = m_rCache->getItem(FilterCache::E_TYPE, sFlatType);
            aLock.clear();

            OUString sDetectService;
            aType[PROPNAME_DETECTSERVICE] >>= sDetectService;

            
            if (sDetectService.isEmpty())
            {
                
                
                
                
                if (rLastChance.isEmpty())
                    rLastChance = sFlatType;

                continue;
            }

            
            
            
            rUsedDetectors.push_back(sDetectService);
            OUString sDeepType = impl_askDetectService(sDetectService, rDescriptor);

            
            if (!sDeepType.isEmpty())
                return sDeepType;
        }
        catch(const css::container::NoSuchElementException&)
            {}
        
    }

    return OUString();
    
}

void TypeDetection::impl_seekStreamToZero(utl::MediaDescriptor& rDescriptor)
{
    
    
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
    
    
    
    
    
    impl_openStream(rDescriptor);

    
    
    impl_seekStreamToZero(rDescriptor);

    css::uno::Reference< css::document::XExtendedFilterDetection > xDetector;
    css::uno::Reference< css::uno::XComponentContext >         xContext;

    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    xContext = m_xContext;
    aLock.clear();
    

    try
    {
        
        
        
        
        
        
        xDetector = css::uno::Reference< css::document::XExtendedFilterDetection >(
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
    catch(const css::uno::Exception&)
        {
            
            
            
            
            sDeepType = OUString();
        }

    
    
    impl_seekStreamToZero(rDescriptor);

    
    
    
    

    
    
    sal_Bool bValidType = impl_validateAndSetTypeOnDescriptor(rDescriptor, sDeepType);
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

    
    
    
    
    
    if (
        (sURL.isEmpty()                                     ) || 
        (!xStream.is()                                         ) || 
        (sURL.equalsIgnoreAsciiCase("private:stream"))    
       )
        return OUString();

    try
    {
        
        ::framework::RequestFilterSelect aRequest(sURL);
        xInteraction->handle(aRequest.GetRequest());

        
        if (aRequest.isAbort())
            return OUString();

        
        
        
        
        

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
    sal_Bool bSuccess = sal_False;
    OUString sURL = rDescriptor.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_URL(), OUString() );
    sal_Bool bRequestedReadOnly = rDescriptor.getUnpackedValueOrDefault( utl::MediaDescriptor::PROP_READONLY(), sal_False );
    if ( !sURL.isEmpty() && ::utl::LocalFileHelper::IsLocalFile( INetURLObject( sURL ).GetMainURL( INetURLObject::NO_DECODE ) ) )
    {
        
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



sal_Bool TypeDetection::impl_validateAndSetTypeOnDescriptor(      utl::MediaDescriptor& rDescriptor,
                                                            const OUString&               sType      )
{
    
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (m_rCache->hasItem(FilterCache::E_TYPE, sType))
    {
        rDescriptor[utl::MediaDescriptor::PROP_TYPENAME()] <<= sType;
        return sal_True;
    }
    aLock.clear();
    

    
    impl_removeTypeFilterFromDescriptor(rDescriptor);
    return sal_False;
}



sal_Bool TypeDetection::impl_validateAndSetFilterOnDescriptor(      utl::MediaDescriptor& rDescriptor,
                                                              const OUString&               sFilter    )
{
    try
    {
        
        ::osl::ResettableMutexGuard aLock(m_aLock);

        CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, sFilter);
        OUString sType;
        aFilter[PROPNAME_TYPE] >>= sType;
        CacheItem aType = m_rCache->getItem(FilterCache::E_TYPE, sType);

        aLock.clear();
        

        
        rDescriptor[utl::MediaDescriptor::PROP_TYPENAME()  ] <<= sType  ;
        rDescriptor[utl::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
        return sal_True;
    }
    catch(const css::container::NoSuchElementException&){}

    
    impl_removeTypeFilterFromDescriptor(rDescriptor);
    return sal_False;
}



OUString TypeDetection::impl_getImplementationName()
{
    return OUString( "com.sun.star.comp.filter.config.TypeDetection" );
}



css::uno::Sequence< OUString > TypeDetection::impl_getSupportedServiceNames()
{
    css::uno::Sequence< OUString > lServiceNames(1);
    lServiceNames[0] = "com.sun.star.document.TypeDetection";
    return lServiceNames;
}



css::uno::Reference< css::uno::XInterface > SAL_CALL TypeDetection::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    TypeDetection* pNew = new TypeDetection( comphelper::getComponentContext(xSMGR) );
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::document::XTypeDetection* >(pNew), css::uno::UNO_QUERY);
}

    } 
} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
