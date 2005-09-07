/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: breakiterator_ctl.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:47:12 $
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
    virtual LineBreakResults SAL_CALL getLineBreak( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
        throw(com::sun::star::uno::RuntimeException);
protected:
    rtl::OUString cachedText; // for cell index
    sal_Int32* nextCellIndex;
    sal_Int32* previousCellIndex;
    sal_Int32 cellIndexSize;
    virtual void SAL_CALL makeIndex(const rtl::OUString& text, sal_Int32 pos) throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif
