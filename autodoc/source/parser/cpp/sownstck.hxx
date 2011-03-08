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

#ifndef ADC_CPP_SOWNSTCK_HXX
#define ADC_CPP_SOWNSTCK_HXX



// USED SERVICES
    // BASE CLASSES
#include "cxt2ary.hxx"
    // COMPONENTS
#include <ary/cpp/c_types4cpp.hxx>
#include <estack.hxx>
    // PARAMETERS
#include <ary/cpp/c_namesp.hxx>
#include <x_parse.hxx>


namespace cpp
{

using ary::cpp::E_Protection;


/** Implementation struct for cpp::ContextForAry.
*/
struct ContextForAry::S_OwnerStack
{
  public:
                        S_OwnerStack();
    void                SetGlobalNamespace(
                            ary::cpp::Namespace &
                                                io_rGlobalNamespace );
                        ~S_OwnerStack();

    void                Reset();

    void                OpenNamespace(
                            ary::cpp::Namespace &
                                                io_rOpenedNamespace );
    void                OpenExternC();
    void                OpenClass(
                            ary::cpp::Class &   io_rOpenedClass );
    void                OpenEnum(
                            ary::cpp::Enum &    io_rOpenedEnum );
    void                CloseBlock();
    void                CloseClass();
    void                CloseEnum();
    void                SetCurProtection(
                            ary::cpp::E_Protection
                                                i_eProtection );
    ary::cpp::Namespace &
                        CurNamespace() const;
    ary::cpp::Class *   CurClass() const;
    ary::cpp::Enum *    CurEnum() const;

    Owner &             CurOwner() const;
    ary::cpp::E_Protection
                        CurProtection() const;
    bool                IsExternC() const       { return nExternC > 0; }

  private:
    typedef std::pair< ary::cpp::Class*, E_Protection >     ClassEnv;
    typedef EStack< ary::cpp::Namespace* >                  Stack_Namespaces;
    typedef EStack< ClassEnv >                              Stack_Classes;
    typedef ary::cpp::InputContext::Owner                   Ifc_Owner;

    void                SetOwner_2CurNamespace();
    void                SetOwner_2CurClass();
    void                SetOwner_2None();

    // DATA
    Stack_Namespaces    aStack_Namespaces;
    Stack_Classes       aStack_Classes;
    ary::cpp::Enum *    pCurEnum;
    uintt               nExternC;               /// This is a number, for the case that extern "C" blocks are nested.

