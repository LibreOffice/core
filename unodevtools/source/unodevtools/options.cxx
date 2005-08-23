/*************************************************************************
 *
 *  $RCSfile: options.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2005-08-23 08:26:33 $
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

#include <stdio.h>

#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/process.h>

#include <com/sun/star/uno/RuntimeException.hpp>

using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace unodevtools {

//-------------------------------------------------------------------------------
static void out( const sal_Char * pText )
{
    fprintf( stderr, pText );
}

//-------------------------------------------------------------------------------
static void out( const OUString & rText )
{
    OString aText( OUStringToOString( rText, RTL_TEXTENCODING_ASCII_US ) );
    fprintf( stderr, aText.getStr() );
}

//-------------------------------------------------------------------------------
sal_Bool readOption( OUString * pValue, const sal_Char * pOpt,
                     sal_Int32 * pnIndex, const OUString & aArg)
    throw (RuntimeException)
{
    const OUString dash = OUString(RTL_CONSTASCII_USTRINGPARAM("-"));
    if(aArg.indexOf(dash) != 0)
        return sal_False;

    OUString aOpt = OUString::createFromAscii( pOpt );

    if (aArg.getLength() < aOpt.getLength())
        return sal_False;

    if (aOpt.equalsIgnoreAsciiCase( aArg.copy(1) ))
    {
        // take next argument
        ++(*pnIndex);

        rtl_getAppCommandArg(*pnIndex, &pValue->pData);
        if (*pnIndex >= (sal_Int32)rtl_getAppCommandArgCount() || pValue->copy(1).equals(dash))
        {
            OUStringBuffer buf( 32 );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("incomplete option \"-") );
            buf.appendAscii( pOpt );
            buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\" given!") );
            throw RuntimeException( buf.makeStringAndClear(), Reference< XInterface >() );
        }
        else
        {
#if OSL_DEBUG_LEVEL > 1
            out( "\n> identified option -" );
            out( pOpt );
            out( " = " );
            OString tmp = OUStringToOString(*pValue, RTL_TEXTENCODING_ASCII_US);
              out( tmp.getStr() );
#endif
            ++(*pnIndex);
            return sal_True;
        }
    }
      else if (aArg.indexOf(aOpt) == 1)
    {
        *pValue = aArg.copy(1 + aOpt.getLength());
#if OSL_DEBUG_LEVEL > 1
        out( "\n> identified option -" );
        out( pOpt );
        out( " = " );
        OString tmp = OUStringToOString(aArg.copy(aOpt.getLength()), RTL_TEXTENCODING_ASCII_US);
        out( tmp.getStr() );
#endif
        ++(*pnIndex);

        return sal_True;
    }
    return sal_False;
}

//-------------------------------------------------------------------------------
sal_Bool readOption( sal_Bool * pbOpt, const sal_Char * pOpt,
                     sal_Int32 * pnIndex, const OUString & aArg)
{
    const OUString dashdash(RTL_CONSTASCII_USTRINGPARAM("--"));
    OUString aOpt = OUString::createFromAscii(pOpt);

    if(aArg.indexOf(dashdash) == 0 && aOpt.equalsIgnoreAsciiCase(aArg.copy(2)))
    {
        ++(*pnIndex);
        *pbOpt = sal_True;
#if OSL_DEBUG_LEVEL > 1
        out( "\n> identified option --" );
        out( pOpt );
#endif
        return sal_True;
    }
    return sal_False;
}

} // end of namespace unodevtools
