/*************************************************************************
 *
 *  $RCSfile: i_reposypart.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-11-01 17:12:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#include <precomp.h>
#include <idl/i_reposypart.hxx>


// NOT FULLY DEFINED SERVICES
#include <commonpart.hxx>
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
        pCePilot(),
        pTypePilot(),
        pSecondariesPilot(),
        pGate(),
        pCenter(&i_rRepository)
{
    pCePilot = new CePilot_Inst( aCeStorage );
    pTypePilot = new TypePilot_Inst( aTypeStorage, *pCePilot );
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

