/*************************************************************************
 *
 *  $RCSfile: unoconversionutilities.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: jl $ $Date: 2000-10-05 11:03:39 $
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
#ifndef _UNO_CONVERSION_UTILITIES
#define _UNO_CONVERSION_UTILITIES

#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory2.hpp>

#include <vos/mutex.hxx>
#include "ole2uno.hxx"

// classes for wrapping uno objects
#define INTERFACE_OLE_WRAPPER_IMPL      1
#define UNO_OBJECT_WRAPPER_REMOTE_OPT   2

#define INVOCATION_SERVICE L"com.sun.star.script.Invocation"


// classes for wrapping ole objects
#define IUNKNOWN_WRAPPER_IMPL           1

#define INTERFACE_ADAPTER_FACTORY  L"com.sun.star.script.InvocationAdapterFactory"
// COM or JScript objects implementing UNO interfaces have to implement this interface as well
#define SUPPORTED_INTERFACES_PROP L"_implementedInterfaces"

using namespace com::sun::star::script;
using namespace com::sun::star::beans;
using namespace vos;

namespace ole_adapter
{
extern OMutex globalWrapperMutex;
extern hash_map<sal_uInt32, sal_uInt32> AdapterToWrapperMap;
extern hash_map<sal_uInt32, sal_uInt32> WrapperToAdapterMap;

    // createUnoObjectWrapper gets a wrapper instance by calling createUnoWrapperInstance
    // and initializes it via XInitialization. The wrapper object is required to implement
    // XBridgeSupplier so that it can convert itself to IDispatch.
    // class T: Deriving class ( must implement XInterface )
template< class >
class UnoConversionUtilities
{
public:
    UnoConversionUtilities(): m_nUnoWrapperClass( INTERFACE_OLE_WRAPPER_IMPL),
                              m_nComWrapperClass( IUNKNOWN_WRAPPER_IMPL)
    {}
    UnoConversionUtilities( sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass )
        : m_nUnoWrapperClass( unoWrapperClass), m_nComWrapperClass( comWrapperClass)
    {}
    UnoConversionUtilities( Reference<XMultiServiceFactory> xFactory, sal_uInt8 unoWrapperClass, sal_uInt8 comWrapperClass )
        : m_xMultiServiceFactory( xFactory), m_nComWrapperClass( comWrapperClass), m_nUnoWrapperClass( unoWrapperClass)
    {}
    // converts only into oleautomation types, that is there is no VT_I1, VT_UI2, VT_UI4
    // a sal_Unicode character is converted into a BSTR
    sal_Bool anyToVariant(VARIANT* pVariant, const Any& rAny);
    SAFEARRAY*  createUnoSequenceWrapper(const Any& rSeq);
    IDispatch*  createUnoObjectWrapper(const Any& rObj);

    sal_Bool variantToAny(const VARIANT* pVariant, Any& rAny, sal_Bool bReduceValueRange = sal_True);
    sal_Bool variantToAny2( const VARIANTARG* pArg, Any& rAny, const Type& ptype, sal_Bool bReduceValueRange = sal_True);
    Any createOleObjectWrapper(IUnknown* pUnknown);
    Any createOleObjectWrapper(IUnknown* pUnknown, const Type& aType);


    sal_Bool convertValueObject( const VARIANTARG *var, Any& any, sal_Bool& bHandled);
    sal_Bool dispatchExObject2Sequence( const VARIANTARG* pvar, Any& anySeq, const Type& type);

    Sequence<Any> createOleArrayWrapperOfDim(SAFEARRAY* pArray, unsigned int dimCount, unsigned int actDim, long* index, VARTYPE type);
    Sequence<Any> createOleArrayWrapper(SAFEARRAY* pArray, VARTYPE type);

//  SAFEARRAY* createUnoSequenceWrapper(const Any& rSeq);

    VARTYPE mapTypeClassToVartype( TypeClass type);

    Reference< XSingleServiceFactory > getInvocationFactory();

    virtual Reference< XInterface > createUnoWrapperInstance()=0;
    virtual Reference< XInterface > createComWrapperInstance()=0;

    static sal_Bool isJScriptArray(const VARIANT* pvar);



// Attributes:
protected:
    // This member determines what class is used to convert a UNO object
    // or struct to a COM object. It is passed along to the o2u_anyToVariant
    // function in the createBridge function implementation
    sal_uInt8 m_nUnoWrapperClass;
    sal_uInt8 m_nComWrapperClass;

    // This factory is set by calling XInitialization::initialize.
    // If this ServiceManager is supplied then it is used to create all
    // necessary services.
    Reference<XMultiServiceFactory> m_xMultiServiceFactory;
    Reference<XSingleServiceFactory> m_xInvocationFactory;


};

template<class T>
Reference< XSingleServiceFactory > UnoConversionUtilities<T>::getInvocationFactory()
{
    if( m_xInvocationFactory.is() )
        return Reference< XSingleServiceFactory >( m_xInvocationFactory );

    if( m_xMultiServiceFactory.is() )
        m_xInvocationFactory= Reference<XSingleServiceFactory >( m_xMultiServiceFactory->createInstance( INVOCATION_SERVICE), UNO_QUERY);
    else
        m_xInvocationFactory= Reference<XSingleServiceFactory>( o2u_getMultiServiceFactory()->createInstance(INVOCATION_SERVICE ), UNO_QUERY);

    return m_xInvocationFactory;
}

template<class T>
sal_Bool UnoConversionUtilities<T>::variantToAny2( const VARIANTARG* pArg, Any& rAny, const Type& ptype,  sal_Bool bReduceValueRange /* = sal_True */)
{
    HRESULT hr;
    sal_Bool retVal= sal_True;

    VARIANT var;
    VariantInit( &var);
    // There is no need to support indirect values, since they're not supported by UNO
    if( FAILED(hr= VariantCopyInd( &var, const_cast<VARIANTARG*>(pArg)))) // remove VT_BYREF
        return sal_False;

    sal_Bool bHandled= sal_False;
    if( pArg->vt == VT_DISPATCH )
    {
        convertValueObject( &var, rAny, bHandled);
        if( bHandled)
            OSL_ENSURE(  rAny.getValueType() == ptype, "type in Value Object must tally with the type parameter");
    }


    if( ! bHandled)
    {
        // convert into a variant type that is the equivalent to the type
        // the sequence expects. Thus variantToAny produces the correct type
        // E.g. An Array object contains VT_I4 and the sequence expects shorts
        // than the vartype must be changed. The reason is, you can't specify the
        // type in JavaScript and the script engine determines the type beeing used.
        switch( ptype.getTypeClass())
        {
        case TypeClass_CHAR: // could be: new Array( 12, 'w', "w")
            if( pArg->vt == VT_BSTR)
            {
                if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_BSTR)))
                    rAny.setValue( (void*)V_BSTR( &var), ptype);
                else
                    retVal= sal_False;
            }
            else if( pArg->vt == VT_I4 ||
                     pArg->vt == VT_UI4 ||
                     pArg->vt == VT_I2  ||
                     pArg->vt == VT_UI2 ||
                     pArg->vt == VT_I1  ||
                     pArg->vt == VT_UI1)
            {
                if( SUCCEEDED( hr= VariantChangeType( &var, & var, 0, VT_UI2)))
                    rAny.setValue(& V_UI2( &var), ptype);
                else
                    retVal= sal_False;
            }
            break;
        case TypeClass_INTERFACE: // could also be an IUnknown
    //      if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_DISPATCH)))
    //          retVal= variantToAny(&var, rAny);
            {
            CComVariant varUnk;
            if( SUCCEEDED( hr= VariantChangeType( &varUnk, &var, 0, VT_UNKNOWN)))
            {
                rAny = createOleObjectWrapper( varUnk.punkVal, ptype);
                retVal = sal_True;
            }
            else
                retVal= sal_False;
            break;
            }
        case TypeClass_SERVICE: break;  // meta construct
        case TypeClass_STRUCT:
            {// dispatch
            CComVariant varUnk;
            if( SUCCEEDED( hr= VariantChangeType( &varUnk, &var, 0, VT_UNKNOWN)))
            {
                rAny= createOleObjectWrapper( varUnk.punkVal, ptype);
                retVal= sal_True;
            }
            else
                retVal= sal_False;
            break;
            }
        case TypeClass_TYPEDEF: break;
        case TypeClass_UNION:   break;
        case TypeClass_ENUM:
            if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_I4)))
            {
                rAny.setValue( (void*)&var.lVal, ptype);
                retVal= sal_True;
            }
            else
                retVal= sal_False;
            break;
        case TypeClass_EXCEPTION: break;
        case TypeClass_ARRAY: break;    // there's no Array at the moment
        case TypeClass_SEQUENCE:
            // There are different ways of receiving a sequence:
            // 1: JScript, VARTYPE: VT_DISPATCH
            // 2. VBScript simple arraysVT_VARIANT|VT_BYREF the referenced VARIANT contains
            //      a VT_ARRAY|  <type>
            // 3. VBSrcript multi dimensional arrays: VT_ARRAY|VT_BYREF
            if( pArg->vt == VT_DISPATCH)
            {
                retVal= dispatchExObject2Sequence( pArg, rAny, ptype);
            }
            else
                retVal= variantToAny(&var, rAny);
            break; // maybe later ( convert to VT_ARRAY| ??? )
        case TypeClass_VOID:
            rAny.setValue(NULL,Type());
            break;

        case TypeClass_ANY:     //  Any
            // There could be a JScript Array that needs special handling
            // If an Any is expected and this Any must contain a Sequence
            // then we cannot figure out what element type is required.
            // Therefore we convert to Sequence< Any >
            if( pArg->vt == VT_DISPATCH &&  isJScriptArray( pArg))
            {
                Sequence<Any> s;
                retVal= dispatchExObject2Sequence( pArg, rAny, getCppuType(&s));
            }
            else
                retVal= variantToAny( &var, rAny);
            break;
        case TypeClass_UNKNOWN: break;
        case TypeClass_BOOLEAN:         // VARIANT could be VARIANT_BOOL or other
            if(SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_BOOL)))
                retVal= variantToAny(&var, rAny);
            else
                retVal= sal_False;
            break;
        case TypeClass_STRING:      // UString
            if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_BSTR)))
                retVal= variantToAny(&var, rAny);
            else
                retVal= sal_False;
            break;
        case TypeClass_FLOAT:       // float
            if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_R4)))
                retVal= variantToAny(&var, rAny);
            else
                retVal= sal_False;
            break;
        case TypeClass_DOUBLE:      // double
            if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_R8)))
                retVal= variantToAny(&var, rAny );
            else
                retVal= sal_False;
            break;
        case TypeClass_BYTE:            // BYTE
            if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_I1)))
                retVal= variantToAny(&var, rAny);
            else
                retVal= sal_False;
            break;
        case TypeClass_SHORT:       // INT16
            if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_I2)))
                retVal= variantToAny(&var, rAny);
            else
                retVal= sal_False;
            break;
        case TypeClass_LONG:        // INT32
            if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_I4)))
                    retVal= variantToAny(&var, rAny, bReduceValueRange);
            else
                retVal= sal_False;
            break;
        case TypeClass_HYPER:   break;  // INT64
        case TypeClass_UNSIGNED_SHORT:  // UINT16
            if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_UI2)))
                retVal= variantToAny(&var, rAny);
            else
                retVal= sal_False;
            break;
        case TypeClass_UNSIGNED_LONG:   // UINT32
            if( SUCCEEDED( hr= VariantChangeType( &var, &var, 0, VT_UI4)))
                retVal= variantToAny(&var, rAny, bReduceValueRange);
            else
                retVal= sal_False;
            break;
        case TypeClass_UNSIGNED_HYPER:  break;// UINT64
        case TypeClass_MODULE:  break;      // module

        default:
            retVal= variantToAny(pArg, rAny); // hopfully delegating ;-)
            break;

        }
    }
    return retVal;
}


