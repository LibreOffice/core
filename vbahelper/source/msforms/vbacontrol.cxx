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
#include <memory>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XWindow2.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/view/XControlAccess.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/form/binding/XBindableValue.hpp>
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#include <com/sun/star/table/CellAddress.hpp>
#include <com/sun/star/table/CellRangeAddress.hpp>
#include <com/sun/star/script/XScriptListener.hpp>
#include <com/sun/star/document/XCodeNameQuery.hpp>
#include <com/sun/star/form/XChangeListener.hpp>
#include <ooo/vba/XControlProvider.hpp>
#include <ooo/vba/msforms/fmMousePointer.hpp>
#include <svtools/bindablecontrolhelper.hxx>
#include "service.hxx"
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
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/window.hxx>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/form/XFormsSupplier.hpp>
#include <svx/svdobj.hxx>
#include <cppuhelper/implbase.hxx>

using namespace com::sun::star;
using namespace ooo::vba;

uno::Reference< css::awt::XWindowPeer >
ScVbaControl::getWindowPeer()
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
    catch(const uno::Exception&)
    {
        throw uno::RuntimeException( "The Control does not exist" );
    }
    return xWinPeer;
}

//ScVbaControlListener
class ScVbaControlListener: public cppu::WeakImplHelper< lang::XEventListener >
{
private:
    ScVbaControl *pControl;
public:
    explicit ScVbaControlListener( ScVbaControl *pTmpControl );

    virtual void SAL_CALL disposing( const lang::EventObject& rEventObject ) override;
};

ScVbaControlListener::ScVbaControlListener( ScVbaControl *pTmpControl ): pControl( pTmpControl )
{
}

void SAL_CALL
ScVbaControlListener::disposing( const lang::EventObject& )
{
    if( pControl )
    {
        pControl->removeResource();
        pControl = nullptr;
    }
}

//ScVbaControl

