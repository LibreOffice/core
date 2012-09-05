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

#ifndef TOOLKIT_CONTROL_MODEL_CONTAINERBASE_HXX
#define TOOLKIT_CONTROL_MODEL_CONTAINERBASE_HXX

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/awt/XTabControllerModel.hpp>
#include <com/sun/star/util/XChangesNotifier.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <cppuhelper/implbase8.hxx>
#include <cppuhelper/implbase2.hxx>
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
//  ----------------------------------------------------
//  class ControlModelContainerBase
//  ----------------------------------------------------
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
    typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >, ::rtl::OUString >
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
    sal_Bool                            mbGroupsUpToDate;

    bool                                    m_bEnabled;
    ::rtl::OUString                         m_sTitle;
    ::rtl::OUString                         m_sImageURL;
    ::rtl::OUString                         m_sTooltip;
    sal_Int16                               m_nTabPageId;

    void    Clone_Impl(ControlModelContainerBase& _rClone) const;
protected:
    ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();

    UnoControlModelHolderList::iterator         ImplFindElement( const ::rtl::OUString& rName );

    void updateUserFormChildren(  const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xAllChildren, const rtl::OUString& aName, ChildOperation Operation,  const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xTarget ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::container::ElementExistException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
public:
                        ControlModelContainerBase( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
                        ControlModelContainerBase( const ControlModelContainerBase& rModel );
                        ~ControlModelContainerBase();

    UnoControlModel*    Clone() const;

    // ::com::sun::star::container::XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAcces
    ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

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
    DECLIMPL_SERVICEINFO_DERIVED(ControlModelContainerBase, ControlModel_Base, "toolkit.ControlModelContainerBase" )

    // XInitialization
    virtual void SAL_CALL initialize (const com::sun::star::uno::Sequence<com::sun::star::uno::Any>& rArguments)
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::tab::XTabPageModel
    virtual ::sal_Int16 SAL_CALL getTabPageID() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getEnabled() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setEnabled( ::sal_Bool _enabled ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTitle() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitle( const ::rtl::OUString& _title ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getImageURL() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setImageURL( const ::rtl::OUString& _imageurl ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getTooltip() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTooltip( const ::rtl::OUString& _tooltip ) throw (::com::sun::star::uno::RuntimeException);

protected:
    void startControlListening( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxChildModel );
    void stopControlListening( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxChildModel );

    void implNotifyTabModelChange( const ::rtl::OUString& _rAccessor );

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

typedef ::cppu::AggImplInheritanceHelper2   < UnoControlContainer
                            ,   ::com::sun::star::container::XContainerListener
                            ,   ::com::sun::star::util::XChangesListener
                            >   ContainerControl_IBase;

class ControlContainerBase : public ContainerControl_IBase
{
    ::Size ImplGetSizePixel( const ::Size& inSize );
protected:
    bool                                                                        mbSizeModified;
    bool                                                                        mbPosModified;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >   mxTabController;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener > mxListener;

    void        ImplInsertControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel, const ::rtl::OUString& rName );
    void        ImplRemoveControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel );
    virtual void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl );
    void        ImplUpdateResourceResolver();
    void        ImplStartListingForResourceEvents();

    ControlContainerBase();

public:
    ControlContainerBase( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ~ControlContainerBase();

    DECLIMPL_SERVICEINFO_DERIVED( ControlContainerBase, UnoControlBase, "toolkit.ControlContainerBase" )

    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainerListener
    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

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
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
