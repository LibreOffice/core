/*************************************************************************
 *
 *  $RCSfile: unocontrols.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: mt $ $Date: 2001-04-04 16:06:33 $
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

#ifndef _COM_SUN_STAR_AWT_XTEXTAREA_HPP_
#include <com/sun/star/awt/XTextArea.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif


#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <toolkit/controls/stdtabcontroller.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/helper/unomemorystream.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/macros.hxx>

#include <cppuhelper/typeprovider.hxx>

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include <vcl/wrkwin.hxx>
#include <vcl/svapp.hxx>

#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>

#include <tools/debug.hxx>

using namespace ::com::sun::star;


//  ----------------------------------------------------
//  class UnoControlHolder
//  ----------------------------------------------------
struct UnoControlModelHolder
{
public:
    uno::Reference< awt::XControlModel > xModel;
    ::rtl::OUString aName;

    UnoControlModelHolder( const ::rtl::OUString& rName, const uno::Reference< awt::XControlModel > & rModel )
        : aName( rName )
    {
        xModel = rModel;
    }
};

DECLARE_LIST( UnoControlModelHolderList, UnoControlModelHolder* );


//  ----------------------------------------------------
//  class UnoControlDialogModel
//  ----------------------------------------------------
UnoControlDialogModel::UnoControlDialogModel()
    : maContainerListeners( *this )
{
    mpModels = new UnoControlModelHolderList;

    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
//  ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_TITLE );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_SIZEABLE );

    uno::Any aBool;
    aBool <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
}

UnoControlDialogModel::UnoControlDialogModel( const UnoControlDialogModel& rModel ) :
    UnoControlModel( rModel ) , maContainerListeners( *this )
{
    mpModels = new UnoControlModelHolderList;
}

UnoControlDialogModel::~UnoControlDialogModel()
{
    for ( sal_uInt32 n = mpModels->Count(); n; )
        delete mpModels->GetObject( --n );
    delete mpModels;
}

uno::Any UnoControlDialogModel::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( lang::XMultiServiceFactory*, this ),
                                        SAL_STATIC_CAST( container::XContainer*, this ),
                                        SAL_STATIC_CAST( container::XElementAccess*, this ),
                                        SAL_STATIC_CAST( container::XNameAccess*, this ),
                                        SAL_STATIC_CAST( container::XNameReplace*, this ),
                                        SAL_STATIC_CAST( container::XNameContainer*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlModel::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoControlDialogModel )
getCppuType( ( uno::Reference< lang::XMultiServiceFactory>* ) NULL ),
getCppuType( ( uno::Reference< container::XContainer>* ) NULL ),
getCppuType( ( uno::Reference< container::XElementAccess>* ) NULL ),
getCppuType( ( uno::Reference< container::XNameAccess>* ) NULL ),
getCppuType( ( uno::Reference< container::XNameReplace>* ) NULL ),
getCppuType( ( uno::Reference< container::XNameContainer>* ) NULL ),
UnoControlModel::getTypes()
IMPL_XTYPEPROVIDER_END


::rtl::OUString UnoControlDialogModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlDialogModel );
}

uno::Any UnoControlDialogModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlDialog );
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlDialogModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlDialogModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

UnoControlModel* UnoControlDialogModel::Clone() const
{
    UnoControlDialogModel* pClone = new UnoControlDialogModel( *this );

    for ( sal_uInt32 n = 0; n < mpModels->Count(); n++ )
    {
        UnoControlModelHolder* pH = mpModels->GetObject( n );

        uno::Reference< util::XCloneable > xC( pH->xModel, uno::UNO_QUERY );
        uno::Reference< util::XCloneable > xNew = xC->createClone();
        uno::Reference< awt::XControlModel > xM( xNew, uno::UNO_QUERY );
        UnoControlModelHolder* pNew = new UnoControlModelHolder( pH->aName, xM );
        pClone->mpModels->Insert( pNew, LIST_APPEND );
    }

    return pClone;
}

UnoControlModelHolder* UnoControlDialogModel::ImplFindElement( const ::rtl::OUString& rName ) const
{
    UnoControlModelHolder* pE = NULL;
    for ( sal_uInt32 n = mpModels->Count(); n && !pE; )
    {
        UnoControlModelHolder* pH = mpModels->GetObject( --n );
        if ( pH->aName == rName )
            pE = pH;
    }
    return pE;
}

// ::lang::XMultiServiceFactory
uno::Reference< uno::XInterface >UnoControlDialogModel::createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(uno::Exception, uno::RuntimeException)
{
    OGeometryControlModel_Base* pNewModel = NULL;

    if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlEditModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlEditModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlFormattedFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFormattedFieldModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlFileControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFileControlModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlButtonModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlButtonModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlImageControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlImageControlModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlRadioButtonModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlRadioButtonModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlCheckBoxModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlCheckBoxModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlFixedTextModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFixedTextModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlGroupBoxModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlGroupBoxModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlListBoxModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlListBoxModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlComboBoxModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlComboBoxModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlDateFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlDateFieldModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlTimeFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlTimeFieldModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlNumericFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlNumericFieldModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlCurrencyFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlCurrencyFieldModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlPatternFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlPatternFieldModel >;

    uno::Reference< uno::XInterface > xNewModel = (::cppu::OWeakObject*)pNewModel;
    return xNewModel;
}

uno::Reference< uno::XInterface > UnoControlDialogModel::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const uno::Sequence< uno::Any >& /* Arguments */ ) throw(uno::Exception, uno::RuntimeException)
{
    return createInstance( ServiceSpecifier );
}

uno::Sequence< ::rtl::OUString > UnoControlDialogModel::getAvailableServiceNames() throw(uno::RuntimeException)
{
    static uno::Sequence< ::rtl::OUString >* pNamesSeq = NULL;
    if ( !pNamesSeq )
    {
        pNamesSeq = new uno::Sequence< ::rtl::OUString >( 16 );
        ::rtl::OUString* pNames = pNamesSeq->getArray();
        pNames[0] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlEditModel );
        pNames[1] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFormattedFieldModel );
        pNames[2] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFileControlModel );
        pNames[3] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlButtonModel );
        pNames[4] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlImageControlModel );
        pNames[5] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlRadioButtonModel );
        pNames[6] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCheckBoxModel );
        pNames[7] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFixedTextModel );
        pNames[8] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlGroupBoxModel );
        pNames[9] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlListBoxModel );
        pNames[10] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlComboBoxModel );
        pNames[11] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlDateFieldModel );
        pNames[12] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlTimeFieldModel );
        pNames[13] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlNumericFieldModel );
        pNames[14] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlCurrencyFieldModel );
        pNames[15] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlPatternFieldModel );
    }
    return *pNamesSeq;
}

// container::XContainer
void UnoControlDialogModel::addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException)
{
    maContainerListeners.addInterface( l );
}

void UnoControlDialogModel::removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l ) throw(::com::sun::star::uno::RuntimeException)
{
    maContainerListeners.removeInterface( l );
}

// container::XElementAcces
uno::Type UnoControlDialogModel::getElementType() throw(uno::RuntimeException)
{
    uno::Type aType = getCppuType( ( uno::Reference< awt::XControlModel>* ) NULL );
    return aType;
}

sal_Bool UnoControlDialogModel::hasElements() throw(uno::RuntimeException)
{
    return mpModels->Count() ? sal_True : sal_False;
}

// container::XNameContainer, XNameReplace, XNameAccess
void UnoControlDialogModel::replaceByName( const ::rtl::OUString& aName, const uno::Any& aElement ) throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    UnoControlModelHolder* pE = ImplFindElement( aName );
    if ( pE )
    {
        container::ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Element <<= aElement;
        aEvent.ReplacedElement <<= pE->xModel;
        aEvent.Accessor <<= aName;

        aElement >>= pE->xModel;

        maContainerListeners.elementReplaced( aEvent );
    }
}

uno::Any UnoControlDialogModel::getByName( const ::rtl::OUString& aName ) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Any aElement;
    UnoControlModelHolder* pE = ImplFindElement( aName );
    if ( pE )
        aElement <<= pE->xModel;
    return aElement;
}

uno::Sequence< ::rtl::OUString > UnoControlDialogModel::getElementNames() throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aNames( mpModels->Count() );
    ::rtl::OUString* pNames = aNames.getArray();

    for ( sal_uInt32 n = 0; n < mpModels->Count(); n++ )
    {
        UnoControlModelHolder* pH = mpModels->GetObject( n );
        pNames[n] = pH->aName;
    }
    return aNames;
}

sal_Bool UnoControlDialogModel::hasByName( const ::rtl::OUString& aName ) throw(uno::RuntimeException)
{
    return ImplFindElement( aName ) ? sal_True : sal_False;
}

void UnoControlDialogModel::insertByName( const ::rtl::OUString& aName, const uno::Any& aElement ) throw(lang::IllegalArgumentException, container::ElementExistException, lang::WrappedTargetException, uno::RuntimeException)
{
    uno::Reference< awt::XControlModel > xM;
    aElement >>= xM;
    UnoControlModelHolder* pNew = new UnoControlModelHolder( aName, xM );
    mpModels->Insert( pNew, LIST_APPEND );

    container::ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element <<= aElement;
    aEvent.Accessor <<= aName;
    maContainerListeners.elementInserted( aEvent );
}

void UnoControlDialogModel::removeByName( const ::rtl::OUString& aName ) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
{
    UnoControlModelHolder* pE = ImplFindElement( aName );
    if ( pE )
    {
        container::ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Element <<= pE->xModel;
        aEvent.Accessor <<= aName;
        maContainerListeners.elementRemoved( aEvent );

        mpModels->Remove( pE );
        delete pE;
    }
}

//  ----------------------------------------------------
//  class UnoDialogControl
//  ----------------------------------------------------
UnoDialogControl::UnoDialogControl()
{
    maComponentInfos.nWidth = 300;
    maComponentInfos.nHeight = 450;
}

