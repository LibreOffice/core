/*************************************************************************
 *
 *  $RCSfile: breakiterator_unicode.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2004-07-30 14:37:06 $
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
#ifndef _I18N_BREAKITERATOR_UNICODE_HXX_
#define _I18N_BREAKITERATOR_UNICODE_HXX_

#include <breakiteratorImpl.hxx>
#include <unicode/brkiter.h>

namespace com { namespace sun { namespace star { namespace i18n {

#define LOAD_CHARACTER_BREAKITERATOR    0
#define LOAD_WORD_BREAKITERATOR         1
#define LOAD_SENTENCE_BREAKITERATOR     2
#define LOAD_LINE_BREAKITERATOR         3

//  ----------------------------------------------------
//  class BreakIterator_Unicode
//  ----------------------------------------------------
class BreakIterator_Unicode : public BreakIteratorImpl
{
public:
    BreakIterator_Unicode();
    ~BreakIterator_Unicode();

    virtual sal_Int32 SAL_CALL previousCharacters( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL nextCharacters( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 nCount,
        sal_Int32& nDone ) throw(com::sun::star::uno::RuntimeException);

    virtual Boundary SAL_CALL previousWord( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException);
    virtual Boundary SAL_CALL nextWord( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType) throw(com::sun::star::uno::RuntimeException);
    virtual Boundary SAL_CALL getWordBoundary( const rtl::OUString& Text, sal_Int32 nPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int16 WordType, sal_Bool bDirection )
        throw(com::sun::star::uno::RuntimeException);

    virtual sal_Int32 SAL_CALL beginOfSentence( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL endOfSentence( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale ) throw(com::sun::star::uno::RuntimeException);

    virtual LineBreakResults SAL_CALL getLineBreak( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException);

    //XServiceInfo
    virtual rtl::OUString SAL_CALL getImplementationName() throw( com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService(const rtl::OUString& ServiceName)
        throw( com::sun::star::uno::RuntimeException );
    virtual com::sun::star::uno::Sequence< rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw( com::sun::star::uno::RuntimeException );

protected:
    const sal_Char *cBreakIterator;
    Boundary result; // for word break iterator

    rtl::OUString characterText, editWordText, dictWordText, countWordText, sentenceText, lineText;
    icu::BreakIterator *characterBreak, *editWordBreak, *dictWordBreak,
                *countWordBreak, *sentenceBreak, *lineBreak;
    virtual icu::BreakIterator* SAL_CALL loadICUWordBreakIterator(const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& rLocale, sal_Int16 rWordType) throw( com::sun::star::uno::RuntimeException);
    icu::BreakIterator* SAL_CALL BreakIterator_Unicode::loadICUBreakIterator(const com::sun::star::lang::Locale& rLocale,
        sal_Int16 which) throw(com::sun::star::uno::RuntimeException);
};

class BreakIterator_ca : public BreakIterator_Unicode
{
    icu::BreakIterator* SAL_CALL loadICUWordBreakIterator(const rtl::OUString& Text, sal_Int32 nStartPos, const com::sun::star::lang::Locale& rLocale,
    sal_Int16 rWordType) throw( com::sun::star::uno::RuntimeException);
};

class BreakIterator_hu : public BreakIterator_Unicode
{
    icu::BreakIterator* SAL_CALL loadICUWordBreakIterator(const rtl::OUString& Text, sal_Int32 nStartPos, const com::sun::star::lang::Locale& rLocale,
    sal_Int16 rWordType) throw( com::sun::star::uno::RuntimeException);
};

} } } }

#endif
