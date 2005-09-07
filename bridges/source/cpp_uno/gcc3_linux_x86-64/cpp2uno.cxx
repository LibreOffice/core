/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cpp2uno.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:26:18 $
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

#include <stdio.h>
#include <stdlib.h>
#include <hash_map>

#include <rtl/alloc.h>
#include <osl/mutex.hxx>

#include <uno/data.h>
#include <typelib/typedescription.hxx>

#include <bridges/cpp_uno/bridge.hxx>
#include <bridges/cpp_uno/type_misc.hxx>

#include "share.hxx"

using namespace ::osl;
using namespace ::rtl;
using namespace ::com::sun::star::uno;

namespace CPPU_CURRENT_NAMESPACE
{

// 6 integral parameters are passed in registers
const sal_uInt32 GPR_COUNT = 6;

// 8 floating point parameters are passed in SSE registers
const sal_uInt32 FPR_COUNT = 8;

//==================================================================================================
rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

//==================================================================================================
static typelib_TypeClass cpp2uno_call(
    cppu_cppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams,
    void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_uInt64 * pRegisterReturn /* space for register return */ )
{
    int nr_gpr = 0; //number of gpr registers used
    int nr_fpr = 0; //number of fpr regsiters used
    void ** pCppStack; //temporary stack pointer

    // gpreg:  [ret *], this, [gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (properly aligned)]

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
            pCppReturn = *(void **)gpreg;
            gpreg++;
            nr_gpr++;

            pUnoReturn = (cppu_relatesToInterface( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }
    }
    // pop this
    gpreg++;
    nr_gpr++;

    // stack space
    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;
    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pUnoArgs + (2 * nParams));
    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndizes   = 0;

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && cppu_isSimpleType( pParamTypeDescr )) // value
        {
            if (pParamTypeDescr->eTypeClass == typelib_TypeClass_FLOAT
                || pParamTypeDescr->eTypeClass == typelib_TypeClass_DOUBLE)
            {
               if (nr_fpr < FPR_COUNT)
               {
                   pCppArgs[nPos] = fpreg;
                   pUnoArgs[nPos] = fpreg;
                   nr_fpr++;
                   fpreg++;
               }
               else
               {
                   pCppArgs[nPos] = ovrflw;
                   pUnoArgs[nPos] = ovrflw;
                   ovrflw++;
               }
            }
            else
            {
                if (nr_gpr < GPR_COUNT)
                {
                    pCppArgs[nPos] = gpreg;
                    pUnoArgs[nPos] = gpreg;
                    nr_gpr++;
                    gpreg++;
                }
                else
                {
                    pCppArgs[nPos] = ovrflw;
                    pUnoArgs[nPos] = ovrflw;
                    ovrflw++;
                }
            }
            // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {
            if (nr_gpr < GPR_COUNT)
            {
                pCppArgs[nPos] = *(void **)gpreg;
                pCppStack = gpreg;
                nr_gpr++;
                gpreg++;
            }
            else
            {
                pCppArgs[nPos] = *(void **)ovrflw;
                pCppStack = ovrflw;
                ovrflw++;
            }

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
        for ( ; nTempIndizes--; )
        {
            sal_Int32 nIndex = pTempIndizes[nTempIndizes];

            if (pParams[nIndex].bIn) // is in/inout => was constructed
                uno_destructData( pUnoArgs[nIndex], ppTempParamTypeDescr[nTempIndizes], 0 );
            TYPELIB_DANGER_RELEASE( ppTempParamTypeDescr[nTempIndizes] );
        }
        if (pReturnTypeDescr)
            TYPELIB_DANGER_RELEASE( pReturnTypeDescr );

        raiseException( &aUnoExc, &pThis->pBridge->aUno2Cpp ); // has to destruct the any
        // is here for dummy
        return typelib_TypeClass_VOID;
    }
    else // else no exception occured...
    {
        // temporary params
        for ( ; nTempIndizes--; )
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
            // complex return ptr is set to return reg
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
static typelib_TypeClass cpp_mediate(
    sal_Int32 nVtableCall,
    void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_uInt64 * pRegisterReturn /* space for register return */ )
{
    // gpreg:  [ret *], this, [other gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (properly aligned)]

    // _this_ ptr is patched cppu_XInterfaceProxy object
    cppu_cppInterfaceProxy * pCppI = NULL;
    if( nVtableCall & 0x80000000 )
    {
        nVtableCall &= 0x7fffffff;
        pCppI = (cppu_cppInterfaceProxy *)(XInterface *)*(gpreg +1);
    }
    else
    {
        pCppI = (cppu_cppInterfaceProxy *)(XInterface *)*(gpreg);
    }

    typelib_InterfaceTypeDescription * pTypeDescr = pCppI->pTypeDescr;

    OSL_ENSURE( nVtableCall < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!\n" );
    if (nVtableCall >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException(
            OUString::createFromAscii("illegal vtable index!"),
            (XInterface *)pCppI );
    }

    // determine called method
    OSL_ENSURE( nVtableCall < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!\n" );
    sal_Int32 nMemberPos = pTypeDescr->pMapFunctionIndexToMemberIndex[nVtableCall];
    OSL_ENSURE( nMemberPos < pTypeDescr->nAllMembers, "### illegal member index!\n" );

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
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceAttributeTypeDescription *)aMemberDescr.get())->pAttributeTypeRef,
                0, 0, // no params
                gpreg, fpreg, ovrflw, pRegisterReturn );
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
                pCppI, aMemberDescr.get(),
                0, // indicates void return
                1, &aParam,
                gpreg, fpreg, ovrflw, pRegisterReturn );
        }
        break;
    }
    case typelib_TypeClass_INTERFACE_METHOD:
    {
        // is METHOD
        switch (nVtableCall)
        {
        case 1: // acquire()
            pCppI->acquireProxy(); // non virtual call!
            eRet = typelib_TypeClass_VOID;
            break;
        case 2: // release()
            pCppI->releaseProxy(); // non virtual call!
            eRet = typelib_TypeClass_VOID;
            break;
        case 0: // queryInterface() opt
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, reinterpret_cast< Type * >( gpreg[2] )->getTypeLibType() );
            if (pTD)
            {
                XInterface * pInterface = 0;
                (*pCppI->pBridge->pCppEnv->getRegisteredInterface)(
                    pCppI->pBridge->pCppEnv,
                    (void **)&pInterface, pCppI->oid.pData, (typelib_InterfaceTypeDescription *)pTD );

                if (pInterface)
                {
                    ::uno_any_construct(
                        reinterpret_cast< uno_Any * >( gpreg[0] ),
                        &pInterface, pTD, cpp_acquire );
                    pInterface->release();
                    TYPELIB_DANGER_RELEASE( pTD );
                    *(void **)pRegisterReturn = gpreg[0];
                    eRet = typelib_TypeClass_ANY;
                    break;
                }
                TYPELIB_DANGER_RELEASE( pTD );
            }
        } // else perform queryInterface()
        default:
            eRet = cpp2uno_call(
                pCppI, aMemberDescr.get(),
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pReturnTypeRef,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->nParams,
                ((typelib_InterfaceMethodTypeDescription *)aMemberDescr.get())->pParams,
                gpreg, fpreg, ovrflw, pRegisterReturn );
        }
        break;
    }
    default:
    {
        throw RuntimeException(
            OUString::createFromAscii("no member description found!"),
            (XInterface *)pCppI );
        // is here for dummy
        eRet = typelib_TypeClass_VOID;
    }
    }

    return eRet;
}

