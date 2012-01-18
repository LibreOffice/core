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

#include <precomp.h>
#include <reposy.hxx>


// NOT FULLY DECLARED SERVICES
#include <idl_internalgate.hxx>


namespace ary
{


//*****************     Repository          ************//

DYN Repository &
Repository::Create_()
{
    return *new RepositoryCenter;
}




RepositoryCenter::RepositoryCenter()
    :   sDisplayedName(),
        aLocation(),
        pIdlPartition(0)
{
    pIdlPartition = & idl::InternalGate::Create_Partition_(*this);
}

RepositoryCenter::~RepositoryCenter()
{
}

const ::ary::idl::Gate &
RepositoryCenter::Gate_Idl() const
{
    csv_assert(pIdlPartition);
    return *pIdlPartition;
}

const String &
RepositoryCenter::Title() const
{
    return sDisplayedName;
}

::ary::idl::Gate &
RepositoryCenter::Gate_Idl()
{
    csv_assert(pIdlPartition);
    return *pIdlPartition;
}

void
RepositoryCenter::Set_Title(const String & i_sName)
{
    sDisplayedName = i_sName;
}




//*********************     Repository Type Info Data       ****************//

// !!! IMPORTANT - NEVER DELETE OR CHANGE - ADDING ALLOWED



/*  ClassType-Ids
    -------------

    idl                 2000
    information         5000
    logic location      6000
    phys location       7000
    sec. prod.          8000


    idl
    ---

    Module              2000
    Interface           2001
    Function            2002
    Service             2003
    Property            2004
    Enum                2005
    EnumValue           2006
    Typedef             2007
    Struct              2008
    StructElement       2009
    Exception           2010
    ConstantGroup       2011
    Constant            2012
    Singleton           2013
    Attribute           2014
    SglIfcService       2015
    SglIfcSingleton     2016

    BuiltInType         2200
    CeType              2201
    Sequence            2202
    ExplicitType        2203
    ExplicitNameRoom    2204
    TemplateParamType   2205

    physical location
    -----------------
    Root                7000
    Directory           7030
    File                7100


    info
    ----
    CodeInformation
        (IDL)          11002
*/


}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
