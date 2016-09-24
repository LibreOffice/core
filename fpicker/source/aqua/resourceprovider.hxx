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


#ifndef INCLUDED_FPICKER_SOURCE_AQUA_RESOURCEPROVIDER_HXX
#define INCLUDED_FPICKER_SOURCE_AQUA_RESOURCEPROVIDER_HXX

#include <sal/types.h>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

#define FOLDERPICKER_TITLE            500
#define FOLDER_PICKER_DEF_DESCRIPTION 501
#define FILE_PICKER_TITLE_OPEN        502
#define FILE_PICKER_TITLE_SAVE        503
#define FILE_PICKER_FILE_TYPE         504
#define FILE_PICKER_OVERWRITE         505

class CResourceProvider_Impl;

class CResourceProvider
{
public:
    CResourceProvider( );
    ~CResourceProvider( );

    NSString* getResString( sal_Int32 aId );

private:
    CResourceProvider_Impl* m_pImpl;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
