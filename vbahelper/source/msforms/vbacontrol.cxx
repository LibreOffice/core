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
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <ooo/vba/XControlProvider.hpp>
#ifdef VBA_OOBUILD_HACK
#include <svtools/bindablecontrolhelper.hxx>
#endif
#include "vbacontrol.hxx"
#include "vbacombobox.hxx"
#include "vbabutton.hxx"
#include "vbalabel.hxx"
#include "vbatextbox.hxx"
#include "vbaradiobutton.hxx"
#include "vbalistbox.hxx"
#include "vbatogglebutton.hxx"
#include "vbacheckbox.hxx"
#include "vbaframe.hxx"
#include "vbascrollbar.hxx"
#include "vbaprogressbar.hxx"
#include "vbamultipage.hxx"
#include "vbaspinbutton.hxx"
#include "vbasystemaxcontrol.hxx"
#include "vbaimage.hxx"
#include <vbahelper/helperdecl.hxx>


using namespace com::sun::star;
using namespace ooo::vba;

uno::Reference< css::awt::XWindowPeer >
ScVbaControl::getWindowPeer() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XControlShape > xControlShape( m_xControl, uno::UNO_QUERY );

    uno::Reference< awt::XControlModel > xControlModel;
    uno::Reference< css::awt::XWindowPeer >  xWinPeer;
    if ( !xControlShape.is() )
    {
        // would seem to be a Userform control
        uno::Reference< awt::XControl > xControl( m_xControl, uno::UNO_QUERY_THROW );
        xWinPeer =  xControl->getPeer();
    return xWinPeer;
    }
    // form control
    xControlModel.set( xControlShape->getControl(), uno::UNO_QUERY_THROW );

    uno::Reference< view::XControlAccess > xControlAccess( m_xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    try
    {
        uno::Reference< awt::XControl > xControl( xControlAccess->getControl( xControlModel ), uno::UNO_QUERY );
        xWinPeer =  xControl->getPeer();
    }
    catch( uno::Exception )
    {
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "The Control does not exsit" ),
                uno::Reference< uno::XInterface >() );
    }
    return xWinPeer;
}

//ScVbaControlListener
class ScVbaControlListener: public cppu::WeakImplHelper1< lang::XEventListener >
{
private:
    ScVbaControl *pControl;
public:
    ScVbaControlListener( ScVbaControl *pTmpControl );
    virtual ~ScVbaControlListener();
    virtual void SAL_CALL disposing( const lang::EventObject& rEventObject ) throw( uno::RuntimeException );
};

ScVbaControlListener::ScVbaControlListener( ScVbaControl *pTmpControl ): pControl( pTmpControl )
{
}

ScVbaControlListener::~ScVbaControlListener()
{
}

void SAL_CALL
ScVbaControlListener::disposing( const lang::EventObject& ) throw( uno::RuntimeException )
{
    if( pControl )
    {
        pControl->removeResouce();
        pControl = NULL;
    }
}

//ScVbaControl

ScVbaControl::ScVbaControl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< ::uno::XInterface >& xControl,  const css::uno::Reference< css::frame::XModel >& xModel, AbstractGeometryAttributes* pGeomHelper ) : ControlImpl_BASE( xParent, xContext ),  m_xControl( xControl ), m_xModel( xModel )
{
    //add listener
    m_xEventListener.set( new ScVbaControlListener( this ) );
    setGeometryHelper( pGeomHelper );
    uno::Reference< lang::XComponent > xComponent( m_xControl, uno::UNO_QUERY_THROW );
    xComponent->addEventListener( m_xEventListener );

    //init m_xProps
    uno::Reference< drawing::XControlShape > xControlShape( m_xControl, uno::UNO_QUERY ) ;
    uno::Reference< awt::XControl> xUserFormControl( m_xControl, uno::UNO_QUERY ) ;
    if ( xControlShape.is() ) // form control
        m_xProps.set( xControlShape->getControl(), uno::UNO_QUERY_THROW );
    else if ( xUserFormControl.is() ) // userform control
        m_xProps.set( xUserFormControl->getModel(), uno::UNO_QUERY_THROW );
}

