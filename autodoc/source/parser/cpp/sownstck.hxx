/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    csv_assert( ! aStack_Namespaces.empty() );
    return *aStack_Namespaces.top();
}

inline ary::cpp::Class *
ContextForAry::
S_OwnerStack::CurClass() const
{
    return !aStack_Classes.empty()
                ?   aStack_Classes.top().first
                :   (ary::cpp::Class *) 0;
}

inline void
ContextForAry::
S_OwnerStack::SetOwner_2CurNamespace()
{
    csv_assert( !aStack_Namespaces.empty() );
    pOwner_Cur = pOwner_Namespace.MutablePtr();
    pOwner_Namespace->SetAnotherNamespace( CurNamespace() );
}

inline void
ContextForAry::
S_OwnerStack::SetOwner_2CurClass()
{
    csv_assert( !aStack_Classes.empty() );
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
    csv_assert( aStack_Namespaces.empty() );
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
    csv_assert( !aStack_Namespaces.empty()  OR io_rOpenedNamespace.Parent() == 0 );
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
    csv_assert( ! aStack_Classes.empty() );
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
    return !aStack_Classes.empty()
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
        // csv_assert( aStack_Classes.empty() );
        if ( !aStack_Classes.empty() )
            throw X_Parser(X_Parser::x_UnspecifiedSyntaxError, "", String::Null_(), 0);

        csv_assert( pCurEnum == 0 );
        aStack_Namespaces.pop();

        // csv_assert( !aStack_Namespaces.empty() );
        if( aStack_Namespaces.empty() )
            throw X_Parser(X_Parser::x_UnspecifiedSyntaxError, "", String::Null_(), 0);

    }
    SetOwner_2CurNamespace();
}

void
ContextForAry::
S_OwnerStack::CloseClass()
{
    // csv_assert( !aStack_Classes.empty() );
    if ( aStack_Classes.empty() )
          throw X_Parser(X_Parser::x_UnspecifiedSyntaxError, "", String::Null_(), 0);

    aStack_Classes.pop();
    if ( !aStack_Classes.empty() )
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
    if ( !aStack_Classes.empty() )
        SetOwner_2CurClass();
    else
        SetOwner_2CurNamespace();
}


}   // namespace cpp


#endif