::rtl::OUString UnoDialogControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "Dialog" );
}

// uno::XInterface
uno::Any UnoDialogControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XDialog*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlContainer::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoDialogControl )
getCppuType( ( uno::Reference< awt::XDialog>* ) NULL ),
UnoControlContainer::getTypes()
IMPL_XTYPEPROVIDER_END


void UnoDialogControl::ImplInsertControl( uno::Reference< awt::XControlModel >& rxModel, const ::rtl::OUString& rName )
{
    uno::Reference< beans::XMultiPropertySet > xMP( rxModel, uno::UNO_QUERY );
    if ( xMP.is() )
    {
        uno::Sequence< ::rtl::OUString > aNames( 4 );
        ::rtl::OUString* pNames = aNames.getArray();
        pNames[0] = ::rtl::OUString::createFromAscii( "PositionX" );
        pNames[1] = ::rtl::OUString::createFromAscii( "PositionY" );
        pNames[2] = ::rtl::OUString::createFromAscii( "Width" );
        pNames[3] = ::rtl::OUString::createFromAscii( "Height" );

        xMP->addPropertiesChangeListener( aNames, (beans::XPropertiesChangeListener*)this );
    }

    uno::Reference< beans::XPropertySet > xP( rxModel, uno::UNO_QUERY );

    ::rtl::OUString aDefCtrl;
    xP->getPropertyValue( GetPropertyName( BASEPROPERTY_DEFAULTCONTROL ) ) >>= aDefCtrl;
    uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    uno::Reference < awt::XControl > xCtrl( xMSF->createInstance( aDefCtrl ), uno::UNO_QUERY );

    xCtrl->setModel( rxModel );
    addControl( rName, xCtrl );

    ImplSetPosSize( xCtrl );
}

void UnoDialogControl::ImplRemoveControl( uno::Reference< awt::XControlModel >& rxModel )
{
    uno::Reference< beans::XMultiPropertySet > xMP( rxModel, uno::UNO_QUERY );
    if ( xMP.is() )
        xMP->removePropertiesChangeListener( (beans::XPropertiesChangeListener*)this );

    uno::Sequence< uno::Reference< awt::XControl > > aControls = getControls();
    uno::Reference< awt::XControl > xCtrl = StdTabController::FindControl( aControls, rxModel );
    if ( xCtrl.is() )
        removeControl( xCtrl );
}

void UnoDialogControl::ImplSetPosSize( uno::Reference< awt::XControl >& rxCtrl )
{
    uno::Reference< beans::XPropertySet > xP( rxCtrl->getModel(), uno::UNO_QUERY );

    sal_Int32 nX, nY, nWidth, nHeight;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ) ) >>= nX;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ) ) >>= nY;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nWidth;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nHeight;

    // Currentley we are simply using MAP_APPFONT
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    DBG_ASSERT( pOutDev, "Missing Default Device!" );
    if ( pOutDev )
    {
        Size aTmp( nX, nY );
        aTmp = pOutDev->LogicToPixel( aTmp, MAP_APPFONT );
        nX = aTmp.Width();
        nY = aTmp.Height();
        aTmp = Size( nWidth, nHeight );
        aTmp = pOutDev->LogicToPixel( aTmp, MAP_APPFONT );
        nWidth = aTmp.Width();
        nHeight = aTmp.Height();
    }
    else
    {
        uno::Reference< awt::XWindowPeer > xPeer = ImplGetCompatiblePeer( sal_True );
        uno::Reference< awt::XDevice > xD( xPeer, uno::UNO_QUERY );

        awt::SimpleFontMetric aFM;
        awt::FontDescriptor aFD;
        uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_FONTDESCRIPTOR ) );
        aVal >>= aFD;
        if ( aFD.StyleName.getLength() )
        {
            uno::Reference< awt::XFont > xFont = xD->getFont( aFD );
            aFM = xFont->getFontMetric();
        }
        else
        {
            uno::Reference< awt::XGraphics > xG = xD->createGraphics();
            aFM = xG->getFontMetric();
        }

        sal_Int16 nH = aFM.Ascent + aFM.Descent;
        sal_Int16 nW = nH/2;    // calculate avarage width?!

        nX *= nW;
        nX /= 4;
        nWidth *= nW;
        nWidth /= 4;
        nY *= nH;
        nY /= 8;
        nHeight *= nH;
        nHeight /= 8;
    }
    uno::Reference < awt::XWindow > xW( rxCtrl, uno::UNO_QUERY );
    xW->setPosSize( nX, nY, nWidth, nHeight, awt::PosSize::POSSIZE );
}

void UnoDialogControl::dispose() throw(uno::RuntimeException)
{
    UnoControlContainer::dispose();
}

sal_Bool UnoDialogControl::setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxModel ) throw(::com::sun::star::uno::RuntimeException)
{
    if ( getModel().is() )
    {
        uno::Sequence< uno::Reference< awt::XControl > > aControls = getControls();
        const uno::Reference< awt::XControl > * pCtrls = aControls.getConstArray();
        sal_Int32 nCtrls = aControls.getLength();
        for ( sal_Int32 n = 0; n < nCtrls; n++ )
            removeControl( pCtrls[n] );

        uno::Reference< container::XContainer > xC( getModel(), uno::UNO_QUERY );
        if ( xC.is() )
            xC->removeContainerListener( (container::XContainerListener*)this );
    }

    sal_Bool bRet = UnoControl::setModel( rxModel );

    if ( getModel().is() )
    {
        uno::Reference< container::XNameAccess > xNA( getModel(), uno::UNO_QUERY );
        if ( xNA.is() )
        {
            uno::Sequence< ::rtl::OUString > aNames = xNA->getElementNames();
            const ::rtl::OUString* pNames = aNames.getConstArray();
            sal_uInt32 nCtrls = aNames.getLength();

            for( sal_uInt32 n = 0; n < nCtrls; n++ )
            {
                uno::Any aA = xNA->getByName( pNames[n] );
                uno::Reference< awt::XControlModel > xCtrlModel;
                aA >>= xCtrlModel;
                ImplInsertControl( xCtrlModel, pNames[n] );
            }
        }

        uno::Reference< container::XContainer > xC( getModel(), uno::UNO_QUERY );
        if ( xC.is() )
            xC->addContainerListener( (container::XContainerListener*)this );
    }
    return bRet;
}

void UnoDialogControl::setDesignMode( sal_Bool bOn ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    UnoControl::setDesignMode( bOn );

    uno::Sequence< uno::Reference< awt::XControl > > xCtrls = getControls();
    sal_Int32 nControls = xCtrls.getLength();
    uno::Reference< awt::XControl >* pControls = xCtrls.getArray();
    for ( sal_Int32 n = 0; n < nControls; n++ )
        pControls[n]->setDesignMode( bOn );
}

void UnoDialogControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControlContainer::createPeer( rxToolkit, rParentPeer );
}

void UnoDialogControl::elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference< awt::XControlModel > xModel;
    ::rtl::OUString aName;

    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ImplInsertControl( xModel, aName );
}

void UnoDialogControl::elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference< awt::XControlModel > xModel;
    Event.Element >>= xModel;
    if ( xModel.is() )
        ImplRemoveControl( xModel );
}

void UnoDialogControl::elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException)
{
    uno::Reference< awt::XControlModel > xModel;
    Event.ReplacedElement >>= xModel;
    if ( xModel.is() )
        ImplRemoveControl( xModel );

    ::rtl::OUString aName;
    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ImplInsertControl( xModel, aName );
}

// beans::XPropertiesChangeListener
void UnoDialogControl::propertiesChange( const uno::Sequence< beans::PropertyChangeEvent >& rEvents ) throw(::com::sun::star::uno::RuntimeException)
{
    if( !isDesignMode() && !IsUpdatingModel() && !mbCreatingCompatiblePeer )
    {
        ::rtl::OUString s1( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) );
        ::rtl::OUString s2( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) );
        ::rtl::OUString s3( RTL_CONSTASCII_USTRINGPARAM( "Width" ) );
        ::rtl::OUString s4( RTL_CONSTASCII_USTRINGPARAM( "Height" ) );

        sal_Int32 nLen = rEvents.getLength();
        for( sal_Int32 i = 0; i < nLen; i++ )
        {
            const beans::PropertyChangeEvent& rEvt = rEvents.getConstArray()[i];
            uno::Reference< awt::XControlModel > xModel( rEvt.Source, uno::UNO_QUERY );
            sal_Bool bOwnModel = (awt::XControlModel*)xModel.get() == (awt::XControlModel*)getModel().get();
            if ( ( rEvt.PropertyName == s1 ) || ( rEvt.PropertyName == s2 ) || ( rEvt.PropertyName == s3 ) || ( rEvt.PropertyName == s4 ) )
            {
                if ( bOwnModel )
                {
                    uno::Reference< awt::XControl > xThis( (uno::XAggregation*)(::cppu::OWeakAggObject*)this, uno::UNO_QUERY );
                    ImplSetPosSize( xThis );
                }
                else
                {
                    ImplSetPosSize( StdTabController::FindControl( getControls(), xModel ) );
                }
                break;
            }

        }
    }

    UnoControlContainer::propertiesChange( rEvents );
}

void UnoDialogControl::setTitle( const ::rtl::OUString& Title ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Title;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ), aAny, sal_True );
}

::rtl::OUString UnoDialogControl::getTitle() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_UString( BASEPROPERTY_TITLE );
}

sal_Int16 UnoDialogControl::execute() throw(uno::RuntimeException)
{
    sal_Int16 nDone = -1;
    if ( getPeer().is() )
    {
        uno::Reference< awt::XDialog > xDlg( getPeer(), uno::UNO_QUERY );
        if( xDlg.is() )
        {
            GetComponentInfos().bVisible = sal_True;
            nDone = xDlg->execute();
            GetComponentInfos().bVisible = sal_False;
        }
    }
    return nDone;
}

