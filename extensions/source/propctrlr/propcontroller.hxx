/*************************************************************************
 *
 *  $RCSfile: propcontroller.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 09:06:45 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_
#define _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_

#ifndef _COM_SUN_STAR_AWT_XFOCUSLISTENER_HPP_
#include <com/sun/star/awt/XFocusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XINTROSPECTIONACCESS_HPP_
#include <com/sun/star/beans/XIntrospectionAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
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
#ifndef _EXTENSIONS_PROPCTRLR_STLOPS_HXX_
#include "stlops.hxx"
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
#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPERTYINFO_HXX_
#include "propertyinfo.hxx"
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XTYPECONVERTER_HPP_
#include <com/sun/star/script/XTypeConverter.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif
#ifndef _COMPHELPER_PROPERTYCONTAINER_HXX_
#include <comphelper/propertycontainer.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
// #95343# -----------------
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif
#ifndef _COMPHELPER_BROADCASTHELPER_HXX_
#include <comphelper/broadcasthelper.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_XLAYOUTCONSTRAINS_HPP_
#include <com/sun/star/awt/XLayoutConstrains.hpp>
#endif


class SvNumberFormatsSupplierObj;
class Font;
class Window;
class SfxItemSet;

//............................................................................
namespace pcr
{
//............................................................................

    class OPropertyListener;
    class OPropertyEditor;
    struct OLineDescriptor;

#define OWN_PROPERTY_ID_INTROSPECTEDOBJECT  0x0010
#define OWN_PROPERTY_ID_CURRENTPAGE         0x0011

    // control types
    const sal_Int16 CONTROL_TYPE_UNKNOWN    =   0;
    const sal_Int16 CONTROL_TYPE_FORM       =   1;
    const sal_Int16 CONTROL_TYPE_DIALOG     =   2;

    //========================================================================
    //= OPropertyBrowserController
    //========================================================================
    // #95343#------------------------------------------------------------------------------------
    typedef ::cppu::WeakImplHelper4 <   ::com::sun::star::frame::XController
                                    ,   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::awt::XFocusListener
                                    // #95343# -----------------------
                                    ,   ::com::sun::star::awt::XLayoutConstrains
                                    >   OPropertyBrowserController_Base;
    typedef ::comphelper::OPropertyContainer    OPropertyBrowserController_PropertyBase1;

    class OPropertyBrowserController;
    typedef ::comphelper::OPropertyArrayUsageHelper< OPropertyBrowserController >
                                    OPropertyBrowserController_PropertyBase2;

    class OPropertyBrowserController
                :public ::comphelper::OMutexAndBroadcastHelper
                ,public OPropertyBrowserController_Base
                ,public IPropertyLineListener
                ,public ::comphelper::OPropertyChangeListener
                ,public OModuleResourceClient
                ,public OPropertyBrowserController_PropertyBase1
                ,public OPropertyBrowserController_PropertyBase2
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                            m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter >
                            m_xTypeConverter;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >
                            m_xFrame;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >
                            m_xView;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                            m_xRowsetConnection;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                            m_xIntrospecteeAsProperty;

        ::cppu::OInterfaceContainerHelper       m_aDisposeListeners;

    protected:
        // meta data about the properties
        const IPropertyInfoService*                 m_pPropertyInfo;

        ::comphelper::OPropertyChangeMultiplexer*   m_pChangeMultiplexer;       /// listener to forward property changes
        OPropertyBrowserView*                       m_pView;

        ::rtl::OUString     m_sStandard;
        ::rtl::OUString     m_sPageSelection;

    protected:
        // the inspected object
        ::com::sun::star::uno::Any                                              m_aIntrospectee;
        // the properties of the current object
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >    m_aObjectProperties;
        // the listener types for the current object
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type >          m_aObjectListenerTypes;
        // the event attacher manager for the current object
        ::com::sun::star::uno::Reference< ::com::sun::star::script::XEventAttacherManager > m_xEventManager;
        // quick access to the property states
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyState >         m_xPropStateAccess;
        // quick access to the property states
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >           m_xPropValueAccess;
        // introspection access to the object
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XIntrospectionAccess >   m_xIntrospection;
        // the parent object (if any) of the introspected object
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >               m_xObjectParent;
        // the class id of the introspectee - if appliable
        sal_Int16                                                                           m_nClassId;

        sal_uInt16  m_nGenericPageId;
        sal_uInt16  m_nDataPageId;
        sal_uInt16  m_nEventPageId;

    private:

        sal_Bool    m_bHasListSource            : 1;
        sal_Bool    m_bHasCursorSource          : 1;
        sal_Bool    m_bContainerFocusListening  : 1;

    protected:
        // good callback candidates:

        /// convert the display string into a property value
        ::com::sun::star::uno::Any getPropertyValueFromStringRep( const ::rtl::OUString& _rString, const ::com::sun::star::beans::Property& _rProp, sal_Int32 _nPropId );
        // convert a property value into a display string
        ::rtl::OUString getStringRepFromPropertyValue(const ::com::sun::star::uno::Any& _rValue, sal_Int32 _nPropId);

        // helper to find a string within a string list
        sal_Int32 GetStringPos(const String& _rEntry, const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rEntries);

        // helper
        ::com::sun::star::beans::Property getIntrospecteeProperty(const ::rtl::OUString& _rName);

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

        // XTypeProvider
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

        // XFocusListener
        virtual void SAL_CALL focusGained( const ::com::sun::star::awt::FocusEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL focusLost( const ::com::sun::star::awt::FocusEvent& _rSource ) throw (::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // XLayoutConstrains #95343# ----------------
        virtual ::com::sun::star::awt::Size SAL_CALL getMinimumSize(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL getPreferredSize(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::awt::Size SAL_CALL calcAdjustedSize( const ::com::sun::star::awt::Size& aNewSize ) throw (::com::sun::star::uno::RuntimeException);

        // XPropertySet and friends
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                        sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw (::com::sun::star::uno::Exception);

    public:
        OPropertyBrowserController(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
        virtual ~OPropertyBrowserController();

        void UpdateUI();
        void InsertEvents();

        // XInterface
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL acquire(  ) throw();
        virtual void SAL_CALL release(  ) throw();

        // XServiceInfo - static versions
        static ::rtl::OUString getImplementationName_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(  ) throw(::com::sun::star::uno::RuntimeException);
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    private:
        // IPropertyLineListener
        virtual void    Modified(   const String& _rName, const String& _rVal, void* _pData);
        virtual void    Clicked(    const String& _rName, const String& _rVal, void* _pData);
        virtual void    Commit(     const String& _rName, const String& _rVal, void* _pData);
        virtual void    Select(     const String& _rName, void* _pData);

        // OPropertyChangeListener
        virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& _rEvent) throw( ::com::sun::star::uno::RuntimeException);
        virtual void _disposing(const ::com::sun::star::lang::EventObject& _rSource) throw( ::com::sun::star::uno::RuntimeException);

        /// start listening for property changes
        void startPropertyListening();
        /// stop listening for property changes
        void stopPropertyListening();
        sal_Bool    isListening() const { return NULL != m_pChangeMultiplexer; }

        void startContainerWindowListening();
        void stopContainerWindowListening();

        // stop the inspection
        void stopIntrospection();

        sal_Bool haveObject() const { return m_aIntrospectee.hasValue(); }
        sal_Bool haveView() const { return NULL != m_pView; }

        OPropertyEditor*    getPropertyBox() { return m_pView->getPropertyBox(); }

        // set a new object (a smaller version of bindToObject)
        virtual sal_Bool setObject(const ::com::sun::star::uno::Any& _rIntrospectee, const ::com::sun::star::uno::Any& _rControl);
            // _rControl is the control the model belongs to. Used for events

        // bind the browser to a new object (a more comprehensive version of setObject)
        void bindToObject(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObject);


        /** retrieves the virtual properties which apply to our introspectee
            @return
                <TRUE/> if any only if the introspectee supports virtual properties. In such a
                case <arg>_rProps</arg> will contain a description of these properties
        */
        bool    describeVirtualProperties( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps );

        /** retrieves the actual value of a virtual property
        */
        ::com::sun::star::uno::Any
                getVirtualPropertyValue( sal_Int32 _nPropId );

        /** sets the actual value of a virtual property
        */
        void    setVirtualPropertyValue(
                    sal_Int32 _nPropId,
                    const ::com::sun::star::uno::Any& _rValue
                );

        /** called to update properties which depend on a given properties's value
        */
        void    updateDependentProperties( sal_Int32 _nPropId, const ::com::sun::star::uno::Any& _rNewValue );

        /** sets the modified flag of our document

            <p>The document is searched by traveling up the component hierarchy, starting with
            our introspectee, until we find an XModel. If this XModel supports XModifiable, then
            we use this to set the flag.</p>
        */
        void    setDocumentModified( );

        /** enables the lines for the properties given
            @param _pPropertyStart
                iterator pointing to the first property name
            @param _pPropertyStart
                iterator pointing behind the last property name
            @param _bEnable
                the enable-status of the lines
        */
        void    enablePropertyLines( const ::rtl::OUString* _pPropertyStart, const ::rtl::OUString* _pPropertyEnd,
            sal_Bool _bEnable );

        void SetCursorSource( sal_Bool _bConnect, sal_Bool _bInit );
        void SetListSource(sal_Bool _bInit = sal_False);
        void SetStringSeq(const ::com::sun::star::beans::Property& rProperty, OLineDescriptor& _rUIData);


        void        connectRowset();
        void        cleanupRowsetConnection();
        sal_Bool    haveRowsetConnection( ) const { return m_xRowsetConnection.is(); }

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                    ensureRowsetConnection();

        /** get the rowset for the object we're inspecting
            <p>if we inspect a form, the rowset is the object itself</p>
            <p>if we're inspecting a form control, the XRowSet interface is looked for at the parent.</p>
            <p>if we're inspectting a grid column, the XRowSet is looked for at the parent of the parent</p>
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >
                    getRowSet( ) const;


        sal_uInt32      GetPropertyPos(const ::rtl::OUString& _rPropName);
        ::rtl::OUString GetPropertyValue(const ::rtl::OUString& _rPropName);
        ::com::sun::star::uno::Any
                        GetPropertyUnoValue( const ::rtl::OUString& _rPropName );

        void syncPropertyToView();
        void syncViewToProperty();

        // good candidates for (onClicked-)callbacks, again ....
        void ChangeFontProperty(const ::rtl::OUString& rName);
        void ChangeEventProperty(const ::rtl::OUString& rName);
        void ChangeFormatProperty(const ::rtl::OUString& rName, const ::rtl::OUString& rCurVal);
        // and again ...
        void OnImageURLClicked( const String& _rName, const String& _rVal, void* _pData );

        void SetTables(OLineDescriptor& _rProperty);
        void SetQueries(OLineDescriptor& _rProperty);
        void SetFields(OLineDescriptor& _rProperty);

        void initFormStuff();
        void deinitFormStuff();

        sal_Bool        implGetCheckFontProperty(const ::rtl::OUString& _rPropName, ::com::sun::star::uno::Any& _rValue);
        ::rtl::OUString implGetStringFontProperty(const ::rtl::OUString& _rPropName, const ::rtl::OUString& _rDefault);
        sal_Int16       implGetInt16FontProperty(const ::rtl::OUString& _rPropName, const sal_Int16 _nDefault);
        sal_Int32       implGetInt32FontProperty(const ::rtl::OUString& _rPropName, const sal_Int32 _nDefault);
        float           implGetFloatFontProperty(const ::rtl::OUString& _rPropName, const float _nDefault);

        void            implInvalidateItem(
                            const ::rtl::OUString& _rPropName,
                            sal_uInt16 _nItemId,
                            SfxItemSet& _rSet,
                            sal_Bool _bForceInvalidation = sal_False);

        ::rtl::OUString convertSimpleToString(const ::com::sun::star::uno::Any& _rValue);

        sal_Bool Construct(Window* _pParentWin);

        sal_Int16 getControlType() const;

    private:
        DECL_LINK(OnPageActivation, void*);
    };

//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_PROPCONTROLLER_HXX_

