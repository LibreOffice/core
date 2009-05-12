/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: imagetype.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef __FRAMEWORK_UICONFIGURATION_IMAGETYPE_HXX_
#define __FRAMEWORK_UICONFIGURATION_IMAGETYPE_HXX_

#include <hash_map>
#include <rtl/ustring.hxx>

namespace framework
{

enum ImageType
{
    ImageType_Color = 0,
    ImageType_Color_Large,
    ImageType_HC,
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
