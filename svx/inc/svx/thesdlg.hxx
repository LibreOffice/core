/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: thesdlg.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _SVX_THESDLG_HXX
#define _SVX_THESDLG_HXX
// include ---------------------------------------------------------------

#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <svx/stddlg.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <boost/shared_ptr.hpp>

#include "svx/svxdllapi.h"


// forward ---------------------------------------------------------------

struct ThesDlg_Impl;

class SvxThesaurusLanguageDlg_Impl;
class ThesaurusAlternativesCtrl_Impl;

namespace com { namespace sun { namespace star {
    namespace linguistic2 {
        class XThesaurus;
        class XMeaning;
    }
    namespace lang {
        struct Locale;
    }
}}}

// class SvxThesaurusDialog ----------------------------------------------

class SVX_DLLPUBLIC SvxThesaurusDialog : public SvxStandardDialog
{
public:
    friend class SvxThesaurusLanguageDlg_Impl;

    SvxThesaurusDialog( Window* pParent,
                        ::com::sun::star::uno::Reference<
                            ::com::sun::star::linguistic2::XThesaurus >  xThesaurus,
                        const String &rWord, sal_Int16 nLanguage );
    ~SvxThesaurusDialog();

    String          GetWord() { return aReplaceEdit.GetText(); }

    sal_uInt16      GetLanguage() const;

private:
    FixedText       aWordText;
    ListBox         aWordLB;
    FixedText       aReplaceText;
    Edit            aReplaceEdit;
    FixedText       aMeanText;
    ListBox         aMeanLB;
    FixedText       aSynonymText;
    ListBox         aSynonymLB;
    FixedText       m_aAlternativesText;
    boost::shared_ptr< ThesaurusAlternativesCtrl_Impl > m_pAlternativesCT;
    FixedLine       aVarFL;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    PushButton      aLookUpBtn;
    PushButton      aLangBtn;
    HelpButton      aHelpBtn;

    String          aErrStr;

    ThesDlg_Impl*   pImpl;      // always != NULL

    SVX_DLLPRIVATE virtual void     Apply();

#ifdef _SVX_THESDLG_CXX
    SVX_DLLPRIVATE void             UpdateSynonymBox_Impl();
    SVX_DLLPRIVATE void             UpdateMeaningBox_Impl(
                        ::com::sun::star::uno::Sequence<
                            ::com::sun::star::uno::Reference<
                                ::com::sun::star::linguistic2::XMeaning >  > *pMeaningSeq = NULL );
    SVX_DLLPRIVATE void         Init_Impl(sal_Int16 nLanguage);

    ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Reference<
                ::com::sun::star::linguistic2::XMeaning > > SAL_CALL
        queryMeanings_Impl(
                ::rtl::OUString& rTerm,
                const ::com::sun::star::lang::Locale& rLocale,
                const ::com::sun::star::beans::PropertyValues& rProperties )
            throw(::com::sun::star::lang::IllegalArgumentException,
                  ::com::sun::star::uno::RuntimeException);

    // Handler
    DECL_LINK( SelectHdl_Impl, ListBox * );
    DECL_LINK( SynonymHdl_Impl, ListBox * );
    DECL_LINK( LookUpHdl_Impl, Button * );
    DECL_LINK( LanguageHdl_Impl, Button * );
    DECL_LINK( EntryHdl_Impl, ListBox * );
    DECL_LINK( SpellErrorHdl_Impl, void * );
#endif
};

#endif

