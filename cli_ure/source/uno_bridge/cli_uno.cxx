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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_cli_ure.hxx"

#include <sal/alloca.h>
#include "rtl/ustrbuf.hxx"
#include "cli_base.h"
#include "cli_bridge.h"

namespace sr=System::Reflection;
namespace css=com::sun::star;

using ::rtl::OUStringBuffer;

namespace cli_uno
{

union largest
{
    sal_Int64 n;
    double d;
    void * p;
    uno_Any a;
};

System::Object* Bridge::call_uno(uno_Interface * pUnoI,
                      typelib_TypeDescription* member_td,
                      typelib_TypeDescriptionReference * return_type,
                      sal_Int32 nParams, typelib_MethodParameter const * pParams,
                      System::Object * args[], System::Type* argTypes[],
                      System::Object** ppExc) const
{
    // return mem
    sal_Int32 return_size = sizeof (largest);
    if ((0 != return_type) &&
        (typelib_TypeClass_STRUCT == return_type->eTypeClass ||
         typelib_TypeClass_EXCEPTION == return_type->eTypeClass))
    {
        TypeDescr return_td( return_type );
        if (return_td.get()->nSize > sizeof (largest))
            return_size = return_td.get()->nSize;
    }
    //Prepare memory that contains all converted arguments and return valuse
    //The memory block contains first pointers to the arguments which are in the same block
    // For example, 2 arguments, 1 ret.
    //
    //      | Pointer
    //      | Pointer
    //      | Return value
    //      | Arg 1
    //      | Arg 2
    //
    // If an argument is larger then union largest, such as some structures, then the pointer
    // points to an extra block of memory. The same goes for a big return value.

    char * mem = (char *)alloca(
        (nParams * sizeof (void *)) + return_size + (nParams * sizeof (largest)) );
    //array of pointers to args
    void ** uno_args = (void **)mem;
    //If an attribute is set, then uno_ret must be null, e.g void setAttribute(int )
    void * uno_ret= NULL;
    if ( !(member_td->eTypeClass == typelib_TypeClass_INTERFACE_ATTRIBUTE && nParams == 1))
        uno_ret = (mem + (nParams * sizeof (void *)));
    largest * uno_args_mem = (largest *)(mem + (nParams * sizeof (void *)) + return_size);

    OSL_ASSERT( (0 == nParams) || (nParams == args->get_Length()) );
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
                // in, in/out params
                map_to_uno(
                    uno_args[ nPos ],args[nPos] , type, false /* no assign */);
            }
            catch (...)
            {
                // cleanup uno in args
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
    // call binary uno

    (*pUnoI->pDispatcher)( pUnoI, member_td, uno_ret, uno_args, &uno_exc );

    if (0 == uno_exc)
    {
        // convert out args; destruct uno args
        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            typelib_MethodParameter const & param = pParams[ nPos ];
            typelib_TypeDescriptionReference * type = param.pTypeRef;
            if (param.bOut)
            {
                try
                {
                    map_to_cli(
                        &args[nPos], uno_args[nPos], param.pTypeRef,
                        argTypes != NULL ? argTypes[nPos] : NULL, false );
                }
                catch (...)
                {
                    // cleanup further uno args
                    for ( sal_Int32 n = nPos; n < nParams; ++n )
                    {
                        uno_type_destructData( uno_args[n], pParams[n].pTypeRef, 0 );
                    }
                    // cleanup uno return value
                    uno_type_destructData( uno_ret, return_type, 0 );
                    throw;
                }
            }
            //cleanup args
            if (typelib_TypeClass_DOUBLE < type->eTypeClass &&
                typelib_TypeClass_ENUM != type->eTypeClass) // opt
            {
                uno_type_destructData(uno_args[nPos], type, 0);
            }
        }

        if ((0 != return_type) &&
            (typelib_TypeClass_VOID != return_type->eTypeClass))
        {
            // convert uno return value
            try
            {
                System::Object* cli_ret;
                 map_to_cli(
                     &cli_ret, uno_ret, return_type, 0, false);
                 uno_type_destructData(uno_ret, return_type, 0);
                return cli_ret;
            }
            catch (...)
            {
                uno_type_destructData(uno_ret, return_type, 0);
                throw;
            }
        }
        return 0; // void return
    }
    else // exception occurred
    {
        // destruct uno in args
        for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
        {
            typelib_MethodParameter const & param = pParams[ nPos ];
            if (param.bIn)
            {
                uno_type_destructData( uno_args[ nPos ], param.pTypeRef, 0 );
            }
        }
        map_to_cli(ppExc, uno_exc_holder.pData,
                uno_exc_holder.pType, NULL, false);
        return 0;
    }
}

void Bridge::call_cli(
    System::Object* cliI,
    sr::MethodInfo* method,
    typelib_TypeDescriptionReference * return_type,
    typelib_MethodParameter * params, int nParams,
    void * uno_ret, void * uno_args [], uno_Any ** uno_exc ) const
{
    System::Object *args[]=  new System::Object*[nParams];
    for (int nPos= 0; nPos < nParams; nPos++)
    {
        typelib_MethodParameter const & param= params[nPos];
        if (param.bIn)
        {
            map_to_cli( &args[nPos],
                uno_args[nPos], param.pTypeRef, 0, false);
        }
    }
    System::Object* retInvoke= NULL;
    try
    {
        retInvoke= method->Invoke(cliI, args);
    }
    catch (sr::TargetInvocationException* e)
    {
        System::Exception* exc= e->get_InnerException();
        css::uno::TypeDescription td(mapCliType(exc->GetType()));
        // memory for exception
        std::auto_ptr< rtl_mem > memExc(rtl_mem::allocate(td.get()->nSize));
        map_to_uno(memExc.get(), exc, td.get()->pWeakRef, false);
        (*uno_exc)->pType= td.get()->pWeakRef;
        (*uno_exc)->pData= memExc.release();
        return;
    }
    catch (System::Exception* e)
    {
        OUStringBuffer buf( 128 );
        buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                             "Unexspected exception during invocation of cli object. "
                             "Original message is: \n") );
        buf.append(mapCliString(e->get_Message()));
        throw BridgeRuntimeError( buf.makeStringAndClear() );
    }

    //convert out, in/out params
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
                     // out array
            }
            catch (...)
            {
                // cleanup uno pure out
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
    // return value
    if (0 != return_type)
    {
        map_to_uno(
            uno_ret, retInvoke, return_type, false /* no assign */);
    }
    // no exception occurred
    *uno_exc = 0;
}




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
