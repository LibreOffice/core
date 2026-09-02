/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

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
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <comphelper/interfacecontainer4.hxx>
#include <rtl/ref.hxx>
#include <mutex>
#include <vector>

namespace com::sun::star::resource { class XStringResourceResolver; }
namespace com::sun::star::uno { class XComponentContext; }
class StdTabController;

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
    // problems with this...
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
    friend struct CompareControlModel;

protected:
    ContainerListenerMultiplexer        maContainerListeners;
    ::comphelper::OInterfaceContainerHelper4<css::util::XChangesListener>   maChangeListeners;
    UnoControlModelHolderVector           maModels;

    AllGroups                           maGroups;
    bool                            mbGroupsUpToDate;

    OUString                         m_sImageURL;
    OUString                         m_sTooltip;
    sal_Int16                        m_nTabPageId;

    void    Clone_Impl(ControlModelContainerBase& _rClone) const;
protected:
    cpo::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;

    UnoControlModelHolderVector::iterator         ImplFindElement( std::u16string_view rName );

    /// @throws css::lang::IllegalArgumentException
    /// @throws css::container::ElementExistException
    /// @throws css::lang::WrappedTargetException
    /// @throws cpo::uno::RuntimeException
    void updateUserFormChildren(  const css::uno::Reference< css::container::XNameContainer >& xAllChildren, const OUString& aName, ChildOperation Operation,  const css::uno::Reference< css::awt::XControlModel >& xTarget );
public:
                        ControlModelContainerBase( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
                        ControlModelContainerBase( const ControlModelContainerBase& rModel );
                        virtual ~ControlModelContainerBase() override;

    rtl::Reference<UnoControlModel> Clone() const override;

    // css::container::XContainer
    void addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    void removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

    // css::container::XElementAccess
    cpo::uno::Type getElementType(  ) override;
    bool hasElements(  ) override;

    // css::container::XNameContainer, XNameReplace, XNameAccess
    void replaceByName( const OUString& aName, const cpo::uno::Any& aElement ) override;
    cpo::uno::Any getByName( const OUString& aName ) override;
    cpo::uno::Sequence< OUString > getElementNames(  ) override;
    bool hasByName( const OUString& aName ) override;
    void insertByName( const OUString& aName, const cpo::uno::Any& aElement ) override;
    void removeByName( const OUString& Name ) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

    // css::lang::XMultiServiceFactory
    css::uno::Reference< css::uno::XInterface > createInstance( const OUString& aServiceSpecifier ) override;
    css::uno::Reference< css::uno::XInterface > createInstanceWithArguments( const OUString& ServiceSpecifier, const cpo::uno::Sequence< cpo::uno::Any >& Arguments ) override;
    cpo::uno::Sequence< OUString > getAvailableServiceNames(  ) override;

    // XComponent
    void dispose(  ) override;

    // XTabControllerModel
    virtual bool getGroupControl(  ) override;
    virtual void setGroupControl( bool GroupControl ) override;
    virtual void setControlModels( const cpo::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Controls ) override;
    virtual cpo::uno::Sequence< css::uno::Reference< css::awt::XControlModel > > getControlModels(  ) override;
    virtual void setGroup( const cpo::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group, const OUString& GroupName ) override;
    virtual sal_Int32 getGroupCount(  ) override;
    virtual void getGroup( sal_Int32 nGroup, cpo::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group, OUString& Name ) override;
    virtual void getGroupByName( const OUString& Name, cpo::uno::Sequence< css::uno::Reference< css::awt::XControlModel > >& Group ) override;

    // XChangesNotifier
    virtual void addChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener ) override;
    virtual void removeChangesListener( const css::uno::Reference< css::util::XChangesListener >& aListener ) override;

    // XPropertyChangeListener
    virtual void propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

    // XEventListener
    using comphelper::OPropertySetHelper::disposing;
    virtual void disposing( const css::lang::EventObject& evt ) override;

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED(ControlModelContainerBase, ControlModel_Base, u"toolkit.ControlModelContainerBase"_ustr )

    // XInitialization
    virtual void initialize (const cpo::uno::Sequence<cpo::uno::Any>& rArguments) override;

    // css::awt::tab::XTabPageModel
    virtual ::sal_Int16 getTabPageID() override;
    virtual bool getEnabled() override;
    virtual void setEnabled( bool _enabled ) override;
    virtual OUString getTitle() override;
    virtual void setTitle( const OUString& _title ) override;
    virtual OUString getImageURL() override;
    virtual void setImageURL( const OUString& _imageurl ) override;
    virtual OUString getToolTip() override;
    virtual void setToolTip( const OUString& _tooltip ) override;

