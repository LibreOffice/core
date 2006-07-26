/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: propcontroller.hxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:59:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_
#define _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_COMPOSEDUIUPDATE_HXX
#include "composeduiupdate.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPLINELISTENER_HXX_
#include "proplinelistener.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_BROWSERVIEW_HXX_
#include "browserview.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPCR_HXX_
#include "modulepcr.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_
#include "propertyinfo.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX
#include "pcrcomponentcontext.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XEVENTATTACHERMANAGER_HPP_
#include <com/sun/star/script/XEventAttacherManager.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XCONTROLCONTAINER_HPP_
#include <com/sun/star/awt/XControlContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYCONTROLFACTORY_HPP_
#include <com/sun/star/inspection/XPropertyControlFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XOBJECTINSPECTOR_HPP_
#include <com/sun/star/inspection/XObjectInspector.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XOBJECTINSPECTORUI_HPP_
#include <com/sun/star/inspection/XObjectInspectorUI.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XPROPERTYHANDLER_HPP_
#include <com/sun/star/inspection/XPropertyHandler.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE7_HXX_
#include <cppuhelper/implbase7.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif

#include <map>
#include <hash_map>
#include <vector>
#include <memory>

class SvNumberFormatsSupplierObj;
class Font;
class Window;
class SfxItemSet;

//............................................................................
namespace pcr
{
//............................................................................

    class OPropertyEditor;
    struct OLineDescriptor;

#if OSL_DEBUG_LEVEL > 0
    const char* CheckPropertyBrowserInvariants( const void* pVoid );
        // for dignostics with DBG_CHKTHIS
#endif
    DBG_NAMEEX( OPropertyBrowserController )

    //========================================================================
    //= OPropertyBrowserController
    //========================================================================
    // #95343#------------------------------------------------------------------------------------
    typedef ::cppu::WeakImplHelper7 <   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::awt::XFocusListener
                                    ,   ::com::sun::star::awt::XLayoutConstrains
                                    ,   ::com::sun::star::beans::XPropertyChangeListener
                                    ,   ::com::sun::star::inspection::XPropertyControlFactory
                                    ,   ::com::sun::star::inspection::XObjectInspectorUI
                                    ,   ::com::sun::star::inspection::XObjectInspector
                                    >   OPropertyBrowserController_Base;

