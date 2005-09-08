/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pcrcommon.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:20:51 $
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

#ifndef _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_
#define _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_

#define EDITOR_LIST_APPEND              (sal_uInt16)-1
#define EDITOR_LIST_REPLACE_EXISTING    (sal_uInt16)-1
#define EDITOR_PAGE_CURRENT             (sal_uInt16)-1

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    #define OWN_PROPERTY_ID_INTROSPECTEDOBJECT      0x0010
    #define OWN_PROPERTY_ID_INTROSPECTEDCOLLECTION  0x0011
    #define OWN_PROPERTY_ID_CURRENTPAGE             0x0012
    #define OWN_PROPERTY_ID_CONTROLCONTEXT          0x0013
    #define OWN_PROPERTY_ID_TABBINGMODEL            0x0014
    #define OWN_PROPERTY_ID_CONTEXTDOCUMENT         0x0015

    String getStandardString();

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_PCRCOMMON_HXX_

