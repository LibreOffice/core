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
#ifndef _SFX_MINFITEM_HXX
#define _SFX_MINFITEM_HXX

#include "sal/config.h"
#include "sfx2/dllapi.h"
#include <tools/string.hxx>
#include <svl/poolitem.hxx>
class BasicManager;

class SFX2_DLLPUBLIC SfxMacroInfoItem: public SfxPoolItem
{
    const BasicManager*     pBasicManager;
    String                  aLibName;
    String                  aModuleName;
    String                  aMethodName;
    String                  aCommentText;

public:
    TYPEINFO();
    SfxMacroInfoItem( sal_uInt16 nWhich,
                    const BasicManager* pMgr,
                    const String &rLibName,
                    const String &rModuleName,
                    const String &rMethodName,
                    const String &rComment);

    SfxMacroInfoItem( const SfxMacroInfoItem& );

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;
    virtual int             operator==( const SfxPoolItem& ) const;
    String                  GetComment() const
                                { return aCommentText; }
    void                    SetComment( const String& r )
                                { aCommentText = r; }
    String                  GetMethod() const
                                { return aMethodName; }
    void                    SetMethod( const String& r )
                                { aMethodName = r; }
    String                  GetModule() const
                                { return aModuleName; }
    void                    SetModule( const String& r )
                                { aModuleName = r; }
    String                  GetLib() const
                                { return aLibName; }
    void                    SetLib( const String& r )
                                { aLibName = r; }
    const BasicManager*     GetBasicManager() const
                            { return pBasicManager; }
    String                  GetQualifiedName() const;
};

#endif
