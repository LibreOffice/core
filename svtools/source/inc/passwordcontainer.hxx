/*************************************************************************
 *
 *  $RCSfile: passwordcontainer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-16 10:19:28 $
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
#ifndef INCLUDED_COMPHELPER_PASSWORDCONTAINER_HXX
#define INCLUDED_COMPHELPER_PASSWORDCONTAINER_HXX

#include <vector>
#include <map>

#ifndef _COM_SUN_STAR_TASK_XPASSWORDCONTAINER_HPP_
#include <com/sun/star/task/XPasswordContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_PASSWORDREQUESTMODE_HPP_
#include <com/sun/star/task/PasswordRequestMode.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif

#ifndef _CPPUHELPER_QUERYINTERFACE_HXX_
#include <cppuhelper/queryinterface.hxx>
#endif

#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif

#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>
#endif

#ifndef _UCBHELPER_INTERATIONREQUEST_HXX
#include <ucbhelper/interactionrequest.hxx>
#endif

#include <rtl/ref.hxx>
#include <osl/mutex.hxx>

#define EMPTY_RECORD        0
#define SINGLE_RECORD       1
#define PERSISTENT_RECORD   2

//----------------------------------------------------------------------------------

struct NamePassRecord
{
    ::rtl::OUString                                     mName;
    ::std::vector< ::rtl::OUString >                    mPass;
    char                                                mStatus;

    NamePassRecord( const ::rtl::OUString& name, const ::std::vector< ::rtl::OUString >& passList, const char stat ):
        mName( name ),
        mPass( passList ),
        mStatus( stat )
    {}

    NamePassRecord( const NamePassRecord& rec ) :
        mName( rec.mName ),
        mPass( rec.mPass ),
        mStatus( rec.mStatus )
    {}

    NamePassRecord() : mStatus( EMPTY_RECORD ) {}
};

//----------------------------------------------------------------------------------

typedef ::std::pair< const ::rtl::OUString, ::std::vector< NamePassRecord > > PairUrlRecord;
typedef ::std::map< ::rtl::OUString, ::std::vector< NamePassRecord > > PassMap;

//----------------------------------------------------------------------------------

class PasswordContainer;

class StorageItem : public ::utl::ConfigItem {
    PasswordContainer*  mainCont;
    sal_Bool            hasEncoded;
    ::rtl::OUString     mEncoded;
public:
    StorageItem( PasswordContainer* point, const ::rtl::OUString& path ) :
        ConfigItem( path, CONFIG_MODE_IMMEDIATE_UPDATE ),
        mainCont( point ),
        hasEncoded( sal_False )
    {
        ::com::sun::star::uno::Sequence< ::rtl::OUString > aNode( 1 );
        *aNode.getArray()  = path;
        *aNode.getArray() += ::rtl::OUString::createFromAscii( "/Store" );
        EnableNotification( aNode );
    }

    PassMap getInfo();
    void update( const ::rtl::OUString& url, const NamePassRecord& rec );
    void remove( const ::rtl::OUString& url, const ::rtl::OUString& rec );
    void clear();

    sal_Bool getEncodedMP( ::rtl::OUString& aResult );
    void setEncodedMP( const ::rtl::OUString& aResult );
    sal_Bool useStorage();

    virtual void            Notify( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames );
    virtual void            Commit();
};

//----------------------------------------------------------------------------------

enum PasswordState {
    no_password,
    entered,
    cancelled
};

class PasswordContainer : public ::cppu::WeakImplHelper3< ::com::sun::star::task::XPasswordContainer, ::com::sun::star::lang::XServiceInfo,
 ::com::sun::star::lang::XEventListener >
{
private:
    PassMap  container;
    StorageItem* storageFile;
    ::osl::Mutex mMutex;
    sal_Bool hasMasterPasswd;
    ::rtl::OUString masterPasswd;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mComponent;

    PassMap getInfoFromStorageFile();
    void removeFromStorageFile( const ::rtl::OUString& url, const ::rtl::OUString& rec );
    void clearStorageFile();

    ::com::sun::star::uno::Sequence< ::com::sun::star::task::UserRecord > copyToUserRecordSequence(
                                        const ::std::vector< NamePassRecord >& original,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence< ::com::sun::star::task::UserRecord > PasswordContainer::findUsr(
                                        const ::std::vector< NamePassRecord >& userlist,
                                        const ::rtl::OUString& name,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    void getMasterPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    void updateVector( const ::rtl::OUString& url, ::std::vector< NamePassRecord >& toUpdate, NamePassRecord& rec, sal_Bool writeFile,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    void privateAdd( const ::rtl::OUString& aUrl,
                              const ::rtl::OUString& aUserName,
                              const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPasswords,
                              char  aMode,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    ::std::vector< ::rtl::OUString > PasswordContainer::decodePasswords( ::rtl::OUString aLine,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    ::rtl::OUString PasswordContainer::encodePasswords( ::std::vector< ::rtl::OUString > lines,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

public:
    PasswordContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
    ~PasswordContainer();

    virtual void SAL_CALL add( const ::rtl::OUString& aUrl,
                               const ::rtl::OUString& aUserName,
                               const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPasswords,
                               const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler  )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addPersistent( const ::rtl::OUString& aUrl,
                                            const ::rtl::OUString& aUserName,
                                         const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPasswords,
                                          const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler  )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::task::UrlRecord SAL_CALL
                            find( const ::rtl::OUString& aUrl,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler  )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::task::UrlRecord SAL_CALL
                            findForName( const ::rtl::OUString& aUrl,
                                         const ::rtl::OUString& aUserName,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler  )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL remove( const ::rtl::OUString& aUrl,
                                  const ::rtl::OUString& aUserName )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removePersistent( const ::rtl::OUString& aUrl,
                                            const ::rtl::OUString& aUserName )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeAllPersistent() throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::task::UrlRecord > SAL_CALL
                            getAllPersistent( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler ) throw(::com::sun::star::uno::RuntimeException);


    // provide factory
    static ::rtl::OUString SAL_CALL     impl_getStaticImplementationName( ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                    impl_getStaticSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > SAL_CALL
                    impl_createFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& ServiceManager ) throw(::com::sun::star::uno::RuntimeException);
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                    PasswordContainer::impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL    getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL           supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                                        getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL       disposing( const ::com::sun::star::lang::EventObject& Source )
                                    throw(::com::sun::star::uno::RuntimeException);


    void            Notify();
};

//----------------------------------------------------------------------------------

class MasterPasswordRequest_Impl : public ucbhelper::InteractionRequest
{
    ::rtl::Reference< ucbhelper::InteractionSupplyAuthentication > m_xAuthSupplier;

public:
    MasterPasswordRequest_Impl( ::com::sun::star::task::PasswordRequestMode Mode );

    const ::rtl::Reference< ucbhelper::InteractionSupplyAuthentication > &
    getAuthenticationSupplier() const { return m_xAuthSupplier; }

};

//----------------------------------------------------------------------------------

class RW_SvMemoryStream : public SvMemoryStream {
public:
    RW_SvMemoryStream( void* pBuf, ULONG nSize, StreamMode eMode ):
            SvMemoryStream( pBuf, nSize, eMode){}

    RW_SvMemoryStream( ULONG nInitSize=512, ULONG nResize=64 ):
            SvMemoryStream( nInitSize, nResize ){}

    const void*     GetData() { Flush(); return pBuf; }
    ULONG getActualSize(){ return nEndOfData; }
};



#endif // #ifndef INCLUDED_COMPHELPER_PASSWORDCONTAINER_HXX

