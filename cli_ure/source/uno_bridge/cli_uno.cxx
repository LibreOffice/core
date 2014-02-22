/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <sal/alloca.h>
#include "rtl/ustrbuf.hxx"
#include "cli_base.h"
#include "cli_bridge.h"

namespace sr=System::Reflection;


namespace cli_uno
{

union largest
{
    sal_Int64 n;
    double d;
    void * p;
    uno_Any a;
};

System::Object^ Bridge::call_uno(uno_Interface * pUnoI,
                      typelib_TypeDescription* member_td,
                      typelib_TypeDescriptionReference * return_type,
                      sal_Int32 nParams, typelib_MethodParameter const * pParams,
                      array<System::Object^>^ args, array<System::Type^>^ argTypes,
                      System::Object^* ppExc) const
{
    
    sal_Int32 return_size = sizeof (largest);
    if ((0 != return_type) &&
        (typelib_TypeClass_STRUCT == return_type->eTypeClass ||
         typelib_TypeClass_EXCEPTION == return_type->eTypeClass))
    {
        TypeDescr return_td( return_type );
        if (return_td.get()->nSize > sizeof (largest))
            return_size = return_td.get()->nSize;
    }
    
    
    
    //
    
    
    
    
    
    //
    
    

    char * mem = (char *)alloca(
        (nParams * sizeof (void *)) + return_size + (nParams * sizeof (largest)) );
    
    void ** uno_args = (void **)mem;
    
    void * uno_ret= NULL;
    if ( !(member_td->eTypeClass == typelib_TypeClass_INTERFACE_ATTRIBUTE && nParams == 1))
        uno_ret = (mem + (nParams * sizeof (void *)));
    largest * uno_args_mem = (largest *)(mem + (nParams * sizeof (void *)) + return_size);

    OSL_ASSERT( (0 == nParams) || (nParams == args->Length) );
    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        typelib_MethodParameter const & param = pParams[ nPos ];
        typelib_TypeDescriptionReference * type = param.pTypeRef;

        uno_args[ nPos ] = &uno_args_mem[ nPos ];
        if (typelib_TypeClass_STRUCT == type->eTypeClass ||
            typelib_TypeClass_EXCEPTION == type->eTypeClass)
        {
            TypeDescr td( type );
            if (td.get()->nSize > sizeof (largest))
                uno_args[ nPos ] = alloca( td.get()->nSize );
        }

        if (param.bIn)
        {
            try
            {
                
                map_to_uno(
                    uno_args[ nPos ],args[nPos] , type, false /* no assign */);
            }
            catch (...)
            {
                
                for (sal_Int32 n = 0; n < nPos; ++n)
                {
                    typelib_MethodParameter const & param = pParams[n];
                    if (param.bIn)
                    {
                        uno_type_destructData(uno_args[n], param.pTypeRef, 0);
                    }
                }
                throw;
            }
        }
    }
    uno_Any uno_exc_holder;
    uno_Any * uno_exc = &uno_exc_holder;
    

    (*pUnoI->pDispatcher)( pUnoI, member_td, uno_ret, uno_args, &uno_exc );

    if (0 == uno_exc)
    {
        
        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            typelib_MethodParameter const & param = pParams[ nPos ];
            typelib_TypeDescriptionReference * type = param.pTypeRef;
            if (param.bOut)
            {
                try
                {
                    pin_ptr<System::Object^> ptr = &args[nPos];
                    map_to_cli(
                        ptr, uno_args[nPos], param.pTypeRef,
                        argTypes != nullptr ? argTypes[nPos] : nullptr, false );
                }
                catch (...)
                {
                    
                    for ( sal_Int32 n = nPos; n < nParams; ++n )
                    {
                        uno_type_destructData( uno_args[n], pParams[n].pTypeRef, 0 );
                    }
                    
                    uno_type_destructData( uno_ret, return_type, 0 );
                    throw;
                }
            }
            
            if (typelib_TypeClass_DOUBLE < type->eTypeClass &&
                typelib_TypeClass_ENUM != type->eTypeClass) 
            {
                uno_type_destructData(uno_args[nPos], type, 0);
            }
        }

        if ((0 != return_type) &&
            (typelib_TypeClass_VOID != return_type->eTypeClass))
        {
            
            try
            {
                System::Object^ cli_ret;
                 map_to_cli(
                     &cli_ret, uno_ret, return_type, nullptr, false);
                 uno_type_destructData(uno_ret, return_type, 0);
                return cli_ret;
            }
            catch (...)
            {
                uno_type_destructData(uno_ret, return_type, 0);
                throw;
            }
        }
        return nullptr; 
    }
    else 
    {
        
        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            typelib_MethodParameter const & param = pParams[ nPos ];
            if (param.bIn)
            {
                uno_type_destructData( uno_args[ nPos ], param.pTypeRef, 0 );
            }
        }
        map_to_cli(ppExc, uno_exc_holder.pData,
                uno_exc_holder.pType, nullptr, false);
        return nullptr;
    }
}

void Bridge::call_cli(
    System::Object^ cliI,
    sr::MethodInfo^ method,
    typelib_TypeDescriptionReference * return_type,
    typelib_MethodParameter * params, int nParams,
    void * uno_ret, void * uno_args [], uno_Any ** uno_exc ) const
{
    array<System::Object^>^ args=  gcnew array<System::Object^>(nParams);
    for (int nPos= 0; nPos < nParams; nPos++)
    {
        typelib_MethodParameter const & param= params[nPos];
        if (param.bIn)
        {
            pin_ptr<System::Object^> ptr = &args[nPos];
            map_to_cli( ptr,
                uno_args[nPos], param.pTypeRef, nullptr, false);
        }
    }
    System::Object^ retInvoke= nullptr;
    try
    {
        retInvoke= method->Invoke(cliI, args);
    }
    catch (sr::TargetInvocationException^ e)
    {
        System::Exception^ exc= e->InnerException;
        css::uno::TypeDescription td(mapCliType(exc->GetType()));
        
        std::auto_ptr< rtl_mem > memExc(rtl_mem::allocate(td.get()->nSize));
        map_to_uno(memExc.get(), exc, td.get()->pWeakRef, false);
        (*uno_exc)->pType= td.get()->pWeakRef;
        (*uno_exc)->pData= memExc.release();
        return;
    }
    catch (System::Exception^ e)
    {
        OUStringBuffer buf( 128 );
        buf.append( "Unexpected exception during invocation of cli object. "
                    "Original message is: \n" );
        buf.append(mapCliString(e->Message));
        throw BridgeRuntimeError( buf.makeStringAndClear() );
    }

    
    for (int nPos = 0; nPos < nParams; ++nPos )
    {
        typelib_MethodParameter const & param = params[ nPos ];

        if (param.bOut)
        {
            try
            {
                map_to_uno(
                    uno_args[ nPos ], args[ nPos ], param.pTypeRef,
                         sal_True == param.bIn /* assign if inout */);
                     
            }
            catch (...)
            {
                
                for ( sal_Int32 n = 0; n < nPos; ++n )
                {
                    typelib_MethodParameter const & param = params[ n ];
                    if (! param.bIn)
                        uno_type_destructData( uno_args[ n ], param.pTypeRef, 0 );
                }
                throw;
            }
        }
    }
    
    if (0 != return_type)
    {
        map_to_uno(
            uno_ret, retInvoke, return_type, false /* no assign */);
    }
    
    *uno_exc = 0;
}




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
