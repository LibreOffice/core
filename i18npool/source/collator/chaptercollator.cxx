/*************************************************************************
 *
 *  $RCSfile: chaptercollator.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:07:52 $
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

// prevent internal compiler error with MSVC6SP3
#include <stl/utility>

#include <chaptercollator.hxx>
#include <com/sun/star/i18n/KCharacterType.hpp>
#ifndef _COM_SUN_STAR_I18N_PARSERESULT_HPP_
#include <com/sun/star/i18n/ParseResult.hpp>
#endif

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::i18n;
using namespace ::rtl;

ChapterCollator::ChapterCollator( const Reference < XMultiServiceFactory >& rxMSF ) : CollatorImpl(rxMSF)
{
    if ( rxMSF.is()) {
        Reference < XInterface > xI =
        rxMSF->createInstance( OUString::createFromAscii("com.sun.star.i18n.CharacterClassification"));
        if ( xI.is() )
            xI->queryInterface(::getCppuType((const Reference< XCharacterClassification>*)0)) >>= cclass;
    }
}

ChapterCollator::~ChapterCollator()
{
}

sal_Int32 SAL_CALL
ChapterCollator::compareString( const OUString& s1, const OUString& s2) throw(RuntimeException)
{
    return compareSubstring(s1, 0, s1.getLength(),  s2, 0, s2.getLength());
}

#define DIGIT KCharacterType::DIGIT

sal_Int32 SAL_CALL
ChapterCollator::compareSubstring( const OUString& str1, sal_Int32 off1, sal_Int32 len1,
    const OUString& str2, sal_Int32 off2, sal_Int32 len2) throw(RuntimeException)
{
    if( len1 <= 1 || len2 <= 1 || ! cclass.is() )
        return CollatorImpl::compareSubstring( str1, off1,  len1, str2, off2, len2 );

    sal_Int32 i1, i2;
    for (i1 = len1; i1 && (cclass->getCharacterType(str1, off1+i1-1, nLocale) & DIGIT); i1--);
    for (i2 = len2; i2 && (cclass->getCharacterType(str2, off2+i2-1, nLocale) & DIGIT); i2--);

    sal_Int32 ans = CollatorImpl::compareSubstring(str1, off1, i1, str2, off2, i2);
    if( ans != 0 )
        return ans;

    OUString &aAddAllowed = OUString::createFromAscii("?");
    ParseResult res1, res2;
    // Bug #100323#, since parseAnyToken does not take length as parameter, we have to copy
    // it to a temp. string.
    OUString s1 = str1.copy(off1+i1, len1-i1), s2 = str2.copy(off2+i2, len2-i2);
    res1 = cclass->parseAnyToken( s1, 0, nLocale, DIGIT, aAddAllowed, DIGIT, aAddAllowed );
    res2 = cclass->parseAnyToken( s2, 0, nLocale, DIGIT, aAddAllowed, DIGIT, aAddAllowed );

    return res1.Value == res2.Value ? 0 : res1.Value > res2.Value ? 1 : -1;
}

const sal_Char *cChapCollator = "com.sun.star.i18n.ChapterCollator";

OUString SAL_CALL
ChapterCollator::getImplementationName() throw( RuntimeException )
{
    return OUString::createFromAscii(cChapCollator);
}

sal_Bool SAL_CALL
ChapterCollator::supportsService(const rtl::OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cChapCollator);
}

Sequence< OUString > SAL_CALL
ChapterCollator::getSupportedServiceNames() throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cChapCollator);
    return aRet;
}