void UnoDialogControl::endExecute() throw(uno::RuntimeException)
{
    if ( getPeer().is() )
    {
        uno::Reference< awt::XDialog > xDlg( getPeer(), uno::UNO_QUERY );
        if( xDlg.is() )
        {
            xDlg->endExecute();
            GetComponentInfos().bVisible = sal_False;
        }
    }
}


//  ----------------------------------------------------
//  class UnoControlEditModel
//  ----------------------------------------------------
UnoControlEditModel::UnoControlEditModel()
{
    ImplRegisterProperty( BASEPROPERTY_ALIGN );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ECHOCHAR );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HARDLINEBREAKS );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_HSCROLL );
    ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
    ImplRegisterProperty( BASEPROPERTY_MULTILINE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_VSCROLL );
}

::rtl::OUString UnoControlEditModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlEditModel );
}

uno::Any UnoControlEditModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlEdit );
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlEditModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlEditModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoEditControl
//  ----------------------------------------------------
UnoEditControl::UnoEditControl()
    : maTextListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoEditControl::GetComponentServiceName()
{
    ::rtl::OUString aName( ::rtl::OUString::createFromAscii( "Edit" ) );
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_MULTILINE ) );
    sal_Bool b;
    if ( ( aVal >>= b ) && b )
        aName= ::rtl::OUString::createFromAscii( "MultiLineEdit" );
    return aName;
}

// uno::XInterface
uno::Any UnoEditControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XTextComponent*, this ),
                                        SAL_STATIC_CAST( awt::XTextListener*, this ),
                                        SAL_STATIC_CAST( lang::XEventListener*, SAL_STATIC_CAST( awt::XTextListener*, this ) ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( awt::XTextLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoEditControl )
getCppuType( ( uno::Reference< awt::XTextComponent>* ) NULL ),
getCppuType( ( uno::Reference< awt::XTextListener>* ) NULL ),
getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
getCppuType( ( uno::Reference< awt::XTextLayoutConstrains>* ) NULL ),
UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END


void UnoEditControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maTextListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoEditControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference< awt::XTextComponent > xText( mxPeer, uno::UNO_QUERY );
    xText->addTextListener( this );
}

void UnoEditControl::textChanged(const awt::TextEvent& e) throw(uno::RuntimeException)
{
    // Neuen Text als beans::Property ins Model treten.
    uno::Reference< awt::XTextComponent > xText( mxPeer, uno::UNO_QUERY );
    DBG_ASSERT( xText.is(), "TextComponentInterface?" );

    uno::Any aAny;
    aAny <<= xText->getText();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), aAny, sal_False );

    if ( maTextListeners.getLength() )
        maTextListeners.textChanged( e );
}

void UnoEditControl::addTextListener(const uno::Reference< awt::XTextListener > & l) throw(uno::RuntimeException)
{
    maTextListeners.addInterface( l );
}

void UnoEditControl::removeTextListener(const uno::Reference< awt::XTextListener > & l) throw(uno::RuntimeException)
{
    maTextListeners.removeInterface( l );
}

void UnoEditControl::setText( const ::rtl::OUString& aText ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= aText;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), aAny, sal_True );
}

void UnoEditControl::insertText( const awt::Selection& rSel, const ::rtl::OUString& aText ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= aText;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TEXT ), aAny, sal_True );
    if ( mxPeer.is() )
    {
        uno::Reference< awt::XTextComponent > xText( mxPeer, uno::UNO_QUERY );
        xText->insertText( rSel, aText );
    }
}

::rtl::OUString UnoEditControl::getText() throw(uno::RuntimeException)
{
    ::rtl::OUString aText;
    ::rtl::OUString aLineBreakProp( GetPropertyName( BASEPROPERTY_HARDLINEBREAKS ) );
    if ( mxPeer.is() && ImplHasProperty( aLineBreakProp ) && ImplGetPropertyValue_BOOL( BASEPROPERTY_HARDLINEBREAKS ) )
    {
        uno::Reference< awt::XTextArea > xText( mxPeer, uno::UNO_QUERY );
        aText = xText->getTextLines();
    }
    else if ( mxPeer.is() )
    {
        // Peer fragen, weil es die beans::Property "Text" nicht geben muss...
        uno::Reference< awt::XTextComponent > xText( mxPeer, uno::UNO_QUERY );
        aText = xText->getText();
    }
    else
    {
        aText = ImplGetPropertyValue_UString( BASEPROPERTY_TEXT );
    }

    return aText;
}

::rtl::OUString UnoEditControl::getSelectedText( void ) throw(uno::RuntimeException)
{
    ::rtl::OUString aSelected;
    if ( mxPeer.is() )
    {
        uno::Reference< awt::XTextComponent > xText( mxPeer, uno::UNO_QUERY );
        aSelected = xText->getSelectedText();
    }
    return aSelected;
}

void UnoEditControl::setSelection( const awt::Selection& aSelection ) throw(uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        uno::Reference< awt::XTextComponent > xText( mxPeer, uno::UNO_QUERY );
        xText->setSelection( aSelection );
    }
}

awt::Selection UnoEditControl::getSelection( void ) throw(uno::RuntimeException)
{
    awt::Selection aSel;
    if ( mxPeer.is() )
    {
        uno::Reference< awt::XTextComponent > xText( mxPeer, uno::UNO_QUERY );
        aSel = xText->getSelection();
    }
    return aSel;
}

sal_Bool UnoEditControl::isEditable( void ) throw(uno::RuntimeException)
{
    return !ImplGetPropertyValue_BOOL( BASEPROPERTY_READONLY );
}

void UnoEditControl::setEditable( sal_Bool bEditable ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Bool)!bEditable;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_READONLY ), aAny, sal_True );
}

sal_Int16 UnoEditControl::getMaxTextLen() throw(uno::RuntimeException)
{
    return !ImplGetPropertyValue_INT16( BASEPROPERTY_MAXTEXTLEN );
}

void UnoEditControl::setMaxTextLen( sal_Int16 nLen ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)nLen;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MAXTEXTLEN ), aAny, sal_True );
}

awt::Size UnoEditControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoEditControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoEditControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

awt::Size UnoEditControl::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize( nCols, nLines );
}

void UnoEditControl::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(uno::RuntimeException)
{
    Impl_getColumnsAndLines( nCols, nLines );
}

//  ----------------------------------------------------
//  class UnoControlFormattedFieldModel
//  ----------------------------------------------------
UnoControlFormattedFieldModel::UnoControlFormattedFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_ALIGN );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_DEFAULT );
    ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_VALUE );
    ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_MAX );
    ImplRegisterProperty( BASEPROPERTY_EFFECTIVE_MIN );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_FORMATKEY );
    ImplRegisterProperty( BASEPROPERTY_FORMATSSUPPLIER );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );

    uno::Any aTreatAsNumber;
    aTreatAsNumber <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_TREATASNUMBER, aTreatAsNumber );
}

::rtl::OUString UnoControlFormattedFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedFieldModel );
}

uno::Any UnoControlFormattedFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    uno::Any aReturn;
    switch (nPropId)
    {
        case BASEPROPERTY_DEFAULTCONTROL: aReturn <<= ::rtl::OUString( ::rtl::OUString::createFromAscii( szServiceName_UnoControlFormattedField ) );

        case BASEPROPERTY_TREATASNUMBER: aReturn <<= (sal_Bool)sal_True; break;

        case BASEPROPERTY_EFFECTIVE_DEFAULT:
        case BASEPROPERTY_EFFECTIVE_VALUE:
        case BASEPROPERTY_EFFECTIVE_MAX:
        case BASEPROPERTY_EFFECTIVE_MIN:
        case BASEPROPERTY_FORMATKEY:
        case BASEPROPERTY_FORMATSSUPPLIER:
            // (void)
            break;

        default : aReturn = UnoControlModel::ImplGetDefaultValue( nPropId ); break;
    }

    return aReturn;
}

::cppu::IPropertyArrayHelper& UnoControlFormattedFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlFormattedFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoFormattedFieldControl
//  ----------------------------------------------------
UnoFormattedFieldControl::UnoFormattedFieldControl()
{
}

::rtl::OUString UnoFormattedFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "FormattedField" );
}

void UnoFormattedFieldControl::textChanged(const awt::TextEvent& e) throw(uno::RuntimeException)
{
    uno::Reference< awt::XVclWindowPeer >  xPeer(mxPeer, uno::UNO_QUERY);
    DBG_ASSERT(xPeer.is(), "UnoFormattedFieldControl::textChanged : what kind of peer do I have ?");
    ::rtl::OUString sEffectiveValue = GetPropertyName( BASEPROPERTY_EFFECTIVE_VALUE );
    ImplSetPropertyValue( sEffectiveValue, xPeer->getProperty( sEffectiveValue ), sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

//  ----------------------------------------------------
//  class UnoControlFileControlModel
//  ----------------------------------------------------
UnoControlFileControlModel::UnoControlFileControlModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlFileControlModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFileControlModel );
}

uno::Any UnoControlFileControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlFileControl );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlFileControlModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlFileControlModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoFileControl
//  ----------------------------------------------------
UnoFileControl::UnoFileControl()
{
}

::rtl::OUString UnoFileControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "filecontrol" );
}

//  ----------------------------------------------------
//  class UnoControlButtonModel
//  ----------------------------------------------------
UnoControlButtonModel::UnoControlButtonModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTBUTTON );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_IMAGEURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

uno::Any UnoControlButtonModel::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType, SAL_STATIC_CAST( awt::XImageProducer*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlModel::queryAggregation( rType ));
}

::rtl::OUString UnoControlButtonModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlButtonModel );
}

uno::Any UnoControlButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlButton );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlButtonModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlButtonModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void UnoControlButtonModel::addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException)
{
    maListeners.push_back( xConsumer );
}

void UnoControlButtonModel::removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException)
{
    maListeners.remove( xConsumer );
}