//==================================================================================================
/**
 * is called on incoming vtable calls
 * (called by asm snippets)
 */
static void cpp_vtable_call(sal_uInt32 nTableEntry,
                            void** ovrflw, void** gpregptr, void** fpregptr)
{
    sal_uInt64    gpreg[GPR_COUNT];
    double        fpreg[FPR_COUNT];

    memcpy( gpreg, gpregptr, sizeof(gpreg) );
    memcpy( fpreg, fpregptr, sizeof(fpreg) );

    volatile sal_uInt64 nRegReturn[3];

#ifdef DEBUG
    fprintf(stderr, "cpp_vtable_call(%08x,...)\n", nTableEntry);
#endif

    sal_Bool bComplex = nTableEntry & 0x80000000 ? sal_True : sal_False;

    typelib_TypeClass aType =
        cpp_mediate( nTableEntry, (void**)gpreg, (void**)fpreg, ovrflw, (sal_uInt64 *)nRegReturn );

    switch( aType )
    {
    case typelib_TypeClass_FLOAT:
        // The value in %xmm register is already prepared to
        // be retrieved as a float. Therefore, we pass the
        // value verbatim, as a double without conversion.
        __asm__( "movsd %0, %%xmm0" : : "m" (*((double *)nRegReturn)) );
        break;

    case typelib_TypeClass_DOUBLE:
        __asm__( "movsd %0, %%xmm0" : : "m" (*((double *)nRegReturn)) );
        break;

    default:
        __asm__( "movq %0, %%rax" : : "m" (*nRegReturn));
        break;
    }

    __asm__ __volatile__ ("" : : : "rax", "xmm0");
}


