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

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>

#include <boost/shared_ptr.hpp>

#include "svx/stddlg.hxx"
#include "svx/svxdllapi.h"


class MenuButton;
class Button;
class ListBox;
class SvxCheckListBox;

/////////////////////////////////////////////////////////////////

struct SvxThesaurusDialog_Impl;

class SVX_DLLPUBLIC SvxThesaurusDialog : public SvxStandardDialog
{
    boost::shared_ptr< SvxThesaurusDialog_Impl > m_pImpl;

#ifdef _SVX_THESDLG_CXX
    SVX_DLLPRIVATE bool     UpdateAlternativesBox_Impl();
    SVX_DLLPRIVATE void     SetWindowTitle(sal_Int16 nLanguage);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XMeaning > > SAL_CALL
            queryMeanings_Impl( ::rtl::OUString& rTerm, const ::com::sun::star::lang::Locale& rLocale, const ::com::sun::star::beans::PropertyValues& rProperties ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // Handler
    DECL_LINK( LanguageHdl_Impl, MenuButton * );
    DECL_LINK( LookUpHdl_Impl, Button * );
    DECL_LINK( WordSelectHdl_Impl, ListBox * );
    DECL_LINK( AlternativesSelectHdl_Impl, SvxCheckListBox * );
    DECL_LINK( AlternativesDoubleClickHdl_Impl, SvxCheckListBox * );
#endif

    SVX_DLLPRIVATE virtual void     Apply();

public:
    SvxThesaurusDialog( Window* pParent,
                        ::com::sun::star::uno::Reference< ::com::sun::star::linguistic2::XThesaurus >  xThesaurus,
                        const String &rWord, sal_Int16 nLanguage );
    ~SvxThesaurusDialog();

    String          GetWord();
    sal_uInt16      GetLanguage() const;
};

#endif

