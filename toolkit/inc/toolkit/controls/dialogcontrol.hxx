/*************************************************************************
 *
 *  $RCSfile: dialogcontrol.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:02:53 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef TOOLKIT_DIALOG_CONTROL_HXX
#define TOOLKIT_DIALOG_CONTROL_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTABCONTROLLERMODEL_HPP_
#include <com/sun/star/awt/XTabControllerModel.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XTOPWINDOW_HPP_
#include <com/sun/star/awt/XTopWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESNOTIFIER_HPP_
#include <com/sun/star/util/XChangesNotifier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCHANGESLISTENER_HPP_
#include <com/sun/star/util/XChangesListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XDIALOG_HPP_
#include <com/sun/star/awt/XDialog.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE6_HXX_
#include <cppuhelper/implbase6.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _TOOLKIT_HELPER_LISTENERMULTIPLEXER_HXX_
#include <toolkit/helper/listenermultiplexer.hxx>
#endif
#ifndef _TOOLKIT_AWT_UNOCONTROLMODEL_HXX_
#include <toolkit/controls/unocontrolmodel.hxx>
#endif
#ifndef _TOOLKIT_HELPER_SERVICENAMES_HXX_
#include "toolkit/helper/servicenames.hxx"
#endif
#include "toolkit/helper/macros.hxx"
#ifndef _TOOLKIT_CONTROLS_UNOCONTROLCONTAINER_HXX_
#include <toolkit/controls/unocontrolcontainer.hxx>
#endif
#ifndef _CPPUHELPER_PROPSHLP_HXX
#include <cppuhelper/propshlp.hxx>
#endif
#include <list>

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
    // would like to make this typedef private, too, but the Forte 7 compiler does have
    // problems with this .....
    typedef ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >, ::rtl::OUString >
                                                        UnoControlModelHolder;
private:
    typedef ::std::list< UnoControlModelHolder >        UnoControlModelHolderList;

    // for grouping control models (XTabControllerModel::getGroupXXX)
    typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > >
                                                        ModelGroup;
    typedef ::std::vector< ModelGroup >                 AllGroups;

    friend struct CloneControlModel;
    friend struct FindControlModel;
    friend struct CompareControlModel;

private:
    ContainerListenerMultiplexer        maContainerListeners;
    ::cppu::OInterfaceContainerHelper   maChangeListeners;
    UnoControlModelHolderList           maModels;

    AllGroups                           maGroups;
    sal_Bool                            mbGroupsUpToDate;

protected:
    ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper();

    UnoControlModelHolderList::iterator         ImplFindElement( const ::rtl::OUString& rName );

public:
                        UnoControlDialogModel();
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
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& evt ) throw (::com::sun::star::uno::RuntimeException);

    // XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlDialogModel,UnoControlDialogModel_Base, szServiceName2_UnoControlDialogModel )

protected:
    void startControlListening( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxChildModel );
    void stopControlListening( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxChildModel );

    void implNotifyTabModelChange( const ::rtl::OUString& _rAccessor );

    void implUpdateGroupStructure();
};

//  ----------------------------------------------------
//  class UnoDialogControl
//  ----------------------------------------------------
typedef ::cppu::ImplHelper4 <   ::com::sun::star::container::XContainerListener
                            ,   ::com::sun::star::awt::XTopWindow
                            ,   ::com::sun::star::awt::XDialog
                            ,   ::com::sun::star::util::XChangesListener
                            >   UnoDialogControl_IBase;

class UnoDialogControl  :public UnoControlContainer
                        ,public UnoDialogControl_IBase
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >         mxMenuBar;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTabController >   mxTabController;
    TopWindowListenerMultiplexer    maTopWindowListeners;

protected:

    void        ImplInsertControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel, const ::rtl::OUString& rName );
    void        ImplRemoveControl( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel );
    void        ImplSetPosSize( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& rxCtrl );

public:

                                UnoDialogControl();
    ::rtl::OUString             GetComponentServiceName();

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlContainer::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { OWeakAggObject::acquire(); }
    void                        SAL_CALL release() throw()  { OWeakAggObject::release(); }

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlContainer::disposing( Source ); }
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XTopWindow
    void SAL_CALL addTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeTopWindowListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XTopWindowListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL toFront(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL toBack(  ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL setMenuBar( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XMenuBar >& xMenu ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertiesChangeListener
    void SAL_CALL propertiesChange( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyChangeEvent >& evt ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainerListener
    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);


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

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO( UnoDialogControl, szServiceName2_UnoControlDialog )

protected:
    virtual void removingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
    virtual void addingControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& _rxControl );
};


#endif // TOOLKIT_DIALOG_CONTROL_HXX
