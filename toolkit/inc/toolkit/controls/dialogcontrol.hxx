/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef TOOLKIT_DIALOG_CONTROL_HXX
#define TOOLKIT_DIALOG_CONTROL_HXX

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/awt/XTabListener.hpp>
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/awt/XDialog2.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/graphic/XGraphicObject.hpp>
#include <cppuhelper/implbase6.hxx>
#include <cppuhelper/implbase5.hxx>
#include <cppuhelper/implbase2.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include "toolkit/helper/servicenames.hxx"
#include "toolkit/helper/macros.hxx"
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/basemutex.hxx>
#include <list>
#include <map>
#include <com/sun/star/graphic/XGraphicObject.hpp>

//  ----------------------------------------------------
//  class UnoControlDialogModel
//  ----------------------------------------------------
typedef UnoControlModel     UnoControlDialogModel_Base;
typedef ::cppu::ImplHelper6 <   ::com::sun::star::lang::XMultiServiceFactory
                            ,   ::com::sun::star::container::XContainer
                            ,   ::com::sun::star::container::XNameContainer
                            ,   ::com::sun::star::awt::XTabControllerModel
                            ,   ::com::sun::star::util::XChangesNotifier
                            ,   ::com::sun::star::beans::XPropertyChangeListener
                            >   UnoControlDialogModel_IBase;