ScVbaControl::~ScVbaControl()
{
    if( m_xControl.is() )
{
        uno::Reference< lang::XComponent > xComponent( m_xControl, uno::UNO_QUERY_THROW );
    xComponent->removeEventListener( m_xEventListener );
}
}

void
ScVbaControl::setGeometryHelper( AbstractGeometryAttributes* pHelper )
{
    mpGeometryHelper.reset( pHelper );
}

void ScVbaControl::removeResouce() throw( uno::RuntimeException )
{
    uno::Reference< lang::XComponent > xComponent( m_xControl, uno::UNO_QUERY_THROW );
    xComponent->removeEventListener( m_xEventListener );
    m_xControl= NULL;
    m_xProps = NULL;
}

//In design model has different behavior
sal_Bool SAL_CALL ScVbaControl::getEnabled() throw (uno::RuntimeException)
{
    uno::Any aValue = m_xProps->getPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enabled" ) ) );
    sal_Bool bRet = false;
    aValue >>= bRet;
    return bRet;
}

void SAL_CALL ScVbaControl::setEnabled( sal_Bool bVisible ) throw (uno::RuntimeException)
{
    uno::Any aValue( bVisible );
    m_xProps->setPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Enabled" ) ), aValue);

}

sal_Bool SAL_CALL ScVbaControl::getVisible() throw (uno::RuntimeException)
{
    sal_Bool bVisible( sal_True );
    m_xProps->getPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableVisible" ) )) >>= bVisible;
    return bVisible;
}

void SAL_CALL ScVbaControl::setVisible( sal_Bool bVisible ) throw (uno::RuntimeException)
{
    uno::Any aValue( bVisible );
    m_xProps->setPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EnableVisible" ) ), aValue);
}
double SAL_CALL ScVbaControl::getHeight() throw (uno::RuntimeException)
{
    return mpGeometryHelper->getHeight();
}
void SAL_CALL ScVbaControl::setHeight( double _height ) throw (uno::RuntimeException)
{
    mpGeometryHelper->setHeight( _height );
}

double SAL_CALL ScVbaControl::getWidth() throw (uno::RuntimeException)
{
    return mpGeometryHelper->getWidth();
}
void SAL_CALL ScVbaControl::setWidth( double _width ) throw (uno::RuntimeException)
{
    mpGeometryHelper->setWidth( _width );
}

double SAL_CALL
ScVbaControl::getLeft() throw (uno::RuntimeException)
{
    return mpGeometryHelper->getLeft();
}

void SAL_CALL
ScVbaControl::setLeft( double _left ) throw (uno::RuntimeException)
{
    mpGeometryHelper->setLeft( _left );
}

double SAL_CALL
ScVbaControl::getTop() throw (uno::RuntimeException)
{
    return mpGeometryHelper->getTop();
}

void SAL_CALL
ScVbaControl::setTop( double _top ) throw (uno::RuntimeException)
{
    mpGeometryHelper->setTop( _top );
}

uno::Reference< uno::XInterface > SAL_CALL
ScVbaControl::getObject() throw (uno::RuntimeException)
{
    uno::Reference< msforms::XControl > xRet( this );
    return xRet;
}

void SAL_CALL ScVbaControl::SetFocus() throw (uno::RuntimeException)
{
    uno::Reference< awt::XWindow > xWin( m_xControl, uno::UNO_QUERY_THROW );
    xWin->setFocus();
}

void SAL_CALL ScVbaControl::Move( double Left, double Top, const uno::Any& Width, const uno::Any& Height )
    throw ( uno::RuntimeException )
{
    double nWidth = 0.0;
    double nHeight = 0.0;

    setLeft( Left );
    setTop( Top );

    if ( Width >>= nWidth )
        setWidth( nWidth );

    if ( Height >>= nHeight )
        setHeight( nHeight );
}

