/*************************************************************************
 *
 *  $RCSfile: ZipEnumeration.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mtg $ $Date: 2000-11-21 17:57:07 $
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
#ifndef _ZIP_ENUMERATION_HXX
#include "ZipEnumeration.hxx"
#endif

#ifndef _COM_SUN_STAR_PACKAGE_ZIPCONSTANTS_HPP_
#include <com/sun/star/package/ZipConstants.hpp>
#endif

#include <iostream.h>
using namespace rtl;
using namespace com::sun::star;

/** Provides an Enumeration over the contents of a Zip file */

ZipEnumeration::ZipEnumeration( uno::Sequence< package::ZipEntry > &xList)
:  nCurrent(0)
{
    xZipList = xList;
}
ZipEnumeration::ZipEnumeration( )
:  nCurrent(0)
{
}

ZipEnumeration::~ZipEnumeration( void )
{
}
sal_Bool SAL_CALL ZipEnumeration::hasMoreElements() throw (uno::RuntimeException)
{
    return (nCurrent < xZipList.getLength());
}

uno::Any SAL_CALL ZipEnumeration::nextElement() throw (uno::RuntimeException)
{
    if (hasMoreElements() == sal_False)
        throw (container::NoSuchElementException() );
    uno::Any aElement;
    //aElement <<= uno::Reference < package::ZipEntry > (static_cast <package::ZipEntry > (xZipList[nCurrent++]));
    aElement <<= xZipList[nCurrent++];
    return aElement;
}
