/*************************************************************************
 *
 *  $RCSfile: cpp2uno.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: dbo $ $Date: 2001-03-30 12:06:53 $
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

#pragma warning( disable : 4237 )
#include <list>
#include <malloc.h>
#ifndef _RTL_ALLOC_H_
#include <rtl/alloc.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

#ifndef _TYPELIB_TYPEDESCRIPTION_HXX_
#include <typelib/typedescription.hxx>
#endif
#ifndef _UNO_DATA_H_
#include <uno/data.h>
#endif
#ifndef _BRIDGES_CPP_UNO_BRIDGE_HXX_
#include <bridges/cpp_uno/bridge.hxx>
#endif
#ifndef _BRIDGES_CPP_UNO_TYPE_MISC_HXX_
#include <bridges/cpp_uno/type_misc.hxx>
#endif

#include "msci.hxx"

#ifdef DEBUG
#define TRACE(x) OSL_TRACE(x)
#else
#define TRACE(x)
#endif

using namespace std;
using namespace rtl;
using namespace osl;
using namespace com::sun::star::uno;


namespace CPPU_CURRENT_NAMESPACE
{

//==================================================================================================
static inline typelib_TypeClass cpp2uno_call(
    cppu_cppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void ** pCallStack,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    // pCallStack: ret, this, [complex return ptr], params
    char * pCppStack = (char *)(pCallStack +2);

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    if (pReturnTypeDescr)
    {
        if (cppu_isSimpleType( pReturnTypeDescr ))
        {
            pUnoReturn = pRegisterReturn; // direct way for simple types
        }
        else // complex return via ptr (pCppReturn)
        {
            pCppReturn = *(void **)pCppStack;
            pCppStack += sizeof(void *);

            pUnoReturn = (cppu_relatesToInterface( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }
    }

    // stack space
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );
    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndizes = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && cppu_isSimpleType( pParamTypeDescr )) // value
        {
            pCppArgs[nPos] = pCppStack;
            pUnoArgs[nPos] = pCppStack;
            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppStack += sizeof(sal_Int32); // extra long
            }
            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
            pCppArgs[nPos] = *(void **)pCppStack;

            if (! rParam.bIn) // is pure out
            {
                // uno out is unconstructed mem!
                pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize );
                pTempIndizes[nTempIndizes] = nPos;
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            // is in/inout
            else if (cppu_relatesToInterface( pParamTypeDescr ))
            {
                uno_copyAndConvertData( pUnoArgs[nPos] = alloca( pParamTypeDescr->nSize ),
                                        *(void **)pCppStack, pParamTypeDescr,
                                        &pThis->pBridge->aCpp2Uno );
                pTempIndizes[nTempIndizes] = nPos; // has to be reconverted
                // will be released at reconversion
                ppTempParamTypeDescr[nTempIndizes++] = pParamTypeDescr;
            }
            else // direct way
            {
                pUnoArgs[nPos] = *(void **)pCppStack;
                // no longer needed
                TYPELIB_DANGER_RELEASE( pParamTypeDescr );
            }
        }
        pCppStack += sizeof(sal_Int32); // standard parameter length
    }

    // ExceptionHolder
    uno_Any aUnoExc; // Any will be constructed by callee
    uno_Any * pUnoExc = &aUnoExc;

    // invoke uno dispatch call
    (*pThis->pUnoI->pDispatcher)( pThis->pUnoI, pMemberTypeDescr, pUnoReturn, pUnoArgs, &pUnoExc );

    // in case an exception occured...
    if (pUnoExc)
    {
        // destruct temporary in/inout params
        while (nTempIndizes--)
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndizes], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndizes] );
        }
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

        msci_raiseException( &aUnoExc, &pThis->pBridge->aUno2Cpp ); // has to destruct the any
        // is here for dummy
        return typelib_TypeClass_VOID;
    }
    else // else no exception occured...
    {
        // temporary params
        while (nTempIndizes--)
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];
            typelib_TypeDescription * pParamTypeDescr = ppTempParamTypeDescr[nTempIndizes];

            if (pParams[nIndex].bOut) // inout/out
            {
                // convert and assign
                uno_destructData( pCppArgs[nIndex], pParamTypeDescr, cpp_release );
                uno_copyAndConvertData( pCppArgs[nIndex], pUnoArgs[nIndex], pParamTypeDescr,
                                        &pThis->pBridge->aUno2Cpp );
            }
            // destroy temp uno param
            uno_destructData( pUnoArgs[nIndex], pParamTypeDescr, 0 );

            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        // return
        if (pCppReturn) // has complex return
        {
            if (pUnoReturn != pCppReturn) // needs reconversion
            {
                uno_copyAndConvertData( pCppReturn, pUnoReturn, pReturnTypeDescr,
                                        &pThis->pBridge->aUno2Cpp );
                // destroy temp uno return
                uno_destructData( pUnoReturn, pReturnTypeDescr, 0 );
            }
            // complex return ptr is set to eax
            *(void **)pRegisterReturn = pCppReturn;
        }
        if (pReturnTypeDescr)
        {
            typelib_TypeClass eRet = (typelib_TypeClass)pReturnTypeDescr->eTypeClass;
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );
            return eRet;
        }
        else
            return typelib_TypeClass_VOID;
    }
}

//==================================================================================================
static typelib_TypeClass __cdecl cpp_mediate(
    void ** pCallStack, sal_Int32 nVtableCall,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

    // pCallStack: ret adr, this, [ret *], params
    // _this_ ptr is patched cppu_XInterfaceProxy object
    cppu_cppInterfaceProxy * pThis = static_cast< cppu_cppInterfaceProxy * >(
        reinterpret_cast< XInterface * >( pCallStack[1] ) );

    typelib_InterfaceTypeDescription * pTypeDescr = pThis->pTypeDescr;
    OSL_ENSURE( nVtableCall < pTypeDescr->nMapFunctionIndexToMemberIndex,
                 "### illegal vtable index!" );
    if (nVtableCall >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException( OUString( RTL_CONSTASCII_USTRINGPARAM("illegal vtable index!") ),
                                (XInterface *)pThis );
    }

    // determine called method
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nVtableCall];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!" );

    TypeDescription aMemberDescr( pTypeDescr->ppAllMembers[nMemberPos] );

    typelib_TypeClass eRet;
    switch (aMemberDescr.get()->eTypeClass)
    {
    case typelib_TypeClass_INTERFACE_ATTRIBUTE:
    {
        if (pTypeDescr->pMapMemberIndexToFunctionIndex[nMemberPos] == nVtableCall)
        {
            // is GET method
            eRet = cpp2uno_call(
                pThis, aMemberDescr.get(),
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                0, 0, // no params
                pCallStack, pRegisterReturn );
        }
        else
        {
            // is SET method
            typelib_MethodParameter aParam;
            aParam.pTypeRef =
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef;
            aParam.bIn      = sal_True;
            aParam.bOut     = sal_False;

            eRet = cpp2uno_call(
                pThis, aMemberDescr.get(),
                0, // indicates void return
                1, &aParam,
                pCallStack, pRegisterReturn );
        }
        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        // is METHOD
        switch (nVtableCall)
        {
            // standard XInterface vtable calls
        case 1: // acquire()
            pThis->acquireProxy(); // non virtual call!
            eRet = typelib_TypeClass_VOID;
            break;
        case 2: // release()
            pThis->releaseProxy(); // non virtual call!
            eRet = typelib_TypeClass_VOID;
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( pCallStack[3] )->getTypeLibType() );
            OSL_ASSERT( pTD );

            XInterface * pInterface = 0;
            (*pThis->pBridge->pCppEnv->getRegisteredInterface)(
                pThis->pBridge->pCppEnv,
                (void **)&pInterface, pThis->oid.pData, (typelib_InterfaceTypeDescription *)pTD );

            if (pInterface)
            {
                uno_any_construct( reinterpret_cast< uno_Any * >( pCallStack[2] ),
                                   &pInterface, pTD, cpp_acquire );
                pInterface->release();
                TYPELIB_DANGER_RELEASE( pTD );
                *(void **)pRegisterReturn = pCallStack[2];
                eRet = typelib_TypeClass_ANY;
                break;
            }
            TYPELIB_DANGER_RELEASE( pTD );
        } // else perform queryInterface()
        default:
            eRet = cpp2uno_call(
                pThis, aMemberDescr.get(),
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->nParams,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pParams,
                pCallStack, pRegisterReturn );
        }
        break;
    }
    default:
    {
        throw RuntimeException(
            OUString( RTL_CONSTASCII_USTRINGPARAM("no member description found!") ),
            (XInterface *)pThis );
        // is here for dummy
        eRet = typelib_TypeClass_VOID;
    }
    }

    return eRet;
}

//==================================================================================================
class MediateVtables
{
    //----------------------------------------------------------------------------------------------
    struct DefaultRTTIEntry
    {
        sal_Int32   _n0, _n1, _n2;
        type_info * _pRTTI;

        DefaultRTTIEntry() throw ()
            : _n0( 0 )
            , _n1( 0 )
            , _n2( 0 )
            { _pRTTI = msci_getRTTI( "com.sun.star.uno.XInterface" ); }
    };

    typedef list<void * > t_pSpacesList;

    Mutex                   _aMutex;
    t_pSpacesList           _aSpaces;

    sal_Int32               _nCurrent;
    const void *            _pCurrent;

public:
    const void *            getMediateVtable( sal_Int32 nSize );

    MediateVtables( sal_Int32 nSize = 256 ) throw ()
        : _nCurrent( 0 )
        , _pCurrent( 0 )
        { getMediateVtable( nSize ); }
    ~MediateVtables() throw ();
};
//__________________________________________________________________________________________________
MediateVtables::~MediateVtables() throw ()
{
    TRACE( "> calling ~MediateVtables(): freeing mediate vtables... <\n" );

    MutexGuard aGuard( _aMutex );

    // this MUST be the absolute last one which is called!
    for ( t_pSpacesList::iterator iPos( _aSpaces.begin() ); iPos != _aSpaces.end(); ++iPos )
    {
        rtl_freeMemory( *iPos );
    }
}

//==================================================================================================
/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static __declspec(naked) void __cdecl cpp_vtable_call(void)
{
__asm
    {
        sub     esp, 8      // space for immediate return type
        push    esp
        push    eax         // vtable index
        mov     eax, esp
        add     eax, 16
        push    eax         // original stack ptr

        call    cpp_mediate
        add     esp, 12

        cmp     eax, typelib_TypeClass_FLOAT
        je      Lfloat
        cmp     eax, typelib_TypeClass_DOUBLE
        je      Ldouble
        cmp     eax, typelib_TypeClass_HYPER
        je      Lhyper
        cmp     eax, typelib_TypeClass_UNSIGNED_HYPER
        je      Lhyper
        // rest is eax
        pop     eax
        add     esp, 4
        ret
Lhyper:
        pop     eax
        pop     edx
        ret
Lfloat:
        fld     dword ptr [esp]
        add     esp, 8
        ret
Ldouble:
        fld     qword ptr [esp]
        add     esp, 8
        ret
    }
}

//__________________________________________________________________________________________________
const void * MediateVtables::getMediateVtable( sal_Int32 nSize ) throw ()
{
    if (_nCurrent < nSize)
    {
        TRACE( "> need larger vtable! <\n" );

        // dont ever guard each time, so ask twice when guarded
        MutexGuard aGuard( _aMutex );
        if (_nCurrent < nSize)
        {
            nSize = (nSize +1) & 0xfffffffe;
            char * pSpace = (char *)rtl_allocateMemory( ((1+nSize)*sizeof(void *)) + (nSize*12) );
            _aSpaces.push_back( pSpace );

            // on index -1 write default rtti entry
            static DefaultRTTIEntry s_defaultInterfaceRTTI;
            *(void **)pSpace = &s_defaultInterfaceRTTI;

            void ** pvft    = (void **)(pSpace + sizeof(void *));
            char * pCode    = pSpace + ((1+nSize)*sizeof(void *));

            // setup vft and code
            for ( sal_Int32 nPos = 0; nPos < nSize; ++nPos )
            {
                unsigned char * codeSnip = (unsigned char *)pCode + (nPos *12);
                pvft[nPos] = codeSnip;

                /**
                 * vtable calls detonate on these code snippets
                 */
                // mov eax, nPos
                *codeSnip++ = 0xb8;
                *(sal_Int32 *)codeSnip = nPos;
                codeSnip += sizeof(sal_Int32);
                // jmp rel32 cpp_vtable_call
                *codeSnip++ = 0xe9;
                *(sal_Int32 *)codeSnip = ((unsigned char *)cpp_vtable_call) - codeSnip - sizeof(sal_Int32);
            }
            _pCurrent = pSpace + sizeof(void *);
            _nCurrent = nSize;
        }
    }
    return _pCurrent;
}

//==================================================================================================
extern "C" void SAL_CALL cppu_cppInterfaceProxy_patchVtable(
    XInterface * pCppI, typelib_InterfaceTypeDescription * pTypeDescr ) throw ()
{
    static MediateVtables * s_pMediateVtables = 0;
    if (! s_pMediateVtables)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pMediateVtables)
        {
#ifdef LEAK_STATIC_DATA
            s_pMediateVtables = new MediateVtables();
#else
            static MediateVtables s_aMediateVtables;
            s_pMediateVtables = &s_aMediateVtables;
#endif
        }
    }
    *(const void **)pCppI = s_pMediateVtables->getMediateVtable(
        pTypeDescr->nMapFunctionIndexToMemberIndex );
}

}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_initEnvironment( uno_Environment * pCppEnv ) throw ()
{
    CPPU_CURRENT_NAMESPACE::cppu_cppenv_initEnvironment( pCppEnv );
}
//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo ) throw ()
{
    CPPU_CURRENT_NAMESPACE::cppu_ext_getMapping( ppMapping, pFrom, pTo );
}