rtl::OUString SAL_CALL
ScVbaControl::getControlSource() throw (uno::RuntimeException)
{
// #FIXME I *hate* having these upstream differences
// but this is necessary until I manage to upstream other
// dependant parts
#ifdef VBA_OOBUILD_HACK
    rtl::OUString sControlSource;
    uno::Reference< form::binding::XBindableValue > xBindable( m_xProps, uno::UNO_QUERY );
    if ( xBindable.is() )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFac( m_xModel, uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.table.CellAddressConversion" ))), uno::UNO_QUERY );
            uno::Reference< beans::XPropertySet > xProps( xBindable->getValueBinding(), uno::UNO_QUERY_THROW );
            table::CellAddress aAddress;
            xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("BoundCell") ) ) >>= aAddress;
            xConvertor->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Address") ), uno::makeAny( aAddress ) );
                    xConvertor->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("XL_A1_Representation") ) ) >>= sControlSource;
        }
        catch( uno::Exception& )
        {
        }
    }
    return sControlSource;
#else
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("getControlSource not supported") ), uno::Reference< uno::XInterface >()); // not supported
#endif
}

void SAL_CALL
ScVbaControl::setControlSource( const rtl::OUString& _controlsource ) throw (uno::RuntimeException)
{
#ifdef VBA_OOBUILD_HACK
    rtl::OUString sEmpty;
    svt::BindableControlHelper::ApplyListSourceAndBindableData( m_xModel, m_xProps, _controlsource, sEmpty );
#else
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("setControlSource not supported ") ).concat( _controlsource ), uno::Reference< uno::XInterface >()); // not supported
#endif
}

rtl::OUString SAL_CALL
ScVbaControl::getRowSource() throw (uno::RuntimeException)
{
#ifdef VBA_OOBUILD_HACK
    rtl::OUString sRowSource;
    uno::Reference< form::binding::XListEntrySink > xListSink( m_xProps, uno::UNO_QUERY );
    if ( xListSink.is() )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFac( m_xModel, uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.table.CellRangeAddressConversion" ))), uno::UNO_QUERY );

            uno::Reference< beans::XPropertySet > xProps( xListSink->getListEntrySource(), uno::UNO_QUERY_THROW );
            table::CellRangeAddress aAddress;
            xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("CellRange") ) ) >>= aAddress;
            xConvertor->setPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Address")), uno::makeAny( aAddress ) );
            xConvertor->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("XL_A1_Representation") ) ) >>= sRowSource;
        }
        catch( uno::Exception& )
        {
        }
    }
    return sRowSource;
#else
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("getRowSource not supported") ), uno::Reference< uno::XInterface >()); // not supported
#endif
}

void SAL_CALL
ScVbaControl::setRowSource( const rtl::OUString& _rowsource ) throw (uno::RuntimeException)
{
#ifdef VBA_OOBUILD_HACK
    rtl::OUString sEmpty;
    svt::BindableControlHelper::ApplyListSourceAndBindableData( m_xModel, m_xProps, sEmpty, _rowsource );
#else
    throw uno::RuntimeException( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("setRowSource not supported ") ).concat( _rowsource ), uno::Reference< uno::XInterface >()); // not supported
#endif
}

rtl::OUString SAL_CALL
ScVbaControl::getName() throw (uno::RuntimeException)
{
    rtl::OUString sName;
    m_xProps->getPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ) ) >>= sName;
    return sName;

}

void SAL_CALL
ScVbaControl::setName( const rtl::OUString& _name ) throw (uno::RuntimeException)
{
    m_xProps->setPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) ), uno::makeAny( _name ) );
    }

rtl::OUString SAL_CALL
ScVbaControl::getControlTipText() throw (css::uno::RuntimeException)
{
    rtl::OUString sName;
    m_xProps->getPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HelpText" ) ) ) >>= sName;
    return sName;
}

