/*************************************************************************
 *
 *  $RCSfile: javavm.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:29:34 $
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


#if STLPORT_VERSION < 321
//  #include <tools/presys.h>
#include <hash_map.h>
//  #include <tools/postsys.h>
#else
#include <stl/vector>
#include <stl/hash_map>
#include <cstdarg>          // std::va_list and friends
#endif

//#include "jre.hxx"
#include <jni.h>
#include <time.h>
#ifdef UNX
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#include <cppuhelper/typeprovider.hxx>

#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif

#ifndef _VOS_PROCESS_HXX_
#include <vos/process.hxx>
#endif

#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif

#ifndef _VOS_MODULE_HXX_
#include <vos/module.hxx>
#endif

#ifndef _VOS_THREAD_HXX_
#include <vos/thread.hxx>
#endif

#ifndef _VOS_CONDITN_HXX_
#include <vos/conditn.hxx>
#endif

#ifndef ___JSETTINGS
#include "settings.hxx"
#endif

#ifndef _VOS_PROFILE_HXX_
#include <vos/profile.hxx>
#endif

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

//  #ifndef _STRING_HXX
//  #include <tools/string.hxx>
//  #endif

//  #ifndef _ISOLANG_HXX
//  #include <tools/isolang.hxx>
//  #endif

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <com/sun/star/java/XJavaVM.hpp>
#include <com/sun/star/java/XJavaThreadRegister_11.hpp>
#include <com/sun/star/uno/Exception.hpp>

#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/registry/InvalidRegistryException.hpp>
#include <com/sun/star/frame/XConfigManager.hpp>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::java;
using namespace com::sun::star::registry;
using namespace com::sun::star::frame;
using namespace vos;
using namespace rtl;
using namespace cppu;
using namespace osl;

#define UNRESOLVED_AWT_SYMBOLS
#if defined(SOLARIS) && defined (UNRESOLVED_AWT_SYMBOLS)
#include <dlfcn.h>
#endif
#if defined(LINUX)
#include <dlfcn.h>
#endif

#if defined(OS2)
#define NOVMTHREAD
#endif

#include "jvmargs.hxx"

#if defined(WNT) || defined(OS2)
#define HAS_SETTINGS_FALLBACK
#endif

#ifdef UNIX
#define INI_FILE "javarc"
#define SLASH_INI_FILE "/javarc"
#else
#define INI_FILE "java.ini"
#define SLASH_INI_FILE "/java.ini"
#endif

#ifdef OS2
#define JCALL JNICALL0
#else
#define JCALL JNICALL
#endif

//=========================================================================
static jint JCALL vm_vfprintf( FILE *fp, const char *format, va_list args )
{
#ifdef DEBUG
    char buff[1024];

    vsprintf( buff, format, args );

    VOS_TRACE("%s", buff);
    return strlen(buff);
#else
    return 0;
#endif
}

//=========================================================================

static void JCALL vm_exit(jint code)
{
  VOS_TRACE("vm_exit: %d\n", code);
}

static void JCALL vm_abort()
{
  VOS_TRACE("vm_abort\n");
}

class VMException : public Exception
{
    OUString    errDescription;
    sal_Int32   errNum;

public:
    VMException( sal_Int32 val )
    {
        errNum = val;
        errDescription = OUString::createFromAscii("error: #") + OUString::valueOf( val );
    }

    VMException( sal_Int32 val, const sal_Unicode* what )
    {
        errNum = val;
        errDescription = OUString::createFromAscii("error: #") + OUString::valueOf( val ) + OUString( what );
    }

    VMException( sal_Int32 val, const char* what )
    {
        errNum = val;
        errDescription = OUString::createFromAscii("error: #") + OUString::valueOf( val ) + OUString::createFromAscii( what );

    }

    sal_Int32 getErrNum()
    {
        return errNum;
    }

/*  virtual const sal_Unicode * what() const throw()
    {
        return errDescription.getStr();
    }
*/

    virtual OUString what() const throw()
    {
        return OUString ( errDescription );
    }

};

typedef ::std::hash_map
<
    sal_uInt32,
    sal_uInt32,
    ::std::hash<sal_uInt32>,
    ::std::equal_to<sal_uInt32>
> UINT32_UINT32_HashMap;

struct JavaVirtualMachine_Mutex
{
    OMutex aMutex;
};

class JavaVirtualMachine_Impl;

class OCreatorThread : public OThread
{
private:
    JavaVirtualMachine_Impl * pJavaVirtualMachine_Impl;
    JAVAVM * pJVM;

    OCondition start_Condition;
    OCondition wait_Condition;

    XSimpleRegistry* pJavaReg;
    XSimpleRegistry* pSOfficeReg;

protected:
    virtual void SAL_CALL run();

public:

    OCreatorThread(JavaVirtualMachine_Impl * pJavaVirtualMachine_Impl);
    JAVAVM * createJavaVM( XSimpleRegistry* pJavaReg, XSimpleRegistry* pSOfficeReg );
    void disposeJavaVM();

};

class JavaVirtualMachine_Impl :
    public ::com::sun::star::java::XJavaVM,
    public ::com::sun::star::java::XJavaThreadRegister_11,
    public ::com::sun::star::lang::XServiceInfo,
    public ::com::sun::star::lang::XTypeProvider,
    public JavaVirtualMachine_Mutex,
    public ::cppu::OWeakObject
{
public:
    int error;

    JavaVirtualMachine_Impl( const Reference< XMultiServiceFactory> & rSMgr );
    ~JavaVirtualMachine_Impl();

    virtual Any SAL_CALL queryInterface( const Type & aType  )
                        throw( RuntimeException );
    virtual void    SAL_CALL acquire() throw()
                        { OWeakObject::acquire(); }
    virtual void    SAL_CALL release() throw()
                        { OWeakObject::release(); }

    // XTypeProvider
    virtual Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
       throw(RuntimeException);
    virtual Sequence< sal_Int8 > SAL_CALL getImplementationId()
       throw( RuntimeException);

    // XJavaVM
    virtual Any SAL_CALL getJavaVM( const Sequence<sal_Int8>& processID )
                            throw(RuntimeException);
    virtual sal_Bool SAL_CALL isVMStarted(void)
                        throw( RuntimeException);
    virtual sal_Bool SAL_CALL isVMEnabled(void)
                        throw( RuntimeException);

    // XJavaThreadRegister_11
    virtual sal_Bool SAL_CALL isThreadAttached(void)
                        throw( RuntimeException);
    virtual void SAL_CALL registerThread(void)
                        throw( RuntimeException);
    virtual void SAL_CALL revokeThread(void)
                        throw( RuntimeException);

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
                        throw( RuntimeException);
    virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& ServiceName)
                        throw( RuntimeException);
    virtual Sequence< OUString > SAL_CALL  getSupportedServiceNames(void)
                        throw( RuntimeException);


    static OUString SAL_CALL getImplementationName_Static()
    {
        return OUString::createFromAscii("com.sun.star.comp.stoc.JavaVirtualMachine");
    }
    static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static()
    {
        Sequence< OUString > aSNS( 1 );
        aSNS.getArray()[0] = OUString::createFromAscii("com.sun.star.java.JavaVirtualMachine");
        return aSNS;
    }

    JAVAVM *                createJavaVM( XSimpleRegistry* pJavaReg, XSimpleRegistry* pSOfficeReg );
    void                    disposeJavaVM();

