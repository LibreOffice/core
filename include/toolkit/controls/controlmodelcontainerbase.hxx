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

#ifndef INCLUDED_TOOLKIT_CONTROLS_CONTROLMODELCONTAINERBASE_HXX
#define INCLUDED_TOOLKIT_CONTROLS_CONTROLMODELCONTAINERBASE_HXX

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <cppuhelper/implbase8.hxx>
#include <cppuhelper/implbase3.hxx>
#include <cppuhelper/weak.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <vector>

namespace com { namespace sun { namespace star { namespace resource { class XStringResourceResolver; } } } }
namespace com { namespace sun { namespace star { namespace uno { class XComponentContext; } } } }

//  class ControlModelContainerBase

typedef UnoControlModel     ControlModel_Base;
typedef ::cppu::AggImplInheritanceHelper8   <   ControlModel_Base
                            ,   css::lang::XMultiServiceFactory
                            ,   css::container::XContainer
                            ,   css::container::XNameContainer
                            ,   css::awt::XTabControllerModel
                            ,   css::util::XChangesNotifier
                            ,   css::beans::XPropertyChangeListener
                            ,   css::awt::tab::XTabPageModel
                            ,   css::lang::XInitialization
                            >   ControlModelContainer_IBase;

class ControlModelContainerBase :   public ControlModelContainer_IBase
{
public:
    enum ChildOperation { Insert = 0, Remove };
    // would like to make this typedef private, too, but the Forte 7 compiler does have
    // problems with this .....
    typedef ::std::pair< css::uno::Reference< css::awt::XControlModel >, OUString >
                                                        UnoControlModelHolder;
private:
    typedef ::std::vector< UnoControlModelHolder >        UnoControlModelHolderVector;

public:
    // for grouping control models (XTabControllerModel::getGroupXXX)
    typedef ::std::vector< css::uno::Reference< css::awt::XControlModel > >
                                                        ModelGroup;
    typedef ::std::vector< ModelGroup >                 AllGroups;

    friend struct CloneControlModel;
    friend struct FindControlModel;
    friend struct CompareControlModel;

protected:
    ContainerListenerMultiplexer        maContainerListeners;
    ::comphelper::OInterfaceContainerHelper2   maChangeListeners;
    UnoControlModelHolderVector           maModels;

    AllGroups                           maGroups;
    bool                            mbGroupsUpToDate;

    bool                         m_bEnabled;
    OUString                         m_sImageURL;
    OUString                         m_sTooltip;
    sal_Int16                        m_nTabPageId;

    void    Clone_Impl(ControlModelContainerBase& _rClone) const;
protected:
    css::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;

    UnoControlModelHolderVector::iterator         ImplFindElement( const OUString& rName );

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::container::ElementExistException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    void updateUserFormChildren(  const css::uno::Reference< css::container::XNameContainer >& xAllChildren, const OUString& aName, ChildOperation Operation,  const css::uno::Reference< css::awt::XControlModel >& xTarget );
public:
                        ControlModelContainerBase( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        ControlModelContainerBase( const ControlModelContainerBase& rModel );
                        virtual ~ControlModelContainerBase() override;

    rtl::Reference<UnoControlModel> Clone() const override;

    // css::container::XContainer
    void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

    // css::container::XElementAccess
    css::uno::Type SAL_CALL getElementType(  ) override;
    sal_Bool SAL_CALL hasElements(  ) override;

    // css::container::XNameContainer, XNameReplace, XNameAccess
    void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) override;
    css::uno::Any SAL_CALL getByName( const OUString& aName ) override;
    css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) override;
    sal_Bool SAL_CALL hasByName( const OUString& aName ) override;
    void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) override;
    void SAL_CALL removeByName( const OUString& Name ) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // css::lang::XMultiServiceFactory
    css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) override;
    css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) override;
    css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) override;

    // XComponent
    void SAL_CALL dispose(  ) override;

    // XTabControllerModel
    virtual sal_Bool SAL_CALL getGroupControl(  ) override;
    virtual void SAL_CALL setGroupControl( sal_Bool GroupControl ) override;
    virtual void SAL_CALL setControlModels( const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Controls ) override;
    virtual css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > SAL_CALL getControlModels(  ) override;
    virtual void SAL_CALL setGroup( const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group, const OUString& GroupName ) override;
    virtual sal_Int32 SAL_CALL getGroupCount(  ) override;
    virtual void SAL_CALL getGroup( sal_Int32 nGroup, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group, OUString& Name ) override;
    virtual void SAL_CALL getGroupByName( const OUString& Name, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group ) override;

    // XChangesNotifier
    virtual void SAL_CALL addChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener ) override;
    virtual void SAL_CALL removeChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener ) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

    // XEventListener
    using cppu::OPropertySetHelper::disposing;
    virtual void SAL_CALL disposing( const css::lang::EventObject& evt ) override;

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED(ControlModelContainerBase, ControlModel_Base, "toolkit.ControlModelContainerBase" )

    // XInitialization
    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments) override;

    // css::awt::tab::XTabPageModel
    virtual ::sal_Int16 SAL_CALL getTabPageID() override;
    virtual sal_Bool SAL_CALL getEnabled() override;
    virtual void SAL_CALL setEnabled( sal_Bool _enabled ) override;
    virtual OUString SAL_CALL getTitle() override;
    virtual void SAL_CALL setTitle( const OUString& _title ) override;
    virtual OUString SAL_CALL getImageURL() override;
    virtual void SAL_CALL setImageURL( const OUString& _imageurl ) override;
    virtual OUString SAL_CALL getToolTip() override;
    virtual void SAL_CALL setToolTip( const OUString& _tooltip ) override;