ScVbaControl::ScVbaControl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< ::uno::XInterface >& xControl,  const css::uno::Reference< css::frame::XModel >& xModel, std::unique_ptr<ov::AbstractGeometryAttributes> pGeomHelper )
    : ControlImpl_BASE( xParent, xContext ), m_xControl( xControl ), m_xModel( xModel )
{
    //add listener
    m_xEventListener.set( new ScVbaControlListener( this ) );
    setGeometryHelper( std::move(pGeomHelper) );
    uno::Reference< lang::XComponent > xComponent( m_xControl, uno::UNO_QUERY_THROW );
    xComponent->addEventListener( m_xEventListener );

    //init m_xProps
    uno::Reference< drawing::XControlShape > xControlShape( m_xControl, uno::UNO_QUERY ) ;
    uno::Reference< awt::XControl> xUserFormControl( m_xControl, uno::UNO_QUERY ) ;
    if ( xControlShape.is() ) // form control
    {
        m_xProps.set( xControlShape->getControl(), uno::UNO_QUERY_THROW );
        OUString sDefaultControl;
        m_xProps->getPropertyValue( "DefaultControl" ) >>= sDefaultControl;
        uno::Reference< lang::XMultiComponentFactory > xMFac( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
        m_xEmptyFormControl.set( xMFac->createInstanceWithContext( sDefaultControl, mxContext ), uno::UNO_QUERY_THROW );
    }
    else if ( xUserFormControl.is() ) // userform control
    {
        m_xProps.set( xUserFormControl->getModel(), uno::UNO_QUERY_THROW );
    }
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
ScVbaControl::setGeometryHelper( std::unique_ptr<AbstractGeometryAttributes> pHelper )
{
    mpGeometryHelper = std::move( pHelper );
}

void ScVbaControl::removeResource()
{
    uno::Reference< lang::XComponent > xComponent( m_xControl, uno::UNO_QUERY_THROW );
    xComponent->removeEventListener( m_xEventListener );
    m_xControl= nullptr;
    m_xProps = nullptr;
}

//In design model has different behavior
sal_Bool SAL_CALL ScVbaControl::getEnabled()
{
    uno::Any aValue = m_xProps->getPropertyValue ( "Enabled" );
    bool bRet = false;
    aValue >>= bRet;
    return bRet;
}

void SAL_CALL ScVbaControl::setEnabled( sal_Bool bVisible )
{
    uno::Any aValue( bVisible );
    m_xProps->setPropertyValue(  "Enabled" , aValue);

}

sal_Bool SAL_CALL ScVbaControl::getVisible()
{
    bool bVisible( true );
    m_xProps->getPropertyValue ( "EnableVisible" ) >>= bVisible;
    uno::Reference< drawing::XControlShape > xControlShape( m_xControl, uno::UNO_QUERY );
    if ( xControlShape.is() )
    {
        bool bEnableVisible = bVisible;
        uno::Reference< beans::XPropertySet > xProps( m_xControl, uno::UNO_QUERY_THROW );
        xProps->getPropertyValue ( "Visible" ) >>= bVisible;
        bVisible = bVisible && bEnableVisible;
    }
    else
        m_xProps->getPropertyValue ( "EnableVisible" ) >>= bVisible;
    return bVisible;
}

void SAL_CALL ScVbaControl::setVisible( sal_Bool bVisible )
{
    uno::Any aValue( bVisible );
    m_xProps->setPropertyValue( "EnableVisible" , aValue);
    uno::Reference< drawing::XControlShape > xControlShape( m_xControl, uno::UNO_QUERY );
    if ( xControlShape.is() )
    {
        uno::Reference< beans::XPropertySet > xProps( m_xControl, uno::UNO_QUERY_THROW );
        xProps->setPropertyValue ( "Visible", aValue );
    }
}
double SAL_CALL ScVbaControl::getHeight()
{
    return mpGeometryHelper->getHeight();
}
void SAL_CALL ScVbaControl::setHeight( double _height )
{
    mpGeometryHelper->setHeight( _height );
}

double SAL_CALL ScVbaControl::getWidth()
{
    return mpGeometryHelper->getWidth();
}
void SAL_CALL ScVbaControl::setWidth( double _width )
{
    mpGeometryHelper->setWidth( _width );
}

double SAL_CALL
ScVbaControl::getLeft()
{
    return mpGeometryHelper->getLeft();
}

void SAL_CALL
ScVbaControl::setLeft( double _left )
{
    mpGeometryHelper->setLeft( _left );
}

double SAL_CALL
ScVbaControl::getTop()
{
    return mpGeometryHelper->getTop();
}

void SAL_CALL
ScVbaControl::setTop( double _top )
{
    mpGeometryHelper->setTop( _top );
}

uno::Reference< uno::XInterface > SAL_CALL
ScVbaControl::getObject()
{
    uno::Reference< msforms::XControl > xRet( this );
    return xRet;
}

void SAL_CALL ScVbaControl::SetFocus()
{
    uno::Reference< awt::XWindow > xWin( m_xControl, uno::UNO_QUERY_THROW );
    xWin->setFocus();
}

void SAL_CALL ScVbaControl::Move( double Left, double Top, const uno::Any& Width, const uno::Any& Height )
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

OUString SAL_CALL
ScVbaControl::getControlSource()
{
// #FIXME I *hate* having these upstream differences
// but this is necessary until I manage to upstream other
// dependent parts
    OUString sControlSource;
    uno::Reference< form::binding::XBindableValue > xBindable( m_xProps, uno::UNO_QUERY );
    if ( xBindable.is() )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFac( m_xModel, uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( "com.sun.star.table.CellAddressConversion" ), uno::UNO_QUERY );
            uno::Reference< beans::XPropertySet > xProps( xBindable->getValueBinding(), uno::UNO_QUERY_THROW );
            table::CellAddress aAddress;
            xProps->getPropertyValue( "BoundCell" ) >>= aAddress;
            xConvertor->setPropertyValue( "Address" , uno::makeAny( aAddress ) );
            xConvertor->getPropertyValue( "XLA1Representation" ) >>= sControlSource;
        }
        catch(const uno::Exception&)
        {
        }
    }
    return sControlSource;
}

