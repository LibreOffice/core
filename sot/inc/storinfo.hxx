/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: storinfo.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:37:24 $
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

#ifndef _SOT_STORINFO_HXX
#define _SOT_STORINFO_HXX

#include <tools/pstm.hxx>
#include <tools/globname.hxx>
#ifndef _TOOLS_OWNLIST_HXX
#include <tools/ownlist.hxx>
#endif
#ifndef INCLUDED_SOTDLLAPI_H
#include "sot/sotdllapi.h"
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
                            SvStorageInfo( const String& rName, ULONG nSz, BOOL bIsStorage )
                                : aName( rName )
                                , nSize( nSz )
                                , bStream( !bIsStorage )
                                , bStorage( bIsStorage )
                            {}

    const SvGlobalName &    GetClassName() const { return aClassName; }
    const String &          GetName() const { return aName; }
    BOOL                    IsStream() const { return bStream; }
    BOOL                    IsStorage() const { return bStorage; }
    ULONG                   GetSize() const { return nSize;      }
};

class SOT_DLLPUBLIC SvStorageInfoList
{
    PRV_SV_DECL_OWNER_LIST(SvStorageInfoList,SvStorageInfo)
    const SvStorageInfo * Get( const String & rName );
};

SOT_DLLPUBLIC ULONG ReadClipboardFormat( SvStream & rStm );
SOT_DLLPUBLIC void WriteClipboardFormat( SvStream & rStm, ULONG nFormat );

#endif // _STORINFO_HXX
