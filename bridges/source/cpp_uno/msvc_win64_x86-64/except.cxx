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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_bridges.hxx"

#pragma warning( disable : 4237 )
#include <hash_map>
#include <sal/config.h>
#include <malloc.h>
#include <typeinfo.h>
#include <signal.h>

#include "rtl/alloc.h"
#include "rtl/strbuf.hxx"
#include "rtl/ustrbuf.hxx"

#include "com/sun/star/uno/Any.hxx"

#include "mscx.hxx"


#pragma pack(push, 8)

using namespace ::com::sun::star::uno;
using namespace ::std;
using namespace ::osl;
using namespace ::rtl;

namespace CPPU_CURRENT_NAMESPACE
{

//==================================================================================================
static inline OUString toUNOname( OUString const & rRTTIname ) throw ()
{
    OUStringBuffer aRet( 64 );
    OUString aStr( rRTTIname.copy( 4, rRTTIname.getLength()-4-2 ) ); // filter .?AUzzz@yyy@xxx@@
    sal_Int32 nPos = aStr.getLength();
    while (nPos > 0)
    {
        sal_Int32 n = aStr.lastIndexOf( '@', nPos );
        aRet.append( aStr.copy( n +1, nPos -n -1 ) );
        if (n >= 0)
        {
            aRet.append( (sal_Unicode)'.' );
        }
        nPos = n;
    }
    return aRet.makeStringAndClear();
}
//==================================================================================================
static inline OUString toRTTIname( OUString const & rUNOname ) throw ()
{
    OUStringBuffer aRet( 64 );
    aRet.appendAscii( RTL_CONSTASCII_STRINGPARAM(".?AV") ); // class ".?AV"; struct ".?AU"
    sal_Int32 nPos = rUNOname.getLength();
    while (nPos > 0)
    {
        sal_Int32 n = rUNOname.lastIndexOf( '.', nPos );
        aRet.append( rUNOname.copy( n +1, nPos -n -1 ) );
        aRet.append( (sal_Unicode)'@' );
        nPos = n;
    }
    aRet.append( (sal_Unicode)'@' );
    return aRet.makeStringAndClear();
}


//##################################################################################################
//#### RTTI simulation #############################################################################
//##################################################################################################


typedef hash_map< OUString, void *, OUStringHash, equal_to< OUString > > t_string2PtrMap;

//==================================================================================================
class RTTInfos
{
    Mutex               _aMutex;
    t_string2PtrMap     _allRTTI;

    static OUString toRawName( OUString const & rUNOname ) throw ();
public:
    type_info * getRTTI( OUString const & rUNOname ) throw ();

    RTTInfos();
    ~RTTInfos();
};

//==================================================================================================
class __type_info
{
    friend type_info * RTTInfos::getRTTI( OUString const & ) throw ();
    friend int mscx_filterCppException(
        LPEXCEPTION_POINTERS, uno_Any *, uno_Mapping * );

public:
    virtual ~__type_info() throw ();

    inline __type_info( void * m_vtable, const char * m_d_name ) throw ()
        : _m_vtable( m_vtable )
        , _m_name( NULL )
        { ::strcpy( _m_d_name, m_d_name ); } // #100211# - checked

    size_t length() const
    {
        return sizeof(__type_info) + strlen(_m_d_name);
    }

private:
    void * _m_vtable;
    char * _m_name;     // cached copy of unmangled name, NULL initially
    char _m_d_name[1];  // mangled name
};
//__________________________________________________________________________________________________
__type_info::~__type_info() throw ()
{
}
//__________________________________________________________________________________________________
type_info * RTTInfos::getRTTI( OUString const & rUNOname ) throw ()
{
    // a must be
    OSL_ENSURE( sizeof(__type_info) == sizeof(type_info), "### type info structure size differ!" );

    MutexGuard aGuard( _aMutex );
    t_string2PtrMap::const_iterator const iFind( _allRTTI.find( rUNOname ) );

    // check if type is already available
    if (iFind == _allRTTI.end())
    {
        // insert new type_info
        OString aRawName( OUStringToOString( toRTTIname( rUNOname ), RTL_TEXTENCODING_ASCII_US ) );
        __type_info * pRTTI = new( ::rtl_allocateMemory( sizeof(__type_info) + aRawName.getLength() ) )
            __type_info( NULL, aRawName.getStr() );

        // put into map
        pair< t_string2PtrMap::iterator, bool > insertion(
            _allRTTI.insert( t_string2PtrMap::value_type( rUNOname, pRTTI ) ) );
        OSL_ENSURE( insertion.second, "### rtti insertion failed?!" );

        return (type_info *)pRTTI;
    }
    else
    {
        return (type_info *)iFind->second;
    }
}
//__________________________________________________________________________________________________
RTTInfos::RTTInfos() throw ()
{
}
//__________________________________________________________________________________________________
RTTInfos::~RTTInfos() throw ()
{
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "> freeing generated RTTI infos... <\n" );
#endif

