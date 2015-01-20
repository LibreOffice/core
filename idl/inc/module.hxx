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
    OUString                aIdlFileName;
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
    virtual void        ReadAttributesSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
    virtual void        ReadContextSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;
public:
                        TYPEINFO_OVERRIDE();

    const OUString &    GetIdlFileName() const { return aIdlFileName; }
    const OString& GetModulePrefix() const { return aModulePrefix.getString(); }

    virtual bool    SetName( const OString& rName, SvIdlDataBase * = NULL  ) SAL_OVERRIDE;

    const OString& GetHelpFileName() const { return aHelpFileName.getString(); }
    const OString& GetTypeLibFileName() const { return aTypeLibFile.getString(); }

    const SvMetaAttributeMemberList & GetAttrList() const { return aAttrList; }
    const SvMetaTypeMemberList & GetTypeList() const { return aTypeList; }
    const SvMetaClassMemberList & GetClassList() const { return aClassList; }

                        SvMetaModule( const OUString & rIdlFileName,
                                      bool bImported );

    bool                FillNextName( SvGlobalName * );
    bool                IsImported() const { return bImported; }
    bool                IsModified() const { return bIsModified; }

    virtual bool        ReadSvIdl( SvIdlDataBase &, SvTokenStream & rInStm ) SAL_OVERRIDE;

    void        WriteSfx( SvIdlDataBase & rBase, SvStream & rOutStm );
};
typedef tools::SvRef<SvMetaModule> SvMetaModuleRef;

class SvMetaModuleMemberList : public SvRefMemberList<SvMetaModule *> {};


#endif // INCLUDED_IDL_INC_MODULE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
