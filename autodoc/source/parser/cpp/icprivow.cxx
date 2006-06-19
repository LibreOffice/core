/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: icprivow.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 12:03:26 $
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
#include <icprivow.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>



namespace cpp
{



//******************          Owner_Namespace          ********************//
Owner_Namespace::Owner_Namespace()
    :   pScope(0)
{
}

void
Owner_Namespace::SetAnotherNamespace( ary::cpp::Namespace & io_rScope )
{
    pScope = &io_rScope;
}

bool
Owner_Namespace::HasClass( const udmstri & i_sLocalName )
{
    return pScope->Search_LocalClass(i_sLocalName) != 0;
}

bool
Owner_Namespace::HasOperation( const udmstri &     i_sLocalName,
                               OSid                i_nSignature )
{
    return pScope->Search_LocalOperation(i_sLocalName, i_nSignature) != 0;
}

void
Owner_Namespace::do_Add_Class( const udmstri &     i_sLocalName,
                               Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalClass(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Enum( const udmstri &     i_sLocalName,
                              Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalEnum(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Typedef( const udmstri &     i_sLocalName,
                                 Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalTypedef(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Operation( const udmstri &     i_sLocalName,
                                   OSid                i_nSignature,
                                   Cid                 i_nId,
                                   bool                )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalOperation(i_sLocalName, i_nSignature, i_nId);
}

void
Owner_Namespace::do_Add_Variable( const udmstri &     i_sLocalName,
                                  Cid                 i_nId,
                                  bool                i_bIsConst,
                                  bool                )
{
    csv_assert(pScope != 0);
    if (i_bIsConst)
        pScope->Add_LocalConstant(i_sLocalName, i_nId);
    else
        pScope->Add_LocalVariable(i_sLocalName, i_nId);
}


Cid
Owner_Namespace::inq_Id() const
{
    csv_assert(pScope != 0);
    return pScope->Id();
}


//******************            Owner_Class          ********************//

Owner_Class::Owner_Class()
    :   pScope(0)
{
}

void
Owner_Class::SetAnotherClass( ary::cpp::Class & io_rScope )
{
    pScope = &io_rScope;
}

bool
Owner_Class::HasClass( const udmstri & )
{
    return false;
}

bool
Owner_Class::HasOperation( const udmstri &     ,
                           OSid                )
{
    return false;
}


void
Owner_Class::do_Add_Class( const udmstri &     i_sLocalName,
                               Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalClass(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Enum( const udmstri &     i_sLocalName,
                              Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalEnum(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Typedef( const udmstri &     i_sLocalName,
                             Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalTypedef(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Operation( const udmstri &     i_sLocalName,
                               OSid                i_nSignature,
                               Cid                 i_nId,
                               bool                i_bIsStatic )
{
    csv_assert(pScope != 0);
    if (i_bIsStatic)
        pScope->Add_LocalStaticOperation(i_sLocalName, i_nSignature, i_nId);
    else
        pScope->Add_LocalOperation(i_sLocalName, i_nSignature, i_nId);
}

void
Owner_Class::do_Add_Variable( const udmstri &     i_sLocalName,
                              Cid                 i_nId,
                              bool                ,
                              bool                i_bIsStatic )
{
    csv_assert(pScope != 0);
    if (i_bIsStatic)
        pScope->Add_LocalStaticData(i_sLocalName, i_nId);
    else
        pScope->Add_LocalData(i_sLocalName, i_nId);
}

Cid
Owner_Class::inq_Id() const
{
    csv_assert(pScope != 0);
    return pScope->Id();
}





}   // namespace cpp
