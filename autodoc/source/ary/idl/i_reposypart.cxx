/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: i_reposypart.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 16:23:53 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_autodoc.hxx"


#include <precomp.h>
#include <idl/i_reposypart.hxx>


// NOT FULLY DEFINED SERVICES
#include <commonpart.hxx>
#include <ary/idl/i_namelookup.hxx>
#include "ii_gate.hxx"
#include "ipi_ce.hxx"
#include "ipi_type.hxx"
#include "ipi_2s.hxx"
#include "is_ce.hxx"
#include "is_type.hxx"




namespace ary
{
namespace idl
{


//**************        CheshireCat     *****************//

struct RepositoryPartition::CheshireCat
{
  public:
    // LIFECYCLE
                        CheshireCat(
                            const n22::RepositoryCenter &
                                                i_rRepository );
                        ~CheshireCat();

    // DATA
    Ce_Storage          aCeStorage;
    Type_Storage        aTypeStorage;
    NameLookup          aNamesDictionary;

    Dyn<CePilot_Inst>   pCePilot;
    Dyn<TypePilot_Inst> pTypePilot;
    Dyn<SecondariesPilot_Inst>
                        pSecondariesPilot;

    Dyn<Gate_Inst>      pGate;

    const n22::RepositoryCenter *
                        pCenter;
};

RepositoryPartition::
CheshireCat::CheshireCat( const n22::RepositoryCenter & i_rRepository )
    :   aCeStorage(),
        aTypeStorage(),
        aNamesDictionary(),
        pCePilot(),
        pTypePilot(),
        pSecondariesPilot(),
        pGate(),
        pCenter(&i_rRepository)
{
    pTypePilot = new TypePilot_Inst( aTypeStorage );
    pCePilot = new CePilot_Inst( aCeStorage, aNamesDictionary, *pTypePilot );
    pTypePilot->Assign_CePilot(*pCePilot);
    pSecondariesPilot = new SecondariesPilot_Inst( aCeStorage, aTypeStorage );
    pGate = new Gate_Inst( *pCePilot, *pTypePilot, *pSecondariesPilot );
}

RepositoryPartition::
CheshireCat::~CheshireCat()
{
}


//**************        RepositoryPartition      *****************//

RepositoryPartition::RepositoryPartition( const n22::RepositoryCenter & i_rRepository )
    :   cat(new CheshireCat(i_rRepository))
{
}

RepositoryPartition::~RepositoryPartition()
{
}

const Gate &
RepositoryPartition::TheGate() const
{
    return * cat->pGate;
}


Gate &
RepositoryPartition::TheGate()
{
    return * cat->pGate;
}








}   //  namespace idl
}   //  namespace ary

