/*************************************************************************
 *
 *  $RCSfile: breakiterator_ctl.hxx,v $
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
#ifndef __BREAKITERATOR_CTL_HXX__
#define __BREAKITERATOR_CTL_HXX__

#include <breakiterator_unicode.hxx>
#include <xdictionary.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

//  ----------------------------------------------------
//  class BreakIterator_CTL
//  ----------------------------------------------------
class BreakIterator_CTL : public BreakIterator_Unicode
{
public:
    BreakIterator_CTL();
    ~BreakIterator_CTL();
    virtual sal_Int32 SAL_CALL previousCharacters(const rtl::OUString& text, sal_Int32 start,
        const lang::Locale& nLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 count,
        sal_Int32& nDone) throw(com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL nextCharacters(const rtl::OUString& text, sal_Int32 start,
        const lang::Locale& rLocale, sal_Int16 nCharacterIteratorMode, sal_Int32 count,
        sal_Int32& nDone) throw(com::sun::star::uno::RuntimeException);

protected:
    rtl::OUString cachedText; // for cell index
    sal_Int32* nextCellIndex;
    sal_Int32* previousCellIndex;
    sal_Int32 cellIndexSize;
    virtual void SAL_CALL makeIndex(const rtl::OUString& text, sal_Int32 pos) throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif
