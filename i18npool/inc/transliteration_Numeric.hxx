/*************************************************************************
 *
 *  $RCSfile: transliteration_Numeric.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-08 15:45:21 $
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
#ifndef _I18N_TRANSLITERATION_TRANSLITERATION_NUMERIC_H_
#define _I18N_TRANSLITERATION_TRANSLITERATION_NUMERIC_H_

#include <transliteration_commonclass.hxx>

namespace com { namespace sun { namespace star { namespace i18n {

class transliteration_Numeric : public transliteration_commonclass {
public:
        virtual ::rtl::OUString SAL_CALL
        transliterate( const ::rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, ::com::sun::star::uno::Sequence< sal_Int32 >& offset )
        throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Unicode SAL_CALL
        transliterateChar2Char( sal_Unicode inChar)
        throw(drafts::com::sun::star::i18n::MultipleCharsOutputException,
                com::sun::star::uno::RuntimeException);

        // Methods which are shared.
        virtual sal_Int16 SAL_CALL getType(  ) throw(::com::sun::star::uno::RuntimeException);

        virtual ::rtl::OUString SAL_CALL
        folding( const ::rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount, ::com::sun::star::uno::Sequence< sal_Int32 >& offset )
        throw(::com::sun::star::uno::RuntimeException);

        virtual sal_Bool SAL_CALL
        equals( const ::rtl::OUString& str1, sal_Int32 pos1, sal_Int32 nCount1, sal_Int32& nMatch1, const ::rtl::OUString& str2, sal_Int32 pos2, sal_Int32 nCount2, sal_Int32& nMatch2 )
        throw(::com::sun::star::uno::RuntimeException);

        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
        transliterateRange( const ::rtl::OUString& str1, const ::rtl::OUString& str2 )
        throw(::com::sun::star::uno::RuntimeException);
protected:
        sal_Int16 nNativeNumberMode;
        sal_Int16 tableSize;
        sal_Unicode* table;
        sal_Bool recycleSymbol;
private:
        rtl::OUString SAL_CALL
        transliterateBullet( const rtl::OUString& inStr, sal_Int32 startPos, sal_Int32 nCount,
        com::sun::star::uno::Sequence< sal_Int32 >& offset )
        throw(com::sun::star::uno::RuntimeException);
};

} } } }

#endif // _I18N_TRANSLITERATION_TRANSLITERATION_NUMERIC_H_
