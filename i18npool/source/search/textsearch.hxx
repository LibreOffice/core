/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textsearch.hxx,v $
 * $Revision: 1.5 $
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

#ifndef INCLUDED_I18NPOOL_TEXTSEARCH_HXX
#define INCLUDED_I18NPOOL_TEXTSEARCH_HXX


#include <com/sun/star/util/XTextSearch.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <cppuhelper/implbase2.hxx>     // helper for implementations
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include <map>

class Regexpr;
class WLevDistance;
typedef ::std::map< sal_Unicode, sal_Int32 > TextSearchJumpTable;

//  ----------------------------------------------------
//  class SearchClass
//  ----------------------------------------------------
class TextSearch: public cppu::WeakImplHelper2
<
    ::com::sun::star::util::XTextSearch,
    ::com::sun::star::lang::XServiceInfo
>
{
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > xMSF;

    ::com::sun::star::util::SearchOptions aSrchPara;
    ::rtl::OUString sSrchStr;
    ::rtl::OUString sSrchStr2;

    mutable com::sun::star::uno::Reference<
        com::sun::star::i18n::XCharacterClassification > xCharClass;

    com::sun::star::uno::Reference<
        com::sun::star::i18n::XExtendedTransliteration > xTranslit;
    com::sun::star::uno::Reference<
        com::sun::star::i18n::XExtendedTransliteration > xTranslit2;

    // define a function pointer for the different search nethods
    typedef ::com::sun::star::util::SearchResult
        (SAL_CALL TextSearch:: *FnSrch)( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos );

    FnSrch fnForward;
    FnSrch fnBackward;

    // Members and methods for the normal (Boyer-Moore) search
    TextSearchJumpTable* pJumpTable;
    TextSearchJumpTable* pJumpTable2;
    bool bIsForwardTab;
    bool bUsePrimarySrchStr;
    void MakeForwardTab();
    void MakeForwardTab2();
    void MakeBackwardTab();
    void MakeBackwardTab2();
    sal_Int32 GetDiff( const sal_Unicode ) const;
    ::com::sun::star::util::SearchResult SAL_CALL
        NSrchFrwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::util::SearchResult SAL_CALL
        NSrchBkwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);

    // Members and methods for the regular expression search
    Regexpr* pRegExp;
    ::com::sun::star::util::SearchResult SAL_CALL
        RESrchFrwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::util::SearchResult SAL_CALL
        RESrchBkwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);

    // Members and methods for the "Weight Levenshtein-Distance" search
    int nLimit;
    WLevDistance* pWLD;
    com::sun::star::uno::Reference < com::sun::star::i18n::XBreakIterator > xBreak;
    ::com::sun::star::util::SearchResult SAL_CALL
        ApproxSrchFrwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::util::SearchResult SAL_CALL
        ApproxSrchBkwrd( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);

    bool IsDelimiter( const ::rtl::OUString& rStr, sal_Int32 nPos ) const;

    sal_Bool checkCTLStart, checkCTLEnd;
    sal_Bool SAL_CALL isCellStart(const ::rtl::OUString& searchStr, sal_Int32 nPos)
                            throw(::com::sun::star::uno::RuntimeException);

public:
    TextSearch(
        const ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory >& rxMSF );

    virtual ~TextSearch();

    // Methods
    virtual void SAL_CALL
        setOptions( const ::com::sun::star::util::SearchOptions& options )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::SearchResult SAL_CALL
        searchForward( const ::rtl::OUString& searchStr,
                        sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::util::SearchResult SAL_CALL
        searchBackward( const ::rtl::OUString& searchStr,
                        sal_Int32 startPos, sal_Int32 endPos )
                            throw(::com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName(void)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
                throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames(void)
                throw( ::com::sun::star::uno::RuntimeException );
};


#endif
