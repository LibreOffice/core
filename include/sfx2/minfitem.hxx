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
#ifndef _SFX_MINFITEM_HXX
#define _SFX_MINFITEM_HXX

#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sfx2/dllapi.h>
#include <svl/poolitem.hxx>

class BasicManager;

class SFX2_DLLPUBLIC SfxMacroInfoItem: public SfxPoolItem
{
    const BasicManager*     pBasicManager;
    OUString                aLibName;
    OUString                aModuleName;
    OUString                aMethodName;
    OUString                aCommentText;

public:
    TYPEINFO();
    SfxMacroInfoItem( sal_uInt16 nWhich,
                    const BasicManager* pMgr,
                    const OUString &rLibName,
                    const OUString &rModuleName,
                    const OUString &rMethodName,
                    const OUString &rComment);

    SfxMacroInfoItem( const SfxMacroInfoItem& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    OUString                  GetComment() const
                                { return aCommentText; }
    void                    SetComment( const OUString& r )
                                { aCommentText = r; }
    OUString                GetMethod() const
                                { return aMethodName; }
    void                    SetMethod( const OUString& r )
                                { aMethodName = r; }
    OUString                GetModule() const
                                { return aModuleName; }
    void                    SetModule( const OUString& r )
                                { aModuleName = r; }
    OUString                GetLib() const
                                { return aLibName; }
    void                    SetLib( const OUString& r )
                                { aLibName = r; }
    const BasicManager*     GetBasicManager() const
                            { return pBasicManager; }
    OUString                GetQualifiedName() const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
