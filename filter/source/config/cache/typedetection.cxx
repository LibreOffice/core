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
#include <unotools/localfilehelper.hxx>
#include <comphelper/processfactory.hxx>


namespace filter{
    namespace config{

namespace css = ::com::sun::star;

TypeDetection::TypeDetection(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    BaseContainer::init(xSMGR                                         ,
                        TypeDetection::impl_getImplementationName()   ,
                        TypeDetection::impl_getSupportedServiceNames(),
                        FilterCache::E_TYPE                           );
}



TypeDetection::~TypeDetection()
{
}



::rtl::OUString SAL_CALL TypeDetection::queryTypeByURL(const ::rtl::OUString& sURL)
    throw (css::uno::RuntimeException)
{
    ::rtl::OUString sType;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    css::util::URL  aURL;
    aURL.Complete = sURL;
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(comphelper::getComponentContext(m_xSMGR)));
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
int getFlatTypeRank(const rtl::OUString& rType)
{
    // List formats from more complex to less complex.
    // TODO: Add more.
    static const char* ranks[] = {
        // Compressed XML
        "writer8_template",
        "writer8",
        "calc8_template",
        "calc8",
        "writer_OOXML_Text_Template",
        "writer_OOXML",
        "writer_MS_Word_2007_Template",
        "writer_MS_Word_2007",
        "Office Open XML Spreadsheet Template",
        "Office Open XML Spreadsheet",
        "MS Excel 2007 XML Template",
        "MS Excel 2007 XML",

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
        "calc_Lotus",
        "calc_QPro",
        "calc_SYLK",
        "calc_DIF",
        "calc_dBase",


        // Non-compressed XML
        "writer_ODT_FlatXML",
        "calc_ODS_FlatXML",
        "calc_MS_Excel_2003_XML",
        "writer_MS_Word_2003_XML",
        "writer_DocBook_File",
        "XHTML_File",

        // Non-compressed text
        "writer_Rich_Text_Format",
        "generic_HTML",
        "generic_Text"
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

struct EqualByName : public std::binary_function<FlatDetectionInfo, FlatDetectionInfo, bool>
{
    bool operator() (const FlatDetectionInfo& r1, const FlatDetectionInfo& r2) const
    {
        return r1.sType == r2.sType;
    }
};

}

::rtl::OUString SAL_CALL TypeDetection::queryTypeByDescriptor(css::uno::Sequence< css::beans::PropertyValue >& lDescriptor,
                                                              sal_Bool                                         bAllowDeep )
    throw (css::uno::RuntimeException)
{
    // make the descriptor more useable :-)
    ::comphelper::MediaDescriptor stlDescriptor(lDescriptor);

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);

    //*******************************************
    // parse given URL to split it into e.g. main and jump marks ...
    ::rtl::OUString sURL = stlDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_URL(), ::rtl::OUString());

#if OSL_DEBUG_LEVEL > 0
    if (stlDescriptor.find(::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FileName" ))) != stlDescriptor.end())
        OSL_FAIL("Detect using of deprecated and already unsupported MediaDescriptor property \"FileName\"!");
#endif

    css::util::URL  aURL;
    aURL.Complete = sURL;
    css::uno::Reference< css::util::XURLTransformer > xParser(css::util::URLTransformer::create(comphelper::getComponentContext(m_xSMGR)));
    xParser->parseStrict(aURL);

    rtl::OUString aSelectedFilter = stlDescriptor.getUnpackedValueOrDefault(
        comphelper::MediaDescriptor::PROP_FILTERNAME(), rtl::OUString());
    if (!aSelectedFilter.isEmpty())
    {
        // Caller specified the filter type.  Honor it.  Just get the default
        // type for that filter, and bail out.
        if (impl_validateAndSetFilterOnDescriptor(stlDescriptor, aSelectedFilter))
            return stlDescriptor[comphelper::MediaDescriptor::PROP_TYPENAME()].get<rtl::OUString>();
    }

    // preselected type or document service? use it as first "flat" detected
    // type later!
    FlatDetection lFlatTypes;
    impl_getPreselection(aURL, stlDescriptor, lFlatTypes);

    //*******************************************
    // get all types, which match to the given descriptor
    // That can be true by: extensions/url pattern/mime type etcpp.
    m_rCache->detectFlatForURL(aURL, lFlatTypes);

    aLock.clear();
    // <- SAFE ----------------------------------

    // Properly prioritize all candidate types.
    lFlatTypes.sort(SortByPriority());
    lFlatTypes.unique(EqualByName());

    ::rtl::OUString sType      ;
    ::rtl::OUString sLastChance;

    try
    {
        //*******************************************
        // verify every flat detected (or preselected!) type
        // by calling its registered deep detection service.
        // But break this loop if a type match to the given descriptor
        // by an URL pattern(!) or if deep detection isnt allowed from
        // outside (bAllowDeep=sal_False) or break the whole detection by
        // throwing an exception if creation of the might needed input
        // stream failed by e.g. an IO exception ...
        OUStringList lUsedDetectors;
        if (lFlatTypes.size()>0)
            sType = impl_detectTypeFlatAndDeep(stlDescriptor, lFlatTypes, bAllowDeep, lUsedDetectors, sLastChance);

        //*******************************************
        // if no flat detected (nor preselected!) type could be
        // verified and no error occurred during creation of
        // the might needed input stream, start detection
        // which uses all registered deep detection services.
        if (
            (sType.isEmpty()) &&
            (bAllowDeep        )
           )
        {
            sType = impl_detectTypeDeepOnly(stlDescriptor, lUsedDetectors);
        }

        //*******************************************
        // flat detection failed
        // pure deep detection failed
        // => ask might existing InteractionHandler
        // means: ask user for it's decision
        if (sType.isEmpty())
            sType = impl_askUserForTypeAndFilterIfAllowed(stlDescriptor);

        //*******************************************
        // no real detected type - but a might valid one.
        // update descriptor and set last chance for return.
        if (sType.isEmpty() && !sLastChance.isEmpty())
        {
            OSL_FAIL("set first flat detected type without a registered deep detection service as \"last chance\" ... nevertheless some other deep detections said \"NO\". I TRY IT!");
            sType = sLastChance;
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { sType = ::rtl::OUString(); }

    //*******************************************
    // adapt media descriptor, so it contains the right values
    // for type/filter name/document service/ etcpp.
    impl_checkResultsAndAddBestFilter(stlDescriptor, sType); // Attention: sType is used as IN/OUT param here and will might be changed inside this method !!!
    impl_validateAndSetTypeOnDescriptor(stlDescriptor, sType);

    stlDescriptor >> lDescriptor;
    return sType;
}



void TypeDetection::impl_checkResultsAndAddBestFilter(::comphelper::MediaDescriptor& rDescriptor,
                                                      ::rtl::OUString&               sType      )
{
    // a)
    // Dont overwrite a might preselected filter!
    ::rtl::OUString sFilter = rDescriptor.getUnpackedValueOrDefault(
                                ::comphelper::MediaDescriptor::PROP_FILTERNAME(),
                                ::rtl::OUString());
    if (!sFilter.isEmpty())
        return;

    // b)
    // check a preselected document service too.
    // Then we have to search a suitable filter witin this module.
    ::rtl::OUString sDocumentService = rDescriptor.getUnpackedValueOrDefault(
                                            ::comphelper::MediaDescriptor::PROP_DOCUMENTSERVICE(),
                                            ::rtl::OUString());
    if (!sDocumentService.isEmpty())
    {
        try
        {
            ::rtl::OUString sRealType = sType;

            // SAFE ->
            ::osl::ResettableMutexGuard aLock(m_aLock);

            // Attention: For executing next lines of code, We must be shure that
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

                    if ((nFlags & FLAGVAL_IMPORT) == FLAGVAL_IMPORT)
                        sFilter = *pIt;
                    if ((nFlags & FLAGVAL_PREFERRED) == FLAGVAL_PREFERRED)
                        break;
                }
                catch(const css::uno::Exception&) {}
                aLock.clear();
                // <- SAFE
            }

            if (!sFilter.isEmpty())
            {
                rDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()  ] <<= sRealType;
                rDescriptor[::comphelper::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
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
    sFilter = ::rtl::OUString();
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
        rDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()  ] <<= sType  ;
        rDescriptor[::comphelper::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
        return;
    }
    catch(const css::uno::Exception&)
        {}

    // d)
    // Search for any import(!) filter, which is registered for this type.
    sFilter = ::rtl::OUString();
    try
    {
        // SAFE ->
        ::osl::ResettableMutexGuard aLock(m_aLock);

        // Attention: For executing next lines of code, We must be shure that
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

                if ((nFlags & FLAGVAL_IMPORT) == FLAGVAL_IMPORT)
                    break;
            }
            catch(const css::uno::Exception&)
                { continue; }
            aLock.clear();
            // <- SAFE

            sFilter = ::rtl::OUString();
        }

        if (!sFilter.isEmpty())
        {
            rDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()  ] <<= sType  ;
            rDescriptor[::comphelper::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
            return;
        }
    }
    catch(const css::uno::Exception&)
        {}
}



sal_Bool TypeDetection::impl_getPreselectionForType(const ::rtl::OUString& sPreSelType,
                                                    const css::util::URL&  aParsedURL ,
                                                          FlatDetection&   rFlatTypes )
{
    // Can be used to supress execution of some parts of this method
    // if its already clear that detected type is valid or not.
    // Its neccessary to use shared code at the end, which update
    // all return parameters constistency!
    sal_Bool bBreakDetection = sal_False;

    // Further we must know if it matches by pattern
    // Every flat detected type by pattern wont be detected deep!
    sal_Bool bMatchByPattern = sal_False;

    // And we must know if a preselection must be preferred, because
    // it matches by it's extension too.
    sal_Bool bMatchByExtension = sal_False;

    // If we e.g. collect all filters of a factory (be a forced factory preselection)
    // we should preferr all filters of this factory, where the type match the given URL.
    // All other types (which sorrespond to filters of the same factory - but dont match
    // the URL) should be "used later" for detection and sorted at the end of our return vector
    // rFlatTypes!
    // => bPreferredPreselection = (matchByExtension || matchByURLPattern)
    sal_Bool bPreferredPreselection = sal_False;

    // validate type
    ::rtl::OUString sType(sPreSelType);
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
        sType = ::rtl::OUString();
        bBreakDetection = sal_True;
    }

    if (!bBreakDetection)
    {
        // We cant check a preselected type for a given stream!
        // So we must believe, that it can work ...
        if ( aParsedURL.Complete == "private:stream" )
            bBreakDetection = sal_True;
    }

    if (!bBreakDetection)
    {
        // extract extension from URL .. to check it case-insensitive !
        INetURLObject   aParser    (aParsedURL.Main);
        ::rtl::OUString sExtension = aParser.getExtension(INetURLObject::LAST_SEGMENT       ,
                                                          sal_True                          ,
                                                          INetURLObject::DECODE_WITH_CHARSET);
        sExtension = sExtension.toAsciiLowerCase();

        // otherwhise we must know, if it matches to the given URL realy.
        // especialy if it matches by its extension or pattern registration.
        OUStringList lExtensions(aType[PROPNAME_EXTENSIONS]);
        OUStringList lURLPattern(aType[PROPNAME_URLPATTERN]);

        for (OUStringList::const_iterator pIt  = lExtensions.begin();
                                          pIt != lExtensions.end()  ;
                                        ++pIt                       )
        {
            ::rtl::OUString sCheckExtension(pIt->toAsciiLowerCase());
            if (sCheckExtension.equals(sExtension))
            {
                bBreakDetection        = sal_True;
                bMatchByExtension      = sal_True;
                bPreferredPreselection = sal_True;
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
                    bBreakDetection        = sal_True;
                    bMatchByPattern        = sal_True;
                    bPreferredPreselection = sal_True;
                    break;
                }
            }
        }
    }