    Dyn<Owner_Namespace>
                        pOwner_Namespace;
    Dyn<Owner_Class>    pOwner_Class;
    Ifc_Owner *         pOwner_Cur;
};


// IMPLEMENTATION

/*  The implementation is in header, though not inline, because this file is included
    in cxt2ary.cxx only!
*/

inline ary::cpp::Namespace &
ContextForAry::
S_OwnerStack::CurNamespace() const
{
    csv_assert( aStack_Namespaces.size() > 0 );
    return *aStack_Namespaces.top();
}

inline ary::cpp::Class *
ContextForAry::
S_OwnerStack::CurClass() const
{
    return aStack_Classes.size() > 0
                ?   aStack_Classes.top().first
                :   (ary::cpp::Class *) 0;
}

inline void
ContextForAry::
S_OwnerStack::SetOwner_2CurNamespace()
{
    csv_assert( aStack_Namespaces.size() > 0 );
    pOwner_Cur = pOwner_Namespace.MutablePtr();
    pOwner_Namespace->SetAnotherNamespace( CurNamespace() );
}

inline void
ContextForAry::
S_OwnerStack::SetOwner_2CurClass()
{
    csv_assert( aStack_Classes.size() > 0 );
    pOwner_Cur = pOwner_Class.MutablePtr();
    pOwner_Class->SetAnotherClass( *CurClass() );
}

ContextForAry::
S_OwnerStack::S_OwnerStack()
    :   // aStack_Namespaces,
        // aStack_Classes,
        pCurEnum(0),
        nExternC(0),
        pOwner_Namespace(new Owner_Namespace),
        pOwner_Class(new Owner_Class),
        pOwner_Cur(0)
{
}

void
ContextForAry::
S_OwnerStack::Reset()
{
    while ( aStack_Namespaces.top()->Owner().IsValid() )
        aStack_Namespaces.pop();
    while ( NOT aStack_Classes.empty() )
        aStack_Classes.pop();
    pCurEnum = 0;
    nExternC = 0;
    SetOwner_2CurNamespace();
}

inline void
ContextForAry::
S_OwnerStack::SetGlobalNamespace( ary::cpp::Namespace & io_rGlobalNamespace )
{
    csv_assert( aStack_Namespaces.size() == 0 );
    aStack_Namespaces.push(&io_rGlobalNamespace);
    SetOwner_2CurNamespace();
}

ContextForAry::
S_OwnerStack::~S_OwnerStack()
{
}

inline void
ContextForAry::
S_OwnerStack::OpenNamespace( ary::cpp::Namespace & io_rOpenedNamespace )
{
    csv_assert( aStack_Namespaces.size() > 0  OR io_rOpenedNamespace.Parent() == 0 );
    aStack_Namespaces.push(&io_rOpenedNamespace);
    SetOwner_2CurNamespace();
}

inline void
ContextForAry::
S_OwnerStack::OpenExternC()
{
    ++nExternC;
//    SetOwner_2None();
}

inline void
ContextForAry::
S_OwnerStack::SetCurProtection( ary::cpp::E_Protection i_eProtection )
{
    csv_assert( aStack_Classes.size() > 0 );
    aStack_Classes.top().second = i_eProtection;
}

inline ary::cpp::Enum *
ContextForAry::
S_OwnerStack::CurEnum() const
{
    return pCurEnum;
}


inline ary::cpp::InputContext::Owner &
ContextForAry::
S_OwnerStack::CurOwner() const
{
    csv_assert( pOwner_Cur != 0 );
    return *pOwner_Cur;
}

inline E_Protection
ContextForAry::
S_OwnerStack::CurProtection() const
{
    return aStack_Classes.size() > 0
                ?   aStack_Classes.top().second
                :   ary::cpp::PROTECT_global;
}

inline void
ContextForAry::
S_OwnerStack::SetOwner_2None()
{
    pOwner_Cur = 0;
}

void
ContextForAry::
S_OwnerStack::OpenClass( ary::cpp::Class & io_rOpenedClass )
{
    E_Protection eDefaultProtection
            = io_rOpenedClass.ClassKey() == ary::cpp::CK_class
                    ?   ary::cpp::PROTECT_private
                    :   ary::cpp::PROTECT_public;
    aStack_Classes.push( ClassEnv(&io_rOpenedClass, eDefaultProtection) );
    SetOwner_2CurClass();
}

inline void
ContextForAry::
S_OwnerStack::OpenEnum( ary::cpp::Enum & io_rOpenedEnum )
{
    csv_assert( pCurEnum == 0 );
    pCurEnum = &io_rOpenedEnum;
    SetOwner_2None();
}

void
ContextForAry::
S_OwnerStack::CloseBlock()
{
    if (nExternC > 0)
    {
        --nExternC;
    }
    else
    {
        // csv_assert( aStack_Classes.size() == 0 );
        if ( aStack_Classes.size() > 0 )
            throw X_Parser(X_Parser::x_UnspecifiedSyntaxError, "", String::Null_(), 0);

        csv_assert( pCurEnum == 0 );
        aStack_Namespaces.pop();

        // csv_assert( aStack_Namespaces.size() > 0 );
        if( aStack_Namespaces.size() == 0 )
            throw X_Parser(X_Parser::x_UnspecifiedSyntaxError, "", String::Null_(), 0);

    }
    SetOwner_2CurNamespace();
}

void
ContextForAry::
S_OwnerStack::CloseClass()
{
    // csv_assert( aStack_Classes.size() > 0 );
    if ( aStack_Classes.size() == 0 )
          throw X_Parser(X_Parser::x_UnspecifiedSyntaxError, "", String::Null_(), 0);

    aStack_Classes.pop();
    if ( aStack_Classes.size() > 0 )
        SetOwner_2CurClass();
    else
        SetOwner_2CurNamespace();
}

void
ContextForAry::
S_OwnerStack::CloseEnum()
{
    csv_assert( pCurEnum != 0 );
    pCurEnum = 0;
    if ( aStack_Classes.size() > 0 )
        SetOwner_2CurClass();
    else
        SetOwner_2CurNamespace();
}


}   // namespace cpp


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
