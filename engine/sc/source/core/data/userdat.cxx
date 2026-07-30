/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <userdat.hxx>

ScMacroInfo::ScMacroInfo() :
    SdrObjUserData(UserDataID::ID_ScMacroInfo)
{
}

ScMacroInfo::~ScMacroInfo()
{
}

std::unique_ptr<SdrObjUserData> ScMacroInfo::Clone( SdrObject* /*pObj*/ ) const
{
   return std::unique_ptr<SdrObjUserData>(new ScMacroInfo( *this ));
}

ScDrawObjData_UserData::ScDrawObjData_UserData()
: SdrObjUserData(UserDataID::ID_ScDrawObjData)
, m_aScDrawObjData()
{
}

std::unique_ptr<SdrObjUserData> ScDrawObjData_UserData::Clone( SdrObject* /*pObj*/ ) const
{
    return std::unique_ptr<SdrObjUserData>(new ScDrawObjData_UserData(*this));
}

ScDrawObjData_UserDataNonRotated::ScDrawObjData_UserDataNonRotated()
: SdrObjUserData(UserDataID::ID_ScDrawObjDataNonRotated)
, m_aScDrawObjData()
{
}

std::unique_ptr<SdrObjUserData> ScDrawObjData_UserDataNonRotated::Clone( SdrObject* /*pObj*/ ) const
{
    return std::unique_ptr<SdrObjUserData>(new ScDrawObjData_UserDataNonRotated(*this));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
