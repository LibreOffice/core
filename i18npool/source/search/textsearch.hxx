/*************************************************************************
 *
 *  $RCSfile: textsearch.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 11:08:13 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

    mutable com::sun::star::uno::Reference<
        com::sun::star::i18n::XCharacterClassification > xCharClass;

    com::sun::star::uno::Reference<
        com::sun::star::i18n::XExtendedTransliteration > xTranslit;

    // define a function pointer for the different search nethods
    typedef ::com::sun::star::util::SearchResult
        (SAL_CALL TextSearch:: *FnSrch)( const ::rtl::OUString& searchStr,
                                sal_Int32 startPos, sal_Int32 endPos );

    FnSrch fnForward;
    FnSrch fnBackward;

    // Members and methods for the normal (Boyer-Moore) search
    TextSearchJumpTable* pJumpTable;
    bool bIsForwardTab;
    void MakeForwardTab();
    void MakeBackwardTab();
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
