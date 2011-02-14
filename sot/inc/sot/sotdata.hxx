/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _SOT_DATA_HXX
#define _SOT_DATA_HXX

/*************************************************************************
*************************************************************************/

#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif
#include "sot/sotdllapi.h"

//==================class SotData_Impl====================================

class List;
class SotFactory;
class SotFactoryList;
class SotObjectList;

struct SotData_Impl
{
    sal_uInt32              nSvObjCount;
    SotObjectList *     pObjectList;
    SotFactoryList *    pFactoryList;
    SotFactory *        pSotObjectFactory;
    SotFactory *        pSotStorageStreamFactory;
    SotFactory *        pSotStorageFactory;
    List*               pDataFlavorList;
    SotData_Impl();
};

SOT_DLLPUBLIC SotData_Impl* SOTDATA();

#endif
