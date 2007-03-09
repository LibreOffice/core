/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Button.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2007-03-09 13:20:08 $
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

#ifndef _FRM_BUTTON_HXX_
#define _FRM_BUTTON_HXX_

#ifndef FORMS_SOURCE_CLICKABLEIMAGE_HXX
#include "clickableimage.hxx"
#endif

#ifndef _COM_SUN_STAR_AWT_MOUSEEVENT_HPP_
#include <com/sun/star/awt/MouseEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_EVENTOBJECT_HPP_
#include <com/sun/star/lang/EventObject.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_ACTIONEVENT_HPP_
#include <com/sun/star/awt/ActionEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XACTIONLISTENER_HPP_
#include <com/sun/star/awt/XActionListener.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XBUTTON_HPP_
#include <com/sun/star/awt/XButton.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef FORMS_FORM_NAVIGATION_HXX
#include "formnavigation.hxx"
#endif

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OButtonModel
//==================================================================
class OButtonModel
        :public OClickableImageBaseModel
{
public:
    DECLARE_DEFAULT_LEAF_XTOR( OButtonModel );

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OButtonModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;

protected:
    DECLARE_XCLONEABLE();
};

//==================================================================
// OButtonControl
//==================================================================
typedef ::cppu::ImplHelper3 <   ::com::sun::star::awt::XButton
                            ,   ::com::sun::star::awt::XActionListener
                            ,   ::com::sun::star::beans::XPropertyChangeListener
                            >   OButtonControl_BASE;

class OButtonControl    :public OButtonControl_BASE
                        ,public OClickableImageBaseControl
                        ,public OFormNavigationHelper
{
private:
    sal_uInt32  m_nClickEvent;
    sal_Int32   m_nTargetUrlFeatureId;
    /// caches the value of the "Enabled" property of our model
    sal_Bool    m_bEnabledByPropertyValue;

protected:

    // UNO Anbindung
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    OButtonControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OButtonControl();

    // XServiceInfo
    IMPLEMENTATION_NAME(OButtonControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OButtonControl, OClickableImageBaseControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XActionListener
    virtual void SAL_CALL actionPerformed(const ::com::sun::star::awt::ActionEvent& rEvent) throw ( ::com::sun::star::uno::RuntimeException);

    // XButton
    virtual void SAL_CALL addActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLabel(const ::rtl::OUString& Label) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActionCommand(const ::rtl::OUString& _rCommand) throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

    // XControl
    virtual sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxModel ) throw ( ::com::sun::star::uno::RuntimeException );
    void SAL_CALL setDesignMode(sal_Bool bOn) throw (::com::sun::star::uno::RuntimeException);

protected:
    // OFormNavigationHelper overriables
    virtual void    getSupportedFeatures( ::std::vector< sal_Int32 >& /* [out] */ _rFeatureIds );
    virtual void    featureStateChanged( sal_Int32 _nFeatureId, sal_Bool _bEnabled );
    virtual void    allFeatureStatesChanged( );
    virtual bool    isEnabled( sal_Int32 _nFeatureId ) const;

    // XDispatchProviderInterception disambiguaiton
    virtual void SAL_CALL registerDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL releaseDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);

    // OImageControl overridables
    virtual void    actionPerformed_Impl( sal_Bool bNotifyListener, const ::com::sun::star::awt::MouseEvent& _rEvt );

private:
    DECL_LINK( OnClick, void* );

    /** to be called whenever the feature URL represented by our model has potentially changed
    */
    void        modelFeatureUrlPotentiallyChanged( );

    /** retrieves the feature id (see OFormNavigationHelper) of the TargetURL of
        the model.
    */
    sal_Int32   getModelUrlFeatureId( ) const;

    /** starts or stops listening for changes in model properties we're interested in
    */
    void        startOrStopModelPropertyListening( bool _bStart );
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_BUTTON_HXX_