    MutexGuard aGuard( _aMutex );
    for ( t_string2PtrMap::const_iterator iPos( _allRTTI.begin() );
          iPos != _allRTTI.end(); ++iPos )
    {
        __type_info * pType = (__type_info *)iPos->second;
        pType->~__type_info(); // obsolete, but good style...
        ::rtl_freeMemory( pType );
    }
}


//##################################################################################################
//#### Exception raising ###########################################################################
//##################################################################################################


//==================================================================================================
static void * __copyConstruct( void * pExcThis, void * pSource, typelib_TypeDescription *pTypeDescr )
    throw ()
{
    ::uno_copyData( pExcThis, pSource, pTypeDescr, cpp_acquire );
    return pExcThis;
}
//==================================================================================================
static void * __destruct( void * pExcThis, typelib_TypeDescription *pTypeDescr )
    throw ()
{
    ::uno_destructData( pExcThis, pTypeDescr, cpp_release );
    return pExcThis;
}

//==================================================================================================

int const codeSnippetSize = 32;

void copyConstructCodeSnippet( unsigned char * code, typelib_TypeDescription * pTypeDescr )
    throw ()
{
    unsigned char * p = code;

    // mov r8, pTypeDescr
    *p++ = 0x49;
    *p++ = 0xb8;
    *p++ = ((sal_uIntPtr)(pTypeDescr)) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 8) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 16) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 24) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 32) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 40) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 48) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 56) & 0xff;

    // mov r9, __copyConstruct
    *p++ = 0x49;
    *p++ = 0xb9;
    *p++ = ((sal_uIntPtr)(&__copyConstruct)) & 0xff;
    *p++ = (((sal_uIntPtr)(&__copyConstruct)) >> 8) & 0xff;
    *p++ = (((sal_uIntPtr)(&__copyConstruct)) >> 16) & 0xff;
    *p++ = (((sal_uIntPtr)(&__copyConstruct)) >> 24) & 0xff;
    *p++ = (((sal_uIntPtr)(&__copyConstruct)) >> 32) & 0xff;
    *p++ = (((sal_uIntPtr)(&__copyConstruct)) >> 40) & 0xff;
    *p++ = (((sal_uIntPtr)(&__copyConstruct)) >> 48) & 0xff;
    *p++ = (((sal_uIntPtr)(&__copyConstruct)) >> 56) & 0xff;

    // jmp r9
    *p++ = 0x41;
    *p++ = 0xff;
    *p++ = 0xe1;

    OSL_ASSERT(p - code <= codeSnippetSize);
}

