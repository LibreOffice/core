/*************************************************************************
 *
 *  $RCSfile: reposy.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:20 $
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
 *  Copyright 2000 by Sun Microsystems, Inc.
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <precomp.h>
#include <reposy.hxx>


// NOT FULLY DECLARED SERVICES
#include <store/storage.hxx>
#include <store/strg_ifc.hxx>
#include <id_gener.hxx>
#include <cpp/c_gate.hxx>
#include <loc/l_gate.hxx>
#include <store/storage.hxx>
#include "../../ary_i/inc/uidl/gate_i.hxx"



namespace ary
{

namespace
{
    static Dyn<RepositoryCenter> pTheInstance_(0);
}

struct RepositoryCenter::CheshireCat
{
    //  DATA
    udmstri             sName;
    Dyn<store::Storage> pStorage;
    Dyn<Storage_Ifc>    pStorage_Ifc;
    Dyn<IdGenerator>    pIdGenerator;

    Dyn<cpp::Gate>      pGate_Cpp;
    Dyn<uidl::Gate_Impl>
                        pGate_Idl;
    Dyn<loc::Gate>      pGate_Locations;

                        CheshireCat(
                            const udmstri &     i_sName,
                            DYN IdGenerator &   let_drIds );
                        ~CheshireCat();
};


Repository &
Repository::Create_( const udmstri &     i_sName,
                     DYN IdGenerator *   let_dpIds )
{
    csv_assert( NOT pTheInstance_ );

    DYN IdGenerator * dpIds =
            let_dpIds != 0
                ?   let_dpIds
                :   new Std_IdGenerator;
    pTheInstance_ = new RepositoryCenter( i_sName, *dpIds );
    return *pTheInstance_;
}

Repository &
Repository::The_()
{
    csv_assert( pTheInstance_ );
    return *pTheInstance_;
}

void
Repository::Destroy_()
{
    pTheInstance_ = 0;
}

RepositoryCenter::RepositoryCenter( const udmstri &     i_sName,
                                    DYN IdGenerator &   let_drIds )
    :   pi( new CheshireCat(i_sName, let_drIds) )
{
}

RepositoryCenter::~RepositoryCenter()
{
}

const cpp::DisplayGate &
RepositoryCenter::inq_DisplayGate_Cpp() const
{
    return *pi->pGate_Cpp;
}

const udmstri &
RepositoryCenter::inq_Name() const
{
     return pi->sName;
}

cpp::RwGate &
RepositoryCenter::access_RwGate_Cpp()
{
    return *pi->pGate_Cpp;
}

uidl::Gate &
RepositoryCenter::access_RwGate_Idl()
{
    return *pi->pGate_Idl;
}


RepositoryCenter::
CheshireCat::CheshireCat( const udmstri &     i_sName,
                          DYN IdGenerator &   let_drIds )
    :   sName(i_sName),
        pStorage(0),
        pStorage_Ifc(0),
        pIdGenerator( &let_drIds ),
        pGate_Cpp(0),
        pGate_Idl(0),
        pGate_Locations(0)
{
    pStorage                = new store::Storage;
    pStorage_Ifc            = new Storage_Ifc( *pStorage );
    pGate_Locations         = new loc::Gate(
                                        pStorage_Ifc->Ifc_Locations(),
                                        *pIdGenerator );
    pGate_Cpp               = new cpp::Gate(
                                        *pStorage_Ifc,
                                        *pIdGenerator,
                                        *pGate_Locations );
    pGate_Idl               = new uidl::Gate_Impl;
}

RepositoryCenter::
CheshireCat::~CheshireCat()
{
}


}   // namespace ary


