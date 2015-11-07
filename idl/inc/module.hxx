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

#ifndef INCLUDED_IDL_INC_MODULE_HXX
#define INCLUDED_IDL_INC_MODULE_HXX

#include <slot.hxx>
#include <object.hxx>

struct SvNamePos
{
    SvGlobalName    aUUId;
    sal_uInt32          nStmPos;
    SvNamePos( const SvGlobalName & rName, sal_uInt32 nPos )
        : aUUId( rName )
        , nStmPos( nPos ) {}
};

class SvMetaModule : public SvMetaExtern
{
    SvMetaClassMemberList       aClassList;
    SvMetaTypeMemberList        aTypeList;
    SvMetaAttributeMemberList   aAttrList;
// browser
    SvString                aHelpFileName;
    SvString                aSlotIdFile;
    SvString                aTypeLibFile;
    SvString                aModulePrefix;

    bool                    bImported   : 1,
                            bIsModified : 1;
    SvGlobalName            aBeginName;
    SvGlobalName            aEndName;
    SvGlobalName            aNextName;
protected:
    virtual void        ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
    virtual void        ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;
public:

    virtual bool        SetName( const OString& rName, SvIdlDataBase * = nullptr  ) override;

                        SvMetaModule( bool bImported );

    bool                FillNextName( SvGlobalName * );
    bool                IsImported() const { return bImported; }

    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) override;

    void                WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
};

class SvMetaModuleMemberList : public SvRefMemberList<SvMetaModule *> {};


#endif // INCLUDED_IDL_INC_MODULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
