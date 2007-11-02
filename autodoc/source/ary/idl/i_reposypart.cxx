/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_reposypart.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:46:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
InternalGate::Create_Partition_(RepositoryCenter & i_center)
{
    return *new RepositoryPartition(i_center);
}



RepositoryPartition::RepositoryPartition( RepositoryCenter & i_repository )
    :   pCenter(&i_repository),
        pCes(0),
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
