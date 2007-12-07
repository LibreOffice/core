/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vbacontrol.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-12-07 10:49:52 $
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
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/container/XChild.hpp>

#include"vbacontrol.hxx"
#include"vbacombobox.hxx"
#include "vbabutton.hxx"
#include "vbalabel.hxx"
#include "vbatextbox.hxx"
#include "vbaradiobutton.hxx"
#include "vbalistbox.hxx"


using namespace com::sun::star;
using namespace org::openoffice;

sal_Int32 pt2mm( double pt );
double mm2pt( sal_Int32 mm );

uno::Reference< css::awt::XWindowPeer >
ScVbaControl::getWindowPeer( const uno::Reference< ::drawing::XControlShape >& xControlShape ) throw (uno::RuntimeException)
{
    uno::Reference< awt::XControlModel > xControlModel( xControlShape->getControl(), uno::UNO_QUERY_THROW );
    //init m_xWindowPeer
    uno::Reference< container::XChild > xChild( xControlModel, uno::UNO_QUERY_THROW );
    xChild.set( xChild->getParent(), uno::UNO_QUERY_THROW );
    xChild.set( xChild->getParent(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XModel > xModel( xChild->getParent(), uno::UNO_QUERY_THROW );
    uno::Reference< view::XControlAccess > xControlAccess( xModel->getCurrentController(), uno::UNO_QUERY_THROW );
    uno::Reference< awt::XControl > xControl;
    try
    {
        xControl.set( xControlAccess->getControl( xControlModel ), uno::UNO_QUERY );
    }
    catch( uno::Exception )
    {
        throw uno::RuntimeException( rtl::OUString::createFromAscii( "The Control does not exsit" ),
                uno::Reference< uno::XInterface >() );
    }
    return xControl->getPeer();
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

ScVbaControl::ScVbaControl( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< ::drawing::XControlShape >& xControlShape ) : m_xContext( xContext ), m_xControlShape( xControlShape )
{
    //init m_xProps
    uno::Reference< awt::XControlModel > xControlModel( xControlShape->getControl(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xControlModel, uno::UNO_QUERY_THROW );
    m_xProps.set( xProps, uno::UNO_QUERY_THROW );
    //add listener
    m_xEventListener.set( new ScVbaControlListener( this ) );
    uno::Reference< lang::XComponent > xComponent( m_xControlShape, uno::UNO_QUERY_THROW );
    xComponent->addEventListener( m_xEventListener );
}

ScVbaControl::~ScVbaControl()
{
    if( m_xControlShape.is() )
    {
        uno::Reference< lang::XComponent > xComponent( m_xControlShape, uno::UNO_QUERY_THROW );
        xComponent->removeEventListener( m_xEventListener );
    }
}

void ScVbaControl::removeResouce() throw( uno::RuntimeException )
{
    uno::Reference< lang::XComponent > xComponent( m_xControlShape, uno::UNO_QUERY_THROW );
    xComponent->removeEventListener( m_xEventListener );
    m_xControlShape = NULL;
    m_xProps = NULL;
}


ScVbaControl::ScVbaControl( const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< beans::XPropertySet >& xProps,
        const uno::Reference< drawing::XControlShape > xControlShape ) : m_xContext( xContext ),
        m_xProps( xProps ), m_xControlShape( xControlShape )
{
    m_xEventListener.set( new ScVbaControlListener( this ) );
    uno::Reference< lang::XComponent > xComponent( m_xControlShape, uno::UNO_QUERY_THROW );
    xComponent->addEventListener( m_xEventListener );
}

void ScVbaControl::SetControl( const uno::Reference< uno::XComponentContext > xContext, const uno::Reference< ::drawing::XControlShape > xControlShape )
{
    m_xEventListener.set( new ScVbaControlListener( this ) );
    uno::Reference< lang::XComponent > xComponent( m_xControlShape, uno::UNO_QUERY_THROW );
    m_xContext.set( xContext, uno::UNO_QUERY_THROW );
    m_xControlShape.set( xControlShape, uno::UNO_QUERY_THROW );
    //init m_xProps
    uno::Reference< awt::XControlModel > xControlModel( xControlShape->getControl(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xControlModel, uno::UNO_QUERY_THROW );
    m_xProps = xProps;
}

void ScVbaControl::SetControl( const uno::Reference< uno::XComponentContext > xContext,
        const uno::Reference< beans::XPropertySet > xProps,
        const uno::Reference< drawing::XControlShape > xControlShape )
{
    m_xEventListener.set( new ScVbaControlListener( this ) );
    uno::Reference< lang::XComponent > xComponent( m_xControlShape, uno::UNO_QUERY_THROW );
    m_xContext.set( xContext, uno::UNO_QUERY_THROW );
    m_xProps.set( xProps, uno::UNO_QUERY_THROW );
    m_xControlShape.set( xControlShape, uno::UNO_QUERY_THROW );
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
    uno::Reference< awt::XWindow2 > xWindow2( getWindowPeer( m_xControlShape ), uno::UNO_QUERY_THROW );
    return xWindow2->isVisible();
}

void SAL_CALL ScVbaControl::setVisible( sal_Bool bVisible ) throw (uno::RuntimeException)
{
    uno::Reference< awt::XWindow2 > xWindow2( getWindowPeer( m_xControlShape ), uno::UNO_QUERY_THROW );
    xWindow2->setVisible( bVisible );
}
double SAL_CALL ScVbaControl::getHeight() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    return mm2pt( xShape->getSize().Height ) / 100;
}
void SAL_CALL ScVbaControl::setHeight( double _height ) throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    awt::Size aSize( xShape->getSize() );
    aSize.Height = pt2mm( _height ) * 100;
    xShape->setSize( aSize );
}

double SAL_CALL ScVbaControl::getWidth() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    return mm2pt( xShape->getSize().Width ) / 100;
}
void SAL_CALL ScVbaControl::setWidth( double _width ) throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    awt::Size aSize( xShape->getSize() );
    aSize.Width = pt2mm( _width ) * 100;
    xShape->setSize( aSize );
}

double SAL_CALL
ScVbaControl::getLeft() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    return mm2pt( xShape->getPosition().X / 100 );
}

