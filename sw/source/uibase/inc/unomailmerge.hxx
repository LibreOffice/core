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
#include <comphelper/interfacecontainer2.hxx>

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

    comphelper::OInterfaceContainerHelper2     m_aEvtListeners;
    comphelper::OInterfaceContainerHelper2     m_aMergeListeners;
    OPropertyListenerContainerHelper     m_aPropListeners;

    const SfxItemPropertySet*   m_pPropSet;

    SfxObjectShellRef m_xDocSh;   // the document

    OUString        m_aTmpFileName;

    // properties of mail merge service
    css::uno::Sequence< css::uno::Any >           m_aSelection;
    css::uno::Reference< css::sdbc::XResultSet >  m_xResultSet;
    css::uno::Reference< css::sdbc::XConnection > m_xConnection;
    css::uno::Reference< css::frame::XModel >     m_xModel;
    OUString   m_aDataSourceName;
    OUString   m_aDataCommand;
    OUString   m_aFilter;
    OUString   m_aDocumentURL;
    OUString   m_aOutputURL;
    OUString   m_aFileNamePrefix;
    sal_Int32       m_nDataCommandType;
    sal_Int16       m_nOutputType;
    bool        m_bEscapeProcessing;
    bool        m_bSinglePrintJobs;
    bool        m_bFileNameFromColumn;

    OUString                                         m_sInServerPassword;
    OUString                                         m_sOutServerPassword;
    OUString                                         m_sSubject;
    OUString                                         m_sAddressFromColumn;
    OUString                                         m_sMailBody;
    OUString                                         m_sAttachmentName;
    OUString                                         m_sAttachmentFilter;
    css::uno::Sequence< OUString >                   m_aCopiesTo;
    css::uno::Sequence< OUString >                   m_aBlindCopiesTo;
    bool                                             m_bSendAsHTML;
    bool                                             m_bSendAsAttachment;

    css::uno::Sequence< css::beans::PropertyValue >  m_aPrintSettings;

    bool                                             m_bSaveAsSingleFile;
    OUString                                         m_sSaveFilter;
    OUString                                         m_sSaveFilterOptions;
    css::uno::Sequence< css::beans::PropertyValue >  m_aSaveFilterData;

    bool        m_bDisposing;
    SwDBManager     *m_pMgr;

    void    launchEvent( const css::beans::PropertyChangeEvent &rEvt ) const;

    SwXMailMerge( const SwXMailMerge & ) = delete;
    SwXMailMerge & operator = ( const SwXMailMerge & ) = delete;
protected:
    virtual ~SwXMailMerge() override;
public:
    SwXMailMerge();

    void LaunchMailMergeEvent( const css::text::MailMergeEvent &rData ) const;

    // XJob
    virtual css::uno::Any SAL_CALL execute( const css::uno::Sequence< css::beans::NamedValue >& Arguments ) override;

    // XCancellable
    virtual void SAL_CALL cancel() override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMailMergeBroadcaster
    virtual void SAL_CALL addMailMergeEventListener( const css::uno::Reference< css::text::XMailMergeListener >& xListener ) override;
    virtual void SAL_CALL removeMailMergeEventListener( const css::uno::Reference< css::text::XMailMergeListener >& xListener ) override;

    // XComponent
    virtual void SAL_CALL dispose(  ) override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName(  ) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