private:
    void setError(int);

#ifndef NOVMTHREAD
    OCreatorThread          creatorThread;
#endif

    OModule aJavaLib;

    JAVAVM  *                       pJavaVM;        /* denotes a Java VM */
    Reference<XMultiServiceFactory> xSMgr;
    UINT32_UINT32_HashMap           aRegisterdThreadMap;

    void                            readINetSection( XSimpleRegistry* pOfficeReg, JVM * pJVM ) throw (VMException);
    void                            readJavaSection( XSimpleRegistry* pJavaReg, JVM * pJVM );
    void                            setLanguageAndCountrySection( XSimpleRegistry* pOfficeReg, JVM* pjvm );

    sal_Bool                            getSettings( JSettings & rSettings,
                                                 const Reference<XRegistryKey> xRegistryKey );

    OUString                        getJavaIniFileName( const Reference<XSimpleRegistry>& xSofficeReg,
                                                        const Reference<XConfigManager>& xConfigManager );

};

void JavaVirtualMachine_Impl::setError(int error)
{
    this->error = error;
}

OCreatorThread::OCreatorThread( JavaVirtualMachine_Impl * pJavaVirtualMachine_Impl )
    : pJVM(NULL)
{
    this->pJavaVirtualMachine_Impl = pJavaVirtualMachine_Impl;
}

void OCreatorThread::run()
{
    start_Condition.wait();
    start_Condition.reset();
    pJVM = pJavaVirtualMachine_Impl->createJavaVM( pJavaReg, pSOfficeReg );
    wait_Condition.set();

#if defined(WNT) || defined(OS2)
    suspend();

#else
    start_Condition.wait();
    start_Condition.reset();

    if ( pJVM )
    {
          pJavaVirtualMachine_Impl->disposeJavaVM();
    }

    wait_Condition.set();
#endif
}

JAVAVM * OCreatorThread::createJavaVM( XSimpleRegistry* pJavaReg, XSimpleRegistry* pSOfficeReg )
{
    this->pJavaReg      = pJavaReg;
    this->pSOfficeReg   = pSOfficeReg;

    create();

    if ( !pJVM )
    {
        start_Condition.set();

        wait_Condition.wait();
        wait_Condition.reset();
    }
    return pJVM;
}

void OCreatorThread::disposeJavaVM()
{
    start_Condition.set(); // start disposing vm

#ifdef UNX
    wait_Condition.wait(); // wait until disposed
    wait_Condition.reset();
#endif
}


//*************************************************************************
//
//  CLASS JAVA_VM IMPLEMENTATION
//


//*************************************************************************
// JavaVirtualMachine_Impl_CreateInstance()
static Reference< XInterface> SAL_CALL JavaVirtualMachine_Impl_createInstance( const Reference< XMultiServiceFactory> & rSMgr )
        throw (VMException)
{
    XJavaVM *pJVM= SAL_STATIC_CAST( XJavaVM*, new JavaVirtualMachine_Impl( rSMgr ));
    return Reference<XInterface> ( SAL_STATIC_CAST( XInterface*, pJVM));

}



Sequence< Type > SAL_CALL JavaVirtualMachine_Impl::getTypes(void)
        throw( RuntimeException )
{
  static OTypeCollection *pCollection = 0;
  if ( ! pCollection )
  {
      MutexGuard guard( Mutex::getGlobalMutex() );
      if ( ! pCollection )
      {
          static OTypeCollection collection(
            getCppuType( (Reference < XTypeProvider > * ) 0 ),
            getCppuType( (Reference < XJavaVM >  * ) 0 ),
            getCppuType( (Reference < XServiceInfo > *)0 ) ,
            getCppuType( (Reference < XJavaThreadRegister_11 > *) 0 ) );
          pCollection = &collection;
      }
  }

  return (*pCollection).getTypes();
}

Sequence< sal_Int8 > SAL_CALL JavaVirtualMachine_Impl::getImplementationId(  )
        throw( RuntimeException )
{
  static OImplementationId *pId = 0;
  if ( ! pId )
  {
      MutexGuard guard( Mutex::getGlobalMutex() );
      if ( ! pId )
      {
          static OImplementationId id( sal_False );
          pId = &id;
      }
  }
  return (*pId).getImplementationId();
}

//*************************************************************************/
//
Any SAL_CALL JavaVirtualMachine_Impl::queryInterface( const Type &aType )
        throw( RuntimeException )

{
    Any a = ::cppu::queryInterface( aType,
                        SAL_STATIC_CAST( XJavaVM * ,this),
                        SAL_STATIC_CAST( XServiceInfo *, this),
                        SAL_STATIC_CAST( XJavaThreadRegister_11 *, this ),
                        SAL_STATIC_CAST( XTypeProvider *, this) );
    if ( a.hasValue() )
    {
        return a;
    }
    return OWeakObject::queryInterface( aType );
}

// XServiceInfo
OUString SAL_CALL JavaVirtualMachine_Impl::getImplementationName()
        throw( RuntimeException )
{
    return JavaVirtualMachine_Impl::getImplementationName_Static();
}

// XServiceInfo
sal_Bool SAL_CALL JavaVirtualMachine_Impl::supportsService( const OUString& ServiceName )
        throw( RuntimeException )
{
    Sequence< OUString > aSNL = getSupportedServiceNames();
    const OUString * pArray = aSNL.getConstArray();

    for ( sal_Int32 i = 0; i < aSNL.getLength(); i++ )
        if ( pArray[i] == ServiceName )
            return sal_True;

    return sal_False;
}

// XServiceInfo
Sequence< OUString > SAL_CALL JavaVirtualMachine_Impl::getSupportedServiceNames()
    throw( RuntimeException)
{
    return getSupportedServiceNames_Static();
}


