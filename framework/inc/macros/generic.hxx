/*************************************************************************
 *
 *  $RCSfile: generic.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:22 $
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

#ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
#define __FRAMEWORK_MACROS_GENERIC_HXX_

//_________________________________________________________________________________________________________________
//  includes
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif

//*****************************************************************************************************************
//  generic macros
//*****************************************************************************************************************

/*_________________________________________________________________________________________________________________
    DECLARE_ASCII( SASCIIVALUE )

    Use it to declare a constant ascii value at compile time in code.
    zB. OUSting sTest = DECLARE_ASCII( "Test" )
_________________________________________________________________________________________________________________*/

#define DECLARE_ASCII( SASCIIVALUE )                                                                            \
    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SASCIIVALUE ))

/*_________________________________________________________________________________________________________________
    U2B( SUNICODEVALUE )
    B2U( SASCIIVALUE )
    U2B_ENC( SUNICODEVALUE, AENCODING )
    B2U_ENC( SASCIIVALUE, AENCODING )

    Use it to convert unicode strings to ascii values and reverse ...
    We use UTF8 as default textencoding. If you will change this use U2B_ENC and B2U_ENC!
_________________________________________________________________________________________________________________*/

#define U2B( SUNICODEVALUE )                                                                                    \
    ::rtl::OUStringToOString( SUNICODEVALUE, RTL_TEXTENCODING_UTF8 )

#define B2U( SASCIIVALUE )                                                                                      \
    ::rtl::OUStringToOString( SASCIIVALUE, RTL_TEXTENCODING_UTF8 )

#define U2B_ENC( SUNICODEVALUE, AENCODING )                                                                     \
    ::rtl::OUStringToOString( SUNICODEVALUE, AENCODING )

#define B2U_ENC( SASCIIVALUE, AENCODING )                                                                       \
    ::rtl::OUStringToOString( SASCIIVALUE, AENCODING )

//*****************************************************************************************************************
//  end of file
//*****************************************************************************************************************

#endif  //  #ifndef __FRAMEWORK_MACROS_GENERIC_HXX_