//==================================================================================================
void destructCodeSnippet( unsigned char * code, typelib_TypeDescription * pTypeDescr )
    throw ()
{
    unsigned char * p = code;

    // mov rdx, pTypeDescr
    *p++ = 0x48;
    *p++ = 0xba;
    *p++ = ((sal_uIntPtr)(pTypeDescr)) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 8) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 16) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 24) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 32) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 40) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 48) & 0xff;
    *p++ = (((sal_uIntPtr)(pTypeDescr)) >> 56) & 0xff;

    // mov r9, __destruct
    *p++ = 0x49;
    *p++ = 0xb9;
    *p++ = ((sal_uIntPtr)(&__destruct)) & 0xff;
    *p++ = (((sal_uIntPtr)(&__destruct)) >> 8) & 0xff;
    *p++ = (((sal_uIntPtr)(&__destruct)) >> 16) & 0xff;
    *p++ = (((sal_uIntPtr)(&__destruct)) >> 24) & 0xff;
    *p++ = (((sal_uIntPtr)(&__destruct)) >> 32) & 0xff;
    *p++ = (((sal_uIntPtr)(&__destruct)) >> 40) & 0xff;
    *p++ = (((sal_uIntPtr)(&__destruct)) >> 48) & 0xff;
    *p++ = (((sal_uIntPtr)(&__destruct)) >> 56) & 0xff;

    // jmp r9
    *p++ = 0x41;
    *p++ = 0xff;
    *p++ = 0xe1;

    OSL_ASSERT(p - code <= codeSnippetSize);
}

//==================================================================================================
static size_t align16(size_t size)
{
    return ((size + 15) >> 4) << 4;
}

//==================================================================================================
// Known as "catchabletype" in https://github.com/icestudent/ontl/blob/master/ntl/nt/exception.hxx
struct ExceptionType
{
    sal_Int32           _n0;
    sal_uInt32          _pTypeInfo;    // type_info *, RVA on Win64
    sal_Int32           _n1, _n2, _n3; // pointer to member descriptor, 12 bytes.
    sal_uInt32          _n4;
    sal_uInt32          _pCopyCtor;    // RVA on Win64
    sal_Int32           _n5;

    static void initialize( unsigned char *p, sal_uInt32 typeInfoRVA, typelib_TypeDescription * pTypeDescr, sal_uInt32 copyConstructorRVA ) throw ()
    {
        ExceptionType *e = (ExceptionType*)p;
        e->_n0 = 0;
        e->_pTypeInfo = typeInfoRVA;
        e->_n1 = 0;
        e->_n2 = -1;
        e->_n3 = 0;
        e->_n4 = pTypeDescr->nSize;
        e->_pCopyCtor = copyConstructorRVA;
        e->_n5 = 0;
    }
};

//==================================================================================================
// Known as "throwinfo" in https://github.com/icestudent/ontl/blob/master/ntl/nt/exception.hxx
struct RaiseInfo
{
    // Microsoft's fields:
    sal_uInt32          _n0;
    sal_uInt32          _pDtor; // RVA on Win64
    sal_uInt32          _n2;
    sal_uInt32          _types; // void *, RVA on Win64

    // Our additional fields:
    typelib_TypeDescription * pTypeDescr;
    unsigned char       *baseAddress; // The RVAs are relative to this field

