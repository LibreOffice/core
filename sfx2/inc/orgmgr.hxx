/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: orgmgr.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:09:56 $
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
#ifndef _SFX_ORGMGR_HXX
#define _SFX_ORGMGR_HXX

#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

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