void SAL_CALL
ScVbaControl::setControlSource( const OUString& _controlsource )
{
    // afaik this is only relevant for Excel documents ( and we need to set up a
    // reference tab in case no Sheet is specified in "_controlsource"
    // Can't use the active sheet either, code may of course access
    uno::Reference< drawing::XDrawPagesSupplier > xSupplier( m_xModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xIndex( xSupplier->getDrawPages(), uno::UNO_QUERY_THROW );
    sal_Int32 nLen = xIndex->getCount();
    bool bMatched = false;
    sal_Int16 nRefTab = 0;
    for ( sal_Int32 index = 0; index < nLen; ++index )
    {
        try
        {
            uno::Reference< form::XFormsSupplier >  xFormSupplier( xIndex->getByIndex( index ), uno::UNO_QUERY_THROW );
            uno::Reference< container::XIndexAccess > xFormIndex( xFormSupplier->getForms(), uno::UNO_QUERY_THROW );
            // get the www-standard container
            uno::Reference< container::XIndexAccess > xFormControls( xFormIndex->getByIndex(0), uno::UNO_QUERY_THROW );
            sal_Int32 nCntrls = xFormControls->getCount();
            for( sal_Int32 cIndex = 0; cIndex < nCntrls; ++cIndex )
            {
                uno::Reference< uno::XInterface > xControl( xFormControls->getByIndex( cIndex ), uno::UNO_QUERY_THROW );
                bMatched = ( m_xProps == xControl );
                if ( bMatched )
                {
                    nRefTab = index;
                    break;
                }
            }
        }
        catch( uno::Exception& ) {}
        if ( bMatched )
            break;
    }

    svt::BindableControlHelper::ApplyListSourceAndBindableData( m_xModel, m_xProps, _controlsource, "", sal_uInt16( nRefTab ) );
}

OUString SAL_CALL
ScVbaControl::getRowSource()
{
    OUString sRowSource;
    uno::Reference< form::binding::XListEntrySink > xListSink( m_xProps, uno::UNO_QUERY );
    if ( xListSink.is() )
    {
        try
        {
            uno::Reference< lang::XMultiServiceFactory > xFac( m_xModel, uno::UNO_QUERY_THROW );
            uno::Reference< beans::XPropertySet > xConvertor( xFac->createInstance( "com.sun.star.table.CellRangeAddressConversion" ), uno::UNO_QUERY );

            uno::Reference< beans::XPropertySet > xProps( xListSink->getListEntrySource(), uno::UNO_QUERY_THROW );
            table::CellRangeAddress aAddress;
            xProps->getPropertyValue( "CellRange" ) >>= aAddress;
            xConvertor->setPropertyValue( "Address" , uno::makeAny( aAddress ) );
            xConvertor->getPropertyValue( "XLA1Representation" ) >>= sRowSource;
        }
        catch(const uno::Exception&)
        {
        }
    }
    return sRowSource;
}

void SAL_CALL
ScVbaControl::setRowSource( const OUString& _rowsource )
{
    svt::BindableControlHelper::ApplyListSourceAndBindableData( m_xModel, m_xProps, "", _rowsource );
}

OUString SAL_CALL
ScVbaControl::getName()
{
    OUString sName;
    m_xProps->getPropertyValue( "Name" ) >>= sName;
    return sName;

}

void SAL_CALL
ScVbaControl::setName( const OUString& _name )
{
    m_xProps->setPropertyValue( "Name" , uno::makeAny( _name ) );
    }

OUString SAL_CALL
ScVbaControl::getControlTipText()
{
    OUString sName;
    m_xProps->getPropertyValue( "HelpText" ) >>= sName;
    return sName;
}

void SAL_CALL
ScVbaControl::setControlTipText( const OUString& rsToolTip )
{
    m_xProps->setPropertyValue( "HelpText" , uno::makeAny( rsToolTip ) );
}

OUString SAL_CALL ScVbaControl::getTag()
{
    return m_aControlTag;
}

void SAL_CALL ScVbaControl::setTag( const OUString& aTag )
{
    m_aControlTag = aTag;
}

::sal_Int32 SAL_CALL ScVbaControl::getForeColor()
{
    Color nForeColor;
    m_xProps->getPropertyValue( "TextColor" ) >>= nForeColor;
    return OORGBToXLRGB( nForeColor );
}


struct PointerStyles
{
   long const msoPointerStyle;
   PointerStyle const loPointStyle;
};

// 1 -> 1 map of styles ( some dubious choices in there though )
PointerStyles const styles[] = {
  /// assuming pointer default is Arrow
  { msforms::fmMousePointer::fmMousePointerDefault, PointerStyle::Arrow },
  { msforms::fmMousePointer::fmMousePointerArrow, PointerStyle::Arrow },
  { msforms::fmMousePointer::fmMousePointerCross, PointerStyle::Cross },
  { msforms::fmMousePointer::fmMousePointerIBeam, PointerStyle::Text },
  { msforms::fmMousePointer::fmMousePointerSizeNESW,  PointerStyle::AutoScrollNSWE   }, // #TODO not correct, need to check, need to find the right one
  { msforms::fmMousePointer::fmMousePointerSizeNS,  PointerStyle::AutoScrollNS  },
  { msforms::fmMousePointer::fmMousePointerSizeNWSE,  PointerStyle::AutoScrollNSWE  }, // #TODO not correct, need to check, need to find the right one
  { msforms::fmMousePointer::fmMousePointerSizeWE,  PointerStyle::AutoScrollWE },
  { msforms::fmMousePointer::fmMousePointerUpArrow, PointerStyle::WindowNSize  },
  { msforms::fmMousePointer::fmMousePointerHourGlass, PointerStyle::Wait  },
  { msforms::fmMousePointer::fmMousePointerNoDrop, PointerStyle::NotAllowed },
  { msforms::fmMousePointer::fmMousePointerAppStarting, PointerStyle::Wait },
  { msforms::fmMousePointer::fmMousePointerHelp, PointerStyle::Help },
  { msforms::fmMousePointer::fmMousePointerSizeAll, PointerStyle::Cross },
  { msforms::fmMousePointer::fmMousePointerCustom, PointerStyle::Arrow }, // not supported I guess

};

static long lcl_loPointerToMsoPointer( PointerStyle eType )
{
    long nRet = msforms::fmMousePointer::fmMousePointerDefault;
    for ( int i = 0; i < int(SAL_N_ELEMENTS( styles )); ++i )
    {
        if ( styles[ i ].loPointStyle == eType )
        {
            nRet = styles[ i ].msoPointerStyle;
            break;
        }
    }
    return nRet;
}

static PointerStyle lcl_msoPointerToLOPointer( long msoPointerStyle )
{
    PointerStyle aPointer( PointerStyle::Arrow );
    for ( int i = 0; i < int(SAL_N_ELEMENTS( styles )); ++i )
    {
        if ( styles[ i ].msoPointerStyle == msoPointerStyle )
        {
            aPointer = styles[ i ].loPointStyle;
            break;
         }
    }
    return aPointer;
}

::sal_Int32 SAL_CALL
ScVbaControl::getMousePointer()
{
    PointerStyle eType = PointerStyle::Arrow; // default ?
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( getWindowPeer() );
    if ( pWindow )
    {
        eType = pWindow->GetPointer();
    }
    return lcl_loPointerToMsoPointer( eType );
}

void SAL_CALL
ScVbaControl::setMousePointer( ::sal_Int32 _mousepointer )
{
    VclPtr<vcl::Window> pWindow = VCLUnoHelper::GetWindow( getWindowPeer() );
    if ( pWindow )
    {
        PointerStyle aPointer = lcl_msoPointerToLOPointer( _mousepointer );
        pWindow->SetPointer( aPointer );
    }
}

void SAL_CALL ScVbaControl::fireEvent( const script::ScriptEvent& rEvt )
{
    script::ScriptEvent evt( rEvt );
    uno::Reference<lang::XMultiComponentFactory > xServiceManager( mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< script::XScriptListener > xScriptListener( xServiceManager->createInstanceWithContext( "ooo.vba.EventListener" , mxContext ), uno::UNO_QUERY_THROW );

    uno::Reference< beans::XPropertySet > xProps( xScriptListener, uno::UNO_QUERY_THROW );
    xProps->setPropertyValue( "Model" , uno::makeAny( m_xModel ) );

    // handling for sheet control
    uno::Reference< msforms::XControl > xThisControl( this );
    try
    {
        evt.Arguments.realloc( 1 );
        lang::EventObject aEvt;

        uno::Reference< drawing::XControlShape > xControlShape( m_xControl, uno::UNO_QUERY ) ;
        uno::Reference< awt::XControl > xControl( m_xControl, uno::UNO_QUERY ) ;

        if ( xControlShape.is() )
        {
            evt.Source = xControlShape;
            aEvt.Source = m_xEmptyFormControl;
            // Set up proper scriptcode
            uno::Reference< lang::XMultiServiceFactory > xDocFac(  m_xModel, uno::UNO_QUERY_THROW );
            uno::Reference< document::XCodeNameQuery > xNameQuery(  xDocFac->createInstance( "ooo.vba.VBACodeNameProvider" ), uno::UNO_QUERY_THROW );
            uno::Reference< uno::XInterface > xIf( xControlShape->getControl(), uno::UNO_QUERY_THROW );
            evt.ScriptCode = xNameQuery->getCodeNameForObject( xIf );
            // handle if we passed in our own arguments
            if ( !rEvt.Arguments.getLength() )
                evt.Arguments[ 0 ] <<= aEvt;
            xScriptListener->firing( evt );
        }
        else
        {
            if ( xControl.is() ) // normal control ( from dialog/userform )
            {
                // #FIXME We should probably store a reference to the
                // parent dialog/userform here ( other wise the name of
                // dialog could be changed and we won't be aware of it.
                // ( OTOH this is probably an unlikely scenario )
                evt.Source = xThisControl;
                aEvt.Source = xControl;
                evt.ScriptCode = m_sLibraryAndCodeName;
                evt.Arguments[ 0 ] <<= aEvt;
                xScriptListener->firing( evt );
            }
        }
    }
    catch(const uno::Exception&)
    {
    }
}
void ScVbaControl::fireChangeEvent()
{
    script::ScriptEvent evt;
    evt.ScriptType = "VBAInterop";
    evt.ListenerType = cppu::UnoType<form::XChangeListener>::get();
    evt.MethodName = "changed";
    fireEvent( evt );
}

void ScVbaControl::fireClickEvent()
{
    script::ScriptEvent evt;
    evt.ScriptType = "VBAInterop";
    evt.ListenerType = cppu::UnoType<awt::XActionListener>::get();
    evt.MethodName = "actionPerformed";
    fireEvent( evt );
}

sal_Int32 SAL_CALL ScVbaControl::getTabIndex()
{
    return 1;
}

void SAL_CALL ScVbaControl::setTabIndex( sal_Int32 /*nTabIndex*/ )
{
}

//ScVbaControlFactory

/*static*/ uno::Reference< msforms::XControl > ScVbaControlFactory::createShapeControl(
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< drawing::XControlShape >& xControlShape,
        const uno::Reference< frame::XModel >& xModel )
{
    uno::Reference< beans::XPropertySet > xProps( xControlShape->getControl(), uno::UNO_QUERY_THROW );
    sal_Int32 nClassId = -1;
    xProps->getPropertyValue( "ClassId" ) >>= nClassId;
    uno::Reference< XHelperInterface > xVbaParent; // #FIXME - should be worksheet I guess
    uno::Reference< drawing::XShape > xShape( xControlShape, uno::UNO_QUERY_THROW );
    ::std::unique_ptr< ConcreteXShapeGeometryAttributes > xGeoHelper( new ConcreteXShapeGeometryAttributes( xShape ) );
    switch( nClassId )
    {
        case form::FormComponentType::COMBOBOX:
            return new ScVbaComboBox( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
        case form::FormComponentType::COMMANDBUTTON:
        {
            bool bToggle = false;
            xProps->getPropertyValue( "Toggle" ) >>= bToggle;
            if ( bToggle )
                return new ScVbaToggleButton( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
            else
                return new VbaButton( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
        }
        case form::FormComponentType::FIXEDTEXT:
            return new ScVbaLabel( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
        case form::FormComponentType::TEXTFIELD:
            return new ScVbaTextBox( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
        case form::FormComponentType::CHECKBOX:
            return new ScVbaCheckbox( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
        case form::FormComponentType::RADIOBUTTON:
            return new ScVbaRadioButton( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
        case form::FormComponentType::LISTBOX:
            return new ScVbaListBox( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
        case form::FormComponentType::SPINBUTTON:
            return new ScVbaSpinButton( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
        case form::FormComponentType::IMAGECONTROL:
            return new ScVbaImage( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
        case form::FormComponentType::SCROLLBAR:
            return new ScVbaScrollBar( xVbaParent, xContext, xControlShape, xModel, std::move(xGeoHelper) );
    }
    throw uno::RuntimeException( "Unsupported control." );
}

/*static*/ uno::Reference< msforms::XControl > ScVbaControlFactory::createUserformControl(
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< awt::XControl >& xControl,
        const uno::Reference< awt::XControl >& xDialog,
        const uno::Reference< frame::XModel >& xModel,
        double fOffsetX, double fOffsetY )
{
    uno::Reference< beans::XPropertySet > xProps( xControl->getModel(), uno::UNO_QUERY_THROW );
    uno::Reference< lang::XServiceInfo > xServiceInfo( xProps, uno::UNO_QUERY_THROW );
    uno::Reference< msforms::XControl > xVBAControl;
    uno::Reference< XHelperInterface > xVbaParent; // #FIXME - should be worksheet I guess
    ::std::unique_ptr< UserFormGeometryHelper > xGeoHelper( new UserFormGeometryHelper( xControl, fOffsetX, fOffsetY ) );

    if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlCheckBoxModel" ) )
        xVBAControl.set( new ScVbaCheckbox( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" ) )
        xVBAControl.set( new ScVbaRadioButton( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlEditModel" ) )
        xVBAControl.set( new ScVbaTextBox( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper), true ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlButtonModel" ) )
    {
        bool bToggle = false;
        xProps->getPropertyValue( "Toggle" ) >>= bToggle;
        if ( bToggle )
            xVBAControl.set( new ScVbaToggleButton( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
        else
            xVBAControl.set( new VbaButton( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    }
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlComboBoxModel" ) )
        xVBAControl.set( new ScVbaComboBox( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlListBoxModel" ) )
        xVBAControl.set( new ScVbaListBox( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlFixedTextModel" ) )
        xVBAControl.set( new ScVbaLabel( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlImageControlModel" ) )
        xVBAControl.set( new ScVbaImage( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlProgressBarModel" ) )
        xVBAControl.set( new ScVbaProgressBar( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlGroupBoxModel" ) )
        xVBAControl.set( new ScVbaFrame( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper), xDialog ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlScrollBarModel" ) )
        xVBAControl.set( new ScVbaScrollBar( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoMultiPageModel" ) )
        xVBAControl.set( new ScVbaMultiPage( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoControlSpinButtonModel" ) )
        xVBAControl.set( new ScVbaSpinButton( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.custom.awt.UnoControlSystemAXContainerModel" ) )
        xVBAControl.set( new VbaSystemAXControl( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    // #FIXME implement a page control
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoPageModel" ) )
        xVBAControl.set( new ScVbaControl( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper) ) );
    else if ( xServiceInfo->supportsService( "com.sun.star.awt.UnoFrameModel" ) )
        xVBAControl.set( new ScVbaFrame( xVbaParent, xContext, xControl, xModel, std::move(xGeoHelper), xDialog ) );
    if( xVBAControl.is() )
        return xVBAControl;
    throw uno::RuntimeException( "Unsupported control." );
}

OUString
ScVbaControl::getServiceImplName()
{
    return OUString("ScVbaControl");
}

uno::Sequence< OUString >
ScVbaControl::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.Control"
    };
    return aServiceNames;
}

sal_Int32 const nSysCols[] = { 0xC8D0D4, 0x0, 0x6A240A, 0x808080, 0xE4E4E4, 0xFFFFFF, 0x0, 0x0, 0x0, 0xFFFFFF, 0xE4E4E4, 0xE4E4E4, 0x808080, 0x6A240A, 0xFFFFFF, 0xE4E4E4, 0x808080, 0x808080, 0x0, 0xC8D0D4, 0xFFFFFF, 0x404040, 0xE4E4E4, 0x0, 0xE1FFFF };

sal_Int32 ScVbaControl::getBackColor()
{
    sal_Int32 nBackColor = 0;
    m_xProps->getPropertyValue( "BackgroundColor" ) >>= nBackColor;
    return nBackColor;
}

void ScVbaControl::setBackColor( sal_Int32 nBackColor )
{
    if ( ( static_cast<sal_uInt32>(nBackColor) >= sal_uInt32(0x80000000) ) &&
         ( static_cast<sal_uInt32>(nBackColor) <= sal_uInt32(0x80000000) + SAL_N_ELEMENTS(nSysCols) ) )
    {
        nBackColor = nSysCols[ nBackColor & 0x0FF];
    }
    m_xProps->setPropertyValue( "BackgroundColor" , uno::makeAny( XLRGBToOORGB( nBackColor ) ) );
}

bool ScVbaControl::getAutoSize()
{
    bool bIsResizeEnabled = false;
    uno::Reference< uno::XInterface > xIf( m_xControl, uno::UNO_QUERY_THROW );
    SdrObject* pObj = SdrObject::getSdrObjectFromXShape( xIf );
    if ( pObj )
        bIsResizeEnabled = !pObj->IsResizeProtect();
    return bIsResizeEnabled;
}

// currently no implementation for this
void ScVbaControl::setAutoSize( bool bAutoSize )
{
    uno::Reference< uno::XInterface > xIf( m_xControl, uno::UNO_QUERY_THROW );
    SdrObject* pObj = SdrObject::getSdrObjectFromXShape( xIf );
    if ( pObj )
        pObj->SetResizeProtect( !bAutoSize );
}

bool ScVbaControl::getLocked()
{
    bool bRes( false );
    m_xProps->getPropertyValue( "ReadOnly" ) >>= bRes;
    return bRes;
}

void ScVbaControl::setLocked( bool bLocked )
{
    m_xProps->setPropertyValue( "ReadOnly" , uno::makeAny( bLocked ) );
}

class ControlProviderImpl : public cppu::WeakImplHelper< XControlProvider >
{
    uno::Reference< uno::XComponentContext > m_xCtx;
public:
    explicit ControlProviderImpl( const uno::Reference< uno::XComponentContext >& xCtx ) : m_xCtx( xCtx ) {}
    virtual uno::Reference< msforms::XControl > SAL_CALL createControl( const uno::Reference< drawing::XControlShape >& xControl, const uno::Reference< frame::XModel >& xDocOwner ) override;
};

uno::Reference< msforms::XControl > SAL_CALL
ControlProviderImpl::createControl( const uno::Reference< drawing::XControlShape >& xControlShape, const uno::Reference< frame::XModel >& xDocOwner )
{
    uno::Reference< msforms::XControl > xControlToReturn;
    if ( xControlShape.is() )
        xControlToReturn = ScVbaControlFactory::createShapeControl( m_xCtx, xControlShape, xDocOwner );
    return xControlToReturn;

}

namespace controlprovider
{
namespace sdecl = comphelper::service_decl;
sdecl::class_<ControlProviderImpl, sdecl::with_args<false> > const serviceImpl;
sdecl::ServiceDecl const serviceDecl(
    serviceImpl,
    "ControlProviderImpl",
    "ooo.vba.ControlProvider" );
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
