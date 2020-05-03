/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <atlbase.h>
#include <stdio.h>
#include "cppuhelper/bootstrap.hxx"
#include "rtl/process.h"
#include "typelib/typedescription.hxx"

#include "com/sun/star/bridge/ModelDependent.hpp"
#include "com/sun/star/bridge/XBridgeSupplier2.hpp"
#include "com/sun/star/uno/TypeClass.hpp"
#include "com/sun/star/script/XInvocation.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include <com/sun/star/bridge/oleautomation/NamedArgument.hpp>
#include "rtl/ustring.hxx"

using namespace com::sun::star::bridge;
using namespace com::sun::star::bridge::ModelDependent;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::script;
using namespace com::sun::star::bridge::oleautomation;
using namespace cppu;


template< class T >
bool equalSequences(const Sequence<T>& seqIn, const Sequence<Any> & returned);


Reference< XMultiServiceFactory > objectFactory;


Reference<XMultiServiceFactory> getMultiServiceFactory()
{
    static Reference< XMultiServiceFactory > factory;
    if( ! objectFactory.is() )
    {
        Reference<XComponentContext> context = defaultBootstrap_InitialComponentContext();
        factory.set(context->getServiceManager(), UNO_QUERY);

    }
    return factory;
}

Reference<XInvocation> getComObject( OUString progId)
{
    HRESULT hr= S_OK;
    Reference< XInvocation > ret;
    if(  ! objectFactory.is())
    {   Reference<XMultiServiceFactory> mgr= getMultiServiceFactory();
        Reference<XInterface> xInt= mgr->createInstance(
            "com.sun.star.bridge.oleautomation.Factory");
        objectFactory.set(xInt, UNO_QUERY);
    }

    if( objectFactory.is())
    {
        Reference<XInterface> xIntAx= objectFactory->createInstance( progId.getStr());
        if( xIntAx.is() )
        {
            Reference< XInvocation > xInv( xIntAx, UNO_QUERY);
            ret= xInv;
        }
    }
    return ret;
}

Reference<XInvocation> convertComObject( IUnknown* pUnk)
{
    Reference< XMultiServiceFactory > mgr= getMultiServiceFactory();
    Reference< XInterface > xIntSupplier= mgr->createInstance("com.sun.star.bridge.OleBridgeSupplier2");
    Reference< XBridgeSupplier2 > xSuppl( xIntSupplier, UNO_QUERY);

    Any any;
    CComVariant var( pUnk);
    any <<= (sal_uIntPtr) &var;
    sal_uInt8 arId[16];
    rtl_getGlobalProcessId( arId);
    Any target= xSuppl->createBridge( any, Sequence<sal_Int8>( (sal_Int8*)arId, 16), OLE, UNO );

    Reference<XInvocation> ret;
    target>>= ret;
    return ret;
}

/*
  Parameter values contains the expected return values. The value at index 0
  correspond to parameter 0 (left - most). For parameters which are not out or
  in/out the value must be a void any.

  The number of items in value must be the
  same as the number of provided parameter during the  call on the  method.

  The parameter outArgs, indices correspond to the sequences which are
  arguments to XInvocation::invoke
 */
bool checkOutArgs(const Sequence<Any> & outArgs,
                  const Sequence<sal_Int16> & indices, const Sequence<Any> & values)
{
    if (values.getLength() != outArgs.getLength())
        return false;
    //iterate over all parameters. i represents the parameter index
    for (int i = 0; i < values.(); i++)
    {
        if (values[i].getValueType() == cppu::UnoType<void>::get())
            continue;
        //out parameter
        //Based on the parameter index find the correspondent out value
        int indexOutSeq = -1;
        for (int iIndices = indices.() - 1; iIndices >= 0; iIndices --)
        {
            if (indices[iIndices] == i)
            {
                indexOutSeq = iIndices;
                break;
            }
        }
        if (indexOutSeq == -1)
            return false;

        Any value;
        Any out;
        values[i] >>= value;
        outArgs[indexOutSeq] >>=out;
        NamedArgument naVal;
        NamedArgument naOut;
        value >>= naVal;
        out >>= naOut;
        if (values[i].getValueType() == cppu::UnoType<NamedArgument>::get())
        {
            NamedArgument inNamed;
            values[i] >>= inNamed;
            value <<= inNamed.Value;
        }
        if (value != outArgs[indexOutSeq])
            return false;
    }
    return true;
}