void SAL_CALL
ScVbaControl::setControlTipText( const rtl::OUString& rsToolTip ) throw (css::uno::RuntimeException)
{
    m_xProps->setPropertyValue
            (rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HelpText" ) ), uno::makeAny( rsToolTip ) );
}

::rtl::OUString SAL_CALL ScVbaControl::getTag()
    throw (css::uno::RuntimeException)
{
    return m_aControlTag;
}

void SAL_CALL ScVbaControl::setTag( const ::rtl::OUString& aTag )
    throw (css::uno::RuntimeException)
{
    m_aControlTag = aTag;
}

sal_Int32 SAL_CALL ScVbaControl::getTabIndex() throw (uno::RuntimeException)
{
    return 1;
}

void SAL_CALL ScVbaControl::setTabIndex( sal_Int32 /*nTabIndex*/ ) throw (uno::RuntimeException)
{
}

//ScVbaControlFactory

/*static*/ uno::Reference< msforms::XControl > ScVbaControlFactory::createShapeControl(
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< drawing::XControlShape >& xControlShape,
        const uno::Reference< frame::XModel >& xModel ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( xControlShape->getControl(), uno::UNO_QUERY_THROW );
    sal_Int32 nClassId = -1;
    const static rtl::OUString sClassId( RTL_CONSTASCII_USTRINGPARAM("ClassId") );
    xProps->getPropertyValue( sClassId ) >>= nClassId;
    uno::Reference< XHelperInterface > xVbaParent; // #FIXME - should be worksheet I guess
    uno::Reference< drawing::XShape > xShape( xControlShape, uno::UNO_QUERY_THROW );
    ::std::auto_ptr< ConcreteXShapeGeometryAttributes > xGeoHelper( new ConcreteXShapeGeometryAttributes( xContext, xShape ) );

    switch( nClassId )
    {
        case form::FormComponentType::COMBOBOX:
            return new ScVbaComboBox( xVbaParent, xContext, xControlShape, xModel, xGeoHelper.release() );
        case form::FormComponentType::COMMANDBUTTON:
            return new ScVbaButton( xVbaParent, xContext, xControlShape, xModel, xGeoHelper.release() );
        case form::FormComponentType::FIXEDTEXT:
            return new ScVbaLabel( xVbaParent, xContext, xControlShape, xModel, xGeoHelper.release() );
        case form::FormComponentType::TEXTFIELD:
            return new ScVbaTextBox( xVbaParent, xContext, xControlShape, xModel, xGeoHelper.release() );
        case form::FormComponentType::RADIOBUTTON:
            return new ScVbaRadioButton( xVbaParent, xContext, xControlShape, xModel, xGeoHelper.release() );
        case form::FormComponentType::LISTBOX:
            return new ScVbaListBox( xVbaParent, xContext, xControlShape, xModel, xGeoHelper.release() );
        case form::FormComponentType::SPINBUTTON:
            return new ScVbaSpinButton( xVbaParent, xContext, xControlShape, xModel, xGeoHelper.release() );
        case form::FormComponentType::IMAGECONTROL:
            return new ScVbaImage( xVbaParent, xContext, xControlShape, xModel, xGeoHelper.release() );
    }
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Unsupported control." ), uno::Reference< uno::XInterface >() );
}

/*static*/ uno::Reference< msforms::XControl > ScVbaControlFactory::createUserformControl(
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< awt::XControl >& xControl,
        const uno::Reference< awt::XControl >& xDialog,
        const uno::Reference< frame::XModel >& xModel,
        double fOffsetX, double fOffsetY ) throw (uno::RuntimeException)
{
    uno::Reference< beans::XPropertySet > xProps( xControl->getModel(), uno::UNO_QUERY_THROW );
    uno::Reference< lang::XServiceInfo > xServiceInfo( xProps, uno::UNO_QUERY_THROW );
    uno::Reference< msforms::XControl > xVBAControl;
    uno::Reference< XHelperInterface > xVbaParent; // #FIXME - should be worksheet I guess
    ::std::auto_ptr< UserFormGeometryHelper > xGeoHelper( new UserFormGeometryHelper( xContext, xControl, fOffsetX, fOffsetY ) );

    if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlCheckBoxModel") ) ) )
        xVBAControl.set( new ScVbaCheckbox( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlRadioButtonModel") ) ) )
        xVBAControl.set( new ScVbaRadioButton( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlEditModel") ) ) )
        xVBAControl.set( new ScVbaTextBox( xVbaParent, xContext, xControl, xModel, xGeoHelper.release(), true ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlButtonModel") ) ) )
    {
        sal_Bool bToggle = sal_False;
        xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Toggle") ) ) >>= bToggle;
        if ( bToggle )
            xVBAControl.set( new ScVbaToggleButton( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
        else
            xVBAControl.set( new ScVbaButton( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
    }
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlComboBoxModel") ) ) )
        xVBAControl.set( new ScVbaComboBox( xVbaParent, xContext, xControl, xModel, xGeoHelper.release(), true ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlListBoxModel") ) ) )
        xVBAControl.set( new ScVbaListBox( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlFixedTextModel") ) ) )
        xVBAControl.set( new ScVbaLabel( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlImageControlModel") ) ) )
        xVBAControl.set( new ScVbaImage( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlProgressBarModel") ) ) )
        xVBAControl.set( new ScVbaProgressBar( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlGroupBoxModel") ) ) )
        xVBAControl.set( new ScVbaFrame( xVbaParent, xContext, xControl, xModel, xGeoHelper.release(), xDialog ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlScrollBarModel") ) ) )
        xVBAControl.set( new ScVbaScrollBar( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoMultiPageModel") ) ) )
        xVBAControl.set( new ScVbaMultiPage( xVbaParent, xContext, xControl, xModel, xGeoHelper.release(), xDialog ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlSpinButtonModel") ) ) )
        xVBAControl.set( new ScVbaSpinButton( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );
    else if ( xServiceInfo->supportsService( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.custom.awt.UnoControlSystemAXContainerModel") ) ) )
        xVBAControl.set( new VbaSystemAXControl( xVbaParent, xContext, xControl, xModel, xGeoHelper.release() ) );

    if( xVBAControl.is() )
        return xVBAControl;
    throw uno::RuntimeException( rtl::OUString::createFromAscii("Unsupported control." ), uno::Reference< uno::XInterface >() );
}

rtl::OUString&
ScVbaControl::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaControl") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
ScVbaControl::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Control" ) );
    }
    return aServiceNames;
}



typedef cppu::WeakImplHelper1< XControlProvider > ControlProvider_BASE;
class ControlProviderImpl : public ControlProvider_BASE
{
    uno::Reference< uno::XComponentContext > m_xCtx;
public:
    ControlProviderImpl( const uno::Reference< uno::XComponentContext >& xCtx ) : m_xCtx( xCtx ) {}
    virtual uno::Reference< msforms::XControl > SAL_CALL createControl( const uno::Reference< drawing::XControlShape >& xControl, const uno::Reference< frame::XModel >& xDocOwner ) throw (uno::RuntimeException);
};

uno::Reference< msforms::XControl > SAL_CALL
ControlProviderImpl::createControl( const uno::Reference< drawing::XControlShape >& xControlShape, const uno::Reference< frame::XModel >& xDocOwner ) throw (uno::RuntimeException)
{
    uno::Reference< msforms::XControl > xControlToReturn;
    if ( xControlShape.is() )
        xControlToReturn = ScVbaControlFactory::createShapeControl( m_xCtx, xControlShape, xDocOwner );
    return xControlToReturn;

}

namespace controlprovider
{
namespace sdecl = comphelper::service_decl;
sdecl::class_<ControlProviderImpl, sdecl::with_args<false> > serviceImpl;
extern sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ControlProviderImpl",
    "ooo.vba.ControlProvider" );
}


