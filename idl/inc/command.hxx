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

#pragma once
#if 1

#include <tools/string.hxx>
#include <rtl/ustring.hxx>
#include <vector>

typedef ::std::vector< String* > StringList;
typedef ::std::vector< rtl::OString* > ByteStringList;

class SvCommand
{
public:
    String      aDataBaseFile;
    StringList  aInFileList;
    String      aListFile;
    String      aSlotMapFile;
    String      aSfxItemFile;
    String      aODLFile;
    String      aCallingFile;
    String      aCxxFile;
    String      aHxxFile;
    String      aSrcFile;
    String      aPath;
    String      aCHeaderFile;
    String      aCSourceFile;
    String      aTargetFile;
    String      aHelpIdFile;
    String      aCSVFile;
    String      aExportFile;
    String      aDocuFile;
    ::rtl::OUString m_DepFile;
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
