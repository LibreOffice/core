/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ordinalsuffix.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2006-12-19 18:05:29 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_i18npool.hxx"
#include <comphelper/processfactory.hxx>
#include <string.h>
#include "ordinalsuffix.hxx"


using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

namespace com { namespace sun { namespace star { namespace i18n {


OrdinalSuffix::OrdinalSuffix(
        const com::sun::star::uno::Reference < com::sun::star::lang::XMultiServiceFactory >& rxMSF) :
    _xServiceManager( rxMSF )
{
}

OrdinalSuffix::~OrdinalSuffix()
{
}


static OUString getOrdinalSuffixEn( sal_Int32 nNumber )
{
    OUString retValue;

    switch( labs( nNumber ) % 100 )
    {
        case 11: case 12: case 13:
            retValue = OUString::createFromAscii( "th" );
            break;
        default:
            switch( nNumber % 10 )
            {
                case 1:
                    retValue = OUString::createFromAscii( "st" );
                    break;
                case 2:
                    retValue = OUString::createFromAscii( "nd" );
                    break;
                case 3:
                    retValue = OUString::createFromAscii( "rd" );
                    break;
                default:
                    retValue = OUString::createFromAscii( "th" );
                    break;
            }
            break;
    }

    return retValue;
}


OUString SAL_CALL OrdinalSuffix::getOrdinalSuffix( sal_Int32 nNumber,
        const Locale &aLocale ) throw( RuntimeException )
{
    OUString retValue;

    if (aLocale.Language.equalsAsciiL("en",2))
        retValue = getOrdinalSuffixEn( nNumber );

    return retValue;
}


const sal_Char cOrdinalSuffix[] = "com.sun.star.i18n.OrdinalSuffix";

OUString SAL_CALL OrdinalSuffix::getImplementationName(void) throw( RuntimeException )
{
    return OUString::createFromAscii(cOrdinalSuffix);
}

sal_Bool SAL_CALL OrdinalSuffix::supportsService( const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii(cOrdinalSuffix);
}

Sequence< OUString > SAL_CALL OrdinalSuffix::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(1);
    aRet[0] = OUString::createFromAscii(cOrdinalSuffix);
    return aRet;
}

} } } }
