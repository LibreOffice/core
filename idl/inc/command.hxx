/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: command.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:45:00 $
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

#ifndef _COMMAND_HXX
#define _COMMAND_HXX


#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

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

void Init( const SvCommand & rCommand );
class SvIdlWorkingBase;
BOOL ReadIdl( SvIdlWorkingBase * pDataBase, const SvCommand & rCommand );
void DeInit();

#endif // _COMMAND_HXX