    // if its a valid type - set it on all return values!
    if (!sType.isEmpty())
    {
        FlatDetectionInfo aInfo;
        aInfo.sType              = sType;
        aInfo.bMatchByExtension  = bMatchByExtension;
        aInfo.bMatchByPattern    = bMatchByPattern;

        if (bPreferredPreselection)
            rFlatTypes.push_front(aInfo);
        else
            rFlatTypes.push_back(aInfo);

        return sal_True;
    }

    // not valid!
    return sal_False;
}



sal_Bool TypeDetection::impl_getPreselectionForFilter(const ::rtl::OUString& sPreSelFilter,
                                                      const css::util::URL&  aParsedURL   ,
                                                            FlatDetection&   rFlatTypes   )
{
    // Can be used to supress execution of some parts of this method
    // if its already clear that detected filter is valid or not.
    // Its neccessary to use shared code at the end, which update
    // all return parameters constistency!
    sal_Bool bBreakDetection = sal_False;

    // validate filter
    ::rtl::OUString sFilter(sPreSelFilter);
    CacheItem       aFilter;
    try
    {
        // SAFE -> --------------------------
        ::osl::ResettableMutexGuard aLock(m_aLock);
        aFilter = m_rCache->getItem(FilterCache::E_FILTER, sFilter);
        aLock.clear();
        // <- SAFE --------------------------
    }
    catch(const css::container::NoSuchElementException&)
    {
        sFilter = ::rtl::OUString();
        bBreakDetection = sal_True;
    }

    if (!bBreakDetection)
    {
        // get its type and check if it matches the given URL
        ::rtl::OUString sType;
        aFilter[PROPNAME_TYPE] >>= sType;

        bBreakDetection = impl_getPreselectionForType(sType, aParsedURL, rFlatTypes);

        // not a valid type? -> not a valid filter!
        if (!bBreakDetection)
            sFilter = ::rtl::OUString();
    }

    if (!sFilter.isEmpty())
        return sal_True;
    else
        return sal_False;
}



sal_Bool TypeDetection::impl_getPreselectionForDocumentService(const ::rtl::OUString& sPreSelDocumentService,
                                                               const css::util::URL&  aParsedURL            ,
                                                                     FlatDetection&   rFlatTypes            )
{
    // get all filters, which match to this doc service
    OUStringList lFilters;
    try
    {
        // SAFE -> --------------------------
        ::osl::ResettableMutexGuard aLock(m_aLock);

        // Attention: For executing next lines of code, We must be shure that
        // all filters already loaded :-(
        // That can disturb our "load on demand feature". But we have no other chance!
        m_rCache->load(FilterCache::E_CONTAINS_FILTERS);

        CacheItem lIProps;
        lIProps[PROPNAME_DOCUMENTSERVICE] <<= sPreSelDocumentService;
        lFilters = m_rCache->getMatchingItemsByProps(FilterCache::E_FILTER, lIProps);

        aLock.clear();
        // <- SAFE --------------------------
    }
    catch(const css::container::NoSuchElementException&)
    {
        lFilters.clear();
    }

    // step over all filters, and check if its registered type
    // match the given URL.
    // But use temp. list of "preselected types" instead of incoming rFlatTypes list!
    // The reason behind: we must filter the getted results. And copying of stl entries
    // is an easier job then removing it .-)
    FlatDetection lPreselections;
    for (OUStringList::const_iterator pFilter  = lFilters.begin();
                                      pFilter != lFilters.end()  ;
                                    ++pFilter                    )
    {
        const ::rtl::OUString sFilter = *pFilter;
        impl_getPreselectionForFilter(sFilter, aParsedURL, lPreselections);
    }

    // We have to mark all retrieved preselection items as "preselected by document service".
    // Further we must ignore all preselected items, which does not match the URL!
    FlatDetection::iterator pIt;
    for (  pIt  = lPreselections.begin();
           pIt != lPreselections.end()  ;
         ++pIt                          )
    {
        FlatDetectionInfo& rInfo = *pIt;
        rInfo.bPreselectedByDocumentService = sal_True ;
        rFlatTypes.push_back(rInfo);
    }

    return sal_True;
}



void TypeDetection::impl_getPreselection(const css::util::URL&                aParsedURL ,
                                               ::comphelper::MediaDescriptor& rDescriptor,
                                               FlatDetection&                 rFlatTypes )
{
    // done to be shure, that only valid results leave this function!
    rFlatTypes.clear();

    /* #i55122#
        Sometimes we must detect files without or with real unknown extensions.
        If it does not work /which can happen of course .-)/, the user tried to preselect
        the right format. But some special dialogs (e.g. "Insert->Sheet From File")
        add it's own preselection too.
        So we have a combination of preselected values ...

        The we should preferr the most important one - set by the user.
        And the user normaly preselects a filter or type. The preslected
        document service cames from the dialog.

        Further it doesnt matter if the user preselected a filter or a document service.
        A filter selection is always more explicit then a document service selection.
        So it must be pereferred. An order between type and filter selection cant be discussed .-)
    */

    ::rtl::OUString sSelectedType = rDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_TYPENAME(), ::rtl::OUString());
    if (!sSelectedType.isEmpty())
        impl_getPreselectionForType(sSelectedType, aParsedURL, rFlatTypes);

