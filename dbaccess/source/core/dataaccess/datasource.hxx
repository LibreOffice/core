/*************************************************************************
 *
 *  $RCSfile: datasource.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:09:36 $
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

#ifndef _DBA_COREDATAACCESS_DATASOURCE_HXX_
#define _DBA_COREDATAACCESS_DATASOURCE_HXX_

#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XBOOKMARKSSUPPLIER_HPP_
#include <com/sun/star/sdb/XBookmarksSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XISOLATEDCONNECTION_HPP_
#include <com/sun/star/sdbc/XIsolatedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _CPPUHELPER_WEAKREF_HXX_
#include <cppuhelper/weakref.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE9_HXX_
#include <cppuhelper/implbase9.hxx>
#endif
#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DBA_REGHELPER_HXX_
#include "dba_reghelper.hxx"
#endif
#ifndef _DBA_CORE_BOOKMARKCONTAINER_HXX_
#include "bookmarkcontainer.hxx"
#endif
#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XREPORTDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XFORMDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XSTORAGE_HPP_
#include <com/sun/star/embed/XStorage.hpp>
#endif
#ifndef DBA_CONTENTHELPER_HXX
#include "ContentHelper.hxx"
#endif
#ifndef _DRAFTS_COM_SUN_STAR_UI_XUICONFIGURATIONMANAGERSUPPLIER_HPP_
#include <drafts/com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTSUBSTORAGESUPPLIER_HPP_
#include <com/sun/star/document/XDocumentSubStorageSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif

//........................................................................
namespace dbaccess
{
//........................................................................

class OSharedConnectionManager;
class OChildCommitListen_Impl;
typedef ::com::sun::star::uno::WeakReference< ::com::sun::star::sdbc::XConnection > OWeakConnection;
typedef std::vector< OWeakConnection > OWeakConnectionArray;

//============================================================
//= ODatabaseSource
//============================================================
::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    ODatabaseSource_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

typedef ::cppu::ImplHelper9 <   ::com::sun::star::lang::XServiceInfo
                            ,   ::com::sun::star::sdbc::XDataSource
                            ,   ::com::sun::star::sdb::XBookmarksSupplier
                            ,   ::com::sun::star::sdb::XQueryDefinitionsSupplier
                            ,   ::com::sun::star::sdb::XCompletedConnection
                            ,   ::com::sun::star::container::XContainerListener
                            ,   ::com::sun::star::sdbc::XIsolatedConnection
                            ,   ::com::sun::star::sdbcx::XTablesSupplier
                            ,   ::com::sun::star::util::XFlushable
                            >   ODatabaseSource_Base;


typedef ::cppu::ImplHelper9 <   ::com::sun::star::frame::XModel
                            ,   ::com::sun::star::util::XModifiable
                            ,   ::com::sun::star::frame::XStorable
                            ,   ::com::sun::star::view::XPrintable
                            ,   ::com::sun::star::sdb::XFormDocumentsSupplier
                            ,   ::com::sun::star::sdb::XReportDocumentsSupplier
                            ,   ::com::sun::star::util::XCloseable
                            ,   ::drafts::com::sun::star::ui::XUIConfigurationManagerSupplier
                            ,   ::com::sun::star::document::XDocumentSubStorageSupplier
                            >   ODatabaseSource_OfficeDocument;


class ODatabaseSource   :public ::comphelper::OBaseMutex
                    ,public OSubComponent
                    ,public ::cppu::OPropertySetHelper
                    ,public ::comphelper::OPropertyArrayUsageHelper < ODatabaseSource >
                    ,public ODatabaseSource_Base
                    ,public ODatabaseSource_OfficeDocument
{
    friend class ODatabaseContext;
    friend class OConnection;
    friend class OSharedConnectionManager;
    friend ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        ODatabaseSource_CreateInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

protected:
    DECLARE_STL_USTRINGACCESS_MAP(::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >,TStorages);

    enum
    {
        E_FORM   = 0,
        E_REPORT = 1,
        E_QUERY  = 2,
        E_TABLE  = 3
    };
    OWeakConnectionArray        m_aConnections;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xServiceFactory;

    OBookmarkContainer              m_aBookmarks;
    ::std::vector<TContentPtr>      m_aContainer;
    TStorages                       m_aStorages;
    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController> > m_aControllers;

    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xCommandDefinitions;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xTableDefinitions;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xForms;
    ::com::sun::star::uno::WeakReference< ::com::sun::star::container::XNameAccess >    m_xReports;

    ::rtl::OUString                                     m_sFileURL;
// <properties>
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >
                                                        m_xNumberFormatsSupplier;
    ::rtl::OUString                                     m_sConnectURL;
    ::rtl::OUString                                     m_sName;        // transient, our creator has to tell us the title
    ::rtl::OUString                                     m_sUser;
    ::rtl::OUString                                     m_aPassword;    // transient !
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableFilter;
    ::com::sun::star::uno::Sequence< ::rtl::OUString >  m_aTableTypeFilter;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
                                                        m_aLayoutInformation;
    sal_Int32                                           m_nLoginTimeout;
    sal_Bool                                            m_bReadOnly : 1;
    sal_Bool                                            m_bPasswordRequired : 1;
    sal_Bool                                            m_bSuppressVersionColumns : 1;
    sal_Bool                                            m_bModified : 1;
    sal_Bool                                            m_bDocumentReadOnly : 1;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                                                        m_aInfo;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >
                                                        m_aArgs;
// </properties>

    ::cppu::OInterfaceContainerHelper                   m_aModifyListeners;
    ::cppu::OInterfaceContainerHelper                   m_aEventListeners;
    ::cppu::OInterfaceContainerHelper                   m_aCloseListener;
    ::cppu::OInterfaceContainerHelper                   m_aFlushListeners;

    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener>                   m_xSharedConnectionManager;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController>                     m_xCurrentController;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >                       m_xStorage;
    ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XUIConfigurationManager>    m_xUIConfigurationManager;


    ODatabaseContext*                                   m_pDBContext;
    OSharedConnectionManager*                           m_pSharedConnectionManager;
    OChildCommitListen_Impl*                            m_pChildCommitListen;
    sal_uInt16                                          m_nControllerLockCount;


    /// write a single XML stream into the package
    sal_Bool WriteThroughComponent(
        /// the component we export
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent> & xComponent,
        const sal_Char* pStreamName,        /// the stream name
        const sal_Char* pServiceName,       /// service name of the component
        /// the argument (XInitialization)
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments,
        /// output descriptor
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rMediaDesc,
        sal_Bool bPlainStream );            /// neither compress nor encrypt

    /// write a single output stream
    /// (to be called either directly or by WriteThroughComponent(...))
    sal_Bool WriteThroughComponent(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::io::XOutputStream> & xOutputStream,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::lang::XComponent> & xComponent,
        const sal_Char* pServiceName,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::uno::Any> & rArguments,
        const ::com::sun::star::uno::Sequence<
            ::com::sun::star::beans::PropertyValue> & rMediaDesc );
    void writeStorage(const ::rtl::OUString& _sURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& lArguments);

    void lateInit();
    void setMeAsParent(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _xName);

    /** stores the model
        @param  sURL
            The URL
        @param  lArguments
            The media descriptor
    */
    void store(const ::rtl::OUString& sURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& lArguments );

    /** dispose all frames for registered controllers
    */
    void disposeControllerFrames();