/* The returned sequence always contains Any elements
*/
bool equalSequences(const Any& orig, const Any& returned)
{
    if (orig.getValueTypeClass() != TypeClass_SEQUENCE)
    {
        OSL_ASSERT(0);
        return false;
    }
    TypeDescription td(orig.getValueTypeRef());
    typelib_IndirectTypeDescription * indirect_td = (typelib_IndirectTypeDescription *) td.get();

    switch (indirect_td->pType->eTypeClass)
    {
        case TypeClass_CHAR:
            {
                Sequence<sal_Unicode> seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        case TypeClass_BOOLEAN:
            {
                Sequence<sal_Bool> seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        case TypeClass_BYTE:
            {
                Sequence<sal_Int8> seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        case TypeClass_SHORT:
            {
                Sequence<sal_Int16> seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        case TypeClass_LONG:
            {
                Sequence<sal_Int32> seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        case TypeClass_FLOAT:
            {
                Sequence<float> seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        case TypeClass_DOUBLE:
            {
                Sequence<double> seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        case TypeClass_STRING:
            {
                Sequence<OUString> seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        case TypeClass_ANY:
            {
                Sequence<Any> seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        case TypeClass_SEQUENCE:
            {
                //Sequence<sal_Unicode> seq;
                //orig >>= seq;
                //Sequence<Any> seq2;
                //returned >>= seq2;
                //return equalSequences(seq, seq2);
                break;
            }
        case TypeClass_INTERFACE:
            {
                Sequence<Reference<XInvocation> > seq;
                orig >>= seq;
                Sequence<Any> seq2;
                returned >>= seq2;
                return equalSequences(seq, seq2);
            }
        default:
            return false;
    }
    return false;
}

template< class T >
bool equalSequences(const Sequence<T>& seqIn, const Sequence<Any> & seqOut)
{
    if (seqIn.getLength() != seqOut.getLength())
        return false;
    int len = seqIn.getLength();
    for (int i = 0; i < len; i++)
    {
        Any anyIn;
        anyIn <<= seqIn[i];
        Any anyOut = seqOut[i];
        if (anyIn != anyOut)
            return false;
    }

    return true;
}

void printSequence( Sequence<Any>& val)
{

//  typelib_TypeDescription* desc;
//  val.getValueTypeDescription( &desc);
//  typelib_typedescription_release( desc);

    USES_CONVERSION;
    char buff[1024];
    buff[0]=0;
    char tmpBuf[1024];
    tmpBuf[0]=0;
    sal_Int32 i;

    for( i=0; i< val.(); i++)
    {
        Any& elem= val[i];
        switch ( elem.getValueTypeClass())
        {
        case TypeClass_BYTE:
             sprintf( tmpBuf, "sal_Int8 %d \n", *(sal_Int8*)elem.getValue());
             break;
        case TypeClass_SHORT:
             sprintf( tmpBuf, "sal_Int16 %d \n", *(sal_Int16*)elem.getValue());
             break;
        case TypeClass_LONG:
             sprintf( tmpBuf, "sal_Int32 %d \n", *(sal_Int32*)elem.getValue());
             break;
        case TypeClass_DOUBLE:
             sprintf( tmpBuf, "double %f \n", *(double*)elem.getValue());
             break;
        case TypeClass_FLOAT:
             sprintf( tmpBuf, "float %f \n", *(float*)elem.getValue());
             break;
        case TypeClass_STRING:
             sprintf( tmpBuf, "%S \n", (*(OUString*)elem.getValue()).getStr());
             break;
        case TypeClass_INTERFACE:
            {
            // we assume that the interface is XInvocation of an AxTestControls.Basic component.
            Reference<XInvocation> inv;
            elem>>= inv;
            if( inv.is())
            {
                Any prpVal= inv->getValue( OUString( L"prpString"));
                sprintf( tmpBuf, "Property prpString: %S \n", (*(OUString*)prpVal.getValue()).getStr());
            }
            break;
            }
        default:break;
        }
        strcat( buff, tmpBuf);

    }

    MessageBox( NULL, A2T(buff), _T("clientTest: printing Sequence elements"), MB_OK);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