    ::rtl::OUString sSelectedDoc = rDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_DOCUMENTSERVICE(), ::rtl::OUString());
    if (!sSelectedDoc.isEmpty())
        impl_getPreselectionForDocumentService(sSelectedDoc, aParsedURL, rFlatTypes);
}



::rtl::OUString TypeDetection::impl_detectTypeFlatAndDeep(      ::comphelper::MediaDescriptor& rDescriptor   ,
                                                          const FlatDetection&                 lFlatTypes    ,
                                                                sal_Bool                       bAllowDeep    ,
                                                                OUStringList&                  rUsedDetectors,
                                                                ::rtl::OUString&               rLastChance   )
{
    // reset it everytimes, so the outside code can distinguish between
    // a set and a not set value.
    rLastChance = ::rtl::OUString();
    rUsedDetectors.clear();

    // step over all possible types for this URL.
    // solutions:
    // a) no types                                => no detection
    // b) deep detection not allowed              => return first valid type of list (because its the preferred or the first valid one)
    //    or(!) match by URLPattern               => in such case a deep detection will be supressed!
    // c) type has no detect service              => safe the first occurred type without a detect service
    //                                               as "last chance"(!). It will be used outside of this method
    //                                               if no further type could be detected.
    //                                               It must be the first one, because it can be a preferred type.
    //                                               Our types list was sorted by such criteria!
    // d) detect service return a valid result    => return its decision
    // e) detect service return an invalid result
    //    or any needed information could not be
    //    getted from the cache                   => ignore it, and continue with search

    for (FlatDetection::const_iterator pFlatIt  = lFlatTypes.begin();
                                       pFlatIt != lFlatTypes.end()  ;
                                     ++pFlatIt                      )
    {
        const FlatDetectionInfo& aFlatTypeInfo = *pFlatIt;
              ::rtl::OUString    sFlatType     = aFlatTypeInfo.sType;

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

            ::rtl::OUString sDetectService;
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

            // dont forget to add every real asked deep detection service here.
            // Such detectors will be ignored if may be "impl_detectTypeDeepOnly()"
            // must be called later!
            rUsedDetectors.push_back(sDetectService);
            ::rtl::OUString sDeepType = impl_askDetectService(sDetectService, rDescriptor);

            // d)
            if (!sDeepType.isEmpty())
                return sDeepType;
        }
        catch(const css::container::NoSuchElementException&)
            {}
        // e)
    }

    return ::rtl::OUString();
    // <- SAFE ----------------------------------
}