static OUString getValue( const Reference<XRegistryKey>& xRegistryRootKey, const OUString& rKey )
{
    OUString aEntryValue;

    if ( xRegistryRootKey.is() && xRegistryRootKey->isValid() )
    {
        try
        {
            Reference<XRegistryKey> aEntry = xRegistryRootKey->openKey( rKey );

            if ( aEntry.is() && aEntry->isValid() )
            {
                aEntryValue = aEntry->getStringValue();
            }
        }
        catch (InvalidRegistryException e)
        {
        }
        catch (InvalidValueException e)
        {
        }
    }

    return aEntryValue;
}

JavaVirtualMachine_Impl::JavaVirtualMachine_Impl( const Reference<XMultiServiceFactory> & rSMgr)
    : pJavaVM(NULL),
      xSMgr(rSMgr),
      error(0)
#ifndef NOVMTHREAD
    , creatorThread(this)
#endif
{
}

JavaVirtualMachine_Impl::~JavaVirtualMachine_Impl()
{
    if ( pJavaVM )
    {
#ifndef NOVMTHREAD
        creatorThread.disposeJavaVM();
#else
        disposeJavaVM();
#endif
    }
}

// XJavaVM
Any JavaVirtualMachine_Impl::getJavaVM( const Sequence<sal_Int8> & processId ) throw (RuntimeException)
{
    OGuard aGuard( aMutex );

    Sequence<sal_Int8> localProcessID(16);
    rtl_getGlobalProcessId( (sal_uInt8*) localProcessID.getArray() );

    if ( localProcessID == processId && !pJavaVM  && !error )
    {
        Reference<XInterface>       xInterfaceRef_1( xSMgr->createInstance( OUString::createFromAscii("com.sun.star.config.SpecialConfigManager") ) );
        Reference<XSimpleRegistry>  xSofficeReg(xInterfaceRef_1, UNO_QUERY);

        // now we try to open it
        // sal_Char    buffer[_MAX_PATH] = "";

//          OUString aOfficeIni;
//          OProfile::getProfileName( aOfficeIni, OUString::createFromAscii( "soffice" ),
//                                    OUString::createFromAscii( STAROFFICE_SVERSION_KEY ) );

        //OProfile::getProfileName( buffer, _MAX_PATH, 0, STAROFFICE_SVERSION_KEY );

        try
        {
            // xSofficeReg->open( aOfficeIni, sal_True, sal_False );

            if ( ! xSofficeReg->isValid() )
            {
                InvalidRegistryException e;
                e.Message  = OUString::createFromAscii("can not open ");
//                  e.Message += aOfficeIni;
                throw e;
            }
        }
        catch ( InvalidRegistryException e )
        {
            e.Message  = OUString::createFromAscii("can not open ");
//              e.Message += aOfficeIni;
            throw e;

        }

        OUString aJavaIniFileName;
        // get name for java.ini
        if ( xSofficeReg.is() &&  xSofficeReg->isValid() )
        {
            Reference<XConfigManager> xConfigManager( xInterfaceRef_1, UNO_QUERY );
            aJavaIniFileName = getJavaIniFileName( xSofficeReg, xConfigManager );
        }

        if ( ! aJavaIniFileName.len() ) // search the java.ini beneath the executable
        {
            aJavaIniFileName = OUString::createFromAscii( INI_FILE );
        }

        Reference<XInterface> xInterfaceRef_2( xSMgr->createInstance(
                    OUString::createFromAscii("com.sun.star.config.ConfigManager") ) );
        Reference<XSimpleRegistry> xJavaReg( xInterfaceRef_2, UNO_QUERY );

        try
        {
            OUString tmpJavaIni;
            File::getFileURLFromNormalizedPath( aJavaIniFileName, tmpJavaIni );
            xJavaReg->open( tmpJavaIni, sal_True, sal_False );

            if ( xJavaReg->isValid() )
            {
                sal_Bool bIsVMEnabled = sal_False;

                Reference<XRegistryKey> xJavaIni = xJavaReg->getRootKey();

                OUString aEntryValue = getValue( xJavaIni, OUString::createFromAscii("Java/Java") );

#ifdef HAS_SETTINGS_FALLBACK
                bIsVMEnabled = aEntryValue.len() == 0 || aEntryValue.compareToAscii("1") == 0;
#else
                   bIsVMEnabled = aEntryValue.len() > 0 && aEntryValue.compareToAscii("1") == 0;
#endif

                if ( bIsVMEnabled )
                {
#ifndef NOVMTHREAD
                    pJavaVM = creatorThread.createJavaVM( xJavaReg.get(), xSofficeReg.get() );
#else
                    pJavaVM = createJavaVM( xJavaReg, xSofficeReg );
#endif
                }

                xJavaReg->close();
            }
            else
            {
                InvalidRegistryException e;
                e.Message  = OUString::createFromAscii("can not open ");
                e.Message += aJavaIniFileName;
                throw e;
            }
        }
        catch ( InvalidRegistryException e )
        {
            e.Message  = OUString::createFromAscii("can not open ");
            e.Message += aJavaIniFileName;
            throw e;
        }

        // don't close soffice
        // if ( xSofficeReg.is() &&  xSofficeReg->isValid() )
        // {
        //  xSofficeReg->close();
        // }
    }
    Any aRet;
    if( sizeof( pJavaVM ) == sizeof( sal_Int32 ) )
    {
        // 32 bit system
        sal_Int32 nP = (sal_Int32)pJavaVM;
        aRet <<= nP;
    }
    else if( sizeof( pJavaVM ) == sizeof( sal_Int64 ) )
    {
        // 64 bit system
        sal_Int64 nP = (sal_Int64)pJavaVM;
        aRet <<= nP;
    }
    return aRet;
}

// XJavaVM
sal_Bool JavaVirtualMachine_Impl::isVMStarted(void)
          throw ( RuntimeException )
{
    return pJavaVM != NULL;
}

