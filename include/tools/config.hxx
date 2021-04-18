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
#ifndef INCLUDED_TOOLS_CONFIG_HXX
#define INCLUDED_TOOLS_CONFIG_HXX

#include <tools/toolsdllapi.h>
#include <rtl/ustring.hxx>
#include <memory>

struct ImplConfigData;
struct ImplGroupData;

class SAL_WARN_UNUSED TOOLS_DLLPUBLIC Config
{
private:
    OUString            maFileName;
    OString             maGroupName;
    std::unique_ptr<ImplConfigData> mpData;
    ImplGroupData*      mpActGroup;
    sal_uInt32          mnDataUpdateId;

    TOOLS_DLLPRIVATE bool           ImplUpdateConfig() const;
    TOOLS_DLLPRIVATE ImplGroupData* ImplGetGroup() const;

public:
                        Config( const OUString& rFileName );
                        ~Config();

    void                SetGroup(const OString& rGroup);
    const OString&      GetGroup() const { return maGroupName; }
    void                DeleteGroup(std::string_view rGroup);
    OString             GetGroupName(sal_uInt16 nGroup) const;
    sal_uInt16          GetGroupCount() const;
    bool                HasGroup(std::string_view rGroup) const;

    OString             ReadKey(const OString& rKey) const;
    OString             ReadKey(const OString& rKey, const OString& rDefault) const;
    void                WriteKey(const OString& rKey, const OString& rValue);
    void                DeleteKey(std::string_view rKey);
    OString             GetKeyName(sal_uInt16 nKey) const;
    OString             ReadKey(sal_uInt16 nKey) const;
    sal_uInt16          GetKeyCount() const;

    void                Flush();

private:
                        Config( const Config& rConfig ) = delete;
    Config&             operator = ( const Config& rConfig ) = delete;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