    RaiseInfo( typelib_TypeDescription * pTypeDescr ) throw ();
    ~RaiseInfo() throw ();
};
//__________________________________________________________________________________________________
RaiseInfo::RaiseInfo( typelib_TypeDescription * pTypeDescr ) throw ()
    : _n0( 0 )
    , _n2( 0 )
{
    // a must be
    OSL_ENSURE( sizeof(sal_Int32) == sizeof(ExceptionType *), "### pointer size differs from sal_Int32!" );

    ::typelib_typedescription_acquire( pTypeDescr );
    this->pTypeDescr = pTypeDescr;

    typelib_CompoundTypeDescription * pCompTypeDescr;

    size_t bytesNeeded = codeSnippetSize; // destructCodeSnippet for _pDtor
    sal_uInt32 typeCount = 0;
    for ( pCompTypeDescr = (typelib_CompoundTypeDescription*)pTypeDescr;
          pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
    {
        ++typeCount;
        bytesNeeded += align16( sizeof( ExceptionType ) );
        __type_info *typeInfo = (__type_info*) mscx_getRTTI( ((typelib_TypeDescription *)pCompTypeDescr)->pTypeName );
        bytesNeeded += align16( typeInfo->length() );
        bytesNeeded += codeSnippetSize; // copyConstructCodeSnippet for its _pCopyCtor
    }
    // type info count accompanied by RVAs of type info ptrs: type, base type, base base type, ...
    bytesNeeded += align16( sizeof( sal_uInt32 ) + (typeCount * sizeof( sal_uInt32 )) );

    unsigned char *p = (unsigned char*) ::rtl_allocateMemory( bytesNeeded );
    DWORD old_protect;
#if OSL_DEBUG_LEVEL > 0
    BOOL success =
#endif
    VirtualProtect( p, bytesNeeded, PAGE_EXECUTE_READWRITE, &old_protect );
    OSL_ENSURE( success, "VirtualProtect() failed!" );
    baseAddress = p;

    destructCodeSnippet( p, pTypeDescr );
    _pDtor = (sal_uInt32)(p - baseAddress);
    p += codeSnippetSize;

    sal_uInt32 *types = (sal_uInt32*)p;
    _types = (sal_uInt32)(p - baseAddress);
    p += align16( sizeof( sal_uInt32 ) + (typeCount * sizeof( sal_uInt32 )) );
    types[0] = typeCount;
    int next = 1;
    for ( pCompTypeDescr = (typelib_CompoundTypeDescription*)pTypeDescr;
          pCompTypeDescr; pCompTypeDescr = pCompTypeDescr->pBaseTypeDescription )
    {
        __type_info *typeInfo = (__type_info*) mscx_getRTTI( ((typelib_TypeDescription *)pCompTypeDescr)->pTypeName );
        memcpy(p, typeInfo, typeInfo->length() );
        sal_uInt32 typeInfoRVA = (sal_uInt32)(p - baseAddress);
        p += align16( typeInfo->length() );

        copyConstructCodeSnippet( p, (typelib_TypeDescription *)pCompTypeDescr );
        sal_uInt32 copyConstructorRVA = (sal_uInt32)(p - baseAddress);
        p += codeSnippetSize;

        ExceptionType::initialize( p, typeInfoRVA, (typelib_TypeDescription *)pCompTypeDescr, copyConstructorRVA );
        types[next++] = (sal_uInt32)(p - baseAddress);
        p += align16( sizeof(ExceptionType) );
    }

    OSL_ASSERT(p - baseAddress <= bytesNeeded);
}
//__________________________________________________________________________________________________
RaiseInfo::~RaiseInfo() throw ()
{
    ::rtl_freeMemory( baseAddress );
    ::typelib_typedescription_release( pTypeDescr );
}

//==================================================================================================
class ExceptionInfos
{
    Mutex           _aMutex;
    t_string2PtrMap _allRaiseInfos;

public:
    static RaiseInfo * getRaiseInfo( typelib_TypeDescription * pTypeDescr ) throw ();

    ExceptionInfos() throw ();
    ~ExceptionInfos() throw ();
};
//__________________________________________________________________________________________________
ExceptionInfos::ExceptionInfos() throw ()
{
}
//__________________________________________________________________________________________________
ExceptionInfos::~ExceptionInfos() throw ()
{
#if OSL_DEBUG_LEVEL > 1
    OSL_TRACE( "> freeing exception infos... <\n" );
#endif

    MutexGuard aGuard( _aMutex );
    for ( t_string2PtrMap::const_iterator iPos( _allRaiseInfos.begin() );
          iPos != _allRaiseInfos.end(); ++iPos )
    {
        delete (RaiseInfo *)iPos->second;
    }
}
//__________________________________________________________________________________________________
RaiseInfo * ExceptionInfos::getRaiseInfo( typelib_TypeDescription * pTypeDescr ) throw ()
{
    static ExceptionInfos * s_pInfos = 0;
    if (! s_pInfos)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pInfos)
        {
#ifdef LEAK_STATIC_DATA
            s_pInfos = new ExceptionInfos();
#else
            static ExceptionInfos s_allExceptionInfos;
            s_pInfos = &s_allExceptionInfos;
#endif
        }
    }

    OSL_ASSERT( pTypeDescr &&
                (pTypeDescr->eTypeClass == typelib_TypeClass_STRUCT ||
                 pTypeDescr->eTypeClass == typelib_TypeClass_EXCEPTION) );

    void * pRaiseInfo;

    OUString const & rTypeName = *reinterpret_cast< OUString * >( &pTypeDescr->pTypeName );
    MutexGuard aGuard( s_pInfos->_aMutex );
    t_string2PtrMap::const_iterator const iFind(
        s_pInfos->_allRaiseInfos.find( rTypeName ) );
    if (iFind == s_pInfos->_allRaiseInfos.end())
    {
        pRaiseInfo = new RaiseInfo( pTypeDescr );
        // put into map
        pair< t_string2PtrMap::iterator, bool > insertion(
            s_pInfos->_allRaiseInfos.insert( t_string2PtrMap::value_type( rTypeName, pRaiseInfo ) ) );
        OSL_ENSURE( insertion.second, "### raise info insertion failed?!" );
    }
    else
    {
        // reuse existing info
        pRaiseInfo = iFind->second;
    }

    return (RaiseInfo*) pRaiseInfo;
}


