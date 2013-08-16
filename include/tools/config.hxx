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
#ifndef _CONFIG_HXX
#define _CONFIG_HXX

#include "tools/toolsdllapi.h"
#include <rtl/ustring.hxx>

struct ImplConfigData;
struct ImplGroupData;

class TOOLS_DLLPUBLIC Config
{
private:
    OUString            maFileName;
    OString             maGroupName;
    ImplConfigData*     mpData;
    ImplGroupData*      mpActGroup;
    sal_uIntPtr         mnDataUpdateId;
    sal_uInt16          mnLockCount;
    bool                mbPersistence;
    bool                mbDummy1;

    TOOLS_DLLPRIVATE bool           ImplUpdateConfig() const;
    TOOLS_DLLPRIVATE ImplGroupData* ImplGetGroup() const;

public:
                        Config( const OUString& rFileName );
                        ~Config();

    const OUString& GetPathName() const { return maFileName; }

    void SetGroup(const OString& rGroup);
    const OString& GetGroup() const { return maGroupName; }
    void DeleteGroup(const OString& rGroup);
    OString GetGroupName(sal_uInt16 nGroup) const;
    sal_uInt16 GetGroupCount() const;
    bool HasGroup(const OString& rGroup) const;

    OString ReadKey(const OString& rKey) const;
    OUString ReadKey(const OString& rKey, rtl_TextEncoding eEncoding) const;
    OString ReadKey(const OString& rKey, const OString& rDefault) const;
    void                WriteKey(const OString& rKey, const OString& rValue);
    void DeleteKey(const OString& rKey);
    OString GetKeyName(sal_uInt16 nKey) const;
    OString ReadKey(sal_uInt16 nKey) const;
    sal_uInt16              GetKeyCount() const;

    bool                IsLocked() const { return (mnLockCount != 0); }
    void                Flush();

    void                EnablePersistence( bool bPersistence = true )
                            { mbPersistence = bPersistence; }
    bool                IsPersistenceEnabled() const { return mbPersistence; }

private:
    TOOLS_DLLPRIVATE    Config( const Config& rConfig );
    TOOLS_DLLPRIVATE Config&            operator = ( const Config& rConfig );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