// XJavaVM
sal_Bool JavaVirtualMachine_Impl::isVMEnabled(void)
          throw ( RuntimeException)
{
    Reference<XInterface> xInterfaceRef_1( xSMgr->createInstance(
                OUString::createFromAscii("com.sun.star.config.SpecialConfigManager") ) );
    Reference<XSimpleRegistry> xSofficeReg( xInterfaceRef_1, UNO_QUERY );

    // now we try to open it
    // sal_Char    buffer[_MAX_PATH] = "";
    // OProfile::getProfileName( buffer, _MAX_PATH, 0, STAROFFICE_SVERSION_KEY );

//      OUString aOfficeIni;
//      OProfile::getProfileName( aOfficeIni, OUString::createFromAscii( "soffice" ),
//                                OUString::createFromAscii( STAROFFICE_SVERSION_KEY ) );

    try
    {
        // xSofficeReg->open( aOfficeIni, sal_True, sal_False );

        if ( ! xSofficeReg->isValid() )
        {
            InvalidRegistryException e;
            e.Message  = OUString::createFromAscii("can not open ");
//              e.Message += aOfficeIni;
            throw e;
        }
    }
    catch ( InvalidRegistryException e )
    {
        e.Message  = OUString::createFromAscii("can not open ");
//          e.Message += aOfficeIni;
        throw e;
    }

    OUString aJavaIniFileName;
    // get name for java.ini
    if ( xSofficeReg.is() &&  xSofficeReg->isValid() )
    {
        Reference<XConfigManager> xConfigManager( xInterfaceRef_1, UNO_QUERY );
        aJavaIniFileName = getJavaIniFileName( xSofficeReg, xConfigManager );
    }

    if ( ! aJavaIniFileName.len() ) // search the java.ini beneath the executable
    {
        aJavaIniFileName = OUString::createFromAscii( INI_FILE );
    }

    Reference<XInterface> xInterfaceRef_2( xSMgr->createInstance(
                OUString::createFromAscii("com.sun.star.config.ConfigManager") ) );
    Reference<XSimpleRegistry> xJavaReg( xInterfaceRef_2, UNO_QUERY );

    try
    {
        xJavaReg->open( aJavaIniFileName, sal_True, sal_False );
        if ( xJavaReg->isValid() )
        {
            Reference<XRegistryKey> xJavaIni = xJavaReg->getRootKey();
            OUString aEntryValue = getValue( xJavaIni, OUString::createFromAscii("Java/Java") );
            xJavaReg->close();

#ifdef HAS_SETTINGS_FALLBACK
            return aEntryValue.len() == 0 || aEntryValue.compareToAscii("1") == 0;
#else
            return aEntryValue.len() > 0 && aEntryValue.compareToAscii("1") == 0;
#endif
        }
        else
        {
            InvalidRegistryException e;
            e.Message  = OUString::createFromAscii("can not open ");
            e.Message += aJavaIniFileName;
            throw e;
        }

    }
    catch ( InvalidRegistryException e )
    {
        e.Message  = OUString::createFromAscii("can not open ");
        e.Message += aJavaIniFileName;
        throw e;
    }

    return sal_False;
}

// XJavaThreadRegister_11
sal_Bool JavaVirtualMachine_Impl::isThreadAttached( void )
          throw ( RuntimeException)
{
    sal_Int32 nThreadID = OThread::getCurrentIdentifier();

    // Mutex holen
    OGuard aGuard(aMutex);
    return aRegisterdThreadMap.end() != aRegisterdThreadMap.find(nThreadID);
}

// XJavaThreadRegister_11
void JavaVirtualMachine_Impl::registerThread(void)
          throw ( RuntimeException)
{
    sal_Int32 nThreadID = OThread::getCurrentIdentifier();

    // Mutex holen
    OGuard aGuard(aMutex);

    UINT32_UINT32_HashMap::iterator aIt = aRegisterdThreadMap.find(nThreadID);

    if ( aIt == aRegisterdThreadMap.end() )
        aRegisterdThreadMap[nThreadID] = 1;
    else
        (*aIt).second++;
}

// XJavaThreadRegister_11
void JavaVirtualMachine_Impl::revokeThread(void)
          throw ( RuntimeException)
{
    sal_Int32 nThreadID = OThread::getCurrentIdentifier();

    // Mutex holen
    OGuard aGuard(aMutex);

    UINT32_UINT32_HashMap::iterator aIt = aRegisterdThreadMap.find(nThreadID);
    VOS_ASSERT( aIt != aRegisterdThreadMap.end() );
    if ( aIt != aRegisterdThreadMap.end() )
    {
        if ( 0 == --((*aIt).second) )
            aRegisterdThreadMap.erase( nThreadID );
    }
}

sal_Bool JavaVirtualMachine_Impl::getSettings( JSettings& rSettings, const Reference<XRegistryKey> xRegistryRootKey)
{
    char * pJavaDebug = getenv( "STAR_JAVADEBUG" );
    sal_Bool bSettingsOk = sal_False;


    if (pJavaDebug)
    {
        rSettings.setDebug( sal_True );
        rSettings.setDebugPort( atoi(pJavaDebug) );
    }

    if ( rSettings.getDebug() && pJavaDebug )
    {
        char * pClassPath = getenv ("CLASSPATH");

        if (pClassPath)
            rSettings.classPath = OUString::createFromAscii( pClassPath );
    }


    OUString aDebug = getValue( xRegistryRootKey, OUString::createFromAscii("Java/Debug") );

    if ( aDebug.len() && aDebug.compareToAscii("1") == 0 )
        rSettings.setDebug(sal_True);

    OUString aJavaVmHome = getValue( xRegistryRootKey, OUString::createFromAscii("Java/Home") );

//      fprintf(stderr, "#### GetSettings: home - %s\n", OUStringToOString(aJavaVmHome, CHARSET_SYSTEM).GetStr());
    OUString aJavaVmVersion = getValue( xRegistryRootKey, OUString::createFromAscii("Java/Version") );
    OUString aJavaVmName    = getValue( xRegistryRootKey, OUString::createFromAscii("Java/RuntimeLib") );

//      fprintf(stderr, "JavaVirtualMachine_Impl::getSettings: %s\n", aJavaVmName.getStr());

    rSettings.setRuntimeLib( aJavaVmName );

    if ( aJavaVmName.len() )
        bSettingsOk = GetVmNameSettings( rSettings, aJavaVmName, aJavaVmVersion, rSettings.getDebug() );
    if ( !bSettingsOk && aJavaVmHome.len() )
        bSettingsOk = GetVmHomeSettings( rSettings, aJavaVmHome, aJavaVmVersion, rSettings.getDebug() );
    if ( !bSettingsOk )
        bSettingsOk = GetDefaultSettings( rSettings, aJavaVmVersion, rSettings.getDebug() );

    OUString aEntryValue;

    aEntryValue = getValue( xRegistryRootKey, OUString::createFromAscii("Java/RuntimeLib") );

    if ( aEntryValue.len() )
        rSettings.setRuntimeLib( aEntryValue );

    aEntryValue = getValue( xRegistryRootKey, OUString::createFromAscii("Java/SystemClasspath") );

    if ( aEntryValue.len() )
        rSettings.classPath = aEntryValue;

    aEntryValue = getValue( xRegistryRootKey, OUString::createFromAscii("Java/UserClasspath") );

    if ( aEntryValue.len() )
    {
#ifdef UNX
    // #68287#  Semikolona to colon
        sal_Int32 tokenCount = aEntryValue.getTokenCount();
        OUString path;

        for ( sal_Int32 i = 0; i < tokenCount; ++i )
        {
            if ( path.len() )
                path += OUString::createFromAscii(":");

            path += aEntryValue.getToken(i);
        }

        aEntryValue = path;
#endif
        rSettings.classPath = rSettings.classPath + OUString(ENV_DEL) + aEntryValue;
    }

    return bSettingsOk;
}