template<class T>
sal_Bool UnoConversionUtilities<T>::anyToVariant(VARIANT* pVariant, const Any& rAny)
{
    sal_Bool ret = sal_False;

    switch (rAny.getValueTypeClass())
    {
        case TypeClass_INTERFACE:   // XInterfaceRef
        {
            Reference<XInterface> xInt( *(XInterface**)rAny.getValue());

            V_VT(pVariant) = VT_DISPATCH;
            V_DISPATCH(pVariant) = NULL;

            if (xInt.is())
            {
                // check, wether the object provides OLE bridging or not
                Reference<XBridgeSupplier2> XBridgeSup(xInt, UNO_QUERY);
                sal_Bool bOwnService= sal_False;
                if (XBridgeSup.is())
                {
                    // check if we try to convert our own OleConverter2 service
                    // If so we would run into a recursion!!!
                    T* pT= static_cast< T* >( this);

                    Any anyXInt= pT->queryInterface( getCppuType( static_cast<Reference<XInterface>* >( 0) ));
                    Reference< XInterface > xIntThis;

                    if( (anyXInt >>= xIntThis) && xInt != xIntThis)
                    { // not our own OleConverter_Impl2
                        sal_uInt8 arId[16];
                        rtl_getGlobalProcessId( arId);
                        Any oleAny = XBridgeSup->createBridge(
                                                rAny,
                                                Sequence<sal_Int8>( (sal_Int8*)arId, 16),
                                                UNO,
                                                OLE);

                        if( oleAny.getValueType() == getCppuType( (sal_uInt32*)0))
                        {
                            VARIANT* pTmpVariant = *(VARIANT**)oleAny.getValue();

                            VariantCopy(pVariant, pTmpVariant);
                            VariantClear(pTmpVariant);
                            CoTaskMemFree(pTmpVariant);
                        }
                        else
                        {
                            XBridgeSup = Reference<XBridgeSupplier2>();
                        }
                    }
                    else
                    {// The object is our OleConverter_Impl2 !!
                        bOwnService= sal_True;
                    }
                }

                if (!XBridgeSup.is() || bOwnService )
                {
                    V_DISPATCH(pVariant)= createUnoObjectWrapper( rAny);
                }
            }
            ret = sal_True;

            break;
        }
        case TypeClass_SERVICE:     // meta construct
            break;
        case TypeClass_STRUCT:      // struct
            V_VT(pVariant) = VT_DISPATCH;
            V_DISPATCH(pVariant) = createUnoObjectWrapper(rAny);
            ret = sal_True;
            break;
        case TypeClass_TYPEDEF:         // typedef compiler construct ???
            break;
        case TypeClass_UNION:       // union
            break;
        case TypeClass_ENUM:        // enumeration
            V_VT(pVariant) = VT_I4;
            V_I4(pVariant) = *(sal_Int32*)rAny.getValue();
            ret = sal_True;
            break;
        case TypeClass_EXCEPTION:   // exception
            break;
        case TypeClass_ARRAY:       // array not implemented
            break;
        case TypeClass_SEQUENCE:        // sequence ??? SafeArray descriptor
        {
            SAFEARRAY* pArray = createUnoSequenceWrapper(rAny);
            if (pArray)
            {
                V_VT(pVariant) = VT_ARRAY | VT_VARIANT;
                V_ARRAY(pVariant) = pArray;
                ret = sal_True;
            }
            break;
        }
        case TypeClass_VOID:        // void
            VariantClear(pVariant);
            ret = sal_True;
            break;
        case TypeClass_ANY:             // Any
            break;
        case TypeClass_UNKNOWN:     // unknown type
            break;
        case TypeClass_BOOLEAN:         // BOOL
        {
            V_VT(pVariant) = VT_BOOL;
            V_BOOL( pVariant)= *(sal_Bool*) rAny.getValue() == sal_True? VARIANT_TRUE: VARIANT_FALSE;
            ret = sal_True;
            break;
        }
        case TypeClass_CHAR:// char
            {
                // Because VT_UI2 does not conform to oleautomation we convert into VT_I2 instead
//          V_VT(pVariant) = VT_BSTR;
//          sal_uInt16 _c = *(sal_uInt16*)rAny.getValue();
//          pVariant->bstrVal= SysAllocStringLen( &_c, 1);
//          ret= pVariant->bstrVal ? sal_True : sal_False;
            V_VT(pVariant) = VT_I2;
            V_I2(pVariant) = *(sal_Int16*)rAny.getValue();
            ret = sal_True;
            break;
            }
        case TypeClass_STRING:      // UString
            V_VT(pVariant) = VT_BSTR;
            V_BSTR(pVariant) = SysAllocString( OUString(*(rtl_uString**)rAny.getValue()));
            ret = sal_True;
            break;
        case TypeClass_FLOAT:       // float
            V_VT(pVariant) = VT_R4;
            V_R4(pVariant) = *(float*)rAny.getValue();
            ret = sal_True;
            break;
        case TypeClass_DOUBLE:      // double
            V_VT(pVariant) = VT_R8;
            V_R8(pVariant) = *(double*)rAny.getValue();
            ret = sal_True;
            break;
        case TypeClass_BYTE:            // BYTE
            // ole automation does not know a signed char but only unsigned char
            V_VT(pVariant) = VT_UI1;
            V_UI1(pVariant) = *(sal_uInt8*)rAny.getValue();
            ret= sal_True;
            break;
        case TypeClass_SHORT:       // INT16
            V_VT(pVariant) = VT_I2;
            V_I2(pVariant) = *(sal_Int16*)rAny.getValue();
            ret = sal_True;
            break;
        case TypeClass_LONG:        // INT32
            V_VT(pVariant) = VT_I4;
            V_I4(pVariant) = *(sal_Int32*)rAny.getValue();
            ret = sal_True;
            break;
        case TypeClass_HYPER:       // INT64
    //          V_VT(pVariant) = VT_CY;
    //          V_CY(pVariant).Lo = rAny.getINT64().p.loPart;
    //          V_CY(pVariant).Lo = rAny.getINT64().p.hiPart;
    //          ret = sal_True;
            break;
        //case TypeClass_UNSIGNED_OCTET:    // ??? not implemented
        //  V_VT(pVariant) = VT_UI1;
        //  V_UI1(pVariant) = rAny.getBYTE();
        //  ret = sal_True;
        //  break;
        case TypeClass_UNSIGNED_SHORT:  // UINT16
            V_VT(pVariant) = VT_I2;
            V_I2(pVariant) = *(sal_Int16*)rAny.getValue();
            ret = sal_True;
            break;
        case TypeClass_UNSIGNED_LONG:   // UINT32
             V_VT(pVariant) = VT_I4;
            V_I4(pVariant) = *(sal_Int32*)rAny.getValue();
            ret = sal_True;
            break;
        case TypeClass_UNSIGNED_HYPER:  // UINT64
    //          V_VT(pVariant) = VT_UI8;
    //          V_UI8(pVariant) = rAny.getUINT64();
    //          ret = sal_True;
            break;
    //  case TypeClass_UNSIGNED_INT:    // int not implemented
    //      break;
        //case TypeClass_UNSIGNED_BYTE:
        //  V_VT(pVariant) = VT_UI1;
        //  V_UI1(pVariant) = rAny.getBYTE();
        //  ret = sal_True;
        //  break;
        case TypeClass_MODULE:          // module
            break;
        default:
            break;
    }

    return ret;
}

