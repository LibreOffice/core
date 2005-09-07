/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: breakiterator_cjk.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:46:55 $
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
    LineBreakResults SAL_CALL getLineBreak( const rtl::OUString& Text, sal_Int32 nStartPos,
        const com::sun::star::lang::Locale& nLocale, sal_Int32 nMinBreakPos,
        const LineBreakHyphenationOptions& hOptions, const LineBreakUserOptions& bOptions )
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
BREAKITERATOR_CJK( ko )
#endif
#undef BREAKITERATOR__CJK

} } } }

#endif // _I18N_BREAKITERATOR_CJK_HXX_
