/*************************************************************************
 *
 *  $RCSfile: roadmapcontrol.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-19 13:42:42 $
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

#ifndef TOOLKIT_ROADMAP_CONTROL_HXX
#define TOOLKIT_ROADMAP_CONTROL_HXX


#ifndef _TOOLKIT_HELPER_UNOCONTROLS_HXX_
#include <toolkit/controls/unocontrols.hxx>
#endif

#ifndef _TOOLKIT_AWT_UNOCONTROLMODEL_HXX_
#include <toolkit/controls/unocontrolmodel.hxx>
#endif

#ifndef _TOOLKIT_HELPER_SERVICENAMES_HXX_
#include <toolkit/helper/servicenames.hxx>
#endif

#ifndef _TOOLKIT_ROADMAPENTRY_HXX_
#include <toolkit/controls/roadmapentry.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINER_HPP_
#include <com/sun/star/container/XContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXCONTAINER_HPP_
#include <com/sun/star/container/XIndexContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERLISTENER_HPP_
#include <com/sun/star/container/XContainerListener.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XITEMLISTENER_HPP_
#include <com/sun/star/awt/XItemListener.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XITEMEVENTBROADCASTER_HPP_
#include <com/sun/star/awt/XItemEventBroadcaster.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XIMAGECONSUMER_HPP_
#include <com/sun/star/awt/XImageConsumer.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XIMAGEPRODUCER_HPP_
#include <com/sun/star/awt/XImageProducer.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE5_HXX_
#include <cppuhelper/implbase4.hxx>
#endif



#include <comphelper/uno3.hxx>

typedef UnoControlModel UnoControlRoadmapModel_Base;


typedef ::cppu::ImplHelper4 <   ::com::sun::star::lang::XSingleServiceFactory
                            ,   ::com::sun::star::container::XContainer
                            ,   ::com::sun::star::container::XIndexContainer
                            ,   ::com::sun::star::awt::XImageProducer
                            >   UnoControlRoadmapModel_IBase;



typedef UnoControlBase  UnoControlRoadmap_Base;
typedef ::cppu::ImplHelper4 <   ::com::sun::star::awt::XItemEventBroadcaster
                            ,   ::com::sun::star::container::XContainerListener
                            ,   ::com::sun::star::awt::XItemListener
                            ,   ::com::sun::star::beans::XPropertyChangeListener
                            >   UnoControlRoadmap_IBase;




typedef ::cppu::ImplHelper2< ::com::sun::star::container::XContainerListener,
                             ::com::sun::star::awt::XItemEventBroadcaster>    SVTXRoadmap_Base;


//........................................................................

namespace toolkit{
//........................................................................
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;


    // ===================================================================
    // = UnoControlRoadmapModel
    // ===================================================================
    class UnoControlRoadmapModel : public UnoControlRoadmapModel_Base,
                                   public UnoControlRoadmapModel_IBase

    {
    private:
//        PropertyChangeListenerMultiplexer                 maPropertyListeners;

        typedef ::std::vector< Reference< XInterface > >    RoadmapItemHolderList;

        std::list< ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer > > maImageListeners;

        ContainerListenerMultiplexer        maContainerListeners;
        RoadmapItemHolderList               maRoadmapItems;

        void                                MakeRMItemValidation( sal_Int32 Index, Reference< XInterface > xRoadmapItem );
        ContainerEvent                      GetContainerEvent(sal_Int32 Index, Reference< XInterface > );
        void                                SetRMItemDefaultProperties( const sal_Int32 _Index, Reference< XInterface > );
        sal_Int16                           GetCurrentItemID( Reference< XPropertySet > xPropertySet );
        sal_Int32                           GetUniqueID();


    protected:
        ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
        ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    public:
        UnoControlRoadmapModel();
        UnoControlRoadmapModel( const UnoControlRoadmapModel& rModel ) : UnoControlModel( rModel ), maContainerListeners( *this ) {;}
        UnoControlModel*    Clone() const { return new UnoControlRoadmapModel( *this ); }


    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )


    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertySet
//  ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlRoadmapModel, UnoControlModel, szServiceName2_UnoControlRoadmapModel )

//    void              ImplPropertyChanged( sal_uInt16 nPropId );

    sal_Int32 SAL_CALL getCount() throw (RuntimeException);
    virtual Any SAL_CALL getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const Any & _Element) throw    (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException );
    virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const Any & _Element) throw (IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException );

    virtual void SAL_CALL addContainerListener( const Reference< XContainerListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeContainerListener( const Reference< XContainerListener >& xListener ) throw (RuntimeException);

    ::com::sun::star::uno::Any  SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException) { return UnoControlModel::queryInterface(rType); }
    ::com::sun::star::uno::Any  SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    void                        SAL_CALL acquire() throw()  { UnoControlModel::acquire(); }
    void                        SAL_CALL release() throw()  { UnoControlModel::release(); }


    // ::com::sun::star::awt::XImageProducer
    void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    void SAL_CALL startProduction(  ) throw (::com::sun::star::uno::RuntimeException);


    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw (::com::sun::star::uno::RuntimeException);


    virtual Reference< XInterface > SAL_CALL createInstance(  ) throw (Exception, RuntimeException);
    virtual Reference< XInterface > SAL_CALL createInstanceWithArguments( const Sequence< Any >& aArguments ) throw (Exception, RuntimeException);

    virtual Type SAL_CALL getElementType() throw (RuntimeException);

    virtual sal_Bool SAL_CALL hasElements() throw (RuntimeException);

    };


    // ===================================================================
    // = UnoRoadmapControl
    // ===================================================================
    class UnoRoadmapControl : public UnoControlRoadmap_Base,
                              public UnoControlRoadmap_IBase
    {
    private:
        ItemListenerMultiplexer                 maItemListeners;
    public:
                            UnoRoadmapControl();
        ::rtl::OUString     GetComponentServiceName();

    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException) { UnoControlBase::disposing( Source ); }

    void SAL_CALL   dispose(  ) throw(::com::sun::star::uno::RuntimeException);


    sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model) throw ( ::com::sun::star::uno::RuntimeException );

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

    void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& rEvent )throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& rEvent )throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& rEvent )throw(::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addItemListener( const Reference< XItemListener >& l ) throw (RuntimeException);
    virtual void SAL_CALL removeItemListener( const Reference< XItemListener >& l ) throw (RuntimeException);


    virtual void SAL_CALL itemStateChanged( const ItemEvent& rEvent ) throw (RuntimeException);

    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException);

    void ImplSetPeerProperty( const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rVal );

    // XTypeProvider
    DECLARE_XTYPEPROVIDER( )
    DECLARE_XINTERFACE()

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoRoadmapControl, UnoControlBase, szServiceName2_UnoControlRoadmap )
    };

//........................................................................
} // toolkit
//........................................................................



#endif // _TOOLKIT_ROADMAP_CONTROL_HXX