//TO-DO: add a priority entry to filter config, e.g. defaulting to 50 and
//flag externally that some filters are lower e.g. 25 and are catch-alls
//to be tried last. Split up writer/calc/etc. filter detection to standalone
//those problematic formats
namespace
{
    bool sort_catchalls_to_end(const rtl::OUString& rA, const rtl::OUString& rB)
    {
        if (rA == rB)
            return false;
        if ( rA == "com.sun.star.text.FormatDetector" )
            return false;
        if ( rB == "com.sun.star.text.FormatDetector" )
            return true;
        return rA < rB;
    }
}

::rtl::OUString TypeDetection::impl_detectTypeDeepOnly(      ::comphelper::MediaDescriptor& rDescriptor          ,
                                                       const OUStringList&                  lOutsideUsedDetectors)
{
    // We must know if a detect service was already used:
    //  i) in a combined flat/deep detection scenario outside or
    // ii) in this method for a deep detection only.
    // Reason: Such deep detection services work differently in these two modes!
    OUStringList                 lInsideUsedDetectors;
    OUStringList::const_iterator pIt;

    // a)
    // The list of "already used detect services" correspond to the list
    // of preselected or flat detected types. But these detect services was called
    // to check these types explicitly and return black/white ... yes/no only.
    // Now they are called to return any possible result. But we should preferr
    // these already used detect services against all other ones!
    for (  pIt  = lOutsideUsedDetectors.begin();
           pIt != lOutsideUsedDetectors.end()  ;
         ++pIt                                 )
    {
        const ::rtl::OUString& sDetectService = *pIt;
              ::rtl::OUString  sDeepType      = impl_askDetectService(sDetectService, rDescriptor);
        if (!sDeepType.isEmpty())
            return sDeepType;
        lInsideUsedDetectors.push_back(sDetectService);
    }

    // SAFE -> ----------------------------------
    ::osl::ResettableMutexGuard aLock(m_aLock);
    OUStringList lDetectors = m_rCache->getItemNames(FilterCache::E_DETECTSERVICE);
    std::sort(lDetectors.begin(), lDetectors.end(), sort_catchalls_to_end);
    aLock.clear();
    // <- SAFE ----------------------------------

    // b)
    // Sometimes it would be nice to ask a special set of detect services before
    // any other detect service is asked. E.g. by using a preselection of a DocumentService.
    // That's needed to prevent us from asking the "wrong application module" and
    // opening the files into the "wrong application".
    ::rtl::OUString sPreselDocumentService = rDescriptor.getUnpackedValueOrDefault(
                                                ::comphelper::MediaDescriptor::PROP_DOCUMENTSERVICE(),
                                                ::rtl::OUString());
    if (!sPreselDocumentService.isEmpty())
    {
        for (  pIt  = lDetectors.begin();
               pIt != lDetectors.end()  ;
             ++pIt                      )
        {
            const ::rtl::OUString& sDetectService = *pIt;

            OUStringList::const_iterator pAlreadyUsed = ::std::find(lInsideUsedDetectors.begin(), lInsideUsedDetectors.end(), sDetectService);
            if (pAlreadyUsed != lInsideUsedDetectors.end())
                continue;

            // SAFE -> --------------------------------------------------------
            aLock.reset();

            CacheItem lIProps;
            lIProps[PROPNAME_DETECTSERVICE] <<= sDetectService;
            OUStringList lTypes = m_rCache->getMatchingItemsByProps(FilterCache::E_TYPE, lIProps);

            aLock.clear();
            // <- SAFE --------------------------------------------------------

            sal_Bool bMatchDetectorToDocumentService = sal_False;
            OUStringList::const_iterator pIt2;
            for (  pIt2  = lTypes.begin();
                   pIt2 != lTypes.end()  ;
                 ++pIt2                  )
            {
                const ::rtl::OUString& sType  = *pIt2;

                try
                {
                    // SAFE -> ----------------------------------------------------
                    aLock.reset();

                    CacheItem aType = m_rCache->getItem(FilterCache::E_TYPE, sType);
                    ::rtl::OUString sFilter;
                    aType[PROPNAME_PREFERREDFILTER] >>= sFilter;
                    CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, sFilter);
                    ::rtl::OUString sCheckDocumentService;
                    aFilter[PROPNAME_DOCUMENTSERVICE] >>= sCheckDocumentService;

                    aLock.clear();
                    // <- SAFE

                    if (sCheckDocumentService.equals(sPreselDocumentService))
                    {
                        bMatchDetectorToDocumentService = sal_True;
                        break;
                    }
                }
                catch(const css::uno::Exception&)
                    { continue; }
            }

            if (bMatchDetectorToDocumentService)
            {
                ::rtl::OUString sDeepType = impl_askDetectService(sDetectService, rDescriptor);
                if (!sDeepType.isEmpty())
                    return sDeepType;
                lInsideUsedDetectors.push_back(sDetectService);
            }
        }
    }

    // c)
    // Last chance. No "used detectors", no "preselected detectors" ... ask any existing detect services
    // for this till know unknown format.
    for (  pIt  = lDetectors.begin();
           pIt != lDetectors.end()  ;
         ++pIt                      )
    {
        const ::rtl::OUString& sDetectService = *pIt;

        OUStringList::const_iterator pAlreadyUsed = ::std::find(lInsideUsedDetectors.begin(), lInsideUsedDetectors.end(), sDetectService);
        if (pAlreadyUsed != lInsideUsedDetectors.end())
            continue;

        ::rtl::OUString sDeepType = impl_askDetectService(sDetectService, rDescriptor);
        if (!sDeepType.isEmpty())
            return sDeepType;
    }

    return ::rtl::OUString();
}