//##################################################################################################
//#### exported ####################################################################################
//##################################################################################################


//##################################################################################################
type_info * mscx_getRTTI( OUString const & rUNOname )
{
    static RTTInfos * s_pRTTIs = 0;
    if (! s_pRTTIs)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! s_pRTTIs)
        {
#ifdef LEAK_STATIC_DATA
            s_pRTTIs = new RTTInfos();
#else
            static RTTInfos s_aRTTIs;
            s_pRTTIs = &s_aRTTIs;
#endif
        }
    }
    return s_pRTTIs->getRTTI( rUNOname );
}

//##################################################################################################
void mscx_raiseException( uno_Any * pUnoExc, uno_Mapping * pUno2Cpp )
{
    // no ctor/dtor in here: this leads to dtors called twice upon RaiseException()!
    // thus this obj file will be compiled without opt, so no inling of
    // ExceptionInfos::getRaiseInfo()

    // construct cpp exception object
    typelib_TypeDescription * pTypeDescr = 0;
    TYPELIB_DANGER_GET( &pTypeDescr, pUnoExc->pType );

    void * pCppExc = alloca( pTypeDescr->nSize );
    ::uno_copyAndConvertData( pCppExc, pUnoExc->pData, pTypeDescr, pUno2Cpp );

    // a must be
    OSL_ENSURE(
        sizeof(sal_Int32) == sizeof(void *),
        "### pointer size differs from sal_Int32!" );
    RaiseInfo *raiseInfo = ExceptionInfos::getRaiseInfo( pTypeDescr );
    ULONG_PTR arFilterArgs[4];
    arFilterArgs[0] = MSVC_magic_number;
    arFilterArgs[1] = (ULONG_PTR)pCppExc;
    arFilterArgs[2] = (ULONG_PTR)raiseInfo;
    arFilterArgs[3] = (ULONG_PTR)raiseInfo->baseAddress;

    // destruct uno exception
    ::uno_any_destruct( pUnoExc, 0 );
    TYPELIB_DANGER_RELEASE( pTypeDescr );

    // last point to release anything not affected by stack unwinding
    RaiseException( MSVC_ExceptionCode, EXCEPTION_NONCONTINUABLE, 4, arFilterArgs );
}