class UnoControlDialogModel :   public UnoControlDialogModel_IBase
                            ,   public UnoControlDialogModel_Base
{
public:
    enum ChildOperation { Insert = 0, Remove };
    // would like to make this typedef private, too, but the Forte 7 compiler does have
    // problems with this .....
    typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >, ::rtl::OUString >
                                                        UnoControlModelHolder;
protected:
    void updateUserFormChildren(  const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xAllChildren, const rtl::OUString& aName, ChildOperation Operation,  const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xTarget ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    typedef ::std::list< UnoControlModelHolder >        UnoControlModelHolderList;

    // for grouping control models (XTabControllerModel::getGroupXXX)
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >
                                                        ModelGroup;
    typedef ::std::vector< ModelGroup >                 AllGroups;

    friend struct CloneControlModel;
    friend struct FindControlModel;
    friend struct CompareControlModel;

    ContainerListenerMultiplexer        maContainerListeners;
    ::cppu::OInterfaceContainerHelper   maChangeListeners;
    UnoControlModelHolderList           maModels;

    AllGroups                           maGroups;
    sal_Bool                            mbGroupsUpToDate;

    ::com::sun::star::uno::Reference< ::com::sun::star::graphic::XGraphicObject > mxGrfObj;
    bool mbAdjustingGraphic;

    ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();

    UnoControlModelHolderList::iterator         ImplFindElement( const ::rtl::OUString& rName );

public:
                        // bRegProps = false => subclass will register its own props
                        UnoControlDialogModel(  bool bRegProps = true );
                        UnoControlDialogModel( const UnoControlDialogModel& rModel );
                        ~UnoControlDialogModel();

    UnoControlModel*    Clone() const;

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlModel::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAcces
    ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XIndexContainer, XIndexReplace, XIndexAcces
    // void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;
    // sal_Int32 SAL_CALL getCount(  ) throw(::com::sun::star::uno::RuntimeException) = 0;
    // ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;
    // void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;
    // void SAL_CALL removeByIndex( sal_Int32 Index ) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException) = 0;

    // ::com::sun::star::container::XNameContainer, XNameReplace, XNameAccess
    void SAL_CALL replaceByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeByName( const ::rtl::OUString& Name ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XMultiServiceFactory
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments ) throw(::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getAvailableServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // XTabControllerModel
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setGroupControl( sal_Bool GroupControl ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setControlModels( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Controls ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > > SAL_CALL getControlModels(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setGroup( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, const ::rtl::OUString& GroupName ) throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Int32 SAL_CALL getGroupCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL getGroup( sal_Int32 nGroup, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group, ::rtl::OUString& Name ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL getGroupByName( const ::rtl::OUString& Name, ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >& Group ) throw (::com::sun::star::uno::RuntimeException);

    // XChangesNotifier
    virtual void SAL_CALL addChangesListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeChangesListener( const ::com::sun::star::uno::Reference< ::com::sun::star::util::XChangesListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

    // XEventListener
    using cppu::OPropertySetHelper::disposing;
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& evt ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlDialogModel,UnoControlDialogModel_Base, szServiceName2_UnoControlDialogModel )

protected:
    void startControlListening( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxChildModel );
    void stopControlListening( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxChildModel );
    // ::cppu::OPropertySetHelper
    void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception);

    void implNotifyTabModelChange( const ::rtl::OUString& _rAccessor );

    void implUpdateGroupStructure();
private:
    void AddRadioButtonToGroup (
            const ::com::sun::star::uno::Reference< XControlModel >& rControlModel,
            const ::rtl::OUString& rPropertyName,
            ::std::map< ::rtl::OUString, ModelGroup >& pNamedGroups,
            ModelGroup*& rpCurrentGroup );
    void AddRadioButtonGroup (
            ::std::map< ::rtl::OUString, ModelGroup >& pNamedGroups );
};

//  ----------------------------------------------------
//  class UnoDialogControl
//  ----------------------------------------------------
typedef ::cppu::ImplHelper6 <   ::com::sun::star::container::XContainerListener
                            ,   ::com::sun::star::awt::XTopWindow
                            ,   ::com::sun::star::awt::XDialog2
                            ,   ::com::sun::star::util::XChangesListener
                            ,   ::com::sun::star::util::XModifyListener
                            ,   ::com::sun::star::awt::XWindowListener
                            >   UnoDialogControl_IBase;

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
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire() throw ();
        virtual void SAL_CALL release() throw ();

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::resource::XStringResourceResolver > m_xResource;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >             m_xListener;
        bool                                                                                    m_bListening;
};

class UnoDialogControl  :public UnoControlContainer
                        ,public UnoDialogControl_IBase
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >         mxMenuBar;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >   mxTabController;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > mxListener;
    TopWindowListenerMultiplexer                                                maTopWindowListeners;
    bool                                                                        mbWindowListener;
    bool                                                                        mbSizeModified;
    bool                                                                        mbPosModified;

protected:

    void        ImplInsertControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel, const ::rtl::OUString& rName );
    void        ImplRemoveControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel );
    void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl );
    void        ImplUpdateResourceResolver();
    void        ImplStartListingForResourceEvents();

public:

                                UnoDialogControl();
    ::rtl::OUString             GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlContainer::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL toFront(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL toBack(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& xMenu ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XWindowListener
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainerListener
    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDialog2
    virtual void SAL_CALL endDialog( ::sal_Int32 Result ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setHelpId( ::sal_Int32 Id ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDialog
    void SAL_CALL setTitle( const ::rtl::OUString& Title ) throw(::com::sun::star::uno::RuntimeException);
    ::rtl::OUString SAL_CALL getTitle() throw(::com::sun::star::uno::RuntimeException);
    sal_Int16 SAL_CALL execute() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL endExecute() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException);

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const ::com::sun::star::util::ChangesEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

    // XModifyListener
    virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoDialogControl, szServiceName2_UnoControlDialog )

protected:
    virtual void ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents ) throw(::com::sun::star::uno::RuntimeException);
    virtual void PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc );

protected:
    virtual void removingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
    virtual void addingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
};


typedef ::cppu::ImplHelper2< ::com::sun::star::container::XContainerListener,
                             ::com::sun::star::util::XChangesListener >
                             UnoParentControl_IBase;

class UnoParentControl : public UnoControlContainer, public UnoParentControl_IBase
{
protected:
    bool                                                                        mbSizeModified;
    bool                                                                        mbPosModified;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >   mxTabController;
    void        ImplInsertControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel, const ::rtl::OUString& rName );
    void        ImplRemoveControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel );
    virtual void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl );

public:
    UnoParentControl();
    ~UnoParentControl();

    DECLIMPL_SERVICEINFO_DERIVED( UnoParentControl, UnoControlBase, "com.sun.star.awt.UnoParentControl" )
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlContainer::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL acquire() throw() { OWeakAggObject::acquire(); }
    void SAL_CALL release() throw() { OWeakAggObject::release(); }

    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainerListener
    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);

    // XChangesListener
    virtual void SAL_CALL changesOccurred( const ::com::sun::star::util::ChangesEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException);