template<class T>
SAFEARRAY*  UnoConversionUtilities<T>::createUnoSequenceWrapper(const Any& rSeq)
{
    SAFEARRAY* pArray = NULL;
    sal_uInt32 n = 0;

    if( rSeq.getValueTypeClass() == TypeClass_SEQUENCE )
    {
        uno_Sequence * punoSeq= *(uno_Sequence**) rSeq.getValue();

        typelib_TypeDescriptionReference* pSeqTypeRef= rSeq.getValueTypeRef();
        typelib_TypeDescription* pSeqType= NULL;
        TYPELIB_DANGER_GET( &pSeqType, pSeqTypeRef)
        typelib_IndirectTypeDescription * pSeqIndDec=   (typelib_IndirectTypeDescription*) pSeqType;
//      typelib_IndirectTypeDescription * pSeqDec=  (typelib_IndirectTypeDescription*)rSeq.getValueTypeDescriptionRef();

        typelib_TypeDescriptionReference * pSeqElementTypeRef= pSeqIndDec->pType;
        TYPELIB_DANGER_RELEASE( pSeqType)



        typelib_TypeDescription* pSeqElementDesc= NULL;
        TYPELIB_DANGER_GET( &pSeqElementDesc, pSeqElementTypeRef)
        sal_Int32 nElementSize= pSeqElementDesc->nSize;
        n= punoSeq->nElements;

        SAFEARRAYBOUND rgsabound[1];
        rgsabound[0].lLbound = 0;
        rgsabound[0].cElements = n;
        VARIANT oleElement;
        long safeI[1];

        pArray = SafeArrayCreate(VT_VARIANT, 1, rgsabound);

        Any unoElement;
    //      sal_uInt8 * pSeqData= (sal_uInt8*) punoSeq->pElements;
        sal_uInt8 * pSeqData= (sal_uInt8*) punoSeq->elements;

        for (sal_uInt32 i = 0; i < n; i++)
        {
            unoElement.setValue( pSeqData + i * nElementSize, pSeqElementDesc);
            VariantInit(&oleElement);

            if (anyToVariant(&oleElement, unoElement))
            {
                safeI[0] = i;
                SafeArrayPutElement(pArray, safeI, &oleElement);
            }

            VariantClear(&oleElement);
        }
        TYPELIB_DANGER_RELEASE( pSeqElementDesc)
    }

    return pArray;
}

