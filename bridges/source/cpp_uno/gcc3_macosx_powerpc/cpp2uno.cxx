/*************************************************************************
 *
 *  $RCSfile: cpp2uno.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-07-16 17:32:29 $
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

// #include <malloc.h>
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

//==================================================================================================
rtl_StandardModuleCount g_moduleCount = MODULE_COUNT_INIT;

//==================================================================================================
static typelib_TypeClass cpp2uno_call(
    cppu_cppInterfaceProxy * pThis,
    const typelib_TypeDescription * pMemberTypeDescr,
    typelib_TypeDescriptionReference * pReturnTypeRef, // 0 indicates void return
    sal_Int32 nParams, typelib_MethodParameter * pParams,
        void ** gpreg, void ** fpreg, void ** ovrflw,
    sal_Int64 * pRegisterReturn /* space for register return */ )
{

        // gpreg:  [ret *], this, [gpr params]
        // fpreg:  [fpr params]
        // ovrflw: [gpr or fpr params (space for entire parameter list in structure format properly aligned)]

    // return
    typelib_TypeDescription * pReturnTypeDescr = 0;
    if (pReturnTypeRef)
        TYPELIB_DANGER_GET( &pReturnTypeDescr, pReturnTypeRef );

    void * pUnoReturn = 0;
    void * pCppReturn = 0; // complex return ptr: if != 0 && != pUnoReturn, reconversion need

    sal_Int32 ngpreg = 0;
    sal_Int32 nfpreg = 0;


    if (pReturnTypeDescr)
    {
        if (cppu_isSimpleType( pReturnTypeDescr ))
            pUnoReturn = pRegisterReturn; // direct way for simple types
        else // complex return via ptr (pCppReturn)
        {
            pCppReturn = *gpreg;
            ngpreg++;
            ++ovrflw;

            pUnoReturn = (cppu_relatesToInterface( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }
    }
    // pop this
    ngpreg++;
    ++ovrflw;

        // after handling optional return pointer and "this"
        // make use of the space that is allocated to store all parameters in the callers stack
        // by comying the proper registers filled with parameters to that space
    char * pCppStack = (char *)ovrflw;


    sal_Int32 nPos;

    for ( nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        if (rParam.bOut)
        {
            if (ngpreg < 8)
            {
                *(sal_Int32 *)pCppStack = ((sal_Int32 *)gpreg)[ngpreg++];
            }
            pCppStack += sizeof (sal_Int32);
        }
        else
        {
        switch (rParam.pTypeRef->eTypeClass)
        {
        case typelib_TypeClass_FLOAT:
            if (nfpreg < 13)
            {
                *(float *)pCppStack = ((double *)fpreg)[nfpreg++];
            }
            pCppStack += sizeof (float);
            ngpreg += 1;
            break;
        case typelib_TypeClass_DOUBLE:
            if (nfpreg < 13)
            {
                *(double *)pCppStack = ((double *)fpreg)[nfpreg++];
            }
            pCppStack += sizeof (double);
            ngpreg += 2;
            break;
        case typelib_TypeClass_UNSIGNED_HYPER:
        case typelib_TypeClass_HYPER:
            if (ngpreg < 8)
            {
                *(sal_Int32 *)pCppStack = ((sal_Int32 *)gpreg)[ngpreg++];
            }
            pCppStack += sizeof (sal_Int32);
                        // fall through on purpose
        default:
            if (ngpreg < 8)
            {
                *(sal_Int32 *)pCppStack = ((sal_Int32 *)gpreg)[ngpreg++];
            }
            pCppStack += sizeof (sal_Int32);
        }
        }
    }

        // now the stack has all of the paramters stored in it ready to be processed
        // so we are ready to build the uno call stack
    pCppStack = (char *)ovrflw;

    // stack space
    OSL_ENSURE( sizeof(void *) == sizeof(sal_Int32), "### unexpected size!" );

    // parameters
    void ** pUnoArgs = (void **)alloca( 4 * sizeof(void *) * nParams );
    void ** pCppArgs = pUnoArgs + nParams;

    // indizes of values this have to be converted (interface conversion cpp<=>uno)
    sal_Int32 * pTempIndizes = (sal_Int32 *)(pUnoArgs + (2 * nParams));

    // type descriptions for reconversions
    typelib_TypeDescription ** ppTempParamTypeDescr = (typelib_TypeDescription **)(pUnoArgs + (3 * nParams));

    sal_Int32 nTempIndizes   = 0;

    for ( nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && cppu_isSimpleType( pParamTypeDescr )) // value
        {
            switch (pParamTypeDescr->eTypeClass)
            {
            case typelib_TypeClass_BOOLEAN:
            case typelib_TypeClass_BYTE:
                pCppArgs[nPos] = pCppStack +3;
                pUnoArgs[nPos] = pCppStack +3;
                break;
            case typelib_TypeClass_CHAR:
            case typelib_TypeClass_SHORT:
            case typelib_TypeClass_UNSIGNED_SHORT:
                pCppArgs[nPos] = pCppStack +2;
                pUnoArgs[nPos] = pCppStack +2;
                break;
            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
            case typelib_TypeClass_DOUBLE:
                pCppArgs[nPos] = pCppStack;
                pUnoArgs[nPos] = pCppStack;
                pCppStack += sizeof(sal_Int32); // extra long (two regs)
                break;
            default:
                pCppArgs[nPos] = pCppStack;
                pUnoArgs[nPos] = pCppStack;
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
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

    // gpreg:  [ret *], this, [other gpr params]
    // fpreg:  [fpr params]
    // ovrflw: [gpr or fpr params (in space allocated for all params properly aligned)]

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

    OSL_ENSURE( nVtableCall < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
    if (nVtableCall >= pTypeDescr->nMapFunctionIndexToMemberIndex)
    {
        throw RuntimeException(
            OUString::createFromAscii("illegal vtable index!"),
            (XInterface *)pCppI );
    }

    // determine called method
    OSL_ENSURE( nVtableCall < pTypeDescr->nMapFunctionIndexToMemberIndex, "### illegal vtable index!" );
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
static void cpp_vtable_call( int nTableEntry, void** gpregptr, void** fpregptr, void** ovrflw)
{
        sal_Int32     gpreg[8];
        double        fpreg[13];

        // FIXME: why are we restoring the volatile ctr register here
    sal_Int32   ctrsave = ((sal_Int32*)gpregptr)[-1];

        memcpy( gpreg, gpregptr, 32);
        memcpy( fpreg, fpregptr, 104);

    volatile long nRegReturn[2];

        sal_Bool bComplex = nTableEntry & 0x80000000 ? sal_True : sal_False;

    typelib_TypeClass aType =
             cpp_mediate( nTableEntry, (void**)gpreg, (void**)fpreg, ovrflw, (sal_Int64*)nRegReturn );

        // FIXME: why are we restoring the volatile ctr register here
        // FIXME: and why are we putting back the values for r4, r5, and r6 as well
        // FIXME: this makes no sense to me, all of these registers are volatile!
    __asm__( "lwz r4, %0\n\t"
        "mtctr r4\n\t"
        "lwz r4, %1\n\t"
        "lwz r5, %2\n\t"
        "lwz r6, %3\n\t"
         : : "m"(ctrsave), "m"(gpreg[1]), "m"(gpreg[2]), "m"(gpreg[3]) );

    switch( aType )
    {

                // move return value into register space
                // (will be loaded by machine code snippet)

                case typelib_TypeClass_BOOLEAN:
                case typelib_TypeClass_BYTE:
                  __asm__( "lbz r3,%0\n\t" : :
               "m"(nRegReturn[0]) );
                  break;

                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_SHORT:
                case typelib_TypeClass_UNSIGNED_SHORT:
                  __asm__( "lhz r3,%0\n\t" : :
               "m"(nRegReturn[0]) );
                  break;

        case typelib_TypeClass_FLOAT:
                  __asm__( "lfs f1,%0\n\t" : :
                           "m" (*((float*)nRegReturn)) );
          break;

        case typelib_TypeClass_DOUBLE:
          __asm__( "lfd f1,%0\n\t" : :
                           "m" (*((double*)nRegReturn)) );
          break;

        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
          __asm__( "lwz r4,%0\n\t" : :
                           "m"(nRegReturn[1]) );  // fall through

        default:
          __asm__( "lwz r3,%0\n\t" : :
                           "m"(nRegReturn[0]) );
          break;
    }
}


//__________________________________________________________________________________________________

// FIXME: this is overkill, we should flush the data caches and invalidate
// the instruction caches in loop and then do the sync, isync at the end

void flush_icache(char *addr)
{
  __asm__ volatile (
                "dcbf 0,%0;"
                "sync;"
                "icbi 0,%0;"
                "sync;"
                "isync;"
                : : "r"(addr) : "memory");
}


void flush_range(char * addr1, int size)
{
#define MIN_LINE_SIZE 32
  int i;
  for (i = 0; i < size; i += MIN_LINE_SIZE)
    flush_icache(addr1+i);
  flush_icache(addr1+size-1);
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
static inline void codeSnippet( long * code, sal_uInt32 vtable_pos, bool simple_ret_type ) SAL_THROW( () )
{
    if (! simple_ret_type)
        vtable_pos |= 0x80000000;
    OSL_ASSERT( sizeof (long) == 4 );

    // FIXME: why are we leaving an 8k gap in the stack here
    // FIXME: is this to allow room for signal handling frames?
    // FIXME: seems like overkill here but this is what was done for Mac OSX for gcc2
    // FIXME: also why no saving of the non-volatile CR pieces here, to be safe
    // FIXME: we probably should

    /* generate this code */

    // # so first save gpr 3 to gpr 10 (aligned to 4)
    //  stw   r3, -8000(r1)
    //  stw   r4, -7996(r1)
    //  stw   r5, -7992(r1)
    //  stw   r6, -7988(r1)
    //  stw   r7, -7984(r1)
    //  stw   r8, -7980(r1)
    //  stw   r9, -7976(r1)
    //  stw   r10,-7972(r1)

    // # next save fpr 1 to fpr 13 (aligned to 8)
    //  stfd  f1, -7968(r1)
    //  stfd  f2, -7960(r1)
    //  stfd  f3, -7952(r1)
    //  stfd  f4, -7944(r1)
    //  stfd  f5, -7936(r1)
    //  stfd  f6, -7928(r1)
    //  stfd  f7, -7920(r1)
    //  stfd  f8, -7912(r1)
    //  stfd  f9, -7904(r1)
    //  stfd  f10,-7896(r1)
    //  stfd  f11,-7888(r1)
    //  stfd  f12,-7880(r1)
    //  stfd  f13,-7872(r1)

    // FIXME: ctr is volatile, while are we saving it and not CR?
    // mfctr r3
    // stw r3, -8004(r1)

    // # now here is where cpp_vtable_call must go
    // lis r3,0xdead
    // ori r3,r3,0xbeef
    // mtctr r3

    // # now load up the the table entry number
    // lis r3, 0xdead
    // ori r3,r3,0xbeef

    // #now load up the pointer to the saved gpr registers
    // addi r4,r1,-8000

    // #now load up the pointer to the saved fpr registers
    // addi r5,r1,-7968

    // #now load up the pointer to the overflow call stack
    // addi r6,r1,24 # frame pointer plus 24

    // bctr


    * code++ = 0x9061e0c0;
    * code++ = 0x9081e0c4;
    * code++ = 0x90a1e0c8;
    * code++ = 0x90c1e0cc;
    * code++ = 0x90e1e0d0;
    * code++ = 0x9101e0d4;
    * code++ = 0x9121e0d8;
    * code++ = 0x9141e0dc;
    * code++ = 0xd821e0e0;
    * code++ = 0xd841e0e8;
    * code++ = 0xd861e0f0;
    * code++ = 0xd881e0f8;
    * code++ = 0xd8a1e100;
    * code++ = 0xd8c1e108;
    * code++ = 0xd8e1e110;
    * code++ = 0xd901e118;
    * code++ = 0xd921e120;
    * code++ = 0xd941e128;
    * code++ = 0xd961e130;
    * code++ = 0xd981e138;
    * code++ = 0xd9a1e140;
    * code++ = 0x7c6902a6;
    * code++ = 0x9061e0bc;
    * code++ = 0x3c600000 | (((unsigned long)cpp_vtable_call) >> 16);
    * code++ = 0x60630000 | (((unsigned long)cpp_vtable_call) & 0x0000FFFF);
    * code++ = 0x7c6903a6;
    * code++ = 0x3c600000 | (vtable_pos >> 16);
    * code++ = 0x60630000 | (vtable_pos & 0x0000FFFF);
    * code++ = 0x3881e0c0;
    * code++ = 0x38a1e0e0;
    * code++ = 0x38c10018;
    * code++ = 0x4e800420;


      // don't forget to flush the data and caches after calling this routine
      // otherwise the self-modifying code we wrote above will not be processed properly
      // and we need to remove stale instruction cache

      // note the size of this snippet of code needs to be kept consistent with
      // nSnippetSize below (it is now 32 instructions 4 bytes long each = 128 bytes)

}


//__________________________________________________________________________________________________
void const * MediateClassData::get_vtable( typelib_InterfaceTypeDescription * pTD ) SAL_THROW( () )
{
    void * buffer;
    const int nSnippetSize = 128;

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
        OSL_ENSURE( insertion.second, "### inserting new vtable buffer failed?!" );

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
                codeSnippet( (long *)code, vtable_pos++, simple_ret );
                flush_range( code, nSnippetSize );
                code += nSnippetSize;
                slots++;
                if (! ((typelib_InterfaceAttributeTypeDescription *)pTD)->bReadOnly)
                {
                    // set method
                    *slots = code;
                    codeSnippet( (long *)code, vtable_pos++, true );
                    flush_range( code, nSnippetSize );
                    code += nSnippetSize;
                    slots++;
                }
            }
            else
            {
                bool simple_ret = cppu_isSimpleType(
                    ((typelib_InterfaceMethodTypeDescription *)pTD)->pReturnTypeRef->eTypeClass );
                *slots = code;
                codeSnippet( (long *)code, vtable_pos++, simple_ret );
                flush_range( code, nSnippetSize );
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
