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
#ifndef INCLUDED_PACKAGE_INC_HASHMAPS_HXX
#define INCLUDED_PACKAGE_INC_HASHMAPS_HXX

#include "ZipEntry.hxx"
#include <rtl/ref.hxx>
#include <unordered_map>

#include <memory>

class ZipPackageFolder;
struct ZipContentInfo;

typedef std::unordered_map < OUString,
                        ZipPackageFolder * > FolderHash;

typedef std::unordered_map < OUString,
                        std::unique_ptr<ZipContentInfo> > ContentHash;

typedef std::unordered_map < OUString,
                        ZipEntry > EntryHash;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
