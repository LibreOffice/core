/*************************************************************************
 *
 *  $RCSfile: storinfo.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:56:51 $
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

#ifndef _SOT_STORINFO_HXX
#define _SOT_STORINFO_HXX

#include <tools/pstm.hxx>
#include <tools/globname.hxx>
#ifndef _TOOLS_OWNLIST_HXX
#include <tools/ownlist.hxx>
#endif

class StgDirEntry;
class SvStorageInfo
{
friend class SvStorage;
    String          aName;
    SvGlobalName    aClassName;
    ULONG           nSize;
    BOOL            bStream:1,
                    bStorage:1;
                            SvStorageInfo(){}; // Fuer SvStorage
public:
                            SvStorageInfo( const StgDirEntry& );

    const SvGlobalName &    GetClassName() const { return aClassName; }
    const String &          GetName() const { return aName; }
    BOOL                    IsStream() const { return bStream; }
    BOOL                    IsStorage() const { return bStorage; }
    ULONG                   GetSize() const { return nSize;      }
};

class SvStorageInfoList
{
    PRV_SV_DECL_OWNER_LIST(SvStorageInfoList,SvStorageInfo)
    const SvStorageInfo * Get( const String & rName );
};

ULONG ReadClipboardFormat( SvStream & rStm );
void WriteClipboardFormat( SvStream & rStm, ULONG nFormat );

#endif // _STORINFO_HXX
