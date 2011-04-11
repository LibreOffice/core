/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svl/svarray.hxx>

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

    const String&   GetBaseName( sal_uInt16 nId ) const;
    const String&   GetFileName( sal_uInt16 nId ) const;
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
    sal_Bool                        bDeleteTemplates    :1;
    sal_Bool                        bModified           :1;

    SfxOrganizeListBox_Impl*    GetOther( SfxOrganizeListBox_Impl* );

public:
    SfxOrganizeMgr( SfxOrganizeListBox_Impl* pLeft,
                    SfxOrganizeListBox_Impl* pRight,
                    SfxDocumentTemplates* pTempl = NULL );
    ~SfxOrganizeMgr();

    sal_Bool    Copy( sal_uInt16 nTargetRegion, sal_uInt16 nTargetIdx, sal_uInt16 nSourceRegion, sal_uInt16 nSourceIdx );
    sal_Bool    Move( sal_uInt16 nTargetRegion, sal_uInt16 nTargetIdx, sal_uInt16 nSourceRegion, sal_uInt16 nSourceIdx );
    sal_Bool    Delete( SfxOrganizeListBox_Impl* pCaller, sal_uInt16 nRegion, sal_uInt16 nIdx );
    sal_Bool    InsertDir( SfxOrganizeListBox_Impl* pCaller, const String& rName, sal_uInt16 nRegion );
    sal_Bool    SetName( const String& rName, sal_uInt16 nRegion, sal_uInt16 nIdx = USHRT_MAX );
    sal_Bool    CopyTo( sal_uInt16 nRegion, sal_uInt16 nIdx, const String& rName ) const;
    sal_Bool    CopyFrom( SfxOrganizeListBox_Impl* pCaller, sal_uInt16 nRegion, sal_uInt16 nIdx, String& rName );

    sal_Bool    Rescan();
    sal_Bool    InsertFile( SfxOrganizeListBox_Impl* pCaller, const String& rFileName );

    sal_Bool    IsModified() const { return bModified ? sal_True : sal_False; }

    const SfxDocumentTemplates* GetTemplates() const { return pTemplates; }
    SfxObjectList&              GetObjectList() { return *pImpl->pDocList; }
    const SfxObjectList&        GetObjectList() const { return *pImpl->pDocList; }

    SfxObjectShellRef   CreateObjectShell( sal_uInt16 nIdx );
    SfxObjectShellRef   CreateObjectShell( sal_uInt16 nRegion, sal_uInt16 nIdx );
    sal_Bool                DeleteObjectShell( sal_uInt16 );
    sal_Bool                DeleteObjectShell( sal_uInt16, sal_uInt16 );
    void                SaveAll( Window* pParent );
};

#endif // #ifndef _SFX_ORGMGR_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
