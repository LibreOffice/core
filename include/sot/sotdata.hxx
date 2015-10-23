/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SOT_SOTDATA_HXX
#define INCLUDED_SOT_SOTDATA_HXX

#include <sot/sotdllapi.h>
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#include <vector>
#include <list>


class SotFactory;
class SotObject;

typedef ::std::vector< SotFactory* > SotFactoryList;
typedef ::std::vector< css::datatransfer::DataFlavor* > tDataFlavorList;

struct SotData_Impl
{
    sal_uInt32          nSvObjCount;
    std::list<SotObject*> aObjectList;
    SotFactoryList *    pFactoryList;
    SotFactory *        pSotObjectFactory;
    SotFactory *        pSotStorageStreamFactory;
    SotFactory *        pSotStorageFactory;
    tDataFlavorList*    pDataFlavorList;
    SotData_Impl();
    ~SotData_Impl();
};

SOT_DLLPUBLIC SotData_Impl* SOTDATA();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
