/*************************************************************************
 *
 *  $RCSfile: stgio.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:56:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _STGIO_HXX
#define _STGIO_HXX

#ifndef _STGCACHE_HXX
#include <stgcache.hxx>
#endif
#ifndef _STGELEM_HXX
#include <stgelem.hxx>
#endif
#ifndef _TOOLS_STRING_HXX
#include <tools/string.hxx>
#endif

class StgFATStrm;
class StgDataStrm;
class StgDirStrm;
class String;

enum FAT_ERROR
{
    FAT_OK,
    FAT_WRONGLENGTH,
    FAT_UNREFCHAIN,
    FAT_OVERWRITE,
    FAT_OUTOFBOUNDS,

    FAT_INMEMORYERROR,
    FAT_ONFILEERROR,
    FAT_BOTHERROR
};

struct StgLinkArg
{
    String aFile;
    ULONG nErr;
};

class StgIo : public StgCache {
    void SetupStreams();            // load all internal streams
    BOOL         bCopied;
public:
    StgIo();
   ~StgIo();
    StgHeader    aHdr;              // storage file header
    StgFATStrm*  pFAT;              // FAT stream
    StgDirStrm*  pTOC;              // TOC stream
    StgDataStrm* pDataFAT;          // small data FAT stream
    StgDataStrm* pDataStrm;         // small data stream
    short        GetDataPageSize(); // get the logical data page size
    BOOL Load();                    // load a storage file
    BOOL Init();                    // set up an empty file
    BOOL CommitAll();               // commit everything (root commit)

    static Link aErrorLink;
    static void SetErrorLink( const Link& );
    static const Link& GetErrorLink() { return aErrorLink; }
    ULONG ValidateFATs( );
};

#endif