//==================================================================================================
class MediateClassData
{
    typedef ::std::hash_map< OUString, void *, OUStringHash > t_classdata_map;
    t_classdata_map m_map;
    Mutex m_mutex;

public:
    void const * get_vtable( typelib_InterfaceTypeDescription * pTD ) SAL_THROW( () );

    inline MediateClassData() SAL_THROW( () )
        {}
    ~MediateClassData() SAL_THROW( () );
};
//__________________________________________________________________________________________________
MediateClassData::~MediateClassData() SAL_THROW( () )
{
    OSL_TRACE( "> calling ~MediateClassData(): freeing mediate vtables." );

    for ( t_classdata_map::const_iterator iPos( m_map.begin() ); iPos != m_map.end(); ++iPos )
    {
        ::rtl_freeMemory( iPos->second );
    }
}
//--------------------------------------------------------------------------------------------------
/* Code to generate. Note: if you change it, make sure patch offsets
   for nTableEntry and cpp_vtable_call() are updated too.  */
const char code_snippet_template[] = {
    // # make room for gpregs (48), fpregs (64)
    0x55,                               // push   %rbp
    0x48, 0x89, 0xe5,                   // mov    %rsp,%rbp
    0x48, 0x83, 0xec, 0x70,             // sub    $112,%rsp
    // # save GP registers
    0x48, 0x89, 0x7d, 0x90,             // mov    %rdi,-112(%rbp)
    0x48, 0x89, 0x75, 0x98,             // mov    %rsi,-104(%rbp)
    0x48, 0x89, 0x55, 0xa0,             // mov    %rdx, -96(%rbp)
    0x48, 0x89, 0x4d, 0xa8,             // mov    %rcx, -88(%rbp)
    0x4c, 0x89, 0x45, 0xb0,             // mov    %r8 , -80(%rbp)
    0x4c, 0x89, 0x4d, 0xb8,             // mov    %r9 , -72(%rbp)
    0x48, 0x8d, 0x55, 0x90,             // lea    -112(%rbp),%rdx
    // # save FP registers
    0xf2, 0x0f, 0x11, 0x45, 0xc0,       // movsd  %xmm0,-64(%rbp)
    0xf2, 0x0f, 0x11, 0x4d, 0xc8,       // movsd  %xmm1,-56(%rbp)
    0xf2, 0x0f, 0x11, 0x55, 0xd0,       // movsd  %xmm2,-48(%rbp)
    0xf2, 0x0f, 0x11, 0x5d, 0xd8,       // movsd  %xmm3,-40(%rbp)
    0xf2, 0x0f, 0x11, 0x65, 0xe0,       // movsd  %xmm4,-32(%rbp)
    0xf2, 0x0f, 0x11, 0x6d, 0xe8,       // movsd  %xmm5,-24(%rbp)
    0xf2, 0x0f, 0x11, 0x75, 0xf0,       // movsd  %xmm6,-16(%rbp)
    0xf2, 0x0f, 0x11, 0x7d, 0xf8,       // movsd  %xmm7, -8(%rbp)
    0x48, 0x8d, 0x4d, 0xc0,             // lea    -64(%rbp),%rcx
    // # perform the call and cleanup to cpp_vtable_call()
    0xbf, 0x00, 0x00, 0x00, 0x00,       // mov    $0,%edi
    0x48, 0x8d, 0x75, 0x10,             // lea    16(%rbp),%rsi
    0x48, 0xb8, 0, 0, 0, 0, 0, 0, 0, 0, // mov    $0,%rax
    0xff, 0xd0,                         // call   *%rax
    0xc9,                               // leave
    0xc3                                // ret
};