void TypeDetection::impl_seekStreamToZero(comphelper::MediaDescriptor& rDescriptor)
{
    // try to seek to 0 ...
    // But because XSeekable is an optional interface ... try it only .-)
    css::uno::Reference< css::io::XInputStream > xStream = rDescriptor.getUnpackedValueOrDefault(
                                                            ::comphelper::MediaDescriptor::PROP_INPUTSTREAM(),
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

::rtl::OUString TypeDetection::impl_askDetectService(const ::rtl::OUString&               sDetectService,
                                                           ::comphelper::MediaDescriptor& rDescriptor   )
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
    css::uno::Reference< css::lang::XMultiServiceFactory >         xSMGR;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    xSMGR = m_xSMGR;
    aLock.clear();
    // <- SAFE

    try
    {
        // Attention! If e.g. an office module was not installed sometimes we
        // find a registered detect service, which is referred inside the
        // configuration ... but not realy installed. On the other side we use
        // third party components here, which can make trouble anyway.  So we
        // should handle errors during creation of such services more
        // gracefully .-)
        xDetector = css::uno::Reference< css::document::XExtendedFilterDetection >(
                xSMGR->createInstance(sDetectService),
                css::uno::UNO_QUERY_THROW);
    }
    catch (...)
    {
    }

    if ( ! xDetector.is())
        return ::rtl::OUString();

    ::rtl::OUString sDeepType;
    try
    {
        // start deep detection
        // Dont forget to convert stl descriptor to its uno representation.

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
            // We should ignore errors here.
            // Thrown exceptions mostly will end in crash recovery ...
            // But might be we find another deep detection service which can detect the same
            // document without a problem .-)
            sDeepType = ::rtl::OUString();
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
    sal_Bool bValidType = impl_validateAndSetTypeOnDescriptor(rDescriptor, sDeepType);
    if (bValidType)
        return sDeepType;

    return ::rtl::OUString();
}



::rtl::OUString TypeDetection::impl_askUserForTypeAndFilterIfAllowed(::comphelper::MediaDescriptor& rDescriptor)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR = m_xSMGR;
    aLock.clear();
    // <- SAFE

    css::uno::Reference< css::task::XInteractionHandler > xInteraction =
        rDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_INTERACTIONHANDLER(),
        css::uno::Reference< css::task::XInteractionHandler >());

    if (!xInteraction.is())
        return ::rtl::OUString();

    ::rtl::OUString sURL =
        rDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_URL(),
        ::rtl::OUString());

    css::uno::Reference< css::io::XInputStream > xStream =
        rDescriptor.getUnpackedValueOrDefault(::comphelper::MediaDescriptor::PROP_INPUTSTREAM(),
        css::uno::Reference< css::io::XInputStream >());

    // Dont distrub the user for "non existing files - means empty URLs" or
    // if we was forced to detect a stream.
    // Reason behind: We must be shure to ask user for "unknown contents" only ...
    // and not for "missing files". Especialy if detection is done by a stream only
    // we cant check if the stream points to an "existing content"!
    if (
        (sURL.isEmpty()                                     ) || // "non existing file" ?
        (!xStream.is()                                         ) || // non existing file !
        (sURL.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("private:stream")))    // not a good idea .-)
       )
        return ::rtl::OUString();

    try
    {
        // create a new request to ask user for it's decision about the usable filter
        ::framework::RequestFilterSelect aRequest(sURL);
        xInteraction->handle(aRequest.GetRequest());

        // "Cancel" pressed? => return with error
        if (aRequest.isAbort())
            return ::rtl::OUString();

        // "OK" pressed => verify the selected filter, get it's coressponding
        // type and return it. (BTW: We must update the media descriptor here ...)
        // The user selected explicitly a filter ... but normaly we are interested on
        // a type here only. But we must be shure, that the selected filter is used
        // too and no ambigous filter registration disturb us .-)

        ::rtl::OUString sFilter = aRequest.getFilter();
        if (!impl_validateAndSetFilterOnDescriptor(rDescriptor, sFilter))
            return ::rtl::OUString();

        ::rtl::OUString sType;
        rDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()] >>= sType;
        return sType;
    }
    catch(const css::uno::Exception&)
        {}

    return ::rtl::OUString();
}



