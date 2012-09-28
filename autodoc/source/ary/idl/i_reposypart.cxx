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
#include "i_reposypart.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_namelookup.hxx>
#include <idl_internalgate.hxx>
#include "ia_ce.hxx"
#include "ia_type.hxx"
#include "i2s_calculator.hxx"
#include "is_ce.hxx"
#include "is_type.hxx"




namespace ary
{
namespace idl
{


DYN InternalGate &
InternalGate::Create_Partition_()
{
    return *new RepositoryPartition;
}



RepositoryPartition::RepositoryPartition()
    :   pCes(0),
        pTypes(0),
        pNamesDictionary(new NameLookup)
{
    pTypes = new TypeAdmin;
    pCes = new CeAdmin(*pNamesDictionary, *pTypes);
}

RepositoryPartition::~RepositoryPartition()
{
}

void
RepositoryPartition::Calculate_AllSecondaryInformation(
                            const String &      i_devman_reffilepath )
{
    // KORR_FUTURE
    //  Forward the options from here.

    SecondariesCalculator
        secalc(*pCes,*pTypes);

    secalc.CheckAllInterfaceBases();
    secalc.Connect_Types2Ces();
    secalc.Gather_CrossReferences();

    if ( NOT i_devman_reffilepath.empty() )
    {
        secalc.Make_Links2DeveloperManual(i_devman_reffilepath);
    }
}

const CePilot &
RepositoryPartition::Ces() const
{
    return *pCes;
}

const TypePilot &
RepositoryPartition::Types() const
{
    return *pTypes;
}

CePilot &
RepositoryPartition::Ces()
{
    return *pCes;
}

TypePilot &
RepositoryPartition::Types()
{
    return *pTypes;
}



}   //  namespace idl
}   //  namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
