/*************************************************************************
 *
 *  $RCSfile: breakiterator_unicode.hxx,v $
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
#ifndef _I18N_BREAKITERATOR_UNICODE_HXX_
#define _I18N_BREAKITERATOR_UNICODE_HXX_

#include <breakiteratorImpl.hxx>
#include <unicode/brkiter.h>

namespace com { namespace sun { namespace star { namespace i18n {

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
    sal_Char *cBreakIterator;
    Boundary result; // for word break iterator

private:
    rtl::OUString cachedText;   // for script boundary
    sal_Int32 scriptStart, scriptLength;
    icu::BreakIterator *characterBreak, *wordBreak, *sentenceBreak, *lineBreak;
    sal_Int32 SAL_CALL setTextByScriptBoundary(const rtl::OUString& Text, sal_Int32 nPos);
};

} } } }

#endif
