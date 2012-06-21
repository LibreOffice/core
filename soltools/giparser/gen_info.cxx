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


#include <gen_info.hxx>
#include <gi_list.hxx>


GenericInfo::GenericInfo( const Simstr &      i_sKey,
                          const Simstr &      i_sValue,
                          const Simstr &      i_sComment )
    :   sKey(i_sKey),
        sValue(i_sValue),
        sComment(i_sComment),
        dpSubList(0)
{
}

GenericInfo::GenericInfo( const GenericInfo & i_rInfo )
    :   sKey(i_rInfo.sKey),
        sValue(i_rInfo.sValue),
        sComment(i_rInfo.sComment),
        dpSubList(0)
{
    if ( i_rInfo.HasSubList() )
    {
        dpSubList = new List_GenericInfo(i_rInfo.SubList());
    }
}

GenericInfo::~GenericInfo()
{
    if ( dpSubList != 0 )
        delete dpSubList;
}

GenericInfo &
GenericInfo::operator=( const GenericInfo & i_rInfo )
{
    sKey = i_rInfo.sKey;
    sValue = i_rInfo.sValue;
    sComment = i_rInfo.sComment;

    if ( dpSubList != 0 )
        delete dpSubList;
    if ( i_rInfo.HasSubList() )
    {
        dpSubList = new List_GenericInfo(i_rInfo.SubList());
    }
    else
        dpSubList = 0;

    return *this;
}

List_GenericInfo &
GenericInfo::CreateMyList() const
{
    return * ( const_cast<GenericInfo&>(*this).dpSubList = new List_GenericInfo);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
