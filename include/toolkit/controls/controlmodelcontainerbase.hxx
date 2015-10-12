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
#include <cppuhelper/implbase8.hxx>
#include <cppuhelper/implbase3.hxx>
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

//  class ControlModelContainerBase

typedef UnoControlModel     ControlModel_Base;
typedef ::cppu::AggImplInheritanceHelper8   <   ControlModel_Base
                            ,   ::com::sun::star::lang::XMultiServiceFactory
                            ,   ::com::sun::star::container::XContainer
                            ,   ::com::sun::star::container::XNameContainer
                            ,   ::com::sun::star::awt::XTabControllerModel
                            ,   ::com::sun::star::util::XChangesNotifier
                            ,   ::com::sun::star::beans::XPropertyChangeListener
                            ,   ::com::sun::star::awt::tab::XTabPageModel
                            ,   ::com::sun::star::lang::XInitialization
                            >   ControlModelContainer_IBase;

class ControlModelContainerBase :   public ControlModelContainer_IBase
{
public:
    enum ChildOperation { Insert = 0, Remove };
    // would like to make this typedef private, too, but the Forte 7 compiler does have
    // problems with this .....
    typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >, OUString >
                                                        UnoControlModelHolder;
private:
    typedef ::std::list< UnoControlModelHolder >        UnoControlModelHolderList;

public:
    // for grouping control models (XTabControllerModel::getGroupXXX)
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >
                                                        ModelGroup;
    typedef ::std::vector< ModelGroup >                 AllGroups;

    friend struct CloneControlModel;
    friend struct FindControlModel;
    friend struct CompareControlModel;

protected:
    ContainerListenerMultiplexer        maContainerListeners;
    ::cppu::OInterfaceContainerHelper   maChangeListeners;
    UnoControlModelHolderList           maModels;

    AllGroups                           maGroups;
    bool                            mbGroupsUpToDate;

    bool                         m_bEnabled;
    OUString                         m_sImageURL;
    OUString                         m_sTooltip;
    sal_Int16                        m_nTabPageId;

    void    Clone_Impl(ControlModelContainerBase& _rClone) const;
protected:
    ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;

    UnoControlModelHolderList::iterator         ImplFindElement( const OUString& rName );

    void updateUserFormChildren(  const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xAllChildren, const OUString& aName, ChildOperation Operation,  const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xTarget ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
public:
                        ControlModelContainerBase( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
                        ControlModelContainerBase( const ControlModelContainerBase& rModel );
                        virtual ~ControlModelContainerBase();

    UnoControlModel*    Clone() const override;

    // ::com::sun::star::container::XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::container::XElementAcces
    ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::container::XNameContainer, XNameReplace, XNameAccess
    void SAL_CALL replaceByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Any SAL_CALL getByName( const OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    sal_Bool SAL_CALL hasByName( const OUString& aName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL insertByName( const OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeByName( const OUString& Name ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XMultiServiceFactory
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException, std::exception) override;
    ::com::sun::star::uno::Sequence< OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTabControllerModel
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setGroupControl( sal_Bool GroupControl ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setControlModels( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > SAL_CALL getControlModels(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, const OUString& GroupName ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getGroupCount(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL getGroup( sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, OUString& Name ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL getGroupByName( const OUString& Name, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XChangesNotifier
    virtual void SAL_CALL addChangesListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeChangesListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XEventListener
    using cppu::OPropertySetHelper::disposing;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& evt ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED(ControlModelContainerBase, ControlModel_Base, "toolkit.ControlModelContainerBase" )

    // XInitialization
    virtual void SAL_CALL initialize (const com::sun::star::uno::Sequence<com::sun::star::uno::Any>& rArguments)
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::tab::XTabPageModel
    virtual ::sal_Int16 SAL_CALL getTabPageID() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getEnabled() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setEnabled( sal_Bool _enabled ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getTitle() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setTitle( const OUString& _title ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getImageURL() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setImageURL( const OUString& _imageurl ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getToolTip() throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setToolTip( const OUString& _tooltip ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    void startControlListening( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxChildModel );
    void stopControlListening( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxChildModel );

    void implNotifyTabModelChange( const OUString& _rAccessor );

    void implUpdateGroupStructure();
};

class ResourceListener  :public ::com::sun::star::util::XModifyListener,
                         public ::cppu::OWeakObject,
                         public ::cppu::BaseMutex
{
    public:
        ResourceListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& xListener );
        virtual ~ResourceListener();

        void startListening( const ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceResolver  >& rResource );
        void stopListening();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL acquire() throw () override;
        virtual void SAL_CALL release() throw () override;

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceResolver > m_xResource;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >             m_xListener;
        bool                                                                                    m_bListening;
};

typedef ::cppu::AggImplInheritanceHelper3   < UnoControlContainer
                            ,   ::com::sun::star::container::XContainerListener
                            ,   ::com::sun::star::util::XChangesListener
                            ,   ::com::sun::star::util::XModifyListener
                            >   ContainerControl_IBase;

class ControlContainerBase : public ContainerControl_IBase
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  m_xContext;
    bool                                                                        mbSizeModified;
    bool                                                                        mbPosModified;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >   mxTabController;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > mxListener;

    void        ImplInsertControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel, const OUString& rName );
    void        ImplRemoveControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel );
    virtual void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl );
    void        ImplUpdateResourceResolver();
    void        ImplStartListingForResourceEvents();

#ifdef _MSC_VER
    // just implemented to let the various FooImplInheritanceHelper compile
    ControlContainerBase();
#endif

public:
    ControlContainerBase( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~ControlContainerBase();

    DECLIMPL_SERVICEINFO_DERIVED( ControlContainerBase, UnoControlBase, "toolkit.ControlContainerBase" )

    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::container::XContainerListener
    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const ::com::sun::star::util::ChangesEvent& Event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    // XModifyListener
    // Using a dummy/no-op implementation here, not sure if every container control needs
    // to implement this, certainly Dialog does, lets see about others
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& ) throw (::com::sun::star::uno::RuntimeException, std::exception) override {}
protected:
    virtual void ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents ) throw(::com::sun::star::uno::RuntimeException) override;
    virtual void removingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl ) override;
    virtual void addingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl ) override;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
