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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_soltools.hxx"

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