void TypeDetection::impl_openStream(::comphelper::MediaDescriptor& rDescriptor)
    throw (css::uno::Exception)
{
    sal_Bool bSuccess = sal_False;
    ::rtl::OUString sURL = rDescriptor.getUnpackedValueOrDefault( ::comphelper::MediaDescriptor::PROP_URL(), ::rtl::OUString() );
    sal_Bool bRequestedReadOnly = rDescriptor.getUnpackedValueOrDefault( ::comphelper::MediaDescriptor::PROP_READONLY(), sal_False );
    if ( !sURL.isEmpty() && ::utl::LocalFileHelper::IsLocalFile( INetURLObject( sURL ).GetMainURL( INetURLObject::NO_DECODE ) ) )
    {
        // OOo uses own file locking mechanics in case of local file
        bSuccess = rDescriptor.addInputStreamOwnLock();
    }
    else
        bSuccess = rDescriptor.addInputStream();

    if ( !bSuccess )
        throw css::uno::Exception(_FILTER_CONFIG_FROM_ASCII_("Could not open stream."), static_cast< css::document::XTypeDetection* >(this));

    if ( !bRequestedReadOnly )
    {
        // The MediaDescriptor implementation adds ReadOnly argument if the file can not be opened for writing
        // this argument should be either removed or an additional argument should be added so that application
        // can separate the case when the user explicitly requests readonly document.
        // The current solution is to remove it here.
        rDescriptor.erase( ::comphelper::MediaDescriptor::PROP_READONLY() );
    }
}