static inline void codeSnippet( char * code, sal_uInt32 vtable_pos, bool simple_ret_type ) SAL_THROW( () )
{
    if (! simple_ret_type)
        vtable_pos |= 0x80000000;

    const int code_snippet_template_size = sizeof(code_snippet_template);
    memcpy(code, code_snippet_template, code_snippet_template_size);

    // Patch nTableEntryValue
    sal_uInt32 *mid_p  = (sal_uInt32 *)(code + code_snippet_template_size - 22);
    *mid_p = vtable_pos;

    // Patch call to cpp_vtable_call()
    sal_uInt64 *call_p  = (sal_uInt64 *)(code + code_snippet_template_size - 12);
    *call_p = (sal_uInt64)cpp_vtable_call;
}
//__________________________________________________________________________________________________
void const * MediateClassData::get_vtable( typelib_InterfaceTypeDescription * pTD ) SAL_THROW( () )
{
    void * buffer;
    const int nSnippetSize = sizeof(code_snippet_template);

    // avoiding locked counts
    OUString const & unoName = *(OUString const *)&((typelib_TypeDescription *)pTD)->pTypeName;
    {
    MutexGuard aGuard( m_mutex );
    t_classdata_map::const_iterator iFind( m_map.find( unoName ) );
    if (iFind == m_map.end())
    {
        // create new vtable
        sal_Int32 nSlots = pTD->nMapFunctionIndexToMemberIndex;
        buffer = ::rtl_allocateMemory( ((2+ nSlots) * sizeof (void *)) + (nSlots *nSnippetSize) );

        ::std::pair< t_classdata_map::iterator, bool > insertion(
            m_map.insert( t_classdata_map::value_type( unoName, buffer ) ) );
        OSL_ENSURE( insertion.second, "### inserting new vtable buffer failed?!\n\n" );

        void ** slots = (void **)buffer;
        *slots++ = 0;
        *slots++ = 0; // rtti
        char * code = (char *)(slots + nSlots);

        sal_uInt32 vtable_pos = 0;
        sal_Int32 nAllMembers = pTD->nAllMembers;
        typelib_TypeDescriptionReference ** ppAllMembers = pTD->ppAllMembers;
        for ( sal_Int32 nPos = 0; nPos < nAllMembers; ++nPos )
        {
            typelib_TypeDescription * pTD = 0;
            TYPELIB_DANGER_GET( &pTD, ppAllMembers[ nPos ] );
            OSL_ASSERT( pTD );
            if (typelib_TypeClass_INTERFACE_ATTRIBUTE == pTD->eTypeClass)
            {
                bool simple_ret = cppu_isSimpleType(
                    ((typelib_InterfaceAttributeTypeDescription *)pTD)->pAttributeTypeRef->eTypeClass );
                // get method
                *slots = code;
                codeSnippet( code, vtable_pos++, simple_ret );
                code += nSnippetSize;
                slots++;
                if (! ((typelib_InterfaceAttributeTypeDescription *)pTD)->bReadOnly)
                {
                    // set method
                    *slots = code;
                    codeSnippet( code, vtable_pos++, true );
                    code += nSnippetSize;
                    slots++;
                }
            }
            else
            {
                bool simple_ret = cppu_isSimpleType(
                    ((typelib_InterfaceMethodTypeDescription *)pTD)->pReturnTypeRef->eTypeClass );
                *slots = code;
                codeSnippet( code, vtable_pos++, simple_ret );
                code += nSnippetSize;
                slots++;
            }
            TYPELIB_DANGER_RELEASE( pTD );
        }
        OSL_ASSERT( vtable_pos == nSlots );
    }
    else
    {
        buffer = iFind->second;
    }
    }

    return ((void **)buffer +2);
}

//==================================================================================================
void SAL_CALL cppu_cppInterfaceProxy_patchVtable(
    XInterface * pCppI, typelib_InterfaceTypeDescription * pTypeDescr ) throw ()
{
    static MediateClassData * s_pMediateClassData = 0;
    if (! s_pMediateClassData)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pMediateClassData)
        {
#ifdef LEAK_STATIC_DATA
            s_pMediateClassData = new MediateClassData();
#else
            static MediateClassData s_aMediateClassData;
            s_pMediateClassData = &s_aMediateClassData;
#endif
        }
    }
    *(void const **)pCppI = s_pMediateClassData->get_vtable( pTypeDescr );
}

}

extern "C"
{
//##################################################################################################
sal_Bool SAL_CALL component_canUnload( TimeValue * pTime )
    SAL_THROW_EXTERN_C()
{
    return CPPU_CURRENT_NAMESPACE::g_moduleCount.canUnload(
        &CPPU_CURRENT_NAMESPACE::g_moduleCount, pTime );
}
//##################################################################################################
void SAL_CALL uno_initEnvironment( uno_Environment * pCppEnv )
    SAL_THROW_EXTERN_C()
{
    CPPU_CURRENT_NAMESPACE::cppu_cppenv_initEnvironment(
        pCppEnv );
}
//##################################################################################################
void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo )
    SAL_THROW_EXTERN_C()
{
    CPPU_CURRENT_NAMESPACE::cppu_ext_getMapping(
        ppMapping, pFrom, pTo );
}
}
