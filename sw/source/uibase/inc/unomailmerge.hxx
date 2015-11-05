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
        css::task::XJob,
        css::util::XCancellable,
        css::beans::XPropertySet,
        css::text::XMailMergeBroadcaster,
        css::lang::XComponent,
        css::lang::XServiceInfo
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
    css::uno::Sequence< css::uno::Any >           aSelection;
    css::uno::Reference< css::sdbc::XResultSet >  xResultSet;
    css::uno::Reference< css::sdbc::XConnection > xConnection;
    css::uno::Reference< css::frame::XModel >     xModel;
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
    css::uno::Sequence< OUString >        aCopiesTo;
    css::uno::Sequence< OUString >        aBlindCopiesTo;
    bool                                                bSendAsHTML;
    bool                                                bSendAsAttachment;

    css::uno::Sequence< css::beans::PropertyValue > aPrintSettings;

    bool                                                bSaveAsSingleFile;
    OUString                                         sSaveFilter;
    OUString                                         sSaveFilterOptions;
    css::uno::Sequence< css::beans::PropertyValue > aSaveFilterData;

    bool        bDisposing;
    SwDBManager     *m_pMgr;

    void    launchEvent( const css::beans::PropertyChangeEvent &rEvt ) const;

    SwXMailMerge( const SwXMailMerge & ) = delete;
    SwXMailMerge & operator = ( const SwXMailMerge & ) = delete;
protected:
    virtual ~SwXMailMerge();
public:
    SwXMailMerge();

    void LaunchMailMergeEvent( const css::text::MailMergeEvent &rData ) const;

    // XJob
    virtual css::uno::Any SAL_CALL execute( const css::uno::Sequence< css::beans::NamedValue >& Arguments )
        throw (css::lang::IllegalArgumentException,
               css::uno::Exception,
               css::uno::RuntimeException,
               std::exception) override;

    // XCancellable
    virtual void SAL_CALL cancel() throw (css::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) throw (css::beans::UnknownPropertyException, css::beans::PropertyVetoException, css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) throw (css::beans::UnknownPropertyException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XMailMergeBroadcaster
    virtual void SAL_CALL addMailMergeEventListener( const css::uno::Reference< css::text::XMailMergeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeMailMergeEventListener( const css::uno::Reference< css::text::XMailMergeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw (css::uno::RuntimeException, std::exception) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
