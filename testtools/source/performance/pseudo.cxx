/**************************************************************************
#*
#*    last change   $Author: vg $ $Date: 2003-04-15 16:42:32 $
#*    $Revision: 1.2 $
#*
#*    $Logfile: $
#*
#*    Copyright (c) 1989 - 2000, Star Office GmbH
#*
#************************************************************************/

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _OSL_INTERLOCK_H_
#include <osl/interlck.h>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _TYPELIB_TYPEDESCRIPTION_H_
#include <typelib/typedescription.h>
#endif
#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h>
#endif
#ifndef _UNO_ENVIRONMENT_H_
#include <uno/environment.h>
#endif
#ifndef _UNO_MAPPING_H_
#include <uno/mapping.h>
#endif
#ifndef _UNO_LBNAMES_H_
#include <uno/lbnames.h>
#endif

using namespace rtl;


namespace pseudo_uno
{

//==================================================================================================
struct pseudo_Mapping : public uno_Mapping
{
    oslInterlockedCount     nRef;

    uno_ExtEnvironment *    pFrom;
    uno_ExtEnvironment *    pTo;

    pseudo_Mapping( uno_ExtEnvironment * pFrom_, uno_ExtEnvironment * pTo_ );
    ~pseudo_Mapping();
};

//==== a uno pseudo proxy =============================================================================
struct pseudo_unoInterfaceProxy : public uno_Interface
{
    oslInterlockedCount                 nRef;
    pseudo_Mapping *                    pPseudoMapping;

    // mapping information
    uno_Interface *                     pUnoI; // wrapped interface
    typelib_InterfaceTypeDescription *  pTypeDescr;
    OUString                            oid;