protected:
    void startControlListening( const css::uno::Reference< css::awt::XControlModel >& _rxChildModel );
    void stopControlListening( const css::uno::Reference< css::awt::XControlModel >& _rxChildModel );

    void implNotifyTabModelChange( const OUString& _rAccessor );

    void implUpdateGroupStructure();
};

class ResourceListener  :public css::util::XModifyListener,
                         public ::cppu::OWeakObject,
                         public ::cppu::BaseMutex
{
    public:
        ResourceListener( const css::uno::Reference< css::util::XModifyListener >& xListener );
        virtual ~ResourceListener() override;

        void startListening( const css::uno::Reference< css::resource::XStringResourceResolver  >& rResource );
        void stopListening();

        // XInterface
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XModifyListener
        virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    private:
        css::uno::Reference< css::resource::XStringResourceResolver > m_xResource;
        css::uno::Reference< css::util::XModifyListener >             m_xListener;
        bool                                                                                    m_bListening;
};

typedef ::cppu::AggImplInheritanceHelper3   < UnoControlContainer
                            ,   css::container::XContainerListener
                            ,   css::util::XChangesListener
                            ,   css::util::XModifyListener
                            >   ContainerControl_IBase;

class ControlContainerBase : public ContainerControl_IBase
{
protected:
    css::uno::Reference< css::uno::XComponentContext >  m_xContext;
    bool                                                                        mbSizeModified;
    bool                                                                        mbPosModified;
    css::uno::Reference< css::awt::XTabController >   mxTabController;
    css::uno::Reference< css::util::XModifyListener > mxListener;

    void        ImplInsertControl( css::uno::Reference< css::awt::XControlModel > const & rxModel, const OUString& rName );
    void        ImplRemoveControl( css::uno::Reference< css::awt::XControlModel > const & rxModel );
    virtual void        ImplSetPosSize( css::uno::Reference< css::awt::XControl >& rxCtrl );
    void        ImplUpdateResourceResolver();
    void        ImplStartListingForResourceEvents();

#ifdef _MSC_VER
    // just implemented to let the various FooImplInheritanceHelper compile
    ControlContainerBase();
#endif

public:
    ControlContainerBase( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~ControlContainerBase() override;

    DECLIMPL_SERVICEINFO_DERIVED( ControlContainerBase, UnoControlBase, "toolkit.ControlContainerBase" )

    void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
    void SAL_CALL dispose() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::container::XContainerListener
    void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
    void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
    void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& Event ) override;

    // css::awt::XControl
    sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& Model ) override;
    void SAL_CALL setDesignMode( sal_Bool bOn ) override;
    // XModifyListener
    // Using a dummy/no-op implementation here, not sure if every container control needs
    // to implement this, certainly Dialog does, lets see about others
    virtual void SAL_CALL modified( const css::lang::EventObject& ) override {}
protected:
    virtual void ImplModelPropertiesChanged( const css::uno::Sequence< css::beans::PropertyChangeEvent >& rEvents ) override;
    virtual void removingControl( const css::uno::Reference< css::awt::XControl >& _rxControl ) override;
    virtual void addingControl( const css::uno::Reference< css::awt::XControl >& _rxControl ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