template<class T>
IDispatch*  UnoConversionUtilities<T>::createUnoObjectWrapper(const Any& rObj)
{
    OGuard guard(globalWrapperMutex);

    IDispatch* pDispatch = NULL;
    Reference<XInvocation> xInv;
    Reference<XInterface> xInt;


    // If the UNO object is already a wrapped COM object then
    // of course the original IDispatch is returned.
    if( rObj.getValueTypeClass() == TypeClass_INTERFACE)
    {
        if( rObj >>= xInt)
        {
            // can the object provide a Wrapper on its own
            if( ! convertSelfToIDispatch( xInt, &pDispatch) )
            {
                // figure out if the UNO object is a Wrapper through global maps
                typedef hash_map<sal_uInt32,sal_uInt32>::iterator _IT;
                _IT it= AdapterToWrapperMap.find( (sal_uInt32) xInt.get());
                if( it != AdapterToWrapperMap.end() )
                {
                    Reference<XInterface> xIntWrapper( (XInterface*)it->second);
                    convertSelfToIDispatch( xIntWrapper, &pDispatch); // should always work on the objects in the map
                }
                else
                    xInv= Reference<XInvocation>( xInt, UNO_QUERY);
            }
        }
    }

    if (! pDispatch && !xInv.is())
    {
    //      static Reference<XSingleServiceFactory> xInvFactory(o2u_getMultiServiceFactory()->createInstance(INVOCATION_SERVICE),UNO_QUERY);
        Reference<XSingleServiceFactory> xInvFactory= getInvocationFactory();
        if( xInvFactory.is())
        {
            Sequence<Any> params(1);
            params.getArray()[0] = rObj;
            Reference<XInterface> xInt = xInvFactory->createInstanceWithArguments(params);
            xInv= Reference<XInvocation>(xInt, UNO_QUERY);
        }
    }

    OSL_ENSURE(xInv.is() || pDispatch, "no invocation interface");
    if( !pDispatch)
    {
        Reference< XInterface > xIntWrapper= createUnoWrapperInstance();
        if( xIntWrapper.is())
        {
            Reference< XInitialization > xInitWrapper( xIntWrapper, UNO_QUERY);
            if( xInitWrapper.is() )
            {
                VARTYPE vartype= getVarType( rObj);

                if( xInt.is())
                {
                    Any params[3];
                    params[0] <<= xInv;
                    params[1] <<= xInt;
                    params[2] <<= vartype;
                    xInitWrapper->initialize( Sequence<Any>(params, 3));
                }
                else
                {
                    Any params[2];
                    params[0] <<= xInv;
                    params[1] <<= vartype;
                    xInitWrapper->initialize( Sequence<Any>(params, 2));
                }

                // Get the IDispatch
                // A wrapper is expected to implement XBridgeSupplier in order to
                // convert itself to an IDispatch:
                IDispatch* pDisp;
                if( convertSelfToIDispatch( xIntWrapper, &pDisp))
                    pDispatch= pDisp;
            }
        }
    }

    return pDispatch;
}