    // ctor
    inline pseudo_unoInterfaceProxy( pseudo_Mapping * pPseudoMapping_,
                                     uno_Interface * pUnoI_,
                                     typelib_InterfaceTypeDescription * pTypeDescr_,
                                     const OUString & rOId_ );
};
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_unoInterfaceProxy_dispatch(
    uno_Interface * pUnoI,
    const typelib_TypeDescription * pMemberType,
    void * pReturn,
    void * pArgs[],
    uno_Any ** ppException )
{
    pseudo_unoInterfaceProxy * pThis = static_cast< pseudo_unoInterfaceProxy * >( pUnoI );
    (*pThis->pUnoI->pDispatcher)( pThis->pUnoI, pMemberType, pReturn, pArgs, ppException );
}

//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_unoInterfaceProxy_free( uno_ExtEnvironment * pEnv, void * pProxy )
{
    pseudo_unoInterfaceProxy * pThis =
        static_cast< pseudo_unoInterfaceProxy * >(
            reinterpret_cast< uno_Interface * >( pProxy ) );
    OSL_ASSERT( pEnv == pThis->pPseudoMapping->pTo );

    (*pThis->pPseudoMapping->pFrom->revokeInterface)( pThis->pPseudoMapping->pFrom, pThis->pUnoI );
    (*pThis->pUnoI->release)( pThis->pUnoI );
    typelib_typedescription_release( (typelib_TypeDescription *)pThis->pTypeDescr );
    (*pThis->pPseudoMapping->release)( pThis->pPseudoMapping );

#if OSL_DEBUG_LEVEL > 1
    *(int *)pProxy = 0xdeadbabe;
#endif
    delete pThis;
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_unoInterfaceProxy_acquire( uno_Interface * pUnoI )
{
    if (1 == osl_incrementInterlockedCount( &static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // rebirth of proxy zombie
        // register at uno env
        void * pThis = static_cast< uno_Interface * >( pUnoI );
        (*static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pPseudoMapping->pTo->registerProxyInterface)(
            static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pPseudoMapping->pTo,
            &pThis, pseudo_unoInterfaceProxy_free,
            static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->oid.pData,
            static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pTypeDescr );
        OSL_ASSERT( pThis == static_cast< uno_Interface * >( pUnoI ) );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_unoInterfaceProxy_release( uno_Interface * pUnoI )
{
    if (! osl_decrementInterlockedCount( & static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->nRef ))
    {
        // revoke from uno env on last release
        (*static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pPseudoMapping->pTo->revokeInterface)(
            static_cast< pseudo_unoInterfaceProxy * >( pUnoI )->pPseudoMapping->pTo, pUnoI );
    }
}
//__________________________________________________________________________________________________
inline pseudo_unoInterfaceProxy::pseudo_unoInterfaceProxy(
    pseudo_Mapping * pPseudoMapping_, uno_Interface * pUnoI_,
    typelib_InterfaceTypeDescription * pTypeDescr_, const OUString & rOId_ )
    : nRef( 1 )
    , pPseudoMapping( pPseudoMapping_ )
    , pUnoI( pUnoI_ )
    , pTypeDescr( pTypeDescr_ )
    , oid( rOId_ )
{
    (*pPseudoMapping->acquire)( pPseudoMapping );
    typelib_typedescription_acquire( (typelib_TypeDescription *)pTypeDescr );
    (*pPseudoMapping->pFrom->registerInterface)(
        pPseudoMapping->pFrom, reinterpret_cast< void ** >( &pUnoI ), oid.pData, pTypeDescr );
    (*pUnoI->acquire)( pUnoI );

    // uno_Interface
    uno_Interface::acquire = pseudo_unoInterfaceProxy_acquire;
    uno_Interface::release = pseudo_unoInterfaceProxy_release;
    uno_Interface::pDispatcher = pseudo_unoInterfaceProxy_dispatch;
}

//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_Mapping_mapInterface(
    uno_Mapping * pMapping, void ** ppOut,
    void * pUnoI, typelib_InterfaceTypeDescription * pTypeDescr )
{
    OSL_ASSERT( ppOut && pTypeDescr );
    if (*ppOut)
    {
        (*reinterpret_cast< uno_Interface * >( *ppOut )->release)(
            reinterpret_cast< uno_Interface * >( *ppOut ) );
        *ppOut = 0;
    }
    if (pUnoI && pTypeDescr)
    {
        // get object id of uno interface to be wrapped
        rtl_uString * pOId = 0;
        (*static_cast< pseudo_Mapping * >( pMapping )->pFrom->getObjectIdentifier)(
            static_cast< pseudo_Mapping * >( pMapping )->pFrom, &pOId, pUnoI );
        OSL_ASSERT( pOId );

        if (pOId)
        {
            // try to get any known interface from target environment
            (*static_cast< pseudo_Mapping * >( pMapping )->pTo->getRegisteredInterface)(
                static_cast< pseudo_Mapping * >( pMapping )->pTo, ppOut, pOId, pTypeDescr );
            if (! *ppOut) // no existing interface, register new proxy interface
            {
                // try to publish a new proxy (ref count initially 1)
                void * pProxy = new pseudo_unoInterfaceProxy(
                    static_cast< pseudo_Mapping * >( pMapping ),
                    reinterpret_cast< uno_Interface * >( pUnoI ), pTypeDescr, pOId );

                // proxy may be exchanged during registration
                (*static_cast< pseudo_Mapping * >( pMapping )->pTo->registerProxyInterface)(
                    static_cast< pseudo_Mapping * >( pMapping )->pTo,
                    &pProxy, pseudo_unoInterfaceProxy_free, pOId, pTypeDescr );

                *ppOut = pProxy;
            }
            rtl_uString_release( pOId );
        }
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_Mapping_free( uno_Mapping * pMapping )
{
    delete static_cast< pseudo_Mapping * >( pMapping );
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_Mapping_acquire( uno_Mapping * pMapping )
{
    if (1 == osl_incrementInterlockedCount( & static_cast< pseudo_Mapping * >( pMapping )->nRef ))
    {
        OUString aMappingPurpose( RTL_CONSTASCII_USTRINGPARAM("pseudo") );
        uno_registerMapping( &pMapping,
                             pseudo_Mapping_free,
                             (uno_Environment *)((pseudo_Mapping *)pMapping)->pFrom,
                             (uno_Environment *)((pseudo_Mapping *)pMapping)->pTo,
                             aMappingPurpose.pData );
    }
}
//--------------------------------------------------------------------------------------------------
static void SAL_CALL pseudo_Mapping_release( uno_Mapping * pMapping )
{
    if (! osl_decrementInterlockedCount( & static_cast< pseudo_Mapping * >( pMapping )->nRef ))
    {
        uno_revokeMapping( pMapping );
    }
}

//__________________________________________________________________________________________________
pseudo_Mapping::pseudo_Mapping( uno_ExtEnvironment * pFrom_, uno_ExtEnvironment * pTo_ )
    : nRef( 1 )
    , pFrom( pFrom_ )
    , pTo( pTo_ )
{
    (*((uno_Environment *)pFrom)->acquire)( (uno_Environment *)pFrom );
    (*((uno_Environment *)pTo)->acquire)( (uno_Environment *)pTo );
    //
    uno_Mapping::acquire = pseudo_Mapping_acquire;
    uno_Mapping::release = pseudo_Mapping_release;
    uno_Mapping::mapInterface = pseudo_Mapping_mapInterface;
}
//__________________________________________________________________________________________________
pseudo_Mapping::~pseudo_Mapping()
{
    (*((uno_Environment *)pTo)->release)( (uno_Environment *)pTo );
    (*((uno_Environment *)pFrom)->release)( (uno_Environment *)pFrom );
}

}

//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_initEnvironment( uno_Environment * pUnoEnv )
{
    OSL_ENSURE( sal_False, "### no impl: unexpected call!" );
}
//##################################################################################################
extern "C" SAL_DLLEXPORT void SAL_CALL uno_ext_getMapping(
    uno_Mapping ** ppMapping, uno_Environment * pFrom, uno_Environment * pTo )
{
    OSL_ASSERT( ppMapping && pFrom && pTo );
    if (ppMapping && pFrom && pTo && pFrom->pExtEnv && pTo->pExtEnv)
    {
        uno_Mapping * pMapping = 0;

        if (0 == rtl_ustr_ascii_compare( pFrom->pTypeName->buffer, UNO_LB_UNO ) &&
            0 == rtl_ustr_ascii_compare( pTo->pTypeName->buffer, UNO_LB_UNO ))
        {
            OUString aMappingPurpose( RTL_CONSTASCII_USTRINGPARAM("pseudo") );
            // ref count is initially 1
            pMapping = new pseudo_uno::pseudo_Mapping( pFrom->pExtEnv, pTo->pExtEnv );
            uno_registerMapping( &pMapping, pseudo_uno::pseudo_Mapping_free,
                                 (uno_Environment *)pFrom->pExtEnv,
                                 (uno_Environment *)pTo->pExtEnv,
                                 aMappingPurpose.pData );
        }

        if (*ppMapping)
            (*(*ppMapping)->release)( *ppMapping );
        *ppMapping = pMapping;
    }
}


/**************************************************************************
    $Log: not supported by cvs2svn $
    Revision 1.1.46.1  2003/04/10 09:20:16  kso
    #108413# - debug macro unification.

    Revision 1.1  2001/05/04 07:05:18  kr
    moved from grande to openoffice

    Revision 1.3  2001/03/12 16:22:44  jl
    OSL_ENSHURE replaced by OSL_ENSURE

    Revision 1.2  2000/06/15 08:58:08  dbo
    runtime changes

    Revision 1.1  2000/05/22 12:50:36  dbo
    new


**************************************************************************/
