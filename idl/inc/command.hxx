/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: command.hxx,v $
 * $Revision: 1.4 $
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

#ifndef _COMMAND_HXX
#define _COMMAND_HXX


#include <tools/list.hxx>
#include <tools/string.hxx>

#ifndef STRING_LIST
#define STRING_LIST
DECLARE_LIST( StringList, String * )
#endif

DECLARE_LIST( ByteStringList, ByteString* )

/******************** class SvCommand ************************************/
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
    ByteString      aSrsLine;
    String      aHelpIdFile;
    String      aCSVFile;
    String      aExportFile;
    String      aDocuFile;
    UINT32      nFlags;

                SvCommand();
                SvCommand( int argc, char ** argv );
                ~SvCommand();
};

void Init();
class SvIdlWorkingBase;
BOOL ReadIdl( SvIdlWorkingBase * pDataBase, const SvCommand & rCommand );
void DeInit();

#endif // _COMMAND_HXX