    class OPropertyBrowserController
                :public ::comphelper::OMutexAndBroadcastHelper
                ,public OPropertyBrowserController_Base
                ,public IPropertyLineListener
                ,public IPropertyExistenceCheck
    {
    private:
        typedef ::std::map< sal_Int32, ::com::sun::star::beans::Property >  OrderedPropertyMap;
        typedef ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >
                                                                            InterfaceArray;

    protected:
        ComponentContext                                                    m_aContext;

    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > m_xFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >  m_xView;

        ::cppu::OInterfaceContainerHelper   m_aDisposeListeners;
        // meta data about the properties
        OPropertyBrowserView*               m_pView;

        ::rtl::OUString                     m_sPageSelection;
        ::rtl::OUString                     m_sLastValidPageSelection;

        typedef ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyHandler >
                                                        PropertyHandlerRef;
        typedef ::std::vector< PropertyHandlerRef >     PropertyHandlerArray;
        typedef ::std::hash_map< ::rtl::OUString, PropertyHandlerRef, ::rtl::OUStringHash >
                                                        PropertyHandlerRepository;
        typedef ::std::hash_multimap< ::rtl::OUString, PropertyHandlerRef, ::rtl::OUStringHash >
                                                        PropertyHandlerMultiRepository;
        PropertyHandlerRepository                       m_aPropertyHandlers;
        PropertyHandlerMultiRepository                  m_aDependencyHandlers;

        ::std::auto_ptr< ComposedPropertyUIUpdate >     m_pUIRequestComposer;

        /// our InspectorModel
        ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorModel >
                                                        m_xModel;
        /// the object(s) we're currently inspecting
        InterfaceArray                                  m_aInspectedObjects;
        /// the properties of the currently inspected object(s)
        OrderedPropertyMap                              m_aProperties;
        /// the property we're just committing
        ::rtl::OUString                                 m_sCommittingProperty;

        typedef ::std::hash_map< ::rtl::OUString, sal_uInt16, ::rtl::OUStringHash >     HashString2Int16;
        HashString2Int16                                m_aPageIds;

        bool        m_bContainerFocusListening      : 1;
        bool        m_bSuspendingPropertyHandlers   : 1;

    protected:
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        // XController
        virtual void SAL_CALL attachFrame( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >& xFrame ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL attachModel( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& xModel ) throw(::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL suspend( sal_Bool bSuspend ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getViewData(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL restoreViewData( const ::com::sun::star::uno::Any& Data ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel > SAL_CALL getModel(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > SAL_CALL getFrame(  ) throw(::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

        // XFocusListener
        virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // XLayoutConstrains #95343# ----------------
        virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw (::com::sun::star::uno::RuntimeException);

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& _rEvent ) throw (::com::sun::star::uno::RuntimeException);

        /** XPropertyControlFactory
        */
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl > SAL_CALL createPropertyControl( ::sal_Int16 ControlType, ::sal_Bool CreateReadOnly ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    public:
        OPropertyBrowserController(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rxContext);

    protected:
        virtual ~OPropertyBrowserController();

    public:
        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);

    protected:
        // IPropertyLineListener
        virtual void    Clicked(    const ::rtl::OUString& _rName, sal_Bool _bPrimary );
        virtual void    Commit(     const ::rtl::OUString& _rName, const ::com::sun::star::uno::Any& _rVal );

        // IPropertyExistenceCheck
        virtual ::sal_Bool SAL_CALL hasPropertyByName( const ::rtl::OUString& _rName ) throw (::com::sun::star::uno::RuntimeException);

        // XObjectInspectorUI
        virtual void SAL_CALL enablePropertyUI( const ::rtl::OUString& _rPropertyName, ::sal_Bool _bEnable ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL enablePropertyUIElements( const ::rtl::OUString& _rPropertyName, ::sal_Int16 _nElements, ::sal_Bool _bEnable ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL rebuildPropertyUI( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL showPropertyUI( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL hidePropertyUI( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL showCategory( const ::rtl::OUString& _rCategory, ::sal_Bool _bShow ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XPropertyControl > SAL_CALL getPropertyControl( const ::rtl::OUString& _rPropertyName ) throw (::com::sun::star::uno::RuntimeException);

        // XObjectInspector
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorModel > SAL_CALL getInspectorModel() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setInspectorModel( const ::com::sun::star::uno::Reference< ::com::sun::star::inspection::XObjectInspectorModel >& _inspectormodel ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL inspect( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > >& Objects ) throw (::com::sun::star::util::VetoException, ::com::sun::star::uno::RuntimeException);

        // XDispatchProvider
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > SAL_CALL queryDispatch( const ::com::sun::star::util::URL& URL, const ::rtl::OUString& TargetFrameName, ::sal_Int32 SearchFlags ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch > > SAL_CALL queryDispatches( const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& Requests ) throw (::com::sun::star::uno::RuntimeException);

    private:
        void UpdateUI();

        void startContainerWindowListening();
        void stopContainerWindowListening();

        // stop the inspection
        void stopInspection( bool _bCommitModified );

        sal_Bool haveView() const { return NULL != m_pView; }

        OPropertyEditor*    getPropertyBox() { return m_pView->getPropertyBox(); }

        // does the inspection of the objects as indicated by our model
        void doInspection();

        // bind the browser to m_xIntrospecteeAsProperty
        void rebindToInspectee( const InterfaceArray& _rObjects );

        /** retrieves special property handlers for our introspectee
        */
        void    getPropertyHandlers( const InterfaceArray& _rObjects, PropertyHandlerArray& _rHandlers );

        /** called when a property changed, to broadcast any handlers which might have
            registered for this property

            @param _bFirstTimeInit
                if set to <FALSE/>, this is a real change in the property value, not just a call
                for purposes of initialization.
        */
        void    impl_broadcastPropertyChange_nothrow( const ::rtl::OUString& _rPropertyName, const ::com::sun::star::uno::Any& _rNewValue, const ::com::sun::star::uno::Any& _rOldValue, bool _bFirstTimeInit ) const;

        /** determines whether the given property is an actuating property, that is, at least one
            handler expressed interest in changes to this property's value.
        */
        inline bool impl_isActuatingProperty_nothrow( const ::rtl::OUString& _rPropertyName ) const
        {
            return ( m_aDependencyHandlers.find( _rPropertyName ) != m_aDependencyHandlers.end() );
        }

        sal_uInt32      GetPropertyPos(const ::rtl::OUString& _rPropName);

        /** retrieves the value of the given property, by asking the appropriate XPropertyHandler
            @param  _rPropertyName
                the name whose handler is to be obtained. Must be the name of a property
                for which a handler is registered.
            @throws
                RuntimeException if there is no handler for the given property
            @return
                the value of this property
        */
        ::com::sun::star::uno::Any
                        impl_getPropertyValue_throw( const ::rtl::OUString& _rPropertyName );

        /// calls XPropertyHandler::suspend for all our property handlers
        sal_Bool    suspendPropertyHandlers_nothrow( sal_Bool _bSuspend );

        /** selects a page according to our current view data
        */
        void selectPageFromViewData();

        /** updates our view data from the currently active page
        */
        void updateViewDataFromActivePage();

        /// describes the UI for the given property
        void describePropertyLine( const ::com::sun::star::beans::Property& _rPropertyName, OLineDescriptor& _rDescriptor )
            SAL_THROW((::com::sun::star::uno::Exception));

        /** retrieves the position of the property given by name in m_aProperties
            @return
                <TRUE/> if and only if the property could be found. In this case, <arg>_pProperty</arg> (if
                not <NULL/> contains the iterator pointing to this property.
        */
        bool impl_findObjectProperty_nothrow( const ::rtl::OUString& _rName, OrderedPropertyMap::const_iterator* _pProperty = NULL );

        sal_Bool Construct(Window* _pParentWin);

        /** retrieves the property handler for a given property name
            @param  _rPropertyName
                the name whose handler is to be obtained. Must be the name of a property
                for which a handler is registered.
            @throws
                RuntimeException if there is no handler for the given property
            @return
                the handler which is responsible for the given property
        */
        PropertyHandlerRef
            impl_getHandlerForProperty_throw( const ::rtl::OUString& _rPropertyName ) const;

        /** determines whether we have a handler for the given property
            @param _rPropertyName
                the name of the property for which the existence of a handler should be checked
        */
        bool
            impl_hasPropertyHandlerFor_nothrow( const ::rtl::OUString& _rPropertyName ) const;

        /** builds up m_aPageIds from InspectorModel::describeCategories, and insert all the
            respective tab pages into our view
            @precond
                m_aPageIds is empty
            @throws ::com::sun::star::uno::RuntimeException
                if one of the callees of this method throws this exception
        */
        void
            impl_buildCategories_throw();

        /** retrieves the id of the tab page which represents a given category.
            @param  _rCategoryName
                the programmatic name of a category.
            @return
                the id of the tab page, or <code>(sal_uInt16)-1</code> if there
                is no tab page for the given category
        */
        sal_uInt16
            impl_getPageIdForCategory_nothrow( const ::rtl::OUString& _rCategoryName ) const;

        /** adds or removes ourself as XEventListener to/from all our inspectees
        */
        void    impl_toggleInspecteeListening_nothrow( bool _bOn );

    private:
        DECL_LINK(OnPageActivation, void*);
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_

