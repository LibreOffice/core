/*************************************************************************
 *
 *  $RCSfile: passwordcontainer.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2001-05-14 14:35:52 $
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

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
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

#include <osl/mutex.hxx>

#define EMPTY_RECORD        0
#define SINGLE_RECORD       1
#define PERSISTENT_RECORD   2

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

typedef ::std::pair< ::rtl::OUString, ::std::vector< NamePassRecord > > PairUrlRecord;
typedef ::std::map< ::rtl::OUString, ::std::vector< NamePassRecord > > PassMap;


class PasswordContainer;

class StorageItem : public ::utl::ConfigItem {
    PasswordContainer*  mainCont;
public:
    StorageItem( PasswordContainer* point, const ::rtl::OUString& path ) : mainCont( point ), ConfigItem( path )
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

    virtual void            Notify( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames );
};


class PasswordContainer : public ::cppu::WeakImplHelper2< ::com::sun::star::task::XPasswordContainer, ::com::sun::star::lang::XServiceInfo >
{
private:
    PassMap  container;
    StorageItem* storageFile;
    ::osl::Mutex mMutex;

    PassMap getInfoFromStorageFile();
    void updateStorageFile( const ::rtl::OUString& url, const NamePassRecord& rec );
    void removeFromStorageFile( const ::rtl::OUString& url, const ::rtl::OUString& rec );
    void clearStorageFile();

    void updateVector( const ::rtl::OUString& url, ::std::vector< NamePassRecord >& toUpdate, NamePassRecord& rec, sal_Bool writeFile );
    void SAL_CALL privateAdd( const ::rtl::OUString& aUrl,
                              const ::rtl::OUString& aUserName,
                              const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPasswords,
                              char  aMode );

public:
    PasswordContainer( const ::com::sun::star::uno::Reference<::com::sun::star::lang::XMultiServiceFactory>& );
    ~PasswordContainer();

    virtual void SAL_CALL add( const ::rtl::OUString& aUrl,
                               const ::rtl::OUString& aUserName,
                               const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPasswords,
                               const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handle  )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addPersistent( const ::rtl::OUString& aUrl,
                                            const ::rtl::OUString& aUserName,
                                         const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPasswords,
                                          const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handle  )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::task::UrlRecord SAL_CALL
                            find( const ::rtl::OUString& aUrl,
                                  const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handle  )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::task::UrlRecord SAL_CALL
                            findForName( const ::rtl::OUString& aUrl,
                                         const ::rtl::OUString& aUserName,
                                            const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handle  )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL remove( const ::rtl::OUString& aUrl,
                                  const ::rtl::OUString& aUserName )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removePersistent( const ::rtl::OUString& aUrl,
                                            const ::rtl::OUString& aUserName )
                                                        throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL removeAllPersistent() throw(::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::task::UrlRecord > SAL_CALL
                            getAllPersistent( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& Handle ) throw(::com::sun::star::uno::RuntimeException);


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

    void            Notify();
};

#endif // #ifndef INCLUDED_COMPHELPER_PASSWORDCONTAINER_HXX

