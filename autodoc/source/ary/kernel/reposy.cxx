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
    pIdlPartition = & idl::InternalGate::Create_Partition_();
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
