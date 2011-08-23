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
Owner_Namespace::HasClass( const String & i_sLocalName )
{
    return pScope->Search_LocalClass(i_sLocalName).IsValid();
}

void
Owner_Namespace::do_Add_Class( const String  &     i_sLocalName,
                               Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalClass(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Enum( const String  &     i_sLocalName,
                              Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalEnum(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Typedef( const String  &     i_sLocalName,
                                 Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalTypedef(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Operation( const String  &     i_sLocalName,
                                   Cid                 i_nId,
                                   bool                )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalOperation(i_sLocalName, i_nId);
}

void
Owner_Namespace::do_Add_Variable( const String  &     i_sLocalName,
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
Owner_Namespace::inq_CeId() const
{
    csv_assert(pScope != 0);
    return pScope->CeId();
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
Owner_Class::HasClass( const String  & )
{
    return false;
}

void
Owner_Class::do_Add_Class( const String  &     i_sLocalName,
                               Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalClass(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Enum( const String  &     i_sLocalName,
                              Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalEnum(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Typedef( const String  &     i_sLocalName,
                             Cid                 i_nId )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalTypedef(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Operation( const String  &      i_sLocalName,
                               Cid                  i_nId,
                               bool                 i_bIsStatic )
{
    csv_assert(pScope != 0);
    if (i_bIsStatic)
        pScope->Add_LocalStaticOperation(i_sLocalName, i_nId);
    else
        pScope->Add_LocalOperation(i_sLocalName, i_nId);
}

void
Owner_Class::do_Add_Variable( const String  &     i_sLocalName,
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
Owner_Class::inq_CeId() const
{
    csv_assert(pScope != 0);
    return pScope->CeId();
}


}   // namespace cpp

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
