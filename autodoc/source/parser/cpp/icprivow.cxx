/*************************************************************************
 *
 *  $RCSfile: icprivow.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:29 $
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
                                   bool                i_bIsStatic )
{
    csv_assert(pScope != 0);
    pScope->Add_LocalOperation(i_sLocalName, i_nSignature, i_nId);
}

void
Owner_Namespace::do_Add_Variable( const udmstri &     i_sLocalName,
                                  Cid                 i_nId,
                                  bool                i_bIsConst,
                                  bool                i_bIsStatic )
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
                              bool                i_bIsConst,
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
