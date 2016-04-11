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
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <cppuhelper/implbase.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/basemutex.hxx>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <tools/gen.hxx>
#include <list>

//  class ControlModelContainerBase

typedef UnoControlModel     ControlModel_Base;
typedef ::cppu::AggImplInheritanceHelper < ControlModel_Base
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
    typedef ::std::list< UnoControlModelHolder >        UnoControlModelHolderList;

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
    UnoControlModelHolderList           maModels;

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

    UnoControlModelHolderList::iterator         ImplFindElement( const OUString& rName );

    void updateUserFormChildren(  const css::uno::Reference< css::container::XNameContainer >& xAllChildren, const OUString& aName, ChildOperation Operation,  const css::uno::Reference< css::awt::XControlModel >& xTarget ) throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException);
public:
                        ControlModelContainerBase( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        ControlModelContainerBase( const ControlModelContainerBase& rModel );
                        virtual ~ControlModelContainerBase() override;

    UnoControlModel*    Clone() const override;

    // css::container::XContainer
    void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;

    // css::container::XElementAcces
    css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::container::XNameContainer, XNameReplace, XNameAccess
    void SAL_CALL replaceByName( const OUString& aName, const css::uno::Any& aElement ) throw(css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    css::uno::Any SAL_CALL getByName( const OUString& aName ) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(css::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL insertByName( const OUString& aName, const css::uno::Any& aElement ) throw(css::lang::IllegalArgumentException, css::container::ElementExistException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeByName( const OUString& Name ) throw(css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XMultiServiceFactory
    css::uno::Reference< css::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const css::uno::Sequence< css::uno::Any >& Arguments ) throw(css::uno::Exception, css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // XTabControllerModel
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setGroupControl( sal_Bool GroupControl ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setControlModels( const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Controls ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > SAL_CALL getControlModels(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setGroup( const css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group, const OUString& GroupName ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getGroupCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL getGroup( sal_Int32 nGroup, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group, OUString& Name ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL getGroupByName( const OUString& Name, css::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group ) throw (css::uno::RuntimeException, std::exception) override;

    // XChangesNotifier
    virtual void SAL_CALL addChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw (css::uno::RuntimeException, std::exception) override;

    // XEventListener
    using cppu::OPropertySetHelper::disposing;
    virtual void SAL_CALL disposing( const css::lang::EventObject& evt ) throw (css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED(ControlModelContainerBase, ControlModel_Base, "toolkit.ControlModelContainerBase" )

    // XInitialization
    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // css::awt::tab::XTabPageModel
    virtual ::sal_Int16 SAL_CALL getTabPageID() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getEnabled() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setEnabled( sal_Bool _enabled ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitle() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTitle( const OUString& _title ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getImageURL() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setImageURL( const OUString& _imageurl ) throw (css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getToolTip() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setToolTip( const OUString& _tooltip ) throw (css::uno::RuntimeException, std::exception) override;

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
        virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type& aType ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XModifyListener
        virtual void SAL_CALL modified( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;

    private:
        css::uno::Reference< css::resource::XStringResourceResolver > m_xResource;
        css::uno::Reference< css::util::XModifyListener >             m_xListener;
        bool                                                                                    m_bListening;
};

typedef ::cppu::AggImplInheritanceHelper < UnoControlContainer
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

    void        ImplInsertControl( css::uno::Reference< css::awt::XControlModel >& rxModel, const OUString& rName );
    void        ImplRemoveControl( css::uno::Reference< css::awt::XControlModel >& rxModel );
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

    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::container::XContainerListener
    void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) throw(css::uno::RuntimeException, std::exception) override;

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const css::util::ChangesEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XControl
    sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& Model ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(css::uno::RuntimeException, std::exception) override;
    // XModifyListener
    // Using a dummy/no-op implementation here, not sure if every container control needs
    // to implement this, certainly Dialog does, lets see about others
    virtual void SAL_CALL modified( const css::lang::EventObject& ) throw (css::uno::RuntimeException, std::exception) override {}
protected:
    virtual void ImplModelPropertiesChanged( const css::uno::Sequence< css::beans::PropertyChangeEvent >& rEvents ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void removingControl( const css::uno::Reference< css::awt::XControl >& _rxControl ) override;
    virtual void addingControl( const css::uno::Reference< css::awt::XControl >& _rxControl ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