protected:
    virtual void ImplModelPropertiesChanged( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& rEvents ) throw(::com::sun::star::uno::RuntimeException);
    virtual void removingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
    virtual void addingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
};

class UnoMultiPageModel : public UnoControlDialogModel
{
public:
    UnoMultiPageModel();
    ~UnoMultiPageModel();
    UnoMultiPageModel( const UnoMultiPageModel& rModel );

    UnoControlModel*    Clone() const;

    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageModel, UnoControlDialogModel, szServiceName_UnoMultiPageModel )

    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException);
    // XNamedContainer
    void SAL_CALL insertByName( const ::rtl::OUString& aName, const ::com::sun::star::uno::Any& aElement ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException);
protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();

};

class UnoMultiPageControl :  public UnoParentControl
                            ,public ::com::sun::star::awt::XSimpleTabController
                            ,public ::com::sun::star::awt::XTabListener
{
    TabListenerMultiplexer maTabListeners;
    void bindPage( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
public:
    UnoMultiPageControl();
    ~UnoMultiPageControl();
    ::rtl::OUString     GetComponentServiceName();

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoMultiPageControl, UnoParentControl, szServiceName_UnoMultiPageControl )
    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoParentControl::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }
    // ::com::sun::star::lang::XTypeProvider
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >  SAL_CALL getTypes() throw(::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< sal_Int8 >                     SAL_CALL getImplementationId() throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    // com::sun::star::awt::XSimpleTabController
    virtual ::sal_Int32 SAL_CALL insertTab() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL setTabProps( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue > SAL_CALL getTabProps( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL activateTab( ::sal_Int32 ID ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getActiveTabID() throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTabListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabListener >& Listener ) throw (::com::sun::star::uno::RuntimeException);
    // XTabListener
    virtual void SAL_CALL inserted( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removed( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL changed( ::sal_Int32 ID, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& Properties ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL activated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL deactivated( ::sal_Int32 ID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& evt ) throw (::com::sun::star::uno::RuntimeException);
    // XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

protected:
    virtual void    impl_createControlPeerIfNecessary(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl
    );

};


class UnoPageModel : public UnoControlDialogModel
{
public:
    UnoPageModel();
    ~UnoPageModel();
    UnoPageModel( const UnoPageModel& rModel );

    UnoControlModel*    Clone() const;

    DECLIMPL_SERVICEINFO_DERIVED( UnoPageModel, UnoControlDialogModel, szServiceName_UnoPageModel )

    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException);

    // Override the method of parent Class
    virtual sal_Bool SAL_CALL getGroupControl(  ) throw (::com::sun::star::uno::RuntimeException);
protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();

};

class UnoPageControl :  public UnoParentControl
{
public:
    UnoPageControl();
    ~UnoPageControl();
    ::rtl::OUString     GetComponentServiceName();


    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoPageControl, UnoParentControl, szServiceName_UnoPageControl )
};

class UnoFrameModel : public UnoControlDialogModel
{
public:
    UnoFrameModel();
    ~UnoFrameModel();
    UnoFrameModel( const UnoFrameModel& rModel );

    UnoControlModel*    Clone() const;

    DECLIMPL_SERVICEINFO_DERIVED( UnoFrameModel, UnoControlDialogModel, szServiceName_UnoFrameModel )

    virtual ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw( ::com::sun::star::uno::RuntimeException);

protected:
    virtual ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();
};

class UnoFrameControl :  public UnoParentControl
{
protected:
    virtual void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl );
public:
    UnoFrameControl();
    ~UnoFrameControl();
    ::rtl::OUString     GetComponentServiceName();

// ::com::sun::star::lang::XServiceInfo
DECLIMPL_SERVICEINFO_DERIVED( UnoFrameControl, UnoParentControl, szServiceName_UnoPageControl )
};

#endif // TOOLKIT_DIALOG_CONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
