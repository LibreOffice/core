/*************************************************************************
 *
 *  $RCSfile: reposy.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:21:54 $
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

//  VERSION:            Autodoc 2.2


#include <precomp.h>
#include <reposy.hxx>


// NOT FULLY DECLARED SERVICES
#include <ary/x_ary.hxx>
#include <ary/actions.hxx>
#include <idl/i_reposypart.hxx>

    // S P L I T //

#include <store/storage.hxx>
#include <store/strg_ifc.hxx>
#include <id_gener.hxx>
#include <cpp/c_gate.hxx>
#include <loc/l_gate.hxx>



namespace ary
{

namespace
{
    static Dyn<RepositoryCenter> pTheOldInstance_(0);
}


namespace n22
{

using ::ary::Command;
using ::ary::X_Ary;

//*****************     Repository          ************//

namespace
{
    static Dyn<RepositoryCenter> pTheInstance_(0);
}

Repository &
Repository::Create_()
{
    if ( pTheInstance_ )
        throw X_Ary(X_Ary::x_MultipleRepository);

    pTheInstance_ = new RepositoryCenter;

    // KORR_FUTURE
    //   Create the Cpp repository:
    ::ary::Repository::Create_(0);

    return *pTheInstance_;
}

Repository &
Repository::The_()
{
    if ( NOT pTheInstance_ )
        throw X_Ary(X_Ary::x_MissingRepository);

    return *pTheInstance_;
}

void
Repository::Destroy_()
{
    pTheInstance_ = 0;

    // KORR_FUTURE
    //   Destroythe Cpp repository:
    ::ary::Repository::Destroy_();
}


//*****************     RepositoryCenter          ************//


RepositoryCenter::RepositoryCenter()
    :   sDisplayedName(),
        aLocation(),
#if 0       // Version 2.2
        pCppPartition(),
#endif // Version 2.2
        pIdlPartition()
{
}

RepositoryCenter::~RepositoryCenter()
{
}

void
RepositoryCenter::RunCommand_ProduceAllSecondaries()
{
    // KORR_FUTURE
}

void
RepositoryCenter::RunCommand_Statistic( ::ary::action::Statistic & io_rCommand )
{
    // KORR_FUTURE
}

void
RepositoryCenter::do_Perform( Command & io_rCommand )
{
    io_rCommand.Run(*this);
}

const String &
RepositoryCenter::inq_Name() const
{
    return sDisplayedName;
}

bool
RepositoryCenter::inq_HasIdl() const
{
    return bool(pIdlPartition);
}

bool
RepositoryCenter::inq_HasCpp() const
{
    return pTheOldInstance_->HasCpp();
}

const ::ary::idl::Gate &
RepositoryCenter::inq_Gate_Idl() const
{
    return const_cast< RepositoryCenter& >(*this).access_Gate_Idl();
}

const ::ary::cpp::DisplayGate &
RepositoryCenter::inq_Gate_Cpp() const
{
    return pTheOldInstance_->DisplayGate_Cpp();
}

::ary::idl::Gate &
RepositoryCenter::access_Gate_Idl()
{
    if (NOT pIdlPartition)
        pIdlPartition = new idl::RepositoryPartition(*this);

    return pIdlPartition->TheGate();
}

::ary::cpp::RwGate &
RepositoryCenter::access_Gate_Cpp()
{
    return pTheOldInstance_->RwGate_Cpp();
}

void
RepositoryCenter::do_Set_Name(const String & i_sName)
{
    sDisplayedName = i_sName;
    pTheOldInstance_->Set_Name(i_sName);
}



#if 0       // Version 2.2
/*
cpp::Gate &
RepositoryCenter::access_Gate_Cpp()
{
    csv_assert( pCppPartition );
    return pCppPartition->TheGate();
}
const cpp::Gate &
RepositoryCenter::inq_Gate_Cpp() const
{
    csv_assert( pCppPartition );
    return pCppPartition->TheGate();
}
*/
#endif    // Version 2.2


}   // namespace n22



/*  ClassType-Ids
    -------------


    cpp                 1000
    idl                 2000
    corba               3000
    java                4000
    information         5000
    logic location      6000
    phys location       7000
    sec. prod.          8000


    cpp
    ---
    Namespace           1000
    Class               1001
    Enum                1002
    Typedef             1003
    Function            1004
    Variable            1005
    EnumValue           1006
    NamespaceAlias      1007

    BuiltInType         1200
    CeType_Final        1201
    CeType_Extern       1202
    PtrType             1211
    RefType             1212
    ConstType           1221
    VolatileType        1222
    ArrayType           1230
    TemplateInstance    1235
    FunctionPtr         1240
    DataMemberPtr       1250
    OperationMemberPtr  1260

    TplParam_Type       1301
    TplParam_Value      1302

    OpSignature         1400

    Define              1601
    Macro               1602


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

    java
    ----
    Package             4000
    Interface           4001
    Class               4002




    info
    ----
    CodeInformation
        (IDL)          11002
*/











                        //      S P L I T           //





struct RepositoryCenter::CheshireCat
{
    //  DATA
    String              sName;
    Dyn<store::Storage> pStorage;
    Dyn<Storage_Ifc>    pStorage_Ifc;
    Dyn<IdGenerator>    pIdGenerator;

    Dyn<cpp::Gate>      pGate_Cpp;
    Dyn<loc::Gate>      pGate_Locations;
    bool                bHasCppContent;

                        CheshireCat(
                            DYN IdGenerator &   let_drIds );
                        ~CheshireCat();
};


Repository &
Repository::Create_( DYN IdGenerator *   let_dpIds )
{
    csv_assert( NOT pTheOldInstance_ );

    DYN IdGenerator * dpIds =
            let_dpIds != 0
                ?   let_dpIds
                :   new Std_IdGenerator;
    pTheOldInstance_ = new RepositoryCenter( *dpIds );
    return *pTheOldInstance_;
}

Repository &
Repository::The_()
{
    csv_assert( pTheOldInstance_ );
    return *pTheOldInstance_;
}

void
Repository::Destroy_()
{
    pTheOldInstance_ = 0;
}

RepositoryCenter::RepositoryCenter( DYN IdGenerator &   let_drIds )
    :   pi( new CheshireCat(let_drIds) )
{
}

RepositoryCenter::~RepositoryCenter()
{
}

bool
RepositoryCenter::HasCpp() const
{
    return pi->bHasCppContent;
}

void
RepositoryCenter::Set_Name( const String & i_name )
{
     pi->sName = i_name;
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
    pi->bHasCppContent = true;
    return *pi->pGate_Cpp;
}


RepositoryCenter::
CheshireCat::CheshireCat( DYN IdGenerator &   let_drIds )
    :   sName(),
        pStorage(0),
        pStorage_Ifc(0),
        pIdGenerator( &let_drIds ),
        pGate_Cpp(0),
        pGate_Locations(0),
        bHasCppContent(false)
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
}

RepositoryCenter::
CheshireCat::~CheshireCat()
{
}


}   // namespace ary