protected:
    ODatabaseSource(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory
        ,ODatabaseContext* _pDBContext = NULL
        );
    virtual ~ODatabaseSource();
public:
    ODatabaseSource(
        ::cppu::OWeakObject& _rParent
        ,const ::rtl::OUString& _rRegistrationName
        ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxFactory
        ,ODatabaseContext* _pDBContext = NULL
        );

    // ::com::sun::star::sdbcx::XTablesSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getTables(  ) throw(::com::sun::star::uno::RuntimeException);

// com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId() throw (::com::sun::star::uno::RuntimeException);

// com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw( );
    virtual void SAL_CALL release() throw( );

// ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XServiceInfo - static methods
    static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
        SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

// OComponentHelper
    virtual void SAL_CALL disposing(void);

// com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

// comphelper::OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

// cppu::OPropertySetHelper
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                            ::com::sun::star::uno::Any & rConvertedValue,
                            ::com::sun::star::uno::Any & rOldValue,
                            sal_Int32 nHandle,
                            const ::com::sun::star::uno::Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const ::com::sun::star::uno::Any& rValue
                                                 )
                                                 throw (::com::sun::star::uno::Exception);
    virtual void SAL_CALL getFastPropertyValue( ::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;

// ::com::sun::star::sdb::XCompletedConnection
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connectWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XDataSource
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection( const ::rtl::OUString& user, const ::rtl::OUString& password ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLoginTimeout( sal_Int32 seconds ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getLoginTimeout(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// :: com::sun::star::sdb::XBookmarksSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getBookmarks(  ) throw (::com::sun::star::uno::RuntimeException);

// :: com::sun::star::sdb::XQueryDefinitionsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getQueryDefinitions(  ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::sdbc::XIsolatedConnection
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getIsolatedConnection( const ::rtl::OUString& user, const ::rtl::OUString& password ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getIsolatedConnectionWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::frame::XModel
    virtual sal_Bool SAL_CALL attachResource( const ::rtl::OUString& URL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Arguments ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::rtl::OUString SAL_CALL getURL(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getArgs(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL connectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& Controller ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL disconnectController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& Controller ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL lockControllers(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL unlockControllers(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL hasControllersLocked(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL getCurrentController(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL setCurrentController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& Controller ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException) ;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getCurrentSelection(  ) throw (::com::sun::star::uno::RuntimeException) ;

// ::com::sun::star::frame::XStorable
    virtual sal_Bool SAL_CALL hasLocation(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual ::rtl::OUString SAL_CALL getLocation(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual sal_Bool SAL_CALL isReadonly(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL store(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL storeAsURL( const ::rtl::OUString& sURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL storeToURL( const ::rtl::OUString& sURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lArguments ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException) ;

// ::com::sun::star::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeModifyListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

// ::com::sun::star::util::XModifiable
    virtual sal_Bool SAL_CALL isModified(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL setModified( sal_Bool bModified ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException) ;

// ::com::sun::star::view::XPrintable
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL getPrinter(  ) throw (::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL setPrinter( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aPrinter ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) ;
    virtual void SAL_CALL print( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& xOptions ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException) ;

// XFormDocumentsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getFormDocuments(  ) throw (::com::sun::star::uno::RuntimeException);

// XReportDocumentsSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess > SAL_CALL getReportDocuments(  ) throw (::com::sun::star::uno::RuntimeException);

// XContainerListener
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

// XCloseable
    virtual void SAL_CALL close( sal_Bool DeliverOwnership ) throw (::com::sun::star::util::CloseVetoException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeCloseListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XCloseListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);

// XUIConfigurationManagerSupplier
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::ui::XUIConfigurationManager > SAL_CALL getUIConfigurationManager(  ) throw (::com::sun::star::uno::RuntimeException);

// XDocumentSubStorageSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage > SAL_CALL getDocumentSubStorage( const ::rtl::OUString& aStorageName, sal_Int32 nMode ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getDocumentSubStoragesNames(  ) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// XFlushable
    virtual void SAL_CALL flush(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeFlushListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XFlushListener >& l ) throw (::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> getStorage(const ::rtl::OUString& _sStorageName, sal_Int32 nMode = ::com::sun::star::embed::ElementModes::READWRITE);
protected:
// helper
    const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatsSupplier >&
            getNumberFormatsSupplier();

    /** open a connection for the current settings. this is the simple connection we get from the driver
        manager, so it can be used as a master for a "high level" sdb connection.
    */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > buildLowLevelConnection(
        const ::rtl::OUString& _rUid, const ::rtl::OUString& _rPwd
        );

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > buildIsolatedConnection(
        const rtl::OUString& user, const rtl::OUString& password
        );


// other stuff
    void    flushDocuments();
    void    flushTables();

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL getConnection( const ::rtl::OUString& user, const ::rtl::OUString& password , sal_Bool _bIsolated) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > SAL_CALL connectWithCompletion( const ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionHandler >& handler , sal_Bool _bIsolated) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

    void clearConnections();

    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage> getStorage();
};

//........................................................................
}   // namespace dbaccess
//........................................................................

#endif // _DBA_COREDATAACCESS_DATALINK_HXX_

