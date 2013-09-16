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

#ifndef _COMMAND_HXX
#define _COMMAND_HXX

#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include <vector>

typedef ::std::vector< OUString > StringList;
typedef ::std::vector< OString* > ByteStringList;

class SvCommand
{
public:
    OUString    aDataBaseFile;
    StringList  aInFileList;
    OUString    aListFile;
    OUString    aSlotMapFile;
    OUString    aSfxItemFile;
    OUString    aPath;
    OUString    aTargetFile;
    OUString    aHelpIdFile;
    OUString    aCSVFile;
    OUString    aExportFile;
    OUString    aDocuFile;
    OUString    m_DepFile;
    sal_uInt32      nVerbosity;
    sal_uInt32      nFlags;

                SvCommand( int argc, char ** argv );
                ~SvCommand();
};

void Init();
class SvIdlWorkingBase;
sal_Bool ReadIdl( SvIdlWorkingBase * pDataBase, const SvCommand & rCommand );
void DeInit();

#endif // _COMMAND_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
