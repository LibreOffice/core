/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: toxhlp.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 08:59:38 $
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

#ifndef _TOXHLP_HXX
#define _TOXHLP_HXX

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

namespace com { namespace sun { namespace star {
namespace i18n {
    class XExtendedIndexEntrySupplier;
}
namespace lang {
    class XMultiServiceFactory;
}
}}}


class String;

class IndexEntrySupplierWrapper
{
    STAR_NMSPC::lang::Locale aLcl;
    STAR_NMSPC::uno::Reference < com::sun::star::i18n::XExtendedIndexEntrySupplier > xIES;

public:
    IndexEntrySupplierWrapper(
            const STAR_NMSPC::lang::Locale& rLcl,
            STAR_REFERENCE( lang::XMultiServiceFactory )& rxMSF );
    ~IndexEntrySupplierWrapper();

    String GetIndexKey( const String& rTxt, const String& rTxtReading,
                        const STAR_NMSPC::lang::Locale& rLocale ) const;

    String GetFollowingText( BOOL bMorePages ) const;

    STAR_NMSPC::uno::Sequence< ::rtl::OUString >
    GetAlgorithmList( const STAR_NMSPC::lang::Locale& rLcl ) const;

    sal_Bool LoadAlgorithm( const STAR_NMSPC::lang::Locale& rLcl,
                            const String& sSortAlgorithm, long nOptions ) const;

    sal_Int16 CompareIndexEntry( const String& rTxt1, const String& rTxtReading1,
                                 const STAR_NMSPC::lang::Locale& rLcl1,
                                 const String& rTxt2, const String& rTxtReading2,
                                 const STAR_NMSPC::lang::Locale& rLcl2 ) const;
};

#endif
