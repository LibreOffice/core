/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ContentHelper.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:00:59 $
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
#ifndef DBA_CONTENTHELPER_HXX
#define DBA_CONTENTHELPER_HXX

#ifndef _COM_SUN_STAR_UCB_XCONTENT_HPP_
#include <com/sun/star/ucb/XContent.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCOMMANDPROCESSOR_HPP_
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTIESCHANGENOTIFIER_HPP_
#include <com/sun/star/beans/XPropertiesChangeNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCONTAINER_HPP_
#include <com/sun/star/beans/XPropertyContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _CPPUHELPER_COMPBASE9_HXX_
#include <cppuhelper/compbase9.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef COMPHELPER_COMPONENTCONTEXT_HXX
#include <comphelper/componentcontext.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XRENAME_HPP_
#include <com/sun/star/sdbcx/XRename.hpp>
#endif
#ifndef CONNECTIVITY_SQLERROR_HXX
#include <connectivity/sqlerror.hxx>
#endif
#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

namespace dbaccess
{
    class ODatabaseModelImpl;
    struct ContentProperties
    {
        ::rtl::OUString aTitle;         // Title
        ::rtl::OUString aContentType;   // ContentType
        sal_Bool        bIsDocument;    // IsDocument
        sal_Bool        bIsFolder;      // IsFolder
        sal_Bool        bAsTemplate;    // AsTemplate
        ::rtl::OUString sPersistentName;// persistent name of the document

        // @@@ Add other properties supported by your content.

        ContentProperties()
        : bIsDocument( sal_True ), bIsFolder( sal_False ), bAsTemplate( sal_False ) {}
    };

    class OContentHelper_Impl
    {
    public:
        OContentHelper_Impl();
        virtual ~OContentHelper_Impl();

        ContentProperties   m_aProps;
        ODatabaseModelImpl* m_pDataSource; // this will stay alive as long as the content exists
    };

    typedef ::boost::shared_ptr<OContentHelper_Impl> TContentPtr;


    typedef ::cppu::OMultiTypeInterfaceContainerHelperVar<  ::rtl::OUString,
                                                            ::comphelper::UStringHash,
                                                            ::comphelper::UStringEqual
                                                        > PropertyChangeListenerContainer;
    typedef ::comphelper::OMutexAndBroadcastHelper  OContentHelper_MBASE;
    typedef ::cppu::WeakComponentImplHelper9    <   ::com::sun::star::ucb::XContent
                                                ,   ::com::sun::star::ucb::XCommandProcessor
                                                ,   ::com::sun::star::lang::XServiceInfo
                                                ,   ::com::sun::star::beans::XPropertiesChangeNotifier
                                                ,   ::com::sun::star::beans::XPropertyContainer
                                                ,   ::com::sun::star::lang::XInitialization
                                                ,   ::com::sun::star::lang::XUnoTunnel
                                                ,   ::com::sun::star::container::XChild
                                                ,   ::com::sun::star::sdbcx::XRename
                                                >   OContentHelper_COMPBASE;

    class OContentHelper :   public OContentHelper_MBASE
                            ,public OContentHelper_COMPBASE
    {
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >
            setPropertyValues( const ::com::sun::star::uno::Sequence<
                                ::com::sun::star::beans::PropertyValue >& rValues,
                            const ::com::sun::star::uno::Reference<
                                ::com::sun::star::ucb::XCommandEnvironment >& xEnv );
        com::sun::star::uno::Sequence< com::sun::star::beans::Property >
            getProperties( const com::sun::star::uno::Reference<
                            com::sun::star::ucb::XCommandEnvironment > & xEnv );

    protected:
        ::cppu::OInterfaceContainerHelper       m_aContentListeners;
        PropertyChangeListenerContainer         m_aPropertyChangeListeners;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                                                m_xParentContainer;
        const ::comphelper::ComponentContext    m_aContext;
        const ::connectivity::SQLError          m_aErrorHelper;
        TContentPtr                             m_pImpl;
        sal_uInt32                              m_nCommandId;


        // helper
        virtual void SAL_CALL disposing();

        virtual void notifyDataSourceModified();

        /**
        * This method can be used to propagate changes of property values.
        *
        * @param evt is a sequence of property change events.
        */
        void notifyPropertiesChange( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyChangeEvent >& evt ) const;
    public:

        OContentHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xORB
                        ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >&   _xParentContainer
                        ,const TContentPtr& _pImpl
                    );

        // com::sun::star::lang::XTypeProvider
        virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< sal_Int8 >  getUnoTunnelImplementationId();
        // ::com::sun::star::lang::XServiceInfo
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);

        // XContent
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentIdentifier > SAL_CALL getIdentifier(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual ::rtl::OUString SAL_CALL getContentType(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL addContentEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentEventListener >& Listener ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL removeContentEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XContentEventListener >& Listener ) throw (::com::sun::star::uno::RuntimeException) ;

        // XCommandProcessor
        virtual sal_Int32 SAL_CALL createCommandIdentifier(  ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::ucb::Command& aCommand, sal_Int32 CommandId, const ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XCommandEnvironment >& Environment ) throw (::com::sun::star::uno::Exception, ::com::sun::star::ucb::CommandAbortedException, ::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL abort( sal_Int32 CommandId ) throw (::com::sun::star::uno::RuntimeException) ;

        // XPropertiesChangeNotifier
        virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener ) throw (::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& PropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& Listener ) throw (::com::sun::star::uno::RuntimeException) ;

        // XPropertyContainer
        virtual void SAL_CALL addProperty( const ::rtl::OUString& Name, sal_Int16 Attributes, const ::com::sun::star::uno::Any& DefaultValue ) throw (::com::sun::star::beans::PropertyExistException, ::com::sun::star::beans::IllegalTypeException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) ;
        virtual void SAL_CALL removeProperty( const ::rtl::OUString& Name ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::NotRemoveableException, ::com::sun::star::uno::RuntimeException) ;

        // XInitialization
        virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        // com::sun::star::lang::XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
        static OContentHelper* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxComponent );

        // ::com::sun::star::container::XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);

        // XRename
        virtual void SAL_CALL rename( const ::rtl::OUString& newName ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::container::ElementExistException, ::com::sun::star::uno::RuntimeException);

        inline const ContentProperties& getContentProperties() const { return m_pImpl->m_aProps; }
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRow >
            getPropertyValues( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::Property >& rProperties );

        inline TContentPtr getImpl() const { return m_pImpl; }
    };

//........................................................................
}   // namespace dbaccess
//........................................................................
#endif // DBA_CONTENTHELPER_HXX