void UnoControlButtonModel::startProduction(  ) throw (::com::sun::star::uno::RuntimeException)
{
    uno::Any aArg = getPropertyValue( GetPropertyName( BASEPROPERTY_IMAGEURL ) );
    rtl::OUString aURL;
    aArg >>= aURL;
    if ( aURL.getLength() )
    {
        uno::Sequence<uno::Any> aArgs(1);
        aArgs.getArray()[0] = aArg;
        uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
        uno::Reference< awt::XImageProducer > xImageProducer( xMSF->createInstanceWithArguments( ::rtl::OUString::createFromAscii( "com.sun.star.awt.ImageProducer" ), aArgs ), uno::UNO_QUERY );
        if ( xImageProducer.is() )
        {
            std::list< uno::Reference< awt::XImageConsumer > >::iterator aIter( maListeners.begin() );
            while ( aIter != maListeners.end() )
            {
                xImageProducer->addConsumer( *aIter );
                aIter++;
            }
            xImageProducer->startProduction();
        }
    }
}

//  ----------------------------------------------------
//  class UnoButtonControl
//  ----------------------------------------------------
UnoButtonControl::UnoButtonControl()
    : maActionListeners( *this )
{
    maComponentInfos.nWidth = 50;
    maComponentInfos.nHeight = 14;
}

::rtl::OUString UnoButtonControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "pushbutton" );
}

// uno::XInterface
uno::Any UnoButtonControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XButton*, this ),
                                        SAL_STATIC_CAST( awt::XImageConsumer*, this ),
                                        SAL_STATIC_CAST( awt::XImageProducer*, this ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoButtonControl )
getCppuType( ( uno::Reference< awt::XButton>* ) NULL ),
getCppuType( ( uno::Reference< awt::XImageConsumer>* ) NULL ),
getCppuType( ( uno::Reference< awt::XImageProducer>* ) NULL ),
getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoButtonControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoButtonControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XButton >  xButton( mxPeer, uno::UNO_QUERY );
    if ( maActionListeners.getLength() )
        xButton->addActionListener( &maActionListeners );
}

void UnoButtonControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const uno::Any& rVal )
{
    sal_uInt16 nType = GetPropertyId( rPropName );
    if ( mxPeer.is() && nType == BASEPROPERTY_IMAGEURL )
    {
        uno::Reference < awt::XImageProducer > xImgProd( getModel(), uno::UNO_QUERY );
        uno::Reference < awt::XImageConsumer > xImgCons( mxPeer, uno::UNO_QUERY );

        uno::Reference < awt::XImageConsumer > xTest1( (awt::XImageConsumer*)this, uno::UNO_QUERY );
        uno::Reference < awt::XImageProducer > xTest2( (awt::XImageConsumer*)this, uno::UNO_QUERY );

        if ( xImgProd.is() && xImgCons.is() )
        {
            xImgProd->addConsumer( xImgCons );
            xImgProd->startProduction();
            xImgProd->removeConsumer( xImgCons );
        }
    }
    else
        UnoControl::ImplSetPeerProperty( rPropName, rVal );
}


void UnoButtonControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( mxPeer, uno::UNO_QUERY );
        xButton->addActionListener( &maActionListeners );
    }
}

void UnoButtonControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( mxPeer, uno::UNO_QUERY );
        xButton->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoButtonControl::setLabel( const ::rtl::OUString&  rLabel ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoButtonControl::setActionCommand( const ::rtl::OUString& rCommand ) throw(uno::RuntimeException)
{
    maActionCommand = rCommand;
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XButton >  xButton( mxPeer, uno::UNO_QUERY );
        xButton->setActionCommand( rCommand );
    }
}

awt::Size UnoButtonControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoButtonControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoButtonControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

void UnoButtonControl::init( sal_Int32 Width, sal_Int32 Height ) throw(uno::RuntimeException)
{
    maImageConsumer.Init( Width, Height );
}

void UnoButtonControl::setColorModel( sal_Int16 BitCount, const uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(uno::RuntimeException)
{
    maImageConsumer.SetColorModel( BitCount, RGBAPal.getLength(), (const unsigned long*)RGBAPal.getConstArray(), RedMask, GreenMask, BlueMask, AlphaMask );
}

void UnoButtonControl::setPixelsByBytes( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const uno::Sequence< sal_Int8 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(uno::RuntimeException)
{
    maImageConsumer.SetPixelsByBytes( X, Y, Width, Height, (const unsigned char*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void UnoButtonControl::setPixelsByLongs( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const uno::Sequence< sal_Int32 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(uno::RuntimeException)
{
    maImageConsumer.SetPixelsByLongs( X, Y, Width, Height, (const unsigned long*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void UnoButtonControl::complete( sal_Int32 Status, const uno::Reference < awt::XImageProducer > & Producer ) throw(uno::RuntimeException)
{
    maImageConsumer.Completed( Status );

    // MT: Controls sollen angemeldet bleiben...
//  Producer->removeConsumer( this );

    ImplUpdateImage( sal_True );
}

void UnoButtonControl::addConsumer( const uno::Reference < awt::XImageConsumer > & Consumer ) throw(uno::RuntimeException)
{
    if ( mxImageProducer.is() )
        mxImageProducer->addConsumer( Consumer );
}

void UnoButtonControl::removeConsumer( const uno::Reference < awt::XImageConsumer > & Consumer ) throw(uno::RuntimeException)
{
    if ( mxImageProducer.is() )
        mxImageProducer->removeConsumer( Consumer );
}

void UnoButtonControl::startProduction() throw(uno::RuntimeException)
{
    if ( mxImageProducer.is() )
    {
//      UnoMemoryStream* pStrm = new UnoMemoryStream( 0x3FFF, 0x3FFF );
//      (*pStrm) << maBitmap;
//      uno::Reference < io::XInputStream >  xIn = pStrm;
//      mxImageProducer->setImage( xIn );
        mxImageProducer->startProduction();
    }
}

void UnoButtonControl::ImplUpdateImage( sal_Bool bGetNewImage )
{
    sal_Bool bOK = bGetNewImage ? maImageConsumer.GetData( maBitmap ) : sal_True;
    if ( bOK && mxPeer.is() && mxImageProducer.is() )
    {
        uno::Reference < awt::XImageConsumer >  xC( mxPeer, uno::UNO_QUERY );
        addConsumer( xC );
        startProduction();
        removeConsumer( xC );
    }
}

//  ----------------------------------------------------
//  class UnoControlImageControlModel
//  ----------------------------------------------------
UnoControlImageControlModel::UnoControlImageControlModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_IMAGEURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
}

::rtl::OUString UnoControlImageControlModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageControlModel );
}

uno::Any UnoControlImageControlModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlImageControl );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlImageControlModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlImageControlModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoImageControlControl
//  ----------------------------------------------------
UnoImageControlControl::UnoImageControlControl()
    : maActionListeners( *this )
{
    // Woher die Defaults nehmen?
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 100;

    uno::Reference< lang::XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    uno::Reference < uno::XInterface > xI = xMSF->createInstance( ::rtl::OUString::createFromAscii( szServiceName_ImageProducer ) );
    if ( xI.is() )
        mxImageProducer = uno::Reference< awt::XImageProducer >( xI, uno::UNO_QUERY );
}

::rtl::OUString UnoImageControlControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "fixedimage" );
}

// uno::XInterface
uno::Any UnoImageControlControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XImageConsumer*, this ),
                                        SAL_STATIC_CAST( awt::XImageProducer*, this ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoImageControlControl )
    getCppuType( ( uno::Reference< awt::XImageConsumer>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XImageProducer>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoImageControlControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

sal_Bool UnoImageControlControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

awt::Size UnoImageControlControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoImageControlControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoImageControlControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

void UnoImageControlControl::init( sal_Int32 Width, sal_Int32 Height ) throw(uno::RuntimeException)
{
    maImageConsumer.Init( Width, Height );
}

void UnoImageControlControl::setColorModel( sal_Int16 BitCount, const uno::Sequence< sal_Int32 >& RGBAPal, sal_Int32 RedMask, sal_Int32 GreenMask, sal_Int32 BlueMask, sal_Int32 AlphaMask ) throw(uno::RuntimeException)
{
    maImageConsumer.SetColorModel( BitCount, RGBAPal.getLength(), (const unsigned long*)RGBAPal.getConstArray(), RedMask, GreenMask, BlueMask, AlphaMask );
}

void UnoImageControlControl::setPixelsByBytes( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const uno::Sequence< sal_Int8 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(uno::RuntimeException)
{
    maImageConsumer.SetPixelsByBytes( X, Y, Width, Height, (const unsigned char*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void UnoImageControlControl::setPixelsByLongs( sal_Int32 X, sal_Int32 Y, sal_Int32 Width, sal_Int32 Height, const uno::Sequence< sal_Int32 >& ProducerData, sal_Int32 Offset, sal_Int32 Scansize ) throw(uno::RuntimeException)
{
    maImageConsumer.SetPixelsByLongs( X, Y, Width, Height, (const unsigned long*)ProducerData.getConstArray(), Offset, Scansize );
    ImplUpdateImage( sal_True );
}

void UnoImageControlControl::complete( sal_Int32 Status, const uno::Reference < awt::XImageProducer > & Producer ) throw(uno::RuntimeException)
{
    maImageConsumer.Completed( Status );

    // MT: Controls sollen angemeldet bleiben...
//  Producer->removeConsumer( this );

    ImplUpdateImage( sal_True );
}

void UnoImageControlControl::addConsumer( const uno::Reference < awt::XImageConsumer > & Consumer ) throw(uno::RuntimeException)
{
    if ( mxImageProducer.is() )
        mxImageProducer->addConsumer( Consumer );
}

void UnoImageControlControl::removeConsumer( const uno::Reference < awt::XImageConsumer > & Consumer ) throw(uno::RuntimeException)
{
    if ( mxImageProducer.is() )
        mxImageProducer->removeConsumer( Consumer );
}

void UnoImageControlControl::startProduction() throw(uno::RuntimeException)
{
    if ( mxImageProducer.is() )
    {
        // UnoMemoryStream* pStrm = new UnoMemoryStream( 0x3FFF, 0x3FFF );
        // (*pStrm) << maBitmap;
        // uno::Reference < io::XInputStream >  xIn = pStrm;
        // mxImageProducer->setImage( xIn );
        mxImageProducer->startProduction();
    }
}

void UnoImageControlControl::ImplUpdateImage( sal_Bool bGetNewImage )
{
    sal_Bool bOK = bGetNewImage ? maImageConsumer.GetData( maBitmap ) : sal_True;
    if ( bOK && mxPeer.is() && mxImageProducer.is() )
    {
        uno::Reference < awt::XImageConsumer >  xC( mxPeer, uno::UNO_QUERY );
        addConsumer( xC );
        startProduction();
        removeConsumer( xC );
    }
}

//  ----------------------------------------------------
//  class UnoControlRadioButtonModel
//  ----------------------------------------------------
UnoControlRadioButtonModel::UnoControlRadioButtonModel()
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_STATE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlRadioButtonModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlRadioButtonModel );
}

uno::Any UnoControlRadioButtonModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlRadioButton );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlRadioButtonModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlRadioButtonModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoRadioButtonControl
//  ----------------------------------------------------
UnoRadioButtonControl::UnoRadioButtonControl()
    : maItemListeners( *this ), maActionListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoRadioButtonControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "radiobutton" );
}

// uno::XInterface
uno::Any UnoRadioButtonControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XButton*, this ),
                                        SAL_STATIC_CAST( awt::XRadioButton*, this ),
                                        SAL_STATIC_CAST( awt::XItemListener*, this ),
                                        SAL_STATIC_CAST( lang::XEventListener*, SAL_STATIC_CAST( awt::XItemListener*, this ) ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoRadioButtonControl )
    getCppuType( ( uno::Reference< awt::XButton>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XRadioButton>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XItemListener>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoRadioButtonControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}


sal_Bool UnoRadioButtonControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

void UnoRadioButtonControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XRadioButton >  xRadioButton( mxPeer, uno::UNO_QUERY );
    xRadioButton->addItemListener( this );

    uno::Reference < awt::XButton >  xButton( mxPeer, uno::UNO_QUERY );
    if ( maActionListeners.getLength() )
        xButton->addActionListener( &maActionListeners );
}

void UnoRadioButtonControl::addItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoRadioButtonControl::removeItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void UnoRadioButtonControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( mxPeer, uno::UNO_QUERY );
        xButton->addActionListener( &maActionListeners );
    }
}

void UnoRadioButtonControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XButton >  xButton( mxPeer, uno::UNO_QUERY );
        xButton->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoRadioButtonControl::setLabel( const ::rtl::OUString&  rLabel ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoRadioButtonControl::setActionCommand( const ::rtl::OUString& rCommand ) throw(uno::RuntimeException)
{
    maActionCommand = rCommand;
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XButton >  xButton( mxPeer, uno::UNO_QUERY );
        xButton->setActionCommand( rCommand );
    }
}

void UnoRadioButtonControl::setState( sal_Bool bOn ) throw(uno::RuntimeException)
{
    sal_Int16 nState = bOn ? 1 : 0;
    uno::Any aAny;
    aAny <<= nState;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_True );
}

sal_Bool UnoRadioButtonControl::getState() throw(uno::RuntimeException)
{
    sal_Int16 nState = 0;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ) );
    aVal >>= nState;
    return nState ? sal_True : sal_False;
}

void UnoRadioButtonControl::itemStateChanged( const awt::ItemEvent& rEvent ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)rEvent.Selected;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_False );

    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}

awt::Size UnoRadioButtonControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoRadioButtonControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoRadioButtonControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

//  ----------------------------------------------------
//  class UnoControlCheckBoxModel
//  ----------------------------------------------------
UnoControlCheckBoxModel::UnoControlCheckBoxModel()
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_STATE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_TRISTATE );
}

::rtl::OUString UnoControlCheckBoxModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlCheckBoxModel );
}

uno::Any UnoControlCheckBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlCheckBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlCheckBoxModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlCheckBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoCheckBoxControl
//  ----------------------------------------------------
UnoCheckBoxControl::UnoCheckBoxControl()
    : maItemListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoCheckBoxControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "checkbox" );
}

// uno::XInterface
uno::Any UnoCheckBoxControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XCheckBox*, this ),
                                        SAL_STATIC_CAST( awt::XItemListener*, this ),
                                        SAL_STATIC_CAST( lang::XEventListener*, SAL_STATIC_CAST( awt::XItemListener*, this ) ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoCheckBoxControl )
    getCppuType( ( uno::Reference< awt::XCheckBox>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XItemListener>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoCheckBoxControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

sal_Bool UnoCheckBoxControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

void UnoCheckBoxControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XCheckBox >  xCheckBox( mxPeer, uno::UNO_QUERY );
    xCheckBox->addItemListener( this );
}

void UnoCheckBoxControl::addItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoCheckBoxControl::removeItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void UnoCheckBoxControl::setLabel( const ::rtl::OUString&  rLabel ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= rLabel;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

void UnoCheckBoxControl::setState( short n ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)n;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_True );
}

short UnoCheckBoxControl::getState() throw(uno::RuntimeException)
{
    short nState = 0;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ) );
    aVal >>= nState;
    return nState;
}

void UnoCheckBoxControl::enableTriState( sal_Bool b ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= b;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TRISTATE ), aAny, sal_True );
}

void UnoCheckBoxControl::itemStateChanged( const awt::ItemEvent& rEvent ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)rEvent.Selected;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STATE ), aAny, sal_False );

    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}

awt::Size UnoCheckBoxControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoCheckBoxControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoCheckBoxControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

//  ----------------------------------------------------
//  class UnoControlFixedTextModel
//  ----------------------------------------------------
UnoControlFixedTextModel::UnoControlFixedTextModel()
{
    ImplRegisterProperty( BASEPROPERTY_ALIGN );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_MULTILINE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlFixedTextModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedTextModel );
}

uno::Any UnoControlFixedTextModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlFixedText );
        return aAny;
    }
    else if ( nPropId == BASEPROPERTY_BORDER )
    {
        uno::Any aAny;
        aAny <<= (sal_Int16)0;
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlFixedTextModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlFixedTextModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoFixedTextControl
//  ----------------------------------------------------
UnoFixedTextControl::UnoFixedTextControl()
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoFixedTextControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "fixedtext" );
}

// uno::XInterface
uno::Any UnoFixedTextControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XFixedText*, this ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoFixedTextControl )
    getCppuType( ( uno::Reference< awt::XFixedText>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

sal_Bool UnoFixedTextControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

void UnoFixedTextControl::setText( const ::rtl::OUString& Text ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Text;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

::rtl::OUString UnoFixedTextControl::getText() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_UString( BASEPROPERTY_LABEL );
}

void UnoFixedTextControl::setAlignment( short nAlign ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)nAlign;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ), aAny, sal_True );
}

short UnoFixedTextControl::getAlignment() throw(uno::RuntimeException)
{
    short nAlign = 0;
    if ( mxModel.is() )
    {
        uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_ALIGN ) );
        aVal >>= nAlign;
    }
    return nAlign;
}

awt::Size UnoFixedTextControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoFixedTextControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoFixedTextControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

//  ----------------------------------------------------
//  class UnoControlGroupBoxModel
//  ----------------------------------------------------
UnoControlGroupBoxModel::UnoControlGroupBoxModel()
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlGroupBoxModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlGroupBoxModel );
}

uno::Any UnoControlGroupBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlGroupBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlGroupBoxModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlGroupBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoGroupBoxControl
//  ----------------------------------------------------
UnoGroupBoxControl::UnoGroupBoxControl()
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 100;
}

::rtl::OUString UnoGroupBoxControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "groupbox" );
}

sal_Bool UnoGroupBoxControl::isTransparent() throw(uno::RuntimeException)
{
    return sal_True;
}

//  ----------------------------------------------------
//  class UnoControlListBoxModel
//  ----------------------------------------------------
UnoControlListBoxModel::UnoControlListBoxModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_DROPDOWN );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LINECOUNT );
    ImplRegisterProperty( BASEPROPERTY_MULTISELECTION );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_SELECTEDITEMS );
    ImplRegisterProperty( BASEPROPERTY_STRINGITEMLIST );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
}

::rtl::OUString UnoControlListBoxModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlListBoxModel );
}

uno::Any UnoControlListBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlListBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlListBoxModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlListBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void UnoControlListBoxModel::ImplPropertyChanged( sal_uInt16 nPropId )
{
    if ( nPropId == BASEPROPERTY_STRINGITEMLIST )
    {
        uno::Sequence<sal_Int16> aSeq;
        uno::Any aAny;
        aAny <<= aSeq;
        setPropertyValue( GetPropertyName( BASEPROPERTY_SELECTEDITEMS ), aAny );
    }
}

//  ----------------------------------------------------
//  class UnoListBoxControl
//  ----------------------------------------------------
UnoListBoxControl::UnoListBoxControl()
    :   maActionListeners( *this ),
        maItemListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoListBoxControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "listbox" );
}