template<class T>
sal_Bool UnoConversionUtilities<T>::variantToAny( const VARIANT* pVariant, Any& rAny,
                                                  sal_Bool bReduceValueRange /* = sal_True */)
{
    sal_Bool ret = sal_False;
    VARTYPE oleTypeFlags = V_VT(pVariant);

    if ((oleTypeFlags & VT_BYREF) != 0)
    {
        oleTypeFlags ^= VT_BYREF;

        if ((oleTypeFlags & VT_ARRAY) != 0)
        {
            oleTypeFlags ^= VT_ARRAY;

            Sequence<Any> unoSeq = createOleArrayWrapper(*V_ARRAYREF(pVariant), oleTypeFlags);
            rAny.setValue( &unoSeq, getCppuType( &unoSeq));

            ret = sal_True;
        }
        else
        {
            switch (oleTypeFlags) // conversion by reference
                {
                case VT_EMPTY: // jo
                    rAny.setValue(NULL, Type());
                    ret= sal_True;
                    break;
                case VT_NULL:
                    rAny.setValue(NULL, Type());
                    ret = sal_True;
                    break;
                case VT_I2:
                    rAny.setValue(V_I2REF(pVariant), getCppuType((sal_Int16*)0));
                    ret = sal_True;
                    break;
                case VT_I4:
                    rAny.setValue(V_I4REF(pVariant), getCppuType((sal_Int32*)0));
                    ret = sal_True;
                    break;
                case VT_R4:
                    rAny.setValue(V_R4REF(pVariant), getCppuType( (float*)0));
                    ret = sal_True;
                    break;
                case VT_R8:
                    rAny.setValue(V_R8REF(pVariant), getCppuType( (double*)0));
                    ret = sal_True;
                    break;
                case VT_CY:
        //          rAny.setHyper(*V_CYREF(pVariant));
        //          ret = sal_True;
                    break;
                case VT_DATE:
                    break;
                case VT_BSTR:
                {
                    OUString s(*V_BSTRREF(pVariant));
                    rAny.setValue( &s, getCppuType( &s));
                    ret = sal_True;
                    break;
                }
                case VT_DISPATCH:
                {
                    IUnknown* pUnknown = NULL;

                    if ((*V_DISPATCHREF(pVariant)) != NULL)
                    {
                        (*V_DISPATCHREF(pVariant))->QueryInterface(IID_IUnknown, (void**)&pUnknown);
                    }

                    rAny = createOleObjectWrapper(pUnknown);

                    if (pUnknown != NULL)
                    {
                        pUnknown->Release();
                    }

                    ret = sal_True;
                    break;
                }
                case VT_ERROR:
                    rAny.setValue(V_ERRORREF(pVariant), getCppuType( (sal_uInt32*)0));
                    ret = sal_True;
                    break;
                case VT_BOOL:
                {
                    sal_Bool b= (*V_BOOLREF(pVariant)) == VARIANT_TRUE;
                    rAny.setValue( &b, getCppuType( &b));
                    ret = sal_True;
                    break;
                }
                case VT_VARIANT:
                    {
                        if (V_VT(V_VARIANTREF(pVariant)) == VT_EMPTY)
                        {
                            rAny.setValue(NULL, Type());
                            ret = sal_True;
                        }
                        else
                        {
                            ret = variantToAny(V_VARIANTREF(pVariant), rAny);
                        }
                    }
                    break;
                case VT_UNKNOWN:
                     rAny = createOleObjectWrapper(*V_UNKNOWNREF(pVariant));
                    ret = sal_True;
                    break;
                case VT_I1:
                    rAny.setValue(V_I1REF(pVariant), getCppuType( (sal_Int8*)0));
                    ret = sal_True;
                    break;
                case VT_UI1:
                  // convert to sal_Int8 because there is no Type for sal_uInt8
                    rAny.setValue(V_UI1REF(pVariant), getCppuType( (sal_Int8*)0));
                    ret = sal_True;
                    break;
                case VT_UI2:
                    rAny.setValue(V_UI2REF(pVariant), getCppuType( (sal_uInt16*)0));
                    ret = sal_True;
                    break;
                case VT_UI4:
                    rAny.setValue(V_UI4REF(pVariant), getCppuType( (sal_uInt32*)0));
                    ret = sal_True;
                    break;
                case VT_INT:
                    rAny.setValue(V_INTREF(pVariant), getCppuType( (sal_Int32*)0));
                    ret = sal_True;
                    break;
                case VT_UINT:
                    rAny.setValue(V_UINTREF(pVariant), getCppuType((sal_uInt32*)0));
                    ret = sal_True;
                    break;
                case VT_VOID:
                    rAny.setValue( NULL, Type());
                    ret = sal_True;
                    break;
                default:
                    break;
            }
        }
    }
    else
    {
        if ((oleTypeFlags & VT_ARRAY) > 0)
        {
            oleTypeFlags ^= VT_ARRAY;

            Sequence<Any> unoSeq = createOleArrayWrapper(V_ARRAY(pVariant), oleTypeFlags);
            rAny.setValue( &unoSeq, getCppuType( &unoSeq));

            ret = sal_True;
        }
        else
        {
            switch (oleTypeFlags)   // conversion by value
            {
                case VT_EMPTY: // jo
                    rAny.setValue(NULL, Type());
                    ret= sal_True;
                    break;

                case VT_NULL:
                    rAny.setValue(NULL, Type());
                    ret = sal_True;
                    break;
                case VT_I2:
                    rAny.setValue(&V_I2(pVariant), getCppuType( (sal_Int16*)0));
                    ret = sal_True;
                    break;
                case VT_I4:
                    rAny.setValue(&V_I4(pVariant), getCppuType( (sal_Int32*)0));
                    // necessary for use in JavaScript ( see "reduceRange")
                    if( bReduceValueRange)
                        reduceRange( rAny);
                    ret = sal_True;
                    break;
                case VT_R4:
                    rAny.setValue(&V_R4(pVariant), getCppuType( (float*)0));
                    ret = sal_True;
                    break;
                case VT_R8:
                    rAny.setValue(&V_R8(pVariant), getCppuType( (double*)0));
                    ret = sal_True;
                    break;
                case VT_CY:
        //          rAny.setHyper(V_CY(pVariant));
        //          ret = sal_True;
                    break;
                case VT_DATE:
                    break;
                case VT_BSTR:
                {
                    OUString b( V_BSTR(pVariant));
                    rAny.setValue( &b, getCppuType( &b));
                    ret = sal_True;
                    break;
                }
                case VT_DISPATCH:
                {
                    IUnknown* pUnknown = NULL;

                    if (V_DISPATCH(pVariant) != NULL)
                    {
                        V_DISPATCH(pVariant)->QueryInterface(IID_IUnknown, (void**)&pUnknown);
                    }

                     rAny = createOleObjectWrapper(pUnknown);

                    if (pUnknown != NULL)
                    {
                        pUnknown->Release();
                    }

                    ret = sal_True;

                    break;
                }
                case VT_ERROR:
                    rAny.setValue(&V_ERROR(pVariant), getCppuType( (sal_uInt32*)0));
                    ret = sal_True;
                    break;
                case VT_BOOL:
                {
                    sal_Bool b= V_BOOL(pVariant) == VARIANT_TRUE;
                    rAny.setValue( &b, getCppuType( &b));
                    ret = sal_True;
                    break;
                }
                case VT_UNKNOWN:
                     rAny = createOleObjectWrapper(V_UNKNOWN(pVariant));
                    ret = sal_True;
                    break;
                case VT_I1:
                    rAny.setValue(&V_I1(pVariant), getCppuType((sal_Int8*)0));
                    ret = sal_True;
                    break;
                case VT_UI1: // there is no unsigned char in UNO
                    rAny.setValue((char*)&V_UI1(pVariant), getCppuType( (sal_Int8*)0));
                    ret = sal_True;
                    break;
                case VT_UI2:
                    rAny.setValue(&V_UI2(pVariant), getCppuType( (sal_uInt16*)0));
                    ret = sal_True;
                    break;
                case VT_UI4:
                    rAny.setValue(&V_UI4(pVariant), getCppuType( (sal_uInt32*)0));
                    ret = sal_True;
                    break;
                case VT_INT:
                    rAny.setValue(&V_INT(pVariant), getCppuType( (sal_Int32*)0));
                    ret = sal_True;
                    break;
                case VT_UINT:
                    rAny.setValue(&V_UINT(pVariant), getCppuType( (sal_uInt32*)0));
                    ret = sal_True;
                    break;
                case VT_VOID:
                    rAny.setValue( NULL, Type());
                    ret = sal_True;
                    break;
                default:
                    break;
            }
        }
    }

    return ret;
}

template<class T>
Any UnoConversionUtilities<T>::createOleObjectWrapper(IUnknown* pUnknown)
{
    Any ret;

    Reference<XInterface> xInt;

    if (pUnknown == NULL)
    {
        ret <<= xInt;
    }
    else
    {
        CComQIPtr<IUnoObjectWrapper> spUno( pUnknown);
        if( spUno)
        {
            Reference<XInterface> xInt;
            spUno->getOriginalUnoObject( &xInt);
            ret <<= xInt;
        }
        else
        {
            Reference<XInterface> xInt= createComWrapperInstance();
            if( xInt.is())
            {
                Reference<XInitialization> xInit( xInt, UNO_QUERY);
                if( xInit.is())
                {
                    Any param;
                    param <<= (sal_uInt32) pUnknown;
                    xInit->initialize( Sequence<Any>(&param, 1));
                }
            }
            ret<<= Reference<XInvocation>( xInt, UNO_QUERY);
        }
    }

    return ret;
}

