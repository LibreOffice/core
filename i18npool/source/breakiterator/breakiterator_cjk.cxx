/*************************************************************************
 *
 *  $RCSfile: breakiterator_cjk.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: bustamam $ $Date: 2002-03-26 06:26:55 $
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
#define BREAKITERATOR_ALL
#include <breakiterator_cjk.hxx>
#include <unicode.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class BreakIterator_CJK
//  ----------------------------------------------------;

BreakIterator_CJK::BreakIterator_CJK() : dict(NULL)
{
    cBreakIterator = "com.sun.star.i18n.BreakIterator_CJK";
}

Boundary SAL_CALL
BreakIterator_CJK::previousWord(const OUString& text, sal_Int32 anyPos,
    const lang::Locale& nLocale, sal_Int16 wordType) throw(RuntimeException)
{
    if (dict)
        return dict->previousWord(text.getStr(), anyPos, text.getLength());
    else
        throw RuntimeException();
}

Boundary SAL_CALL
BreakIterator_CJK::nextWord(const OUString& text, sal_Int32 anyPos,
    const lang::Locale& nLocale, sal_Int16 wordType) throw(RuntimeException)
{
    if (dict)
        return dict->nextWord(text.getStr(), anyPos, text.getLength());
    else
        throw RuntimeException();
}

Boundary SAL_CALL
BreakIterator_CJK::getWordBoundary( const OUString& text, sal_Int32 anyPos,
    const lang::Locale& nLocale, sal_Int16 wordType, sal_Bool bDirection )
    throw(RuntimeException)
{
    if (dict)
        return dict->getWordBoundary(text.getStr(), anyPos, text.getLength(), bDirection);
    else
        throw RuntimeException();
}

//  ----------------------------------------------------
//  class BreakIterator_zh
//  ----------------------------------------------------;
BreakIterator_zh::BreakIterator_zh()
{
    dict = new xdictionary("zh");
    cBreakIterator = "com.sun.star.i18n.BreakIterator_zh";
}

BreakIterator_zh::~BreakIterator_zh()
{
    delete dict;
}

//  ----------------------------------------------------
//  class BreakIterator_ja
//  ----------------------------------------------------;
BreakIterator_ja::BreakIterator_ja()
{
    dict = new xdictionary("ja");
    cBreakIterator = "com.sun.star.i18n.BreakIterator_ja";
}

BreakIterator_ja::~BreakIterator_ja()
{
    delete dict;
}

} } } }
