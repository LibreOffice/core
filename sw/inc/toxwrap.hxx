/*************************************************************************
 *
 *  $RCSfile: toxwrap.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fme $ $Date: 2002-08-23 08:10:20 $
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

#ifndef _TOXWRAP_HXX
#define _TOXWRAP_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _SWUNODEF_HXX
#include <swunodef.hxx>
#endif

namespace drafts { namespace com { namespace sun { namespace star {
namespace i18n {
    class XExtendedIndexEntrySupplier;
}
namespace lang {
    class XMultiServiceFactory;
}
}}}};


class String;

class IndexEntrySupplierWrapper
{
    STAR_NMSPC::lang::Locale aLcl;
    STAR_NMSPC::uno::Reference < drafts::com::sun::star::i18n::XExtendedIndexEntrySupplier > xIES;

public:
    IndexEntrySupplierWrapper();
    ~IndexEntrySupplierWrapper();

    void SetLocale( const STAR_NMSPC::lang::Locale& rLocale ) { aLcl = rLocale; }

    String GetIndexKey( const String& rTxt, const String& rTxtReading,
                        const STAR_NMSPC::lang::Locale& rLocale ) const;

    String GetFollowingText( BOOL bMorePages ) const;

    STAR_NMSPC::uno::Sequence< ::rtl::OUString >
    GetAlgorithmList( const STAR_NMSPC::lang::Locale& rLcl ) const;

    STAR_NMSPC::uno::Sequence < com::sun::star::lang::Locale >
    GetLocaleList() const;

    sal_Bool LoadAlgorithm( const STAR_NMSPC::lang::Locale& rLcl,
                            const String& sSortAlgorithm, long nOptions ) const;

    sal_Int16 CompareIndexEntry( const String& rTxt1, const String& rTxtReading1,
                                 const STAR_NMSPC::lang::Locale& rLcl1,
                                 const String& rTxt2, const String& rTxtReading2,
                                 const STAR_NMSPC::lang::Locale& rLcl2 ) const;
};

#endif
