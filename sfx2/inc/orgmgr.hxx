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
#ifndef _SFX_ORGMGR_HXX
#define _SFX_ORGMGR_HXX

#include <o3tl/sorted_vector.hxx>

class SfxDocumentTemplates;
class SfxOrganizeListBox_Impl;
struct _FileListEntry;

//=========================================================================

class SfxObjectList : public o3tl::sorted_vector<_FileListEntry*, o3tl::less_ptr_to<_FileListEntry> >
{
public:
    SfxObjectList();
    ~SfxObjectList();

    const String& GetBaseName( const _FileListEntry* p ) const;
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
    sal_Bool                        bDeleteTemplates    :1;
    sal_Bool                        bModified           :1;

public:
    SfxOrganizeMgr(SfxDocumentTemplates* pTempl = NULL);
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