// uno::XInterface
uno::Any UnoListBoxControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XListBox*, this ),
                                        SAL_STATIC_CAST( awt::XItemListener*, this ),
                                        SAL_STATIC_CAST( lang::XEventListener*, SAL_STATIC_CAST( awt::XItemListener*, this ) ),
                                        SAL_STATIC_CAST( awt::XLayoutConstrains*, this ),
                                        SAL_STATIC_CAST( awt::XTextLayoutConstrains*, this ) );
    return (aRet.hasValue() ? aRet : UnoControlBase::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoListBoxControl )
    getCppuType( ( uno::Reference< awt::XListBox>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XItemListener>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XLayoutConstrains>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XTextLayoutConstrains>* ) NULL ),
    UnoControlBase::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoListBoxControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoListBoxControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const uno::Any& rVal )
{
    UnoControl::ImplSetPeerProperty( rPropName, rVal );

    // Wenn die SelectedItems vor der StringItemList gesetzt werden,
    // hat das keine Auswirkung...
    if ( rPropName == GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) )
    {
        ::rtl::OUString aSelPropName = GetPropertyName( BASEPROPERTY_SELECTEDITEMS );
        uno::Any aVal = ImplGetPropertyValue( aSelPropName );
        if ( !( aVal.getValueType().getTypeClass() == uno::TypeClass_VOID ) )
        {
            uno::Reference< awt::XVclWindowPeer > xW( mxPeer, uno::UNO_QUERY );
            if (xW.is())
                // same comment as in UnoControl::ImplSetPeerProperty - see there
                xW->setProperty( aSelPropName, aVal );
        }
    }
}

void UnoListBoxControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
    xListBox->addItemListener( this );

    if ( maActionListeners.getLength() )
        xListBox->addActionListener( &maActionListeners );
}

void UnoListBoxControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        xListBox->addActionListener( &maActionListeners );
    }
}

void UnoListBoxControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        xListBox->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoListBoxControl::addItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.addInterface( l );
}

void UnoListBoxControl::removeItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.removeInterface( l );
}

void UnoListBoxControl::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aSeq( 1 );
    aSeq.getArray()[0] = aItem;
    addItems( aSeq, nPos );
}

void UnoListBoxControl::addItems( const uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nNewItems = (sal_uInt16)aItems.getLength();
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    sal_uInt16 nNewLen = nOldLen + nNewItems;

    uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
    ::rtl::OUString* pNewData = aNewSeq.getArray();
    ::rtl::OUString* pOldData = aSeq.getArray();

    if ( nPos > nOldLen )
        nPos = (sal_uInt16) nOldLen;

    sal_uInt16 n;
    // Items vor der Einfuege-Position
    for ( n = 0; n < nPos; n++ )
        pNewData[n] = pOldData[n];

    // Neue Items
    for ( n = 0; n < nNewItems; n++ )
        pNewData[nPos+n] = aItems.getConstArray()[n];

    // Rest der alten Items
    for ( n = nPos; n < nOldLen; n++ )
        pNewData[nNewItems+n] = pOldData[n];

    uno::Any aAny;
    aAny <<= aNewSeq;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
}

void UnoListBoxControl::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    if ( nOldLen && ( nPos < nOldLen ) )
    {
        if ( nCount > ( nOldLen-nPos ) )
            nCount = nOldLen-nPos;

        sal_uInt16 nNewLen = nOldLen - nCount;

        uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
        ::rtl::OUString* pNewData = aNewSeq.getArray();
        ::rtl::OUString* pOldData = aSeq.getArray();

        sal_uInt16 n;
        // Items vor der Entfern-Position
        for ( n = 0; n < nPos; n++ )
            pNewData[n] = pOldData[n];

        // Rest der Items
        for ( n = nPos; n < (nOldLen-nCount); n++ )
            pNewData[n] = pOldData[n+nCount];

        uno::Any aAny;
        aAny <<= aNewSeq;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
    }
}

sal_Int16 UnoListBoxControl::getItemCount() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return (sal_Int16)aSeq.getLength();
}

::rtl::OUString UnoListBoxControl::getItem( sal_Int16 nPos ) throw(uno::RuntimeException)
{
    ::rtl::OUString aItem;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    if ( nPos < aSeq.getLength() )
        aItem = aSeq.getConstArray()[nPos];
    return aItem;
}

uno::Sequence< ::rtl::OUString> UnoListBoxControl::getItems() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return aSeq;
}

sal_Int16 UnoListBoxControl::getSelectedItemPos() throw(uno::RuntimeException)
{
    sal_Int16 n = -1;
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        n = xListBox->getSelectedItemPos();
    }
    return n;
}

uno::Sequence<sal_Int16> UnoListBoxControl::getSelectedItemsPos() throw(uno::RuntimeException)
{
    uno::Sequence<sal_Int16> aSeq;
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        aSeq = xListBox->getSelectedItemsPos();
    }
    return aSeq;
}

::rtl::OUString UnoListBoxControl::getSelectedItem() throw(uno::RuntimeException)
{
    ::rtl::OUString aItem;
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        aItem = xListBox->getSelectedItem();
    }
    return aItem;
}

uno::Sequence< ::rtl::OUString> UnoListBoxControl::getSelectedItems() throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aSeq;
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        aSeq = xListBox->getSelectedItems();
    }
    return aSeq;
}

void UnoListBoxControl::selectItemPos( sal_Int16 nPos, sal_Bool bSelect ) throw(uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        xListBox->selectItemPos( nPos, bSelect );
    }
}

void UnoListBoxControl::selectItemsPos( const uno::Sequence<sal_Int16>& aPositions, sal_Bool bSelect ) throw(uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        xListBox->selectItemsPos( aPositions, bSelect );
    }
}

void UnoListBoxControl::selectItem( const ::rtl::OUString& aItem, sal_Bool bSelect ) throw(uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        xListBox->selectItem( aItem, bSelect );
    }
}

void UnoListBoxControl::makeVisible( sal_Int16 nEntry ) throw(uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
        xListBox->makeVisible( nEntry );
    }
}

void UnoListBoxControl::setDropDownLineCount( sal_Int16 nLines ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= (sal_Int16)nLines;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINECOUNT ), aAny, sal_True );
}

sal_Int16 UnoListBoxControl::getDropDownLineCount() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT16( BASEPROPERTY_LINECOUNT );
}

sal_Bool UnoListBoxControl::isMutipleMode() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRINGITEMLIST );
}

void UnoListBoxControl::setMultipleMode( sal_Bool bMulti ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bMulti;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTISELECTION ), aAny, sal_True );
}

void UnoListBoxControl::itemStateChanged( const awt::ItemEvent& rEvent ) throw(uno::RuntimeException)
{
    // Neue uno::Sequence als beans::Property ins Model treten.
    uno::Reference < awt::XListBox >  xListBox( mxPeer, uno::UNO_QUERY );
    DBG_ASSERT( xListBox.is(), "XListBox?" );

    uno::Sequence<sal_Int16> aSeq = xListBox->getSelectedItemsPos();
    uno::Any aAny;
    aAny <<= aSeq;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_SELECTEDITEMS ), aAny, sal_False );

    if ( maItemListeners.getLength() )
        maItemListeners.itemStateChanged( rEvent );
}

awt::Size UnoListBoxControl::getMinimumSize(  ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize();
}

awt::Size UnoListBoxControl::getPreferredSize(  ) throw(uno::RuntimeException)
{
    return Impl_getPreferredSize();
}

awt::Size UnoListBoxControl::calcAdjustedSize( const awt::Size& rNewSize ) throw(uno::RuntimeException)
{
    return Impl_calcAdjustedSize( rNewSize );
}

awt::Size UnoListBoxControl::getMinimumSize( sal_Int16 nCols, sal_Int16 nLines ) throw(uno::RuntimeException)
{
    return Impl_getMinimumSize( nCols, nLines );
}

void UnoListBoxControl::getColumnsAndLines( sal_Int16& nCols, sal_Int16& nLines ) throw(uno::RuntimeException)
{
    Impl_getColumnsAndLines( nCols, nLines );
}

//  ----------------------------------------------------
//  class UnoControlComboBoxModel
//  ----------------------------------------------------
UnoControlComboBoxModel::UnoControlComboBoxModel()
{
    ImplRegisterProperty( BASEPROPERTY_AUTOCOMPLETE );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_DROPDOWN );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LINECOUNT );
    ImplRegisterProperty( BASEPROPERTY_MAXTEXTLEN );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_STRINGITEMLIST );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlComboBoxModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlComboBoxModel );
}

uno::Any UnoControlComboBoxModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlComboBox );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlComboBoxModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlComboBoxModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoComboBoxControl
//  ----------------------------------------------------
UnoComboBoxControl::UnoComboBoxControl()
    :   maActionListeners( *this ),
        maItemListeners( *this )
{
    maComponentInfos.nWidth = 100;
    maComponentInfos.nHeight = 12;
}

::rtl::OUString UnoComboBoxControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "combobox" );
}

// uno::XInterface
uno::Any UnoComboBoxControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XComboBox*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoComboBoxControl )
    getCppuType( ( uno::Reference< awt::XComboBox>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoComboBoxControl::dispose() throw(uno::RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maActionListeners.disposeAndClear( aEvt );
    maItemListeners.disposeAndClear( aEvt );
    UnoControl::dispose();
}

void UnoComboBoxControl::createPeer( const uno::Reference< awt::XToolkit > & rxToolkit, const uno::Reference< awt::XWindowPeer >  & rParentPeer ) throw(uno::RuntimeException)
{
    UnoEditControl::createPeer( rxToolkit, rParentPeer );

    uno::Reference < awt::XComboBox >  xComboBox( mxPeer, uno::UNO_QUERY );
    if ( maActionListeners.getLength() )
        xComboBox->addActionListener( &maActionListeners );
    if ( maItemListeners.getLength() )
        xComboBox->addItemListener( &maItemListeners );
}

void UnoComboBoxControl::addActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    maActionListeners.addInterface( l );
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XComboBox >  xComboBox( mxPeer, uno::UNO_QUERY );
        xComboBox->addActionListener( &maActionListeners );
    }
}

void UnoComboBoxControl::removeActionListener(const uno::Reference< awt::XActionListener > & l) throw(uno::RuntimeException)
{
    if( mxPeer.is() && maActionListeners.getLength() == 1 )
    {
        uno::Reference < awt::XComboBox >  xComboBox( mxPeer, uno::UNO_QUERY );
        xComboBox->removeActionListener( &maActionListeners );
    }
    maActionListeners.removeInterface( l );
}