//##############################################################################
int mscx_filterCppException(
    EXCEPTION_POINTERS * pPointers, uno_Any * pUnoExc, uno_Mapping * pCpp2Uno )
{
    if (pPointers == 0)
        return EXCEPTION_CONTINUE_SEARCH;
    EXCEPTION_RECORD * pRecord = pPointers->ExceptionRecord;
    // handle only C++ exceptions:
    if (pRecord == 0 || pRecord->ExceptionCode != MSVC_ExceptionCode)
        return EXCEPTION_CONTINUE_SEARCH;

#if _MSC_VER < 1300 // MSVC -6
    bool rethrow = (pRecord->NumberParameters < 3 ||
                    pRecord->ExceptionInformation[ 2 ] == 0);
#else
    bool rethrow = __CxxDetectRethrow( &pRecord );
    OSL_ASSERT( pRecord == pPointers->ExceptionRecord );
#endif
    if (rethrow && pRecord == pPointers->ExceptionRecord)
    {
        // hack to get msvcrt internal _curexception field:
        pRecord = *reinterpret_cast< EXCEPTION_RECORD ** >(
            reinterpret_cast< char * >( __pxcptinfoptrs() ) +
            // as long as we don't demand msvcr source as build prerequisite
            // (->platform sdk), we have to code those offsets here.
            //
            // crt\src\mtdll.h:
            // offsetof (_tiddata, _curexception) -
            // offsetof (_tiddata, _tpxcptinfoptrs):
            48
            );
    }
    // rethrow: handle only C++ exceptions:
    if (pRecord == 0 || pRecord->ExceptionCode != MSVC_ExceptionCode)
        return EXCEPTION_CONTINUE_SEARCH;

    if (pRecord->NumberParameters == 4 &&
//          pRecord->ExceptionInformation[ 0 ] == MSVC_magic_number &&
        pRecord->ExceptionInformation[ 1 ] != 0 &&
        pRecord->ExceptionInformation[ 2 ] != 0 &&
        pRecord->ExceptionInformation[ 3 ] != 0)
    {
        unsigned char *baseAddress = (unsigned char*) pRecord->ExceptionInformation[ 3 ];
        sal_uInt32 * types = (sal_uInt32*)(baseAddress + reinterpret_cast< RaiseInfo * >(
            pRecord->ExceptionInformation[ 2 ] )->_types );
        if (types != 0 && (sal_uInt32)(types[0]) > 0) // count
        {
            ExceptionType * pType = reinterpret_cast< ExceptionType * >(
                baseAddress + types[ 1 ] );
            if (pType != 0 && pType->_pTypeInfo != 0)
            {
                OUString aRTTIname(
                    OStringToOUString(
                        reinterpret_cast< __type_info * >(
                            baseAddress + pType->_pTypeInfo )->_m_d_name,
                        RTL_TEXTENCODING_ASCII_US ) );
                OUString aUNOname( toUNOname( aRTTIname ) );

                typelib_TypeDescription * pExcTypeDescr = 0;
                typelib_typedescription_getByName(
                    &pExcTypeDescr, aUNOname.pData );
                if (pExcTypeDescr == 0)
                {
                    OUStringBuffer buf;
                    buf.appendAscii(
                        RTL_CONSTASCII_STRINGPARAM(
                            "[mscx_uno bridge error] UNO type of "
                            "C++ exception unknown: \"") );
                    buf.append( aUNOname );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM(
                                         "\", RTTI-name=\"") );
                    buf.append( aRTTIname );
                    buf.appendAscii( RTL_CONSTASCII_STRINGPARAM("\"!") );
                    RuntimeException exc(
                        buf.makeStringAndClear(), Reference< XInterface >() );
                    uno_type_any_constructAndConvert(
                        pUnoExc, &exc,
                        ::getCppuType( &exc ).getTypeLibType(), pCpp2Uno );
#if _MSC_VER < 1400 // msvcr80.dll cleans up, different from former msvcrs
                    // if (! rethrow):
                    // though this unknown exception leaks now, no user-defined
                    // exception is ever thrown through the binary C-UNO dispatcher
                    // call stack.
#endif
                }
                else
                {
                    // construct uno exception any
                    uno_any_constructAndConvert(
                        pUnoExc, (void *) pRecord->ExceptionInformation[1],
                        pExcTypeDescr, pCpp2Uno );
#if _MSC_VER < 1400 // msvcr80.dll cleans up, different from former msvcrs
                    if (! rethrow)
                    {
                        uno_destructData(
                            (void *) pRecord->ExceptionInformation[1],
                            pExcTypeDescr, cpp_release );
                    }
#endif
                    typelib_typedescription_release( pExcTypeDescr );
                }

                return EXCEPTION_EXECUTE_HANDLER;
            }
        }
    }
    // though this unknown exception leaks now, no user-defined exception
    // is ever thrown through the binary C-UNO dispatcher call stack.
    RuntimeException exc(
        OUString( RTL_CONSTASCII_USTRINGPARAM(
                      "[mscx_uno bridge error] unexpected "
                      "C++ exception occurred!") ),
        Reference< XInterface >() );
    uno_type_any_constructAndConvert(
        pUnoExc, &exc, ::getCppuType( &exc ).getTypeLibType(), pCpp2Uno );
    return EXCEPTION_EXECUTE_HANDLER;
}

}

#pragma pack(pop)

