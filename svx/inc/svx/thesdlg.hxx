/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: thesdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:42:31 $
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
#ifndef _SVX_THESDLG_HXX
#define _SVX_THESDLG_HXX
// include ---------------------------------------------------------------

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SVX_STDDLG_HXX //autogen
#include <svx/stddlg.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUES_HPP_
#include <com/sun/star/beans/PropertyValues.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// forward ---------------------------------------------------------------

struct ThesDlg_Impl;

class SvxThesaurusLanguageDlg_Impl;

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