void UnoComboBoxControl::addItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    maItemListeners.addInterface( l );
    if( mxPeer.is() && maItemListeners.getLength() == 1 )
    {
        uno::Reference < awt::XComboBox >  xComboBox( mxPeer, uno::UNO_QUERY );
        xComboBox->addItemListener( &maItemListeners );
    }
}

void UnoComboBoxControl::removeItemListener(const uno::Reference < awt::XItemListener > & l) throw(uno::RuntimeException)
{
    if( mxPeer.is() && maItemListeners.getLength() == 1 )
    {
        uno::Reference < awt::XComboBox >  xComboBox( mxPeer, uno::UNO_QUERY ); // MT: Mal alles so umstellen, schoener als Ref anlegen und query rufen
        xComboBox->removeItemListener( &maItemListeners );
    }
    maItemListeners.removeInterface( l );
}

void UnoComboBoxControl::addItem( const ::rtl::OUString& aItem, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString> aSeq( 1 );
    aSeq.getArray()[0] = aItem;
    addItems( aSeq, nPos );
}

void UnoComboBoxControl::addItems( const uno::Sequence< ::rtl::OUString>& aItems, sal_Int16 nPos ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nNewItems = (sal_uInt16)aItems.getLength();
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    sal_uInt16 nNewLen = nOldLen + nNewItems;

    uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
    ::rtl::OUString* pNewData = aNewSeq.getArray();
    const ::rtl::OUString* pOldData = aSeq.getConstArray();

    if ( nPos > nOldLen )
        nPos = (sal_uInt16) nOldLen;

    sal_uInt16 n;
    // Items vor der Einfuege-Position
    for ( n = 0; n < nPos; n++ )
        pNewData[n] = pOldData[n];

    // Neue Items
    for ( n = 0; n < nNewItems; n++ )
        pNewData[nPos+n] = aItems.getConstArray()[n];

    // Rest der alten Items
    for ( n = nPos; n < nOldLen; n++ )
        pNewData[nNewItems+n] = pOldData[n];

    uno::Any aAny;
    aAny <<= aNewSeq;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
}

void UnoComboBoxControl::removeItems( sal_Int16 nPos, sal_Int16 nCount ) throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    sal_uInt16 nOldLen = (sal_uInt16)aSeq.getLength();
    if ( nOldLen && ( nPos < nOldLen ) )
    {
        if ( nCount > ( nOldLen-nPos ) )
            nCount = nOldLen-nPos;

        sal_uInt16 nNewLen = nOldLen - nCount;

        uno::Sequence< ::rtl::OUString> aNewSeq( nNewLen );
        ::rtl::OUString* pNewData = aNewSeq.getArray();
        ::rtl::OUString* pOldData = aSeq.getArray();

        sal_uInt16 n;
        // Items vor der Entfern-Position
        for ( n = 0; n < nPos; n++ )
            pNewData[n] = pOldData[n];

        // Rest der Items
        for ( n = nPos; n < (nOldLen-nCount); n++ )
            pNewData[n] = pOldData[n+nCount];

        uno::Any aAny;
        aAny <<= aNewSeq;
        ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ), aAny, sal_True );
    }
}

sal_Int16 UnoComboBoxControl::getItemCount() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return (sal_Int16)aSeq.getLength();
}

::rtl::OUString UnoComboBoxControl::getItem( sal_Int16 nPos ) throw(uno::RuntimeException)
{
    ::rtl::OUString aItem;
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    if ( nPos < aSeq.getLength() )
        aItem = aSeq.getConstArray()[nPos];
    return aItem;
}

uno::Sequence< ::rtl::OUString> UnoComboBoxControl::getItems() throw(uno::RuntimeException)
{
    uno::Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_STRINGITEMLIST ) );
    uno::Sequence< ::rtl::OUString> aSeq;
    aVal >>= aSeq;
    return aSeq;
}

void UnoComboBoxControl::setDropDownLineCount( sal_Int16 nLines ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= nLines;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LINECOUNT ), aAny, sal_True );
}

sal_Int16 UnoComboBoxControl::getDropDownLineCount() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT16( BASEPROPERTY_LINECOUNT );
}

//  ----------------------------------------------------
//  class UnoControlDateFieldModel
//  ----------------------------------------------------
UnoControlDateFieldModel::UnoControlDateFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DATE );
    ImplRegisterProperty( BASEPROPERTY_DATEMAX );
    ImplRegisterProperty( BASEPROPERTY_DATEMIN );
    ImplRegisterProperty( BASEPROPERTY_DATESHOWCENTURY );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_DROPDOWN );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_EXTDATEFORMAT );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlDateFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlDateFieldModel );
}

uno::Any UnoControlDateFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlDateField );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlDateFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlDateFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoDateFieldControl
//  ----------------------------------------------------
UnoDateFieldControl::UnoDateFieldControl()
{
}

::rtl::OUString UnoDateFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "datefield" );
}

// uno::XInterface
uno::Any UnoDateFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XDateField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoDateFieldControl )
    getCppuType( ( uno::Reference< awt::XDateField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoDateFieldControl::textChanged( const awt::TextEvent& e ) throw(uno::RuntimeException)
{
    uno::Reference < awt::XDateField >  xField( mxPeer, uno::UNO_QUERY );
    uno::Any aValue;
    if ( !xField->isEmpty() )
        aValue <<= xField->getDate();

    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATE ), aValue, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoDateFieldControl::setDate( sal_Int32 Date ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Date;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATE ), aAny, sal_True );
}

sal_Int32 UnoDateFieldControl::getDate() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_DATE );
}

void UnoDateFieldControl::setMin( sal_Int32 Date ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Date;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATEMIN ), aAny, sal_True );
}

sal_Int32 UnoDateFieldControl::getMin() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_DATEMIN );
}

void UnoDateFieldControl::setMax( sal_Int32 Date ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Date;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_DATEMAX ), aAny, sal_True );
}

sal_Int32 UnoDateFieldControl::getMax() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_DATEMAX );
}

void UnoDateFieldControl::setFirst( sal_Int32 Date ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::setFirst not supported" );
}

sal_Int32 UnoDateFieldControl::getFirst() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::getFirst not supported" );
    return 0;
}

void UnoDateFieldControl::setLast( sal_Int32 Date ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::setLast not supported" );
}

sal_Int32 UnoDateFieldControl::getLast() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::getLast not supported" );
    return 0;
}

void UnoDateFieldControl::setLongFormat( sal_Bool bLong ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::setLongFormat not supported" );
}

sal_Bool UnoDateFieldControl::isLongFormat() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoDateFieldControl::getLongFormat not supported" );
    return sal_False;
}

void UnoDateFieldControl::setEmpty() throw(uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XDateField >  xField( mxPeer, uno::UNO_QUERY );
        xField->setEmpty();
    }
}

sal_Bool UnoDateFieldControl::isEmpty() throw(uno::RuntimeException)
{
    sal_Bool bEmpty = sal_False;
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XDateField > xField( mxPeer, uno::UNO_QUERY );
        bEmpty = xField->isEmpty();
    }
    return bEmpty;
}

void UnoDateFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoDateFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

//  ----------------------------------------------------
//  class UnoControlTimeFieldModel
//  ----------------------------------------------------
UnoControlTimeFieldModel::UnoControlTimeFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED);
    ImplRegisterProperty( BASEPROPERTY_EXTTIMEFORMAT );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_TIME );
    ImplRegisterProperty( BASEPROPERTY_TIMEMAX );
    ImplRegisterProperty( BASEPROPERTY_TIMEMIN );
}

::rtl::OUString UnoControlTimeFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlTimeFieldModel );
}

uno::Any UnoControlTimeFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlTimeField );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlTimeFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlTimeFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoTimeFieldControl
//  ----------------------------------------------------
UnoTimeFieldControl::UnoTimeFieldControl()
{
}

::rtl::OUString UnoTimeFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "timefield" );
}

// uno::XInterface
uno::Any UnoTimeFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XTimeField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoTimeFieldControl )
    getCppuType( ( uno::Reference< awt::XTimeField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoTimeFieldControl::textChanged( const awt::TextEvent& e ) throw(uno::RuntimeException)
{
    uno::Reference < awt::XTimeField >  xField( mxPeer, uno::UNO_QUERY );
    uno::Any aValue;
    if ( !xField->isEmpty() )
        xField->getTime();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIME ), aValue, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoTimeFieldControl::setTime( sal_Int32 Time ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Time;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIME ), aAny, sal_True );
}

sal_Int32 UnoTimeFieldControl::getTime() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_TIME );
}

void UnoTimeFieldControl::setMin( sal_Int32 Time ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Time;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIMEMIN ), aAny, sal_True );
}

sal_Int32 UnoTimeFieldControl::getMin() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_TIMEMIN );
}

void UnoTimeFieldControl::setMax( sal_Int32 Time ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Time;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TIMEMAX ), aAny, sal_True );
}

sal_Int32 UnoTimeFieldControl::getMax() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_INT32( BASEPROPERTY_TIMEMAX );
}

void UnoTimeFieldControl::setFirst( sal_Int32 Time ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoTimeFieldControl::setFirst not supported" );
}

sal_Int32 UnoTimeFieldControl::getFirst() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoTimeFieldControl::getFirst not supported" );
    return 0;
}

void UnoTimeFieldControl::setLast( sal_Int32 Time ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoTimeFieldControl::setLast not supported" );
}

sal_Int32 UnoTimeFieldControl::getLast() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoTimeFieldControl::getLast not supported" );
    return 0;
}

void UnoTimeFieldControl::setEmpty() throw(uno::RuntimeException)
{
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XTimeField >  xField( mxPeer, uno::UNO_QUERY );
        xField->setEmpty();
    }
}

sal_Bool UnoTimeFieldControl::isEmpty() throw(uno::RuntimeException)
{
    sal_Bool bEmpty = sal_False;
    if ( mxPeer.is() )
    {
        uno::Reference < awt::XTimeField >  xField( mxPeer, uno::UNO_QUERY );
        bEmpty = xField->isEmpty();
    }
    return bEmpty;
}

void UnoTimeFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoTimeFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

//  ----------------------------------------------------
//  class UnoControlNumericFieldModel
//  ----------------------------------------------------
UnoControlNumericFieldModel::UnoControlNumericFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DECIMALACCURACY );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_NUMSHOWTHOUSANDSEP );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_VALUEMAX_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUEMIN_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUESTEP_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUE_DOUBLE );
}

::rtl::OUString UnoControlNumericFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlNumericFieldModel );
}

uno::Any UnoControlNumericFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlNumericField );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}


::cppu::IPropertyArrayHelper& UnoControlNumericFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlNumericFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}



//  ----------------------------------------------------
//  class UnoNumericFieldControl
//  ----------------------------------------------------
UnoNumericFieldControl::UnoNumericFieldControl()
{
}

::rtl::OUString UnoNumericFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "numericfield" );
}

// uno::XInterface
uno::Any UnoNumericFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XNumericField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoNumericFieldControl )
    getCppuType( ( uno::Reference< awt::XNumericField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoNumericFieldControl::textChanged( const awt::TextEvent& e ) throw(uno::RuntimeException)
{
    uno::Reference < awt::XNumericField >  xField( mxPeer, uno::UNO_QUERY );
    uno::Any aAny;
    aAny <<= xField->getValue();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoNumericFieldControl::setValue( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getValue() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUE_DOUBLE );
}

void UnoNumericFieldControl::setMin( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMIN_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getMin() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMIN_DOUBLE );
}

void UnoNumericFieldControl::setMax( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMAX_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getMax() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMAX_DOUBLE );
}

void UnoNumericFieldControl::setFirst( double Value ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::setFirst not supported" );
}

double UnoNumericFieldControl::getFirst() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::getFirst not supported" );
    return 0;
}

void UnoNumericFieldControl::setLast( double Value ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::setLast not supported" );
}

double UnoNumericFieldControl::getLast() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::getLast not supported" );
    return 0;
}

void UnoNumericFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoNumericFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

void UnoNumericFieldControl::setSpinSize( double Digits ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Digits;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUESTEP_DOUBLE ), aAny, sal_True );
}

double UnoNumericFieldControl::getSpinSize() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUESTEP_DOUBLE );
}

void UnoNumericFieldControl::setDecimalDigits( sal_Int16 Digits ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::setDecimalDigits not supported" );
}

sal_Int16 UnoNumericFieldControl::getDecimalDigits() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoNumericFieldControl::getDecimalDigits not supported" );
    return 0;
}

//  ----------------------------------------------------
//  class UnoControlCurrencyFieldModel
//  ----------------------------------------------------
UnoControlCurrencyFieldModel::UnoControlCurrencyFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_CURRENCYSYMBOL );
    ImplRegisterProperty( BASEPROPERTY_CURSYM_POSITION );
    ImplRegisterProperty( BASEPROPERTY_DECIMALACCURACY );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_NUMSHOWTHOUSANDSEP );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_SPIN );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
    ImplRegisterProperty( BASEPROPERTY_VALUEMAX_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUEMIN_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUESTEP_DOUBLE );
    ImplRegisterProperty( BASEPROPERTY_VALUE_DOUBLE );
}

::rtl::OUString UnoControlCurrencyFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyFieldModel );
}

uno::Any UnoControlCurrencyFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlCurrencyField );
        return aAny;
    }
    if ( nPropId == BASEPROPERTY_CURSYM_POSITION )
    {
        uno::Any aAny;
        aAny <<= (sal_Bool)sal_False;
        return aAny;
    }

    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlCurrencyFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlCurrencyFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//  ----------------------------------------------------
//  class UnoCurrencyFieldControl
//  ----------------------------------------------------
UnoCurrencyFieldControl::UnoCurrencyFieldControl()
{
}

::rtl::OUString UnoCurrencyFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "longcurrencyfield" );
}

// uno::XInterface
uno::Any UnoCurrencyFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XCurrencyField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoCurrencyFieldControl )
    getCppuType( ( uno::Reference< awt::XCurrencyField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoCurrencyFieldControl::textChanged( const awt::TextEvent& e ) throw(uno::RuntimeException)
{
    uno::Reference < awt::XCurrencyField >  xField( mxPeer, uno::UNO_QUERY );
    uno::Any aAny;
    aAny <<= xField->getValue();
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_False );

    if ( GetTextListeners().getLength() )
        GetTextListeners().textChanged( e );
}

void UnoCurrencyFieldControl::setValue( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUE_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getValue() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUE_DOUBLE );
}

void UnoCurrencyFieldControl::setMin( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMIN_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getMin() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMIN_DOUBLE );
}

void UnoCurrencyFieldControl::setMax( double Value ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Value;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUEMAX_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getMax() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUEMAX_DOUBLE );
}

void UnoCurrencyFieldControl::setFirst( double Value ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::setFirst not supported" );
}

double UnoCurrencyFieldControl::getFirst() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::getFirst not supported" );
    return 0;
}

void UnoCurrencyFieldControl::setLast( double Value ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::setLast not supported" );
}

double UnoCurrencyFieldControl::getLast() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::getLast not supported" );
    return 0;
}

void UnoCurrencyFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoCurrencyFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}

void UnoCurrencyFieldControl::setSpinSize( double Digits ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= Digits;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_VALUESTEP_DOUBLE ), aAny, sal_True );
}

double UnoCurrencyFieldControl::getSpinSize() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_DOUBLE( BASEPROPERTY_VALUESTEP_DOUBLE );
}

void UnoCurrencyFieldControl::setDecimalDigits( sal_Int16 Digits ) throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::setDecimalDigits not supported" );
}

sal_Int16 UnoCurrencyFieldControl::getDecimalDigits() throw(uno::RuntimeException)
{
    DBG_WARNING( "UnoCurrencyFieldControl::getDecimalDigits not supported" );
    return 0;
}

//  ----------------------------------------------------
//  class UnoControlPatternFieldModel
//  ----------------------------------------------------
UnoControlPatternFieldModel::UnoControlPatternFieldModel()
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_EDITMASK );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_LITERALMASK );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_READONLY );
    ImplRegisterProperty( BASEPROPERTY_STRICTFORMAT );
    ImplRegisterProperty( BASEPROPERTY_TABSTOP );
    ImplRegisterProperty( BASEPROPERTY_TEXT );
    ImplRegisterProperty( BASEPROPERTY_TEXTCOLOR );
}

::rtl::OUString UnoControlPatternFieldModel::getServiceName() const
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlPatternFieldModel );
}

uno::Any UnoControlPatternFieldModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlPatternField );
        return aAny;
    }
    return UnoControlModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoControlPatternFieldModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>    aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoControlPatternFieldModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}


//  ----------------------------------------------------
//  class UnoPatternFieldControl
//  ----------------------------------------------------
UnoPatternFieldControl::UnoPatternFieldControl()
{
}

::rtl::OUString UnoPatternFieldControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "patternfield" );
}

void UnoPatternFieldControl::ImplSetPeerProperty( const ::rtl::OUString& rPropName, const uno::Any& rVal )
{
    sal_uInt16 nType = GetPropertyId( rPropName );
    if ( ( nType == BASEPROPERTY_TEXT ) || ( nType == BASEPROPERTY_EDITMASK ) || ( nType == BASEPROPERTY_LITERALMASK ) )
    {
        // Die Masken koennen nicht nacheinander gesetzt werden.
        ::rtl::OUString Text = ImplGetPropertyValue_UString( BASEPROPERTY_TEXT );
        ::rtl::OUString EditMask = ImplGetPropertyValue_UString( BASEPROPERTY_EDITMASK );
        ::rtl::OUString LiteralMask = ImplGetPropertyValue_UString( BASEPROPERTY_LITERALMASK );

        uno::Reference < awt::XPatternField >  xPF( mxPeer, uno::UNO_QUERY );
        if (xPF.is())
        {   // same comment as in UnoControl::ImplSetPeerProperty - see there
            xPF->setString( Text );
            xPF->setMasks( EditMask, LiteralMask );
        }
    }
    else
        UnoControl::ImplSetPeerProperty( rPropName, rVal );
}


// uno::XInterface
uno::Any UnoPatternFieldControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XPatternField*, this ) );
    return (aRet.hasValue() ? aRet : UnoEditControl::queryAggregation( rType ));
}

// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoPatternFieldControl )
    getCppuType( ( uno::Reference< awt::XPatternField>* ) NULL ),
    UnoEditControl::getTypes()
IMPL_XTYPEPROVIDER_END

void UnoPatternFieldControl::setString( const ::rtl::OUString& rString ) throw(uno::RuntimeException)
{
    setText( rString );
}

::rtl::OUString UnoPatternFieldControl::getString() throw(uno::RuntimeException)
{
    return getText();
}

void UnoPatternFieldControl::setMasks( const ::rtl::OUString& EditMask, const ::rtl::OUString& LiteralMask ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= EditMask;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_EDITMASK ), aAny, sal_True );
    aAny <<= LiteralMask;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_LITERALMASK ), aAny, sal_True );
}

void UnoPatternFieldControl::getMasks( ::rtl::OUString& EditMask, ::rtl::OUString& LiteralMask ) throw(uno::RuntimeException)
{
    EditMask = ImplGetPropertyValue_UString( BASEPROPERTY_EDITMASK );
    LiteralMask = ImplGetPropertyValue_UString( BASEPROPERTY_LITERALMASK );
}

void UnoPatternFieldControl::setStrictFormat( sal_Bool bStrict ) throw(uno::RuntimeException)
{
    uno::Any aAny;
    aAny <<= bStrict;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_STRICTFORMAT ), aAny, sal_True );
}

sal_Bool UnoPatternFieldControl::isStrictFormat() throw(uno::RuntimeException)
{
    return ImplGetPropertyValue_BOOL( BASEPROPERTY_STRICTFORMAT );
}