#ifdef SOLARIS
#include <sys/utsname.h>
/* getSolarisRelease
 * make a single integer from solaris release string
 */

int getSolarisRelease()
{
    struct utsname aRelease;
    static int nRelease = 0;

    if ( nRelease != 0 )
    {
        /* computed once, returned many */
        return nRelease;
    }
    else
    if ( uname( &aRelease ) > -1 )
    {
        int nTokens;
        int nMajor, nMinor, nMMinor;

        /* release will be something like 5.5.1 or 5.6 */
        nTokens = sscanf(aRelease.release, "%i.%i.%i\n",
                    &nMajor, &nMinor, &nMMinor );

        switch ( nTokens )
        {
            case 0: nMajor  = 0;
                    /* fall thru */
            case 1: nMinor  = 0;
                    /* fall thru */
            case 2: nMMinor = 0;
                    break;
            case 3:
            default:
                    /* 3 tokens cannot match more than 3 times */
                    break;
        }

        /* will be something like 551 or 560, dont expect a minor release
         * number larger than 9 */
        nRelease = nMajor * 100 + nMinor * 10 + nMMinor;

        /* okay, the paranoic case */
        if ( nRelease == 0 )
            nRelease = -1;
    }
    else
    {
        /* never saw uname fail, but just in case
         * (must be very old solaris) */
        nRelease = -1;
    }

    return nRelease;
}
#endif

#ifdef UNX
static OUString preOpen( sal_Bool bDebug,
                         const OUString & usLibraryPath,
                         const OUString & usRuntimeLib,
                         const OUString & rCompiler)
{
    //**************************************************************************************/
    // now try to load the java lib ********************************************************/

    OUString usCompiler = rCompiler;
    char *   error = NULL;

    dlerror(); // CLEAR

#if defined(LINUX)

    OString sRuntime = OUStringToOString( usLibraryPath + usRuntimeLib, RTL_TEXTENCODING_ASCII_US );
    dlopen( sRuntime.getStr(),  RTLD_LAZY | RTLD_GLOBAL );
    error = dlerror();

    if (error) throw VMException( -1, error );

      if (bDebug)
    {
        OString sAgent = OUStringToOString(usLibraryPath, RTL_TEXTENCODING_ASCII_US) + "libagent_g.so";

        void *handle = dlopen( sAgent.getStr(), RTLD_GLOBAL | RTLD_LAZY);
        error = dlerror();
        if (error) throw VMException( -2, error );
    }
#endif // LINUX

//      String sLibZip = OUStringToOString( usLibraryPath + OUString::createFromAscii("libzip.so") );
//      void  * libzipso = dlopen(sLibZip.GetStr(), RTLD_LAZY | RTLD_GLOBAL);
//      error = dlerror();
//      if (error)
//          fprintf (stderr, "preOpen - %s: %s\n", sLibZip.GetStr(), error);


#if defined(SOLARIS) && defined(UNRESOLVED_AWT_SYMBOLS)
    OString sRuntime = OUStringToOString(usLibraryPath + usRuntimeLib, RTL_TEXTENCODING_ASCII_US);

    if ( !dlopen(sRuntime.getStr(), RTLD_LAZY | RTLD_GLOBAL) )
    {
        error = dlerror();
        throw VMException( -3, error );
    }

    void* libJITHandel;

    if ( ! usCompiler.equalsIgnoreCase( OUString::createFromAscii("NONE") ) )
    {
#if defined(INTEL)
        OUString preopen;
        if ( usCompiler.len() )
            usCompiler = OUString::createFromAscii("sunwjit");

        preopen = OUString::createFromAscii("lib") + usCompiler + OUString::createFromAscii(".so");
        OString sAgent = OUStringToOString( usLibraryPath + preopen, RTL_TEXTENCODING_ASCII_US );
        libJITHandel = dlopen(sAgent.getStr(), RTLD_GLOBAL | RTLD_LAZY);

#elif defined(SPARC)
           int  release = getSolarisRelease();
        OUString preopen;

           if ( release < 560 )
        {   // seems to be solaris 2.5.x or less
            //fprintf(stderr, "seems to be solaris 2.5.x\n");

            if ( usCompiler.len() )
            {   // enable JIT only if recommendet by user (entry in sofficerc)
                preopen =  OUString::createFromAscii("lib") + usCompiler + OUString::createFromAscii(".so");
                OString sAgent = OUStringToOString(preopen, RTL_TEXTENCODING_ASCII_US);
                //fprintf(stderr, "TRY to open %s\n", sAgent.GetStr());
                libJITHandel = dlopen(sAgent.getStr(), RTLD_GLOBAL | RTLD_LAZY);
            }
            else
            {
                //fprintf(stderr, "no compiler set: set to NONE");
                usCompiler = OUString::createFromAscii("NONE");
            }
        }
        else
        {
            //fprintf(stderr, "seems to be solaris 2.6.x\n");

            if ( !usCompiler.len() )
            {
                usCompiler = OUString::createFromAscii("sunwjit");
                preopen    = OUString::createFromAscii("libjit.so");
            }
            else
                preopen  = OUString::createFromAscii("lib") + usCompiler + OUString::createFromAscii(".so");

            OString sJIT = OUStringToOString( preopen, RTL_TEXTENCODING_ASCII_US );

            libJITHandel = dlopen(sJIT.getStr(), RTLD_GROUP | RTLD_LAZY);
        }

#endif // SPARC
        if ( !usCompiler.equalsIgnoreCase( OUString::createFromAscii("NONE") ) )
        {
//              error = dlerror();

//              if (error)
//                  fprintf (stderr, "preopen JIT DLL: %s\n", error);

//              if ( libJITHandel == NULL )
//                  usCompiler = OUString::createFromAscii("NONE";
        }
    }
#endif
    return usCompiler;
}
#endif // UNX


inline sal_Bool IsEqualToAscii( const OUString & rString, const sal_Char* anotherString )
{
    return rString.compareToAscii( anotherString ) == 0;
}

void JavaVirtualMachine_Impl::readJavaSection( XSimpleRegistry* pJavaReg, JVM* pjvm )
{
    sal_Bool bSetDefaultToolkit = sal_True;
    sal_Bool bSetDefaultTKTLib  = sal_True;

    //if ( pJavaReg->isValid() )
    //{
    //  try
    //  {
            Reference<XRegistryKey> xJavaSection = pJavaReg->getRootKey()->openKey( OUString::createFromAscii("Java") );

            if ( xJavaSection.is() &&  xJavaSection->isValid() )
            {
                Sequence< OUString > aJavaProperties = xJavaSection->getKeyNames();
                OUString*           pSectionEntry   = aJavaProperties.getArray();
                sal_Int32           nCount          = aJavaProperties.getLength();

                for ( sal_Int32 i=0; i<nCount; i++ )
                {
                    OUString aEntryValue = ( xJavaSection->openKey( pSectionEntry[i] ))->getStringValue();

                    if ( !pSectionEntry[i].len() )
                                ;
                    else if ( pSectionEntry[i] == OUString::createFromAscii("Debug") )
                        pjvm->setDebug( IsEqualToAscii( aEntryValue, "1" ) );
                    else if ( IsEqualToAscii(pSectionEntry[i], "Java") )
                        ;
                    else if ( IsEqualToAscii(pSectionEntry[i], "Version") )
                        ;
                    else if ( IsEqualToAscii(pSectionEntry[i], "Home") )
                        ;
                    else if ( IsEqualToAscii(pSectionEntry[i], "SystemClasspath") )
                        ;
                    else if ( IsEqualToAscii(pSectionEntry[i], "awt.toolkit") )
                    {
                        bSetDefaultToolkit = sal_False;

                        pjvm->pushProp( pSectionEntry[i] + OUString::createFromAscii("=") + aEntryValue );
                    }
                    else if ( IsEqualToAscii(pSectionEntry[i], "java.compiler") )
                        pjvm->setCompiler( aEntryValue );

                    else if ( IsEqualToAscii(pSectionEntry[i], "DisableAsyncGC") )
                        pjvm->disableAsyncGC(IsEqualToAscii( aEntryValue, "1") );

                    else if ( IsEqualToAscii(pSectionEntry[i], "EnableClassGC") )
                        pjvm->enableClassGC(IsEqualToAscii(aEntryValue, "0") );

                    else if ( IsEqualToAscii(pSectionEntry[i], "EnableVerboseGC") )
                        pjvm->enableVerboseGC(IsEqualToAscii(aEntryValue, "1") );

                    else if ( IsEqualToAscii(pSectionEntry[i], "Verbose") )
                        pjvm->verbose(IsEqualToAscii(aEntryValue, "1") );

                    else if ( pSectionEntry[i] == OUString::createFromAscii("NativeStackSize") )
                        pjvm->nativeStackSize( aEntryValue.toInt32() );

                    else if ( IsEqualToAscii(pSectionEntry[i], "JavaStackSize") )
                        pjvm->javaStackSize( aEntryValue.toInt32() );

                    else if ( IsEqualToAscii(pSectionEntry[i], "VerifyMode") )
                        pjvm->verifyMode(  aEntryValue );

                    else if ( IsEqualToAscii(pSectionEntry[i], "MinHeapSize") )
                        pjvm->minHeapSize( aEntryValue.toInt32() );

                    else if ( IsEqualToAscii(pSectionEntry[i], "MaxHeapSize") )
                        pjvm->maxHeapSize( aEntryValue.toInt32() );

                    else if ( IsEqualToAscii(pSectionEntry[i], "DebugPort") )
                        pjvm->setDebugPort( aEntryValue.toInt32() );
                    else
                    {
                        if ( aEntryValue.len() )
                            pSectionEntry[i] = pSectionEntry[i] + OUString::createFromAscii("=") + aEntryValue;

                        pjvm->pushProp( pSectionEntry[i] );
                    }
                }
            }
            else
            {
                InvalidRegistryException e;
                e.Message = OUString::createFromAscii("no Java Section found");
                throw e;
            }
}

void JavaVirtualMachine_Impl::setLanguageAndCountrySection( XSimpleRegistry* pOfficeReg, JVM* pjvm )
{
// TEMPORARY DISABLED
//      OUString aValue;

//      Reference<XRegistryKey> xRegistryRootKey = pOfficeReg->getRootKey();
//      aValue = getValue( xRegistryRootKey, OUString::createFromAscii("User/Language") );

//      if ( aValue.len() )
//      {
//          OUString  aLanguage;
//          OUString  aCountry;

//          // LanguageType aLangNum = (LanguageType) String( OUStringToOString( aValue, RTL_TEXTENCODING_ASCII_US) );
//          LanguageType aLangNum = (LanguageType) aValue.toInt32();

//          {
//              String  langStr;
//              String  countryStr;

//              ConvertLanguageToIsoNames( aLangNum, langStr, countryStr );

//              aLanguage = OUString( langStr );
//              aCountry  = OUString( countryStr );
//          }

//          if ( aLanguage.getLength() ) pjvm->pushProp( OUString::createFromAscii("user.language=") + aLanguage );

//          if ( aCountry.getLength() ) pjvm->pushProp( OUString::createFromAscii("user.region=") + aCountry );
//      }
}

void JavaVirtualMachine_Impl::readINetSection( XSimpleRegistry* pOfficeReg, JVM* pjvm ) throw (VMException)
{
    Reference<XRegistryKey> xRegistryRootKey = pOfficeReg->getRootKey();

    // HTTPProxyHost, HTTPProxyPort, FTPProxyHost, FTPProxyPort
    // Reading proxy and port values in the INet section

    OUString aValue;

    // HTTPProxy
    aValue = getValue( xRegistryRootKey, OUString::createFromAscii("INet/HTTPProxy") );
    if ( aValue.len() ) pjvm->pushProp( OUString::createFromAscii("http.proxyHost=") + aValue );

    // HTTPProxyPort
    aValue = getValue( xRegistryRootKey, OUString::createFromAscii("INet/HTTPProxyPort") );
    if ( aValue.len() ) pjvm->pushProp( OUString::createFromAscii("http.proxyPort=") + aValue );

    // FTPPProxy
    aValue = getValue( xRegistryRootKey, OUString::createFromAscii("INet/FTPProxy" ) );
    if ( aValue.len() ) pjvm->pushProp( OUString::createFromAscii("ftpProxyHost=") + aValue );

    // FTPProxyPort
    aValue = getValue( xRegistryRootKey, OUString::createFromAscii("INet/FTPProxyPort" ) );
    if ( aValue.len() ) pjvm->pushProp( OUString::createFromAscii("FTPProxyPort=") + aValue );
}