// If the parameter "aType" has a value then the COM object ( pUnknown) is supposed to
// implement the interface described by "aType". Moreover it ( pUnknown) can implement
// several other
// UNO interfaces in which case it has to support the SUPPORTED_INTERFACES_PROP (see
// #define) property. That property contains all names of interfaces.
// "pUnknown" is wrapped by a COM wrapper object that implements XInvocation, e.g.
// IUnknownWrapper_Impl. Additionally an object of type "aType" is created by help
// of the INTERFACE_ADAPTER_FACTORY (see #define) service. The implementation of
// "aType" calls on the COM wrapper's XInvocation::invoke. If the COM object supports
// more then one UNO interfaces, as can be determined by the property
// SUPPORTED_INTERFACES_PROP, then the INTERFACE_ADAPTER_FACTORY creates an object that
// implements all these interfaces.
// This is only done if "pUnknown" is not already a UNO wrapper,
// that is it is actually NOT an UNO object that was converted to a COM object. If it is an
// UNO wrapper than the original UNO object is being extracted, queried for "aType" (if
// it is no struct) and returned.
template<class T>
Any UnoConversionUtilities<T>::createOleObjectWrapper(IUnknown* pUnknown, const Type& aType)
{
    OGuard guard( globalWrapperMutex);
    Any ret;

    Reference<XInterface> xInt;

    if (pUnknown == NULL)
    {
        if( aType.getTypeClass() == TypeClass_INTERFACE)
            ret.setValue( &xInt, aType);
        else if( aType.getTypeClass() == TypeClass_STRUCT)
            ret.setValue( NULL, aType);
    }
    else
    {
        // Check if "pUnknown" is a UNO wrapper. Then it supports IUnoObjectWrapper
        // and we extract the original UNO object.
        CComQIPtr<IUnoObjectWrapper> spUno( pUnknown);
        if( spUno)
        {
            if( aType.getTypeClass() == TypeClass_INTERFACE)
            {
                Reference<XInterface> xInt;
                spUno->getOriginalUnoObject( &xInt);
                if( xInt.is())
                {
                    ret= xInt->queryInterface( aType);
                }

            }
            else if( aType.getTypeClass() == TypeClass_STRUCT)
            {
                Any any;
                if( SUCCEEDED( spUno->getOriginalUnoStruct(&any)))
                    ret= any;
            }
        }
        else
        {
            // "pUnknown" is a real COM object.
            Reference<XInterface> xInt= createComWrapperInstance();
            if( xInt.is())
            {
                Reference<XInvocation> xInv( xInt, UNO_QUERY);

                Sequence<Type> seqTypes;

                // create the actual Interface of type "Type"
                // If the type is XInvocation than we use the COM wrapper directly.
                if( aType.getTypeName() == OUString::createFromAscii("com.sun.star.script.XInvocation"))
                {
                    ret <<= xInv;
                }
                else
                {
                    Reference< XInterface> xIntAdapterFac;
                    if( m_xMultiServiceFactory.is())
                    {
                        xIntAdapterFac= m_xMultiServiceFactory->createInstance( INTERFACE_ADAPTER_FACTORY);

                    }
                    else
                    {
                        xIntAdapterFac= o2u_getMultiServiceFactory()->createInstance( INTERFACE_ADAPTER_FACTORY);
                    }

                    // We create an adapter object that does not only implement the required type but also
                    // all types that the COM object pretends to implement. An COM object must therefore
                    // support the property "_implementedInterfaces".
                    CComDispatchDriver disp( pUnknown);
                    if( disp)
                    {
                        CComVariant var;
                        if( SUCCEEDED( disp.GetPropertyByName( SUPPORTED_INTERFACES_PROP, &var)))
                        {// we exspect an array( SafeArray or IDispatch) of Strings.
                            Any anyNames;
                            if( variantToAny2( &var, anyNames, getCppuType( (Sequence<Any>*) 0)))
                            {
                                Sequence<Any> seqAny;
                                if( anyNames >>= seqAny)
                                {
                                    seqTypes.realloc( seqAny.getLength());
                                    for( sal_Int32 i=0; i < seqAny.getLength(); i++)
                                    {
                                        OUString typeName;
                                        seqAny[i] >>= typeName;
                                        seqTypes[i]= Type( TypeClass_INTERFACE, typeName);
                                    }
                                }
                            }
                        }
                    }

                    Reference<XInterface> xIntAdapted;
                    if( seqTypes.getLength() >0)
                    {
                        Reference< XInvocationAdapterFactory2> xAdapterFac( xIntAdapterFac, UNO_QUERY);
                        if( xAdapterFac.is())
                            xIntAdapted= xAdapterFac->createAdapter( xInv, seqTypes);
                    }
                    else
                    {
                        Reference<XInvocationAdapterFactory> xAdapterFac( xIntAdapterFac, UNO_QUERY);
                        if( xAdapterFac.is())
                            xIntAdapted= xAdapterFac->createAdapter( xInv, aType);
                    }
                    if( xIntAdapted.is())
                    {
                        ret= xIntAdapted->queryInterface( aType);
                        // Put the pointer to the wrapper object and the interface pointer of the adapted interface
                        // in a global map. Thus we can determine in a call to createUnoObjectWrapper whether the UNO
                        // object is a wrapped COM object. In that case we extract the original COM object rather than
                        // creating a wrapper around the UNO object.
                        typedef hash_map<sal_uInt32,sal_uInt32>::value_type VALUE;
                        AdapterToWrapperMap.insert( VALUE( (sal_uInt32) xIntAdapted.get(), (sal_uInt32) xInt.get()));
                        WrapperToAdapterMap.insert( VALUE( (sal_uInt32) xInt.get(), (sal_uInt32) xIntAdapted.get()));

                    }
                }

                // initialize the COM wrapper ( IUnknown + Type s)
                Reference<XInitialization> xInit( xInt, UNO_QUERY);
                if( xInit.is())
                {
                    Any  params[2];
                    params[0] <<= (sal_uInt32) pUnknown;
                    if( ! seqTypes.getLength()  )
                    {
                        params[1] <<= Sequence<Type>( &aType, 1);
                    }
                    else
                    {
                        params[1] <<= seqTypes;
                    }

                    xInit->initialize( Sequence<Any>( params, 2));
                }

            }
        }
    }

    return ret;
}
// "convertValueObject" converts a JScriptValue object contained in "var" into
// an any. The type contained in the any is stipulated by a "type value" thas
// was set within the JScript script on the value object ( see JScriptValue).
template<class T>
sal_Bool UnoConversionUtilities<T>::convertValueObject( const VARIANTARG *var, Any& any, sal_Bool& bHandled)
{
    sal_Bool ret= sal_True;
    bHandled= sal_False;
    HRESULT hr= S_OK;

    if( var->vt == VT_DISPATCH)
    {
        CComPtr <IJScriptValueObject> spValue;
        VARIANT_BOOL varBool;
        CComBSTR bstrType;
        CComVariant varValue;

        if( SUCCEEDED( var->pdispVal->QueryInterface( __uuidof( IJScriptValueObject),
            reinterpret_cast<void**> (&spValue))))
        {
            // Out Parameter --------------------------------------------------
            if( SUCCEEDED( hr= spValue->IsOutParam( &varBool) )
                && varBool == VARIANT_TRUE )
            {// no conversion necessary
                bHandled= sal_True;
            }
            // In / Out Parameter ---------------------------------------------
            else if( SUCCEEDED( hr= spValue->IsInOutParam( &varBool) )
                                && varBool == VARIANT_TRUE)
            {
                if( SUCCEEDED( hr= spValue->GetValue( &bstrType, &varValue)))
                {
                    if(  variantToAny2( &varValue, any, getType( bstrType)))
                    {
                        bHandled= sal_True;
                    }
                } //IJScriptValueObject::GetValue()
            } // in/out parameter
            // In Parameter
            else
            {
                if( SUCCEEDED( hr= spValue->GetValue( &bstrType, &varValue)))
                {
                    if(  variantToAny2(&varValue, any, getType( bstrType)))
                    {
                        bHandled= sal_True;
                    }
                }
            }

        }// if IJScriptValueObject
    }// if IDispatch
    if( FAILED( hr) )
        ret= sal_False;
    return ret;
}
template<class T>
sal_Bool UnoConversionUtilities<T>::dispatchExObject2Sequence( const VARIANTARG* pvar, Any& anySeq, const Type& type)
{
    sal_Bool retVal= TRUE;

    if( pvar->vt != VT_DISPATCH) return FALSE;
    IDispatchEx* pdispEx;
    HRESULT hr;
    if( FAILED( hr= pvar->pdispVal->QueryInterface( IID_IDispatchEx,
        reinterpret_cast<void**>( &pdispEx)))) return FALSE;

    DISPID dispid;
    OUString sindex;
    DISPPARAMS param= {0,0,0,0};
    VARIANT result;
    VariantInit( & result);
    OLECHAR* sLength= L"length";

    // Get the length of the array. Can also be obtained throu GetNextDispID. The
    // method only returns DISPIDs of the array data. Their names are like "0", "1" etc.
    if( FAILED( hr= pdispEx->GetIDsOfNames(IID_NULL, &sLength , 1, LOCALE_USER_DEFAULT, &dispid)))
        return FALSE;
    if( FAILED( hr= pdispEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                        &param, &result, NULL, NULL)))
        return FALSE;
    if( FAILED( VariantChangeType( &result, &result, 0, VT_I4)))
        return FALSE;
    long length= result.lVal;
    VariantClear( &result);

    // get a few basic facts about the sequence, and reallocate:
    // create the Sequences
    // get the size of the elements
    typelib_TypeDescription *pDesc= NULL;
    type.getDescription( &pDesc);

    typelib_IndirectTypeDescription *pSeqDesc= reinterpret_cast<typelib_IndirectTypeDescription*>(pDesc);
    typelib_TypeDescriptionReference *pSeqElemDescRef= pSeqDesc->pType; // type of the Sequence' elements
    Type elemType( pSeqElemDescRef);
    _typelib_TypeDescription* pSeqElemDesc=NULL;
    TYPELIB_DANGER_GET( &pSeqElemDesc, pSeqElemDescRef)
    sal_uInt32 nelementSize= pSeqElemDesc->nSize;
    TYPELIB_DANGER_RELEASE( pSeqElemDesc)

    uno_Sequence *p_uno_Seq;
    uno_sequence_construct( &p_uno_Seq, pDesc, NULL, length, cpp_acquire);

    typelib_TypeClass typeElement= pSeqDesc->pType->eTypeClass;
    char *pArray= p_uno_Seq->elements;

    // Get All properties in the object, convert their values to the expected type and
    // put them into the passed in sequence
    for( sal_Int32 i= 0; i< length; i++)
    {
        OLECHAR* sindex =  (sal_Unicode*)(const sal_Unicode*)OUString::valueOf( i);

        if( FAILED( hr= pdispEx->GetIDsOfNames(IID_NULL, &sindex , 1, LOCALE_USER_DEFAULT, &dispid)))
        {
            retVal= FALSE;
            break;
        }
        if( FAILED( hr= pdispEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET,
                        &param, &result, NULL, NULL)))
        {
            retVal= FALSE;
            break;
        }

        // If the result is VT_DISPATCH than the Sequence's element type could be Sequence
        // Look that up in the CoreReflection to make clear.
        // That requires a recursiv conversion
        Any any;
        // Destination address within the out-Sequence "anySeq" where to copy the next converted element
        void* pDest= (void*)(pArray + (i * nelementSize));

        if( result.vt & VT_DISPATCH && typeElement == typelib_TypeClass_SEQUENCE)
        {
            if( variantToAny2( &result, any, elemType, sal_False) )
            {
               // copy the converted VARIANT, that is a Sequence to the Sequence
                uno_Sequence * p_unoSeq= *(uno_Sequence**)any.getValue();
                // just copy the pointer of the uno_Sequence
                // nelementSize should be 4 !!!!
                memcpy( pDest, &p_unoSeq, nelementSize);
                osl_incrementInterlockedCount( &p_unoSeq->nRefCount);
            }
            else
            {
                retVal= FALSE;
                break;
            }
        }
        else // Element type is no Sequence -> do one conversion
        {
            if( variantToAny2( &result, any, elemType, sal_False) )
            {

                if( typeElement == typelib_TypeClass_ANY)
                {
                    // copy the converted VARIANT to the Sequence
                    uno_type_assignData( pDest, pSeqElemDescRef , &any, pSeqElemDescRef,cpp_queryInterface,
                                    cpp_acquire, cpp_release);
                }
                else
                {
                    // type after conversion must be the element type of the sequence
                    OSL_ENSURE( (any.getValueTypeClass() == typeElement), "wrong conversion");
                    uno_type_assignData( pDest, pSeqElemDescRef,const_cast<void*>( any.getValue()), any.getValueTypeRef(),
                                    cpp_queryInterface, cpp_acquire, cpp_release);
                }
            }   // if
            else
            {
                retVal= FALSE;
                break;
            }
        } // else
        VariantClear(&result);
        if( retVal == FALSE)
            break;
    } // for
    VariantClear(&result);
    uno_Sequence **pps= &p_uno_Seq;
    anySeq.setValue( &p_uno_Seq, pDesc);
    uno_destructData( &p_uno_Seq, pDesc, cpp_release);
    typelib_typedescription_release( pDesc);
    return retVal;
}
template<class T>
Sequence<Any> UnoConversionUtilities<T>::createOleArrayWrapperOfDim(SAFEARRAY* pArray, unsigned int dimCount, unsigned int actDim, long* index, VARTYPE type)
{
    HRESULT hr= S_OK;
    long lBound;
    long uBound;
    long nCountElements;

    SafeArrayGetLBound(pArray, actDim + 1, &lBound);
    SafeArrayGetUBound(pArray, actDim + 1, &uBound);
    nCountElements= uBound - lBound +1;


    Sequence<Any>   anySeq(nCountElements);
    Any*            pUnoArray = anySeq.getArray();

    for (index[actDim] = lBound; index[actDim] < nCountElements; index[actDim]++)
    {
        if (actDim < (dimCount - 1))
        {
            Sequence<Any> element = createOleArrayWrapperOfDim(pArray, dimCount, actDim + 1, index, type);

            pUnoArray[index[actDim] - lBound].setValue(&element, getCppuType(&element));
        }
        else
        {
            VARIANT variant;

            VariantInit(&variant);

            V_VT(&variant) = type;

            switch (type)
            {
                case VT_I2:
                    SafeArrayGetElement(pArray, index, &V_I2(&variant));
                    break;
                case VT_I4:
                    SafeArrayGetElement(pArray, index, &V_I4(&variant));
                    break;
                case VT_R4:
                    SafeArrayGetElement(pArray, index, &V_R4(&variant));
                    break;
                case VT_R8:
                    SafeArrayGetElement(pArray, index, &V_R8(&variant));
                    break;
                case VT_CY:
                    SafeArrayGetElement(pArray, index, &V_CY(&variant));
                    break;
                case VT_DATE:
                    SafeArrayGetElement(pArray, index, &V_DATE(&variant));
                    break;
                case VT_BSTR:
                    hr= SafeArrayGetElement(pArray, index, &V_BSTR(&variant));
                    break;
                case VT_DISPATCH:
                    SafeArrayGetElement(pArray, index, &V_DISPATCH(&variant));
                    break;
                case VT_ERROR:
                    SafeArrayGetElement(pArray, index, &V_ERROR(&variant));
                    break;
                case VT_BOOL:
                    SafeArrayGetElement(pArray, index, &V_BOOL(&variant));
                    break;
                case VT_VARIANT:
                    SafeArrayGetElement(pArray, index, &variant);
                    break;
                case VT_UNKNOWN:
                    SafeArrayGetElement(pArray, index, &V_UNKNOWN(&variant));
                    break;
                case VT_I1:
                    SafeArrayGetElement(pArray, index, &V_I1(&variant));
                    break;
                case VT_UI1:
                    SafeArrayGetElement(pArray, index, &V_UI1(&variant));
                    break;
                case VT_UI2:
                    SafeArrayGetElement(pArray, index, &V_UI2(&variant));
                    break;
                case VT_UI4:
                    SafeArrayGetElement(pArray, index, &V_UI4(&variant));
                    break;
                default:
                    break;
            }

            variantToAny(&variant, pUnoArray[index[actDim] - lBound], sal_False);

            VariantClear(&variant);
        }
    }

    return anySeq;
}
template<class T>
Sequence<Any> UnoConversionUtilities<T>::createOleArrayWrapper(SAFEARRAY* pArray, VARTYPE type)
{
    sal_uInt32 dim = SafeArrayGetDim(pArray);

    Sequence<Any> ret;

    if (dim > 0)
    {
        long * index =  new long[dim];

        for (unsigned int i = 0; i < dim; i++)
        {
            index[i] = 0;
        }

        ret = createOleArrayWrapperOfDim(pArray, dim, 0, index, type);

        delete[] index;
    }

    return ret;
}

