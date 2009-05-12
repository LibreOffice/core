/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: orgmgr.hxx,v $
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
#ifndef _SFX_ORGMGR_HXX
#define _SFX_ORGMGR_HXX

#include <svtools/svarray.hxx>

class SfxDocumentTemplates;
class SfxOrganizeListBox_Impl;
class SfxObjectShell;
struct _FileListEntry;

//=========================================================================

SV_DECL_PTRARR_SORT(_SfxObjectList, _FileListEntry*, 1, 4)

class SfxObjectList: public _SfxObjectList
{
public:
    SfxObjectList();
    ~SfxObjectList();

    const String&   GetBaseName( USHORT nId ) const;
    const String&   GetFileName( USHORT nId ) const;
};

class IntlWrapper;
struct SfxOrganizeMgr_Impl
{
    SfxObjectList*      pDocList;
    IntlWrapper*        pIntlWrapper;

    SfxOrganizeMgr_Impl() :  pDocList( NULL ), pIntlWrapper( NULL ) {}
};

//=========================================================================

class SfxOrganizeMgr
{
private:
    SfxOrganizeMgr_Impl*        pImpl;
    SfxDocumentTemplates*       pTemplates;
    SfxOrganizeListBox_Impl*    pLeftBox;
    SfxOrganizeListBox_Impl*    pRightBox;
    BOOL                        bDeleteTemplates    :1;
    BOOL                        bModified           :1;

    SfxOrganizeListBox_Impl*    GetOther( SfxOrganizeListBox_Impl* );

public:
    SfxOrganizeMgr( SfxOrganizeListBox_Impl* pLeft,
                    SfxOrganizeListBox_Impl* pRight,
                    SfxDocumentTemplates* pTempl = NULL );
    ~SfxOrganizeMgr();

    BOOL    Copy( USHORT nTargetRegion, USHORT nTargetIdx, USHORT nSourceRegion, USHORT nSourceIdx );
    BOOL    Move( USHORT nTargetRegion, USHORT nTargetIdx, USHORT nSourceRegion, USHORT nSourceIdx );
    BOOL    Delete( SfxOrganizeListBox_Impl* pCaller, USHORT nRegion, USHORT nIdx );
    BOOL    InsertDir( SfxOrganizeListBox_Impl* pCaller, const String& rName, USHORT nRegion );
    BOOL    SetName( const String& rName, USHORT nRegion, USHORT nIdx = USHRT_MAX );
    BOOL    CopyTo( USHORT nRegion, USHORT nIdx, const String& rName ) const;
    BOOL    CopyFrom( SfxOrganizeListBox_Impl* pCaller, USHORT nRegion, USHORT nIdx, String& rName );

    BOOL    Rescan();
    BOOL    InsertFile( SfxOrganizeListBox_Impl* pCaller, const String& rFileName );

    BOOL    IsModified() const { return bModified ? TRUE : FALSE; }

    const SfxDocumentTemplates* GetTemplates() const { return pTemplates; }
    SfxObjectList&              GetObjectList() { return *pImpl->pDocList; }
    const SfxObjectList&        GetObjectList() const { return *pImpl->pDocList; }

    SfxObjectShellRef   CreateObjectShell( USHORT nIdx );
    SfxObjectShellRef   CreateObjectShell( USHORT nRegion, USHORT nIdx );
    BOOL                DeleteObjectShell( USHORT );
    BOOL                DeleteObjectShell( USHORT, USHORT );
    void                SaveAll( Window* pParent );
};

#endif // #ifndef _SFX_ORGMGR_HXX