static void initSJSettings( JNIEnv * pJNIEnv ) throw (VMException)
{
    // init SjSettings -> SecurityManager
        jclass jcSjSettings = pJNIEnv->FindClass("stardiv/controller/SjSettings");
        if (!jcSjSettings) throw VMException( -9, "Can't find class \"stardiv/controller/SjSettings\"." );

        jmethodID jmChangeProperties = pJNIEnv->GetStaticMethodID( jcSjSettings , "changeProperties", "(Ljava/util/Properties;)V" );
        if ( !jmChangeProperties ) throw VMException( -9, "Can't find method \"changeProperties\" of class \"stardiv/controller/SjSettingss\"." );

        pJNIEnv->CallStaticVoidMethod( jcSjSettings, jmChangeProperties, NULL );

        if ( pJNIEnv->ExceptionOccurred() )
        {
            pJNIEnv->ExceptionClear();
            throw VMException( -9, "An exception occured when calling method \"changeProperties\" of class \"stardiv/controller/SjSettingss\".");
        }
}


static void setTimeZone( JVM * pjvm )
{
    /* A Bug in the Java function
    ** struct Hjava_util_Properties * java_lang_System_initProperties(
    ** struct Hjava_lang_System *this,
    ** struct Hjava_util_Properties *props);
    ** This function doesn't detect MEZ, MET or "W. Europe Standard Time"
    */
    struct tm *tmData;
    time_t clock = time(NULL);
    tzset();
    tmData = localtime(&clock);
    char * p = tzname[0];
#ifdef WNT
    if ( OString( "MET" ) == p )
#else
    if ( OString( "MEZ" ) == p )
#endif
    {
        pjvm->pushProp( OUString::createFromAscii("user.timezone=ECT") );
    }
}

static void setVMLibraryPath(const OString & usLibraryPath, JNIEnv * pJNIEnv)
{
    // setzte den privaten LD_LIBRARY_PATH an java.lang.Runtime
    jclass jcRuntime = pJNIEnv->FindClass("java/lang/Runtime");                          if (pJNIEnv->ExceptionOccurred()) return;
    jmethodID jmGetRuntime = pJNIEnv->GetStaticMethodID(jcRuntime,
                                                         "getRuntime",
                                                         "()Ljava/lang/Runtime;");       if (pJNIEnv->ExceptionOccurred()) return;
    jobject joRuntime = pJNIEnv->CallStaticObjectMethod(jcRuntime, jmGetRuntime);        if (pJNIEnv->ExceptionOccurred()) return;

    jfieldID jfPaths = pJNIEnv->GetFieldID(jcRuntime, "paths", "[Ljava/lang/String;");   if (pJNIEnv->ExceptionOccurred()) return;

//  jstring jsJavaLibPath = pJNIEnv->NewStringUTF(usLibraryPath.GetStr());               if (pJNIEnv->ExceptionOccurred()) return;
    jstring jsJavaLibPath = pJNIEnv->NewStringUTF(usLibraryPath);               if (pJNIEnv->ExceptionOccurred()) return;
    jstring jsOfficeLibPath = pJNIEnv->NewStringUTF("");                                 if (pJNIEnv->ExceptionOccurred()) return;


    jclass jcString = pJNIEnv->FindClass("java/lang/String");                            if (pJNIEnv->ExceptionOccurred()) return;
    jarray jaPaths = pJNIEnv->NewObjectArray(2, jcString, NULL);                         if (pJNIEnv->ExceptionOccurred()) return;
    pJNIEnv->SetObjectArrayElement((jobjectArray)jaPaths, 0, jsJavaLibPath);             if (pJNIEnv->ExceptionOccurred()) return;
    pJNIEnv->SetObjectArrayElement((jobjectArray)jaPaths, 1, jsOfficeLibPath);           if (pJNIEnv->ExceptionOccurred()) return;

    pJNIEnv->SetObjectField(joRuntime, jfPaths, jaPaths);                                if (pJNIEnv->ExceptionOccurred()) return;
}

#ifdef UNX
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

/*
static OUString pathExpansion()
{
    OUString aClassPath;

    // Application home
    char buf[_MAX_PATH];
    if ( OStartupInfo::E_None == OStartupInfo().getExecutableFile(buf, _MAX_PATH ) )
    {
        OString aPath = buf;
        sal_Int16 nPos = aPath.getLength();
        if ( nPos )
        {
            while ( nPos-- && aPath[nPos] != PATH_DEL[0] );


// #ifdef UNX
//          // cut /bin too
//          while( nPos-- && aPath[nPos] != PATH_DEL[0] )
//              ;
// #endif

//          aPath.Erase( nPos );
            aPath= aPath.copy( 0, nPos+1); // last character is the path delimiter
        }

        OUString aAppHome = OUString::createFromAscii( aPath );
        // classpath is set by the java INI
        // aClassPath += ENV_DEL + aAppHome + OUString::createFromAscii("classes") + PATH_DEL + OUString::createFromAscii("classes.jar");
        // aClassPath += ENV_DEL + aAppHome + OUString::createFromAscii("classes") + PATH_DEL + OUString::createFromAscii("sandbox.jar");
    }

    return aClassPath;
}
*/

