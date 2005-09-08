/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imagetype.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:39:43 $
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

#ifndef __FRAMEWORK_UICONFIGURATION_IMAGETYPE_HXX_
#define __FRAMEWORK_UICONFIGURATION_IMAGETYPE_HXX_

#include <hash_map>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace framework
{

enum ImageType
{
    ImageType_Color = 0,
    ImageType_HC,
    ImageType_Color_Large,
    ImageType_HC_Large,
    ImageType_COUNT
};

typedef std::hash_map< rtl::OUString,
                       rtl::OUString,
                       OUStringHashCode,
                       ::std::equal_to< ::rtl::OUString > > CommandToImageNameMap;
typedef std::hash_map< rtl::OUString,
                       bool,
                       OUStringHashCode,
                       ::std::equal_to< ::rtl::OUString > > CommandMap;

}

#endif // __FRAMEWORK_UICONFIGURATION_IMAGETYPE_HXX_