void SAL_CALL
ScVbaControl::setLeft( double _left ) throw (uno::RuntimeException)
{
    awt::Point oldPosition;
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    oldPosition = xShape->getPosition();
    oldPosition.X = pt2mm( _left ) * 100;
    xShape->setPosition( oldPosition );

}

double SAL_CALL
ScVbaControl::getTop() throw (uno::RuntimeException)
{
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    return mm2pt( xShape->getPosition().Y / 100 );
}

void SAL_CALL
ScVbaControl::setTop( double _top ) throw (uno::RuntimeException)
{
    awt::Point oldPosition;
    uno::Reference< drawing::XShape > xShape( m_xControlShape, uno::UNO_QUERY_THROW );
    oldPosition = xShape->getPosition();
    oldPosition.Y = pt2mm( _top ) * 100;;
    xShape->setPosition( oldPosition );
}

//ScVbaControlFactory

ScVbaControlFactory::ScVbaControlFactory( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< drawing::XControlShape >& xControlShape ): m_xContext( xContext ), m_xControlShape( xControlShape )
{
    uno::Reference< awt::XControlModel > xControlModel( xControlShape->getControl(), uno::UNO_QUERY_THROW );
    uno::Reference< beans::XPropertySet > xProps( xControlModel, uno::UNO_QUERY_THROW );
    m_xProps.set( xProps, uno::UNO_QUERY_THROW );
}

ScVbaControlFactory::ScVbaControlFactory( const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< beans::XPropertySet >& xProps, const uno::Reference< drawing::XControlShape > xControlShape ): m_xContext( xContext ), m_xProps( xProps ), m_xControlShape( xControlShape )
{
}

ScVbaControl* ScVbaControlFactory::createControl( const sal_Int16 nClassId )  throw (uno::RuntimeException)
{
    switch( nClassId )
    {
        case form::FormComponentType::COMBOBOX:
            return new ScVbaComboBox( m_xContext, m_xProps, m_xControlShape );
        case form::FormComponentType::COMMANDBUTTON:
            return new ScVbaButton( m_xContext, m_xControlShape );
        case form::FormComponentType::FIXEDTEXT:
            return new ScVbaLabel( m_xContext, m_xControlShape );
        case form::FormComponentType::TEXTFIELD:
            return new ScVbaTextBox( m_xContext, m_xControlShape );
        case form::FormComponentType::RADIOBUTTON:
            return new ScVbaRadioButton( m_xContext, m_xControlShape );
        case form::FormComponentType::LISTBOX:
            return new ScVbaListBox( m_xContext, m_xControlShape );
        default:
            throw uno::RuntimeException( rtl::OUString::createFromAscii(
                    "Donot surpport this Control Type." ), uno::Reference< uno::XInterface >() );
    }
}

ScVbaControl* ScVbaControlFactory::createControl()  throw (uno::RuntimeException)
{
    sal_Int32 nClassId = -1;
    const static rtl::OUString sClassId( RTL_CONSTASCII_USTRINGPARAM("ClassId") );
    m_xProps->getPropertyValue( sClassId ) >>= nClassId;
    switch( nClassId )
    {
        case form::FormComponentType::COMBOBOX:
            return new ScVbaComboBox( m_xContext, m_xControlShape );
        case form::FormComponentType::COMMANDBUTTON:
            return new ScVbaButton( m_xContext, m_xControlShape );
        case form::FormComponentType::FIXEDTEXT:
            return new ScVbaLabel( m_xContext, m_xControlShape );
        case form::FormComponentType::TEXTFIELD:
            return new ScVbaTextBox( m_xContext, m_xControlShape );
        case form::FormComponentType::RADIOBUTTON:
            return new ScVbaRadioButton( m_xContext, m_xControlShape );
        case form::FormComponentType::LISTBOX:
            return new ScVbaListBox( m_xContext, m_xControlShape );
        default:
            throw uno::RuntimeException( rtl::OUString::createFromAscii(
                    "Donot surpport this Control Type." ), uno::Reference< uno::XInterface >() );
    }
}
