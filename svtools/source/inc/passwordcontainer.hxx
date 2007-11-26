/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: passwordcontainer.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-26 16:44:41 $
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
#ifndef INCLUDED_COMPHELPER_PASSWORDCONTAINER_HXX
#define INCLUDED_COMPHELPER_PASSWORDCONTAINER_HXX

#include <list>
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

#ifndef _COM_SUN_STAR_TASK_XMASTERPASSWORDHANDLING_HPP_
#include <com/sun/star/task/XMasterPasswordHandling.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
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

typedef ::std::pair< const ::rtl::OUString, ::std::list< NamePassRecord > > PairUrlRecord;
typedef ::std::map< ::rtl::OUString, ::std::list< NamePassRecord > > PassMap;

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
    void setUseStorage( sal_Bool bUse );
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

class PasswordContainer : public ::cppu::WeakImplHelper4<
        ::com::sun::star::task::XPasswordContainer,
        ::com::sun::star::task::XMasterPasswordHandling,
        ::com::sun::star::lang::XServiceInfo,
        ::com::sun::star::lang::XEventListener >
{
private:
    PassMap  container;
    StorageItem* m_pStorageFile;
    ::osl::Mutex mMutex;
    ::rtl::OUString m_aMasterPasswd; // master password is set when the string is not empty
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > mComponent;

    ::com::sun::star::uno::Sequence< ::com::sun::star::task::UserRecord > CopyToUserRecordSequence(
                                        const ::std::list< NamePassRecord >& original,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence< ::com::sun::star::task::UserRecord > FindUsr(
                                        const ::std::list< NamePassRecord >& userlist,
                                        const ::rtl::OUString& name,
                                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    ::rtl::OUString RequestPasswordFromUser(
                    ::com::sun::star::task::PasswordRequestMode aRMode,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler );

    ::rtl::OUString GetMasterPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    void UpdateVector( const ::rtl::OUString& url, ::std::list< NamePassRecord >& toUpdate, NamePassRecord& rec, sal_Bool writeFile,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    void PrivateAdd( const ::rtl::OUString& aUrl,
                              const ::rtl::OUString& aUserName,
                              const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPasswords,
                              char  aMode,
                              const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handler )
                                                        throw(::com::sun::star::uno::RuntimeException);

    ::std::vector< ::rtl::OUString > DecodePasswords( const ::rtl::OUString& aLine, const ::rtl::OUString& aMasterPassword )
                                                        throw(::com::sun::star::uno::RuntimeException);

    ::rtl::OUString EncodePasswords( ::std::vector< ::rtl::OUString > lines, const ::rtl::OUString& aMasterPassword )
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
                    impl_createInstance( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) throw( ::com::sun::star::uno::RuntimeException );

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL    getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL           supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL
                                        getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL       disposing( const ::com::sun::star::lang::EventObject& Source )
                                    throw(::com::sun::star::uno::RuntimeException);

    // XMasterPasswordHandling
    virtual ::sal_Bool SAL_CALL authorizateWithMasterPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler )
        throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL changeMasterPassword( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& xHandler ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMasterPassword() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasMasterPassword(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL allowPersistentStoring( ::sal_Bool bAllow ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isPersistentStoringAllowed(  ) throw (::com::sun::star::uno::RuntimeException);

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
    RW_SvMemoryStream( void* Buf, ULONG Size, StreamMode eMode ):
            SvMemoryStream( Buf, Size, eMode){}

    RW_SvMemoryStream( ULONG InitSize=512, ULONG Resize=64 ):
            SvMemoryStream( InitSize, Resize ){}

    ULONG getActualSize(){ return nEndOfData; }
};



#endif // #ifndef INCLUDED_COMPHELPER_PASSWORDCONTAINER_HXX