// If an VARIANT has the type VT_DISPATCH it can either be an JScript Array
// or some other object. This function finds out if it is such an array or
// not. Currently there's no way to make sure it's an array
// so we assume that when the object has a property "0" then it is an Array.
// An JScript has property like "0", "1", "2" etc. which represent the
// value at the corresponding index of the array
template<class T>
sal_Bool UnoConversionUtilities<T>::isJScriptArray(const VARIANT* rvar)
{
    OSL_ENSURE( rvar->vt == VT_DISPATCH, "param is not a VT_DISPATCH");
    HRESULT hr;
    OLECHAR* sindex= L"0";
    DISPID id;
    hr= rvar->pdispVal->GetIDsOfNames( IID_NULL, &sindex, 1,
        LOCALE_USER_DEFAULT, &id);

    if( SUCCEEDED ( hr) )
        return sal_True;
    return sal_False;
}

template<class T>
VARTYPE UnoConversionUtilities<T>::mapTypeClassToVartype( TypeClass type)
{
    VARTYPE ret;
    switch( type)
    {
    case TypeClass_INTERFACE: ret= VT_DISPATCH;
        break;
    case TypeClass_STRUCT: ret= VT_DISPATCH;
        break;
    case TypeClass_ENUM: ret= VT_I4;
        break;
    case TypeClass_SEQUENCE: ret= VT_ARRAY;
        break;
    case TypeClass_ANY: ret= VT_VARIANT;
        break;
    case TypeClass_BOOLEAN: ret= VT_BOOL;
        break;
    case TypeClass_CHAR: ret= VT_UI2;
        break;
    case TypeClass_STRING: ret= VT_BSTR;
        break;
    case TypeClass_FLOAT: ret= VT_R4;
        break;
    case TypeClass_DOUBLE: ret= VT_R8;
        break;
    case TypeClass_BYTE: ret= VT_I1;
        break;
    case TypeClass_SHORT: ret= VT_I2;
        break;
    case TypeClass_LONG: ret= VT_I4;
        break;
    case TypeClass_UNSIGNED_SHORT: ret= VT_UI2;
         break;
    case TypeClass_UNSIGNED_LONG: ret= VT_UI4;
        break;
    default:
        ret= VT_EMPTY;
    }
    return ret;
}


