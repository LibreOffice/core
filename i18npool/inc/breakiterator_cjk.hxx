/*************************************************************************
 *
 *  $RCSfile: breakiterator_cjk.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 13:36:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
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
#ifndef _I18N_BREAKITERATOR_CJK_HXX_
#define _I18N_BREAKITERATOR_CJK_HXX_

#include <breakiterator_unicode.hxx>
#include <xdictionary.hxx>

namespace com { namespace sun { namespace star { namespace i18n {
//  ----------------------------------------------------
//  class BreakIterator_CJK
//  ----------------------------------------------------
class BreakIterator_CJK : public BreakIterator_Unicode
{
public:
    BreakIterator_CJK();

    Boundary SAL_CALL nextWord( const rtl::OUString& Text, sal_Int32 nStartPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType)
            throw(com::sun::star::uno::RuntimeException);
    Boundary SAL_CALL previousWord( const rtl::OUString& Text, sal_Int32 nStartPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType)
            throw(com::sun::star::uno::RuntimeException);
    Boundary SAL_CALL getWordBoundary( const rtl::OUString& Text, sal_Int32 nPos,
            const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection )
            throw(com::sun::star::uno::RuntimeException);

protected:
    xdictionary *dict;
};

#define BREAKITERATOR_CJK( lang ) \
class BreakIterator_##lang : public BreakIterator_CJK {\
public:\
    BreakIterator_##lang (); \
    ~BreakIterator_##lang (); \
};

#ifdef BREAKITERATOR_ALL
BREAKITERATOR_CJK( zh )
BREAKITERATOR_CJK( ja )
#endif
#undef BREAKITERATOR__CJK

} } } }

#endif // _I18N_BREAKITERATOR_CJK_HXX_
