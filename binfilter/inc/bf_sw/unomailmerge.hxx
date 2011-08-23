/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _UNOMAILMERGE_HXX_
#define _UNOMAILMERGE_HXX_

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase5.hxx> // WeakImplHelper4
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>    // OMultiTypeInterfaceContainerHelperVar
#endif
#ifndef _UTL_CONFIGITEM_HXX_
#include <unotools/configitem.hxx>  // !! needed for OMultiTypeInterfaceContainerHelperVar !!
#endif


#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XMAILMERGEBROADCASTER_HPP_
#include <com/sun/star/text/XMailMergeBroadcaster.hpp>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <bf_svtools/itemprop.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <bf_sfx2/objsh.hxx>   // SfxObjectShellRef
#endif
namespace com { namespace sun { namespace star { namespace sdbc {
    class XResultSet;
    class XConnection;
}}}}

namespace com { namespace sun { namespace star { namespace frame {
    class XModel;
}}}}

namespace com { namespace sun { namespace star { namespace lang {
    class XMultiServiceFactory;
}}}}

namespace com { namespace sun { namespace star { namespace text {
    class XMailMergeListener;
    struct MailMergeEvent;
}}}}

namespace rtl {
    class OUString;
}
namespace binfilter {

//STRIP008 namespace com { namespace sun { namespace star { namespace sdbc {
//STRIP008     class XResultSet;
//STRIP008     class XConnection;
//STRIP008 }}}}
//STRIP008 
//STRIP008 namespace com { namespace sun { namespace star { namespace frame {
//STRIP008     class XModel;
//STRIP008 }}}}
//STRIP008 
//STRIP008 namespace com { namespace sun { namespace star { namespace lang {
//STRIP008     class XMultiServiceFactory;
//STRIP008 }}}}
//STRIP008 
//STRIP008 namespace com { namespace sun { namespace star { namespace text {
//STRIP008     class XMailMergeListener;
//STRIP008     struct MailMergeEvent;
//STRIP008 }}}}
//STRIP008 
//STRIP008 namespace rtl {
//STRIP008     class OUString;
//STRIP008 }

///////////////////////////////////////////////////////////////////////////

// uses templates from <cppuhelper/interfacecontainer.h>
// and <unotools/configitem.hxx>

// helper function call class
struct PropHashType_Impl
{
    size_t operator()(const INT32 &s) const { return s; }
};

typedef cppu::OMultiTypeInterfaceContainerHelperVar
    <
        INT32,
        PropHashType_Impl,
        std::equal_to< INT32 >
    > OPropertyListenerContainerHelper;

////////////////////////////////////////////////////////////

class SwXMailMerge : 
    public cppu::WeakImplHelper5
    <
        ::com::sun::star::task::XJob,
        ::com::sun::star::beans::XPropertySet,
        ::com::sun::star::text::XMailMergeBroadcaster,
        ::com::sun::star::lang::XComponent,
        ::com::sun::star::lang::XServiceInfo
    >
{
    cppu::OInterfaceContainerHelper     aEvtListeners;
    cppu::OInterfaceContainerHelper     aMergeListeners;
    OPropertyListenerContainerHelper    aPropListeners;

    //SfxItemPropertySet          aPropSet;
    const SfxItemPropertyMap*   pMap;

    SfxObjectShellRef xDocSh;   // the document

    String          aTmpFileName;

    // properties of mail merge service
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >           aSelection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >  xResultSet;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >     xModel;
    ::rtl::OUString   aDataSourceName;
    ::rtl::OUString   aDataCommand;
    ::rtl::OUString   aFilter;
    ::rtl::OUString   aDocumentURL;
    ::rtl::OUString   aOutputURL;
    ::rtl::OUString   aFileNamePrefix;
    sal_Int32       nDataCommandType;
    sal_Int16       nOutputType;
    sal_Bool        bEscapeProcessing;
    sal_Bool        bSinglePrintJobs;
    sal_Bool        bFileNameFromColumn;

    sal_Bool        bDisposing;

    
    void    launchEvent( const ::com::sun::star::beans::PropertyChangeEvent &rEvt ) const;

    // disallow use of copy-constructor and assignment-operator for now
    SwXMailMerge( const SwXMailMerge & );
    SwXMailMerge & operator = ( const SwXMailMerge & );
protected:
    virtual ~SwXMailMerge();
public:
    SwXMailMerge();
    

    void LaunchMailMergeEvent( const ::com::sun::star::text::MailMergeEvent &rData ) const;

    // XJob
    virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMailMergeBroadcaster
    virtual void SAL_CALL addMailMergeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XMailMergeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeMailMergeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XMailMergeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);
};


extern ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL SwXMailMerge_getSupportedServiceNames() throw();
extern ::rtl::OUString SAL_CALL SwXMailMerge_getImplementationName() throw();
extern ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL SwXMailMerge_createInstance(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > & rSMgr) throw( ::com::sun::star::uno::Exception );

////////////////////////////////////////////////////////////

} //namespace binfilter
#endif