// This function tries to the change the type of a value (contained in the Any)
// to the smallest possible that can hold the value. This is actually done only
// for types of VT_I4 (see o2u_variantToAny). The reason is the following:
// JavaScript passes integer values always as VT_I4. If there is a parameter or
// property of type any then the bridge converts the any's content according
// to "o2u_variantToAny". Because the VARTYPE is VT_I4 the value would be converted
// to TypeClass_LONG. Say the method XPropertySet::setPropertyValue( string name, any value)
// would be called on an object and the property actually is of TypeClass_SHORT.
// After conversion of the VARIANT parameter the Any would contain type
// TypeClass_LONG. Because the corereflection does not cast from long to short
// the "setPropertValue" would fail as the value has not the right type.

// The corereflection does convert small integer types to bigger types.
// Therefore we can reduce the type if possible and avoid the above mentioned
// problem.

// The function is not used when elements are to be converted for Sequences.

#ifndef _REDUCE_RANGE
#define _REDUCE_RANGE
inline void reduceRange( Any& any)
{
    OSL_ASSERT( any.getValueTypeClass() == TypeClass_LONG);

    sal_Int32 value= *(sal_Int32*)any.getValue();
    if( value <= 0x7f &&  value >= -0x80)
    {// -128 bis 127
        sal_Int8 charVal= static_cast<sal_Int8>( value);
        any.setValue( &charVal, getCppuType( (sal_Int8*)0));
    }
    else if( value <= 0x7fff && value >= -0x8000)
    {// -32768 bis 32767
        sal_Int16 shortVal= static_cast<sal_Int16>( value);
        any.setValue( &shortVal, getCppuType( (sal_Int16*)0));
    }
}
#endif
} // end namespace
#endif