JAVAVM * JavaVirtualMachine_Impl::createJavaVM( XSimpleRegistry* pJavaReg, XSimpleRegistry* pOfficeReg)
{
    try
    {
        JSettings aSettings;

        try
        {
            Reference<XRegistryKey> xJavaIni( pJavaReg->getRootKey(), UNO_QUERY );

            // test if Java is enabled
            OUString aEntryValue = getValue( xJavaIni, OUString::createFromAscii("Java/Java") );

            if ( aEntryValue.len() == 0 || aEntryValue == OUString::createFromAscii("0") ) throw VMException( -4, "Can't find entry \"Java\" in section \"Java\".");
            if ( ! getSettings( aSettings, xJavaIni ) ) throw VMException( -5, "An exception occured while reading JVM settings.");

#ifdef UNX
            OUString usCompiler = getValue( xJavaIni, OUString::createFromAscii("Java/java.compiler") );

            if ( usCompiler.len() != 0 )
                aSettings.setCompiler( usCompiler );

            aSettings.setCompiler(preOpen(  aSettings.getDebug(),
                                            aSettings.getLibraryPath(),
                                            aSettings.getRuntimeLib(),
                                            aSettings.getCompiler()
                                          )
            );
#endif

            sal_Bool bLoaded = aJavaLib.load( aSettings.getLibraryPath() + aSettings.getRuntimeLib() );

            if ( !bLoaded )
            {
                throw VMException( -6, OUString::createFromAscii("can't load ") +
                                       aSettings.getLibraryPath() +
                                       aSettings.getRuntimeLib() );
            }

            JNI_InitArgs_Type * initArgs = (JNI_InitArgs_Type *) aJavaLib.getSymbol(OUString::createFromAscii("JNI_GetDefaultJavaVMInitArgs"));

            if ( !initArgs ) throw VMException( -7, "can't load symbol \"JNI_GetDefaultJavaVMInitArgs\"." );

            JVM jvm(initArgs);
            readJavaSection( pJavaReg, &jvm );

            if ( aSettings.getCompiler().len() )
            {
                jvm.setCompiler( aSettings.getCompiler() );
            }

            // read out the INet section in the soffice.ini / sofficerc
            readINetSection( pOfficeReg, &jvm );
            setLanguageAndCountrySection( pOfficeReg, &jvm );

            // security settings
            jvm.pushProp( OUString::createFromAscii("stardiv.security.defaultSecurityManager=true") );
            jvm.pushProp( OUString::createFromAscii("stardiv.security.noExit=true") );
            jvm.pushProp( OUString::createFromAscii("stardiv.controller.installConsole=true") );

            OUString aClassPath( aSettings.classPath );
            //aClassPath = aClassPath + pathExpansion();

            jvm.classPath( OUStringToOString(aClassPath, RTL_TEXTENCODING_ASCII_US) );

            jvm.abort(vm_abort);
            jvm.exit(vm_exit);
            jvm.vfprintf(vm_vfprintf);

            setTimeZone( &jvm );

            JNI_CreateVM_Type * pCreateJavaVM = (JNI_CreateVM_Type *) aJavaLib.getSymbol( OUString::createFromAscii("JNI_CreateJavaVM") );
            if ( !pCreateJavaVM ) throw VMException( -8, "Can't load symbol \"JNI_CreateJavaVM\"." );

            JNIEnv * pJNIEnv = NULL;
            jint err = pCreateJavaVM( &pJavaVM, &pJNIEnv, (void *)jvm.getJDK1_1InitArgs() );

            if ( err ) throw VMException( -9, "Can't create Java VM" );

            initSJSettings( pJNIEnv );

        }
        catch ( InvalidRegistryException e )
        {
        }
    }
    catch ( VMException e )
    {
        setError( e.getErrNum() );
          fprintf(stderr, "error - could not load java, cause %s.", e.what());
    }

    return pJavaVM;
}

void    JavaVirtualMachine_Impl::disposeJavaVM()
{
    if ( pJavaVM )
    {
//          pJavaVM->DestroyJavaVM();
        pJavaVM = NULL;
    }
}

OUString JavaVirtualMachine_Impl::getJavaIniFileName( const Reference<XSimpleRegistry>& xSofficeReg,
                                                      const Reference<XConfigManager>& xConfigManager )
{
    OUString iniFileName;
    sal_Bool    bIniFound = sal_False;

    if ( xSofficeReg.is() && xSofficeReg->isValid() )
    {
        Reference<XRegistryKey> xRegistryRootKey = xSofficeReg->getRootKey();

        if ( xRegistryRootKey.is() && xRegistryRootKey->isValid() )
        {
            try
            {
                ::osl::DirectoryItem aJavaIni;

                try
                {
                    OUString    tmpFileName;
                    OUString    fileName;
                    tmpFileName = getValue( xRegistryRootKey, OUString::createFromAscii("Directories/UserConfig-Path") );
                    tmpFileName = xConfigManager->substituteVariables( tmpFileName );

                    File::normalizePath( tmpFileName, fileName );
                    fileName += OUString::createFromAscii(SLASH_INI_FILE);
                    bIniFound = DirectoryItem::get( fileName, aJavaIni ) == FileBase::E_None;

                }
                catch ( Exception e )
                {
                    bIniFound = sal_False;
                }

                if ( !bIniFound )
                {
                    OUString    tmpFileName;
                    OUString    fileName;
                    tmpFileName = getValue( xRegistryRootKey, OUString::createFromAscii("Directories/Config-Dir") );
                    tmpFileName = xConfigManager->substituteVariables( tmpFileName );

                    File::normalizePath( tmpFileName, fileName );
                    fileName += OUString::createFromAscii(SLASH_INI_FILE);
                    bIniFound = DirectoryItem::get( fileName, aJavaIni ) == FileBase::E_None;
                }

                if ( bIniFound )
                {
                    FileStatus fileStatus( FileStatusMask_FilePath /*FileStatusMask_NativePath*/ );

                    if ( FileBase::E_None == aJavaIni.getFileStatus( fileStatus ) )
                        iniFileName = fileStatus.getFilePath();
                        //iniFileName = fileStatus.getNativePath();
                    else
                        bIniFound = sal_False;
                }

            }
            catch (Exception e)
            {
                bIniFound = sal_False;
            }
        }
    }

    if ( ! bIniFound )
    {
        iniFileName = OUString::createFromAscii("");
    }

    return iniFileName;
}

// ***************************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//==================================================================================================
void SAL_CALL component_getImplementationEnvironment(
    const sal_Char ** ppEnvTypeName, uno_Environment ** ppEnv )
{
    *ppEnvTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME;
}
//==================================================================================================
sal_Bool SAL_CALL component_writeInfo(
    void * pServiceManager, void * pRegistryKey )
{
    if (pRegistryKey)
    {
        try
        {
            Reference< XRegistryKey > xNewKey(
                reinterpret_cast< XRegistryKey * >( pRegistryKey )->createKey(
                    OUString::createFromAscii("/") +
                    JavaVirtualMachine_Impl::getImplementationName_Static() +
                    OUString::createFromAscii("/UNO/SERVICES" )) );

            const Sequence< OUString > & rSNL =
                JavaVirtualMachine_Impl::getSupportedServiceNames_Static();
            const OUString * pArray = rSNL.getConstArray();
            for ( sal_Int32 nPos = rSNL.getLength(); nPos--; )
                xNewKey->createKey( pArray[nPos] );

            return sal_True;
        }
        catch (InvalidRegistryException &)
        {
            OSL_ENSHURE( sal_False, "### InvalidRegistryException!" );
        }
    }
    return sal_False;
}
//==================================================================================================
void * SAL_CALL component_getFactory(
    const sal_Char * pImplName, void * pServiceManager, void * pRegistryKey )
{
    void * pRet = 0;

    if ( JavaVirtualMachine_Impl::getImplementationName_Static().equals(OUString::createFromAscii(pImplName)) )
    {
        Reference< XSingleServiceFactory > xFactory( createOneInstanceFactory(
            reinterpret_cast< XMultiServiceFactory * >( pServiceManager ),
            JavaVirtualMachine_Impl::getImplementationName_Static(),
            JavaVirtualMachine_Impl_createInstance,
            JavaVirtualMachine_Impl::getSupportedServiceNames_Static() ) );

        if (xFactory.is())
        {
            xFactory->acquire();
            pRet = xFactory.get();
        }
    }

    return pRet;
}

#ifdef __cplusplus
} //  END of extern "C"
#endif

