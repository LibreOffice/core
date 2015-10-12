/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_UNOMAILMERGE_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_UNOMAILMERGE_HXX

#include <cppuhelper/implbase.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <unotools/configitem.hxx>

#include <com/sun/star/task/XJob.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/text/XMailMergeBroadcaster.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <svl/itemprop.hxx>
#include <sfx2/objsh.hxx>

namespace com { namespace sun { namespace star {

    namespace sdbc {
        class XResultSet;
        class XConnection;
    }
    namespace frame {
        class XModel;
    }
    namespace lang {
        class XMultiServiceFactory;
    }
    namespace text {
        class XMailMergeListener;
        struct MailMergeEvent;
    }
    namespace beans{
        struct PropertyValue;
    }

}}}

typedef cppu::OMultiTypeInterfaceContainerHelperVar<sal_Int32>
    OPropertyListenerContainerHelper;

class SwDBManager;
class MailMergeExecuteFinalizer;

class SwXMailMerge :
    public cppu::WeakImplHelper
    <
        com::sun::star::task::XJob,
        com::sun::star::util::XCancellable,
        com::sun::star::beans::XPropertySet,
        com::sun::star::text::XMailMergeBroadcaster,
        com::sun::star::lang::XComponent,
        com::sun::star::lang::XServiceInfo
    >
{
    friend class MailMergeExecuteFinalizer;

    cppu::OInterfaceContainerHelper     aEvtListeners;
    cppu::OInterfaceContainerHelper     aMergeListeners;
    OPropertyListenerContainerHelper    aPropListeners;

    const SfxItemPropertySet*   pPropSet;

    SfxObjectShellRef xDocSh;   // the document

    OUString        aTmpFileName;

    // properties of mail merge service
    com::sun::star::uno::Sequence< com::sun::star::uno::Any >           aSelection;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XResultSet >  xResultSet;
    com::sun::star::uno::Reference< com::sun::star::sdbc::XConnection > xConnection;
    com::sun::star::uno::Reference< com::sun::star::frame::XModel >     xModel;
    OUString   aDataSourceName;
    OUString   aDataCommand;
    OUString   aFilter;
    OUString   aDocumentURL;
    OUString   aOutputURL;
    OUString   aFileNamePrefix;
    sal_Int32       nDataCommandType;
    sal_Int16       nOutputType;
    bool        bEscapeProcessing;
    bool        bSinglePrintJobs;
    bool        bFileNameFromColumn;

    OUString                                         sInServerPassword;
    OUString                                         sOutServerPassword;
    OUString                                         sSubject;
    OUString                                         sAddressFromColumn;
    OUString                                         sMailBody;
    OUString                                         sAttachmentName;
    OUString                                         sAttachmentFilter;
    com::sun::star::uno::Sequence< OUString >        aCopiesTo;
    com::sun::star::uno::Sequence< OUString >        aBlindCopiesTo;
    bool                                                bSendAsHTML;
    bool                                                bSendAsAttachment;

    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aPrintSettings;

    bool                                                bSaveAsSingleFile;
    OUString                                         sSaveFilter;
    OUString                                         sSaveFilterOptions;
    com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aSaveFilterData;

    bool        bDisposing;
    SwDBManager     *m_pMgr;

    void    launchEvent( const com::sun::star::beans::PropertyChangeEvent &rEvt ) const;

    SwXMailMerge( const SwXMailMerge & ) = delete;
    SwXMailMerge & operator = ( const SwXMailMerge & ) = delete;
protected:
    virtual ~SwXMailMerge();
public:
    SwXMailMerge();

    void LaunchMailMergeEvent( const com::sun::star::text::MailMergeEvent &rData ) const;

    // XJob
    virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Arguments )
        throw (::com::sun::star::lang::IllegalArgumentException,
               ::com::sun::star::uno::Exception,
               ::com::sun::star::uno::RuntimeException,
               std::exception) override;

    // XCancellable
    virtual void SAL_CALL cancel() throw (com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XMailMergeBroadcaster
    virtual void SAL_CALL addMailMergeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XMailMergeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeMailMergeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XMailMergeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