protected:
    void startControlListening( const css::uno::Reference< css::awt::XControlModel >& _rxChildModel );
    void stopControlListening( const css::uno::Reference< css::awt::XControlModel >& _rxChildModel );

    void implNotifyTabModelChange( const OUString& _rAccessor );

    void implUpdateGroupStructure();
};

class ResourceListener final : public css::util::XModifyListener,
                         public ::cppu::OWeakObject
{
    public:
        ResourceListener( const css::uno::Reference< css::util::XModifyListener >& xListener );
        virtual ~ResourceListener() override;

        void startListening( const css::uno::Reference< css::resource::XStringResourceResolver  >& rResource );
        void stopListening();

        // XInterface
        virtual cpo::uno::Any queryInterface( const cpo::uno::Type& aType ) override;
        virtual void acquire() noexcept override;
        virtual void release() noexcept override;

        // XModifyListener
        virtual void modified( const css::lang::EventObject& aEvent ) override;

        // XEventListener
        virtual void disposing( const css::lang::EventObject& Source ) override;

    private:
        std::mutex m_aMutex;
        css::uno::Reference< css::resource::XStringResourceResolver > m_xResource;
        css::uno::Reference< css::util::XModifyListener >             m_xListener;
        bool                                                                                    m_bListening;
};

typedef ::cppu::AggImplInheritanceHelper3   < UnoControlContainer
                            ,   css::container::XContainerListener
                            ,   css::util::XChangesListener
                            ,   css::util::XModifyListener
                            >   ControlContainer_IBase;

class ControlContainerBase : public ControlContainer_IBase
{
protected:
    css::uno::Reference< css::uno::XComponentContext >  m_xContext;
    bool                                                                        mbSizeModified;
    bool                                                                        mbPosModified;
    rtl::Reference< StdTabController >                mxTabController;
    rtl::Reference< ResourceListener > mxListener;

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

    DECLIMPL_SERVICEINFO_DERIVED( ControlContainerBase, UnoControlBase, u"toolkit.ControlContainerBase"_ustr )

    void disposing( const css::lang::EventObject& Source ) override;
    void dispose() override;

    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::container::XContainerListener
    void elementInserted( const css::container::ContainerEvent& Event ) override;
    void elementRemoved( const css::container::ContainerEvent& Event ) override;
    void elementReplaced( const css::container::ContainerEvent& Event ) override;

    // XChangesListener
    virtual void changesOccurred( const css::util::ChangesEvent& Event ) override;

    // css::awt::XControl
    bool setModel( const css::uno::Reference< css::awt::XControlModel >& Model ) override;
    void setDesignMode( bool bOn ) override;
    // XModifyListener
    // Using a dummy/no-op implementation here, not sure if every container control needs
    // to implement this, certainly Dialog does, let's see about others
    virtual void modified( const css::lang::EventObject& ) override {}
protected:
    virtual void ImplModelPropertiesChanged( const cpo::uno::Sequence< css::beans::PropertyChangeEvent >& rEvents ) override;
    virtual void removingControl( const css::uno::Reference< css::awt::XControl >& _rxControl ) override;
    virtual void addingControl( const css::uno::Reference< css::awt::XControl >& _rxControl ) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
