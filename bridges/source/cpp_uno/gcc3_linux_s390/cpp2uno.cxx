/*************************************************************************
 *
 *  $RCSfile: cpp2uno.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-18 19:06:54 $
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

#include <malloc.h>
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
        int ng = 0; //number of gpr registers used
        int nf = 0; //number of fpr regsiters used
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
                        ng++;

            pUnoReturn = (cppu_relatesToInterface( pReturnTypeDescr )
                          ? alloca( pReturnTypeDescr->nSize )
                          : pCppReturn); // direct way
        }
    }
    // pop this
        gpreg++;
        ng++;

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

    for ( sal_Int32 nPos = 0; nPos < nParams; ++nPos )
    {
        const typelib_MethodParameter & rParam = pParams[nPos];
        typelib_TypeDescription * pParamTypeDescr = 0;
        TYPELIB_DANGER_GET( &pParamTypeDescr, rParam.pTypeRef );

        if (!rParam.bOut && cppu_isSimpleType( pParamTypeDescr )) // value
        {

            switch (pParamTypeDescr->eTypeClass)
            {

              case typelib_TypeClass_DOUBLE:
               if (nf < 2) {
                  pCppArgs[nPos] = fpreg;
                  pUnoArgs[nPos] = fpreg;
                  nf++;
                  fpreg += 2;
               } else {
                pCppArgs[nPos] = ovrflw;
                pUnoArgs[nPos] = ovrflw;
                    ovrflw += 2;
               }
               break;

               case typelib_TypeClass_FLOAT:
                // fpreg are all double values so need to
                // modify fpreg to be a single word float value
                if (nf < 2) {
//                 float tmp = (float) (*((double *)fpreg));
//                 (*((float *) fpreg)) = tmp;
                   pCppArgs[nPos] = fpreg;
                   pUnoArgs[nPos] = fpreg;
                   nf++;
                   fpreg += 2;
                } else {
                              pCppArgs[nPos] = ovrflw;
                  pUnoArgs[nPos] = ovrflw;
                  ovrflw += 1;
                            }
                break;

            case typelib_TypeClass_HYPER:
            case typelib_TypeClass_UNSIGNED_HYPER:
             if (ng < 4) {
                pCppArgs[nPos] = gpreg;
                pUnoArgs[nPos] = gpreg;
                ng += 2;
                gpreg += 2;
             } else {
                pCppArgs[nPos] = ovrflw;
                pUnoArgs[nPos] = ovrflw;
                ovrflw += 2;
              }
              break;

            case typelib_TypeClass_BYTE:
            case typelib_TypeClass_BOOLEAN:
             if (ng < 5) {
                  pCppArgs[nPos] = (((char *)gpreg) + 3);
                  pUnoArgs[nPos] = (((char *)gpreg) + 3);
                  ng++;
                  gpreg++;
             } else {
                  pCppArgs[nPos] = (((char *)ovrflw) + 3);
                  pUnoArgs[nPos] = (((char *)ovrflw) + 3);
                  ovrflw++;
             }
             break;


               case typelib_TypeClass_CHAR:
               case typelib_TypeClass_SHORT:
               case typelib_TypeClass_UNSIGNED_SHORT:
            if (ng < 5) {
                  pCppArgs[nPos] = (((char *)gpreg)+ 2);
                  pUnoArgs[nPos] = (((char *)gpreg)+ 2);
                  ng++;
                  gpreg++;
            } else {
                  pCppArgs[nPos] = (((char *)ovrflw) + 2);
                  pUnoArgs[nPos] = (((char *)ovrflw) + 2);
                  ovrflw++;
            }
            break;


              default:
            if (ng < 5) {
                  pCppArgs[nPos] = gpreg;
                  pUnoArgs[nPos] = gpreg;
                  ng++;
                  gpreg++;
            } else {
                  pCppArgs[nPos] = ovrflw;
                  pUnoArgs[nPos] = ovrflw;
                  ovrflw++;
            }
                        break;

                }
                // no longer needed
            TYPELIB_DANGER_RELEASE( pParamTypeDescr );
        }
        else // ptr to complex value | ref
        {

                if (ng < 5) {
                  pCppArgs[nPos] = *(void **)gpreg;
                  pCppStack = gpreg;
                  ng++;
                  gpreg++;
                } else {
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
    sal_Int64 * pRegisterReturn /* space for register return */ )
{
    OSL_ENSURE( sizeof(sal_Int32)==sizeof(void *), "### unexpected!" );

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
        double        fpreg[8];

        memcpy( gpreg, gpregptr, 32);
        memcpy( fpreg, fpregptr, 64);

    volatile long nRegReturn[2];

        sal_Bool bComplex = nTableEntry & 0x80000000 ? sal_True : sal_False;

    typelib_TypeClass aType =
             cpp_mediate( nTableEntry, (void**)gpreg, (void**)fpreg, ovrflw, (sal_Int64*)nRegReturn );

    switch( aType )
    {

                // move return value into register space
                // (will be loaded by machine code snippet)

                case typelib_TypeClass_BOOLEAN:
          {
                  unsigned long tmp = (unsigned long)(*(unsigned char *)nRegReturn);
                  __asm__ volatile ( "l 2,%0\n\t" : :
               "m"(tmp) : "2" );
                  break;
          }
                case typelib_TypeClass_BYTE:
          {
                  long tmp = (long)(*(signed char *)nRegReturn);
                  __asm__ volatile ( "l 2,%0\n\t" : :
               "m"(tmp) : "2" );
                  break;
          }
                case typelib_TypeClass_CHAR:
                case typelib_TypeClass_UNSIGNED_SHORT:
          {
                  unsigned long tmp = (unsigned long)(*(unsigned short *)nRegReturn);
                  __asm__ volatile ( "l 2,%0\n\t" : :
               "m"(tmp) : "2" );
                  break;
          }
                case typelib_TypeClass_SHORT:
          {
                  long tmp = (long)(*(short *)nRegReturn);
                  __asm__ volatile ( "l 2,%0\n\t" : :
               "m"(tmp) : "2" );
                  break;
          }
        case typelib_TypeClass_FLOAT:
                  __asm__ volatile ( "le 0,%0\n\t" : :
                           "m" (*((float*)nRegReturn)) : "16" );
          break;

        case typelib_TypeClass_DOUBLE:
          __asm__ volatile ( "ld 0,%0\n\t" : :
                           "m" (*((double*)nRegReturn)) : "16" );
          break;

        case typelib_TypeClass_HYPER:
        case typelib_TypeClass_UNSIGNED_HYPER:
          __asm__ volatile ( "lm 2,3,%0\n\t" : :
                           "m"(nRegReturn[0]) : "2", "3" );
          break;

        default:
          __asm__ volatile ( "l 2,%0\n\t" : :
                           "m"(nRegReturn[0]) : "2" );
          break;
    }
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

     /* generate this code */
     // lr      %r0,%r13
     // bras    %r13,0x6
     // .long   cpp_vtable_call
     // .long   vtable_pos
     // stm     %r2,%r6,8(%r15)
     // std     %f0,64(%r15)
     // std     %f2,72(%r15)

     // l       %r2,4(%r13)
     // la      %r3,8(%r15)
     // la      %r4,64(%r15)
     // la      %r5,96(%r15)
     // l       %r1,0(%r13)
     // lr      %r13,%r0
     // br      %r1


      * ((short*)code)++ = 0x180d;
      * code++ = 0xa7d50006;
      * code++ = cpp_vtable_call;
      * code++ = vtable_pos;
      * code++ = 0x9026f008;
      * code++ = 0x6000f040;
      * code++ = 0x6020f048;
      * code++ = 0x5820d004;
      * code++ = 0x4130f008;
      * code++ = 0x4140f040;
      * code++ = 0x4150f060;
      * code++ = 0x5810d000;
      * ((short*)code)++ = 0x18d0;
      * ((short*)code)++ = 0x07f1;

}
//__________________________________________________________________________________________________
void const * MediateClassData::get_vtable( typelib_InterfaceTypeDescription * pTD ) SAL_THROW( () )
{
    void * buffer;
    const int nSnippetSize = 50;

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
                code += nSnippetSize;
                slots++;
                if (! ((typelib_InterfaceAttributeTypeDescription *)pTD)->bReadOnly)
                {
                    // set method
                    *slots = code;
                    codeSnippet( (long *)code, vtable_pos++, true );
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