void TypeDetection::impl_removeTypeFilterFromDescriptor(::comphelper::MediaDescriptor& rDescriptor)
{
    ::comphelper::MediaDescriptor::iterator pItType   = rDescriptor.find(::comphelper::MediaDescriptor::PROP_TYPENAME()  );
    ::comphelper::MediaDescriptor::iterator pItFilter = rDescriptor.find(::comphelper::MediaDescriptor::PROP_FILTERNAME());
    if (pItType != rDescriptor.end())
        rDescriptor.erase(pItType);
    if (pItFilter != rDescriptor.end())
        rDescriptor.erase(pItFilter);
}



sal_Bool TypeDetection::impl_validateAndSetTypeOnDescriptor(      ::comphelper::MediaDescriptor& rDescriptor,
                                                            const ::rtl::OUString&               sType      )
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);
    if (m_rCache->hasItem(FilterCache::E_TYPE, sType))
    {
        rDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()] <<= sType;
        return sal_True;
    }
    aLock.clear();
    // <- SAFE

    // remove all related informations from the descriptor
    impl_removeTypeFilterFromDescriptor(rDescriptor);
    return sal_False;
}



sal_Bool TypeDetection::impl_validateAndSetFilterOnDescriptor(      ::comphelper::MediaDescriptor& rDescriptor,
                                                              const ::rtl::OUString&               sFilter    )
{
    try
    {
        // SAFE ->
        ::osl::ResettableMutexGuard aLock(m_aLock);

        CacheItem aFilter = m_rCache->getItem(FilterCache::E_FILTER, sFilter);
        ::rtl::OUString sType;
        aFilter[PROPNAME_TYPE] >>= sType;
        CacheItem aType = m_rCache->getItem(FilterCache::E_TYPE, sType);

        aLock.clear();
        // <- SAFE

        // found valid type and filter => set it on the given descriptor
        rDescriptor[::comphelper::MediaDescriptor::PROP_TYPENAME()  ] <<= sType  ;
        rDescriptor[::comphelper::MediaDescriptor::PROP_FILTERNAME()] <<= sFilter;
        return sal_True;
    }
    catch(const css::container::NoSuchElementException&){}

    // remove all related informations from the descriptor
    impl_removeTypeFilterFromDescriptor(rDescriptor);
    return sal_False;
}



::rtl::OUString TypeDetection::impl_getImplementationName()
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.filter.config.TypeDetection" ));
}



css::uno::Sequence< ::rtl::OUString > TypeDetection::impl_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames(1);
    lServiceNames[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.TypeDetection" ));
    return lServiceNames;
}



css::uno::Reference< css::uno::XInterface > SAL_CALL TypeDetection::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    TypeDetection* pNew = new TypeDetection(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::document::XTypeDetection* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
