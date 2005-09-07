/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textsearch.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:23:43 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_I18NPOOL_TEXTSEARCH_HXX
#define INCLUDED_I18NPOOL_TEXTSEARCH_HXX


#ifndef _COM_SUN_STAR_UTIL_XTEXTSEARCH_HPP_
#include <com/sun/star/util/XTextSearch.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XBREAKITERATOR_HPP_
#include <com/sun/star/i18n/XBreakIterator.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>     // helper for implementations
#endif
#ifndef _COM_SUN_STAR_I18N_XEXTENDEDTRANSLITERATION_HPP_
#include <com/sun/star/i18n/XExtendedTransliteration.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XCHARACTERCLASSIFICATION_HPP_
#include <com/sun/star/i18n/XCharacterClassification.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

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
