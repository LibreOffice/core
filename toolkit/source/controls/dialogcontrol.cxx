/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dialogcontrol.cxx,v $
 * $Revision: 1.27 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wall.hxx>
#include <vos/mutex.hxx>
#include <toolkit/controls/dialogcontrol.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <toolkit/controls/unocontrols.hxx>
#include "toolkit/controls/formattedcontrol.hxx"
#include "toolkit/controls/roadmapcontrol.hxx"
#ifndef TOOLKIT_INC_TOOLKIT_CONTROLS_TKSCROLLBAR_HXX
#include "toolkit/controls/tkscrollbar.hxx"
#endif
#include <toolkit/controls/stdtabcontroller.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <tools/list.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <comphelper/types.hxx>

#include <comphelper/componentcontext.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/graph.hxx>
#include <vcl/image.hxx>

#include "tree/treecontrol.hxx"

#include <map>
#include <algorithm>
#include <functional>
#include "tools/urlobj.hxx"
#include "osl/file.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace toolkit;

#define PROPERTY_RESOURCERESOLVER ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceResolver" ))
#define PROPERTY_DIALOGSOURCEURL ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogSourceURL" ))
#define PROPERTY_IMAGEURL ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ImageURL" ))
#define PROPERTY_GRAPHIC ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Graphic" ))

//HELPER
::rtl::OUString getPhysicalLocation( const ::com::sun::star::uno::Any& rbase, const ::com::sun::star::uno::Any& rUrl );

struct LanguageDependentProp
{
    const char* pPropName;
    sal_Int32   nPropNameLength;
};

// ----------------------------------------------------------------------------
namespace
{
    static const Sequence< ::rtl::OUString >& lcl_getLanguageDependentProperties()
    {
        static Sequence< ::rtl::OUString > s_aLanguageDependentProperties;
        if ( s_aLanguageDependentProperties.getLength() == 0 )
        {
            ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
            if ( s_aLanguageDependentProperties.getLength() == 0 )
            {
                s_aLanguageDependentProperties.realloc( 2 );
                s_aLanguageDependentProperties[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HelpText" ) );
                s_aLanguageDependentProperties[1] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Title" ) );
                // note: properties must be sorted
            }
        }
        return s_aLanguageDependentProperties;
    }

    static uno::Reference< graphic::XGraphic > lcl_getGraphicFromURL_nothrow( const ::rtl::OUString& _rURL )
    {
        uno::Reference< graphic::XGraphic > xGraphic;
        if ( !_rURL.getLength() )
            return xGraphic;

        try
        {
            ::comphelper::ComponentContext aContext( ::comphelper::getProcessServiceFactory() );
            uno::Reference< graphic::XGraphicProvider > xProvider;
            if ( aContext.createComponent( "com.sun.star.graphic.GraphicProvider", xProvider ) )
            {
                uno::Sequence< beans::PropertyValue > aMediaProperties(1);
                aMediaProperties[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "URL" ) );
                aMediaProperties[0].Value <<= _rURL;
                xGraphic = xProvider->queryGraphic( aMediaProperties );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return xGraphic;
    }

}

// ----------------------------------------------------------------------------
// functor for disposing a control model
struct DisposeControlModel : public ::std::unary_function< Reference< XControlModel >, void >
{
    void operator()( Reference< XControlModel >& _rxModel )
    {
        try
        {
            ::comphelper::disposeComponent( _rxModel );
        }
        catch( const Exception& )
        {
            DBG_ERROR( "DisposeControlModel::(): caught an exception while disposing a component!" );
        }
    }
};

// ----------------------------------------------------------------------------
// functor for searching control model by name
struct FindControlModel : public ::std::unary_function< UnoControlDialogModel::UnoControlModelHolder, bool >
{
private:
    const ::rtl::OUString& m_rName;

public:
    FindControlModel( const ::rtl::OUString& _rName ) : m_rName( _rName ) { }

    bool operator()( const UnoControlDialogModel::UnoControlModelHolder& _rCompare )
    {
        return ( _rCompare.second == m_rName ) ? true : false;
    }
};

// ----------------------------------------------------------------------------
// functor for cloning a control model, and insertion into a target list
struct CloneControlModel : public ::std::unary_function< UnoControlDialogModel::UnoControlModelHolder, void >
{
private:
    UnoControlDialogModel::UnoControlModelHolderList&   m_rTargetList;

public:
    CloneControlModel( UnoControlDialogModel::UnoControlModelHolderList& _rTargetList )
        :m_rTargetList( _rTargetList )
    {
    }

    void operator()( const UnoControlDialogModel::UnoControlModelHolder& _rSource )
    {
        // clone the source object
        Reference< XCloneable > xCloneSource( _rSource.first, UNO_QUERY );
        Reference< XControlModel > xClone( xCloneSource->createClone(), UNO_QUERY );
        // add to target list
        m_rTargetList.push_back( UnoControlDialogModel::UnoControlModelHolder( xClone, _rSource.second ) );
    }
};

// ----------------------------------------------------------------------------
// functor for comparing a XControlModel with a given reference
struct CompareControlModel : public ::std::unary_function< UnoControlDialogModel::UnoControlModelHolder, bool >
{
private:
    Reference< XControlModel > m_xReference;
public:
    CompareControlModel( const Reference< XControlModel >& _rxReference ) : m_xReference( _rxReference ) { }

    bool operator()( const UnoControlDialogModel::UnoControlModelHolder& _rCompare )
    {
        return ( _rCompare.first.get() == m_xReference.get() ) ? true : false;
    }
};

// ----------------------------------------------------------------------------
static void lcl_throwIllegalArgumentException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw IllegalArgumentException();
}

// ----------------------------------------------------------------------------
static void lcl_throwNoSuchElementException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw NoSuchElementException();
}

// ----------------------------------------------------------------------------
static void lcl_throwElementExistException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw ElementExistException();
}

// ----------------------------------------------------------------------------
static const ::rtl::OUString& getTabIndexPropertyName( )
{
    static const ::rtl::OUString s_sTabIndexProperty( RTL_CONSTASCII_USTRINGPARAM( "TabIndex" ) );
    return s_sTabIndexProperty;
}

// ----------------------------------------------------------------------------
static const ::rtl::OUString& getStepPropertyName( )
{
    static const ::rtl::OUString s_sStepProperty( RTL_CONSTASCII_USTRINGPARAM( "Step" ) );
    return s_sStepProperty;
}

//  ----------------------------------------------------
//  class UnoControlDialogModel
//  ----------------------------------------------------
UnoControlDialogModel::UnoControlDialogModel()
    :maContainerListeners( *this )
    ,maChangeListeners ( GetMutex() )
    ,mbGroupsUpToDate( sal_False )
{
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
//  ImplRegisterProperty( BASEPROPERTY_BORDER );
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
//  ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_TITLE );
    ImplRegisterProperty( BASEPROPERTY_SIZEABLE );
    ImplRegisterProperty( BASEPROPERTY_DESKTOP_AS_PARENT );
    ImplRegisterProperty( BASEPROPERTY_DECORATION );
    ImplRegisterProperty( BASEPROPERTY_DIALOGSOURCEURL );
    ImplRegisterProperty( BASEPROPERTY_GRAPHIC );
    ImplRegisterProperty( BASEPROPERTY_IMAGEURL );

    Any aBool;
    aBool <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
}

UnoControlDialogModel::UnoControlDialogModel( const UnoControlDialogModel& rModel )
    : UnoControlDialogModel_IBase( rModel )
    , UnoControlDialogModel_Base( rModel )
    , maContainerListeners( *this )
    , maChangeListeners ( GetMutex() )
    , mbGroupsUpToDate( sal_False )
{
}

UnoControlDialogModel::~UnoControlDialogModel()
{
    maModels.clear();
    mbGroupsUpToDate = sal_False;
}

Any UnoControlDialogModel::queryAggregation( const Type & rType ) throw(RuntimeException)
{
    Any aRet( UnoControlDialogModel_IBase::queryInterface( rType ) );
    return (aRet.hasValue() ? aRet : UnoControlDialogModel_Base::queryAggregation( rType ));
}

// XTypeProvider
IMPL_IMPLEMENTATION_ID( UnoControlDialogModel )
Sequence< Type > UnoControlDialogModel::getTypes() throw(RuntimeException)
{
    return ::comphelper::concatSequences(
        UnoControlDialogModel_IBase::getTypes(),
        UnoControlDialogModel_Base::getTypes()
    );
}

::rtl::OUString UnoControlDialogModel::getServiceName( ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoControlDialogModel );
}

Any UnoControlDialogModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    Any aAny;

    switch ( nPropId )
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoControlDialog );
            break;
        default:
            aAny = UnoControlModel::ImplGetDefaultValue( nPropId );
    }

    return aAny;
}

::cppu::IPropertyArrayHelper& UnoControlDialogModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

void SAL_CALL UnoControlDialogModel::dispose(  ) throw(RuntimeException)
{
    // ====================================================================
    // tell our listeners
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = static_cast< XAggregation* >( static_cast< ::cppu::OWeakAggObject* >( this ) );

        maContainerListeners.disposeAndClear( aDisposeEvent );
        maChangeListeners.disposeAndClear( aDisposeEvent );
    }

    // ====================================================================
    // call the base class
    UnoControlModel::dispose();

    // ====================================================================
    // dispose our child models
    // for this, collect the models (we collect them from maModels, and this is modified when disposing children)
    ::std::vector< Reference< XControlModel > > aChildModels( maModels.size() );

    ::std::transform(
        maModels.begin(), maModels.end(),               // source range
        aChildModels.begin(),                           // target location
        ::std::select1st< UnoControlModelHolder >( )    // operation to apply -> select the XControlModel part
    );

    // now dispose
    ::std::for_each( aChildModels.begin(), aChildModels.end(), DisposeControlModel() );
    aChildModels.clear();

    mbGroupsUpToDate = sal_False;
}

// XMultiPropertySet
Reference< XPropertySetInfo > UnoControlDialogModel::getPropertySetInfo(  ) throw(RuntimeException)
{
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

UnoControlModel* UnoControlDialogModel::Clone() const
{
    // clone the container itself
    UnoControlDialogModel* pClone = new UnoControlDialogModel( *this );

    // clone all children
    ::std::for_each(
        maModels.begin(), maModels.end(),
        CloneControlModel( pClone->maModels )
    );

    return pClone;
}

UnoControlDialogModel::UnoControlModelHolderList::iterator UnoControlDialogModel::ImplFindElement( const ::rtl::OUString& rName )
{
    return ::std::find_if( maModels.begin(), maModels.end(), FindControlModel( rName ) );
}

// ::XMultiServiceFactory
Reference< XInterface > UnoControlDialogModel::createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(Exception, RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

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
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoControlFixedHyperlinkModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFixedHyperlinkModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoControlFixedTextModel ) == 0 )
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
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlProgressBarModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlProgressBarModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlScrollBarModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlScrollBarModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlFixedLineModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFixedLineModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlRoadmapModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlRoadmapModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName_TreeControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoTreeModel >;

    if ( !pNewModel )
    {
        Reference< XMultiServiceFactory > xORB( ::comphelper::getProcessServiceFactory() );
        if ( xORB.is() )
        {
            Reference< XInterface > xObject = xORB->createInstance( aServiceSpecifier );
            Reference< XServiceInfo > xSI( xObject, UNO_QUERY );
            Reference< XCloneable > xCloneAccess( xSI, UNO_QUERY );
            Reference< XAggregation > xAgg( xCloneAccess, UNO_QUERY );
            if ( xAgg.is() )
            {
                if ( xSI->supportsService( ::rtl::OUString::createFromAscii( "com.sun.star.awt.UnoControlModel" ) ) )
                {
                    // release 3 of the 4 references we have to the object
                    xAgg.clear();
                    xSI.clear();
                    xObject.clear();

                    pNewModel = new OCommonGeometryControlModel( xCloneAccess, aServiceSpecifier );
                }
            }
        }
    }

    Reference< XInterface > xNewModel = (::cppu::OWeakObject*)pNewModel;
    return xNewModel;
}

Reference< XInterface > UnoControlDialogModel::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const Sequence< Any >& /* Arguments */ ) throw(Exception, RuntimeException)
{
    return createInstance( ServiceSpecifier );
}

Sequence< ::rtl::OUString > UnoControlDialogModel::getAvailableServiceNames() throw(RuntimeException)
{
    static Sequence< ::rtl::OUString >* pNamesSeq = NULL;
    if ( !pNamesSeq )
    {
        pNamesSeq = new Sequence< ::rtl::OUString >( 21 );
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
        pNames[16] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlProgressBarModel );
        pNames[17] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlScrollBarModel );
        pNames[18] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlFixedLineModel );
        pNames[19] = ::rtl::OUString::createFromAscii( szServiceName2_UnoControlRoadmapModel );
        pNames[20] = ::rtl::OUString::createFromAscii( szServiceName_TreeControlModel );

    }
    return *pNamesSeq;
}

// XContainer
void UnoControlDialogModel::addContainerListener( const Reference< XContainerListener >& l ) throw(RuntimeException)
{
    maContainerListeners.addInterface( l );
}

void UnoControlDialogModel::removeContainerListener( const Reference< XContainerListener >& l ) throw(RuntimeException)
{
    maContainerListeners.removeInterface( l );
}

// XElementAcces
Type UnoControlDialogModel::getElementType() throw(RuntimeException)
{
    Type aType = getCppuType( ( Reference< XControlModel>* ) NULL );
    return aType;
}

sal_Bool UnoControlDialogModel::hasElements() throw(RuntimeException)
{
    return !maModels.empty();
}

// XNameContainer, XNameReplace, XNameAccess
void UnoControlDialogModel::replaceByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    Reference< XControlModel > xNewModel;
    aElement >>= xNewModel;
    if ( !xNewModel.is() )
        lcl_throwIllegalArgumentException();

    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() == aElementPos )
        lcl_throwNoSuchElementException();

    // stop listening at the old model
    stopControlListening( aElementPos->first );
    Reference< XControlModel > xReplaced( aElementPos->first );
    // remember the new model, and start listening
    aElementPos->first = xNewModel;
    startControlListening( xNewModel );

    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element = aElement;
    aEvent.ReplacedElement <<= xReplaced;
    aEvent.Accessor <<= aName;

    // notify the container listener
    maContainerListeners.elementReplaced( aEvent );

    // our "tab controller model" has potentially changed -> notify this
    implNotifyTabModelChange( aName );
}

Any UnoControlDialogModel::getByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() == aElementPos )
        lcl_throwNoSuchElementException();

    return makeAny( aElementPos->first );
}

Sequence< ::rtl::OUString > UnoControlDialogModel::getElementNames() throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aNames( maModels.size() );

    ::std::transform(
        maModels.begin(), maModels.end(),               // source range
        aNames.getArray(),                              // target range
        ::std::select2nd< UnoControlModelHolder >()     // operator to apply: select the second element (the name)
    );

    return aNames;
}

sal_Bool UnoControlDialogModel::hasByName( const ::rtl::OUString& aName ) throw(RuntimeException)
{
    return maModels.end() != ImplFindElement( aName );
}

void UnoControlDialogModel::insertByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    Reference< XControlModel > xM;
    aElement >>= xM;

    if ( xM.is() )
    {
        Reference< beans::XPropertySet > xProps( xM, UNO_QUERY );
            if ( xProps.is() )
            {

                Reference< beans::XPropertySetInfo > xPropInfo = xProps.get()->getPropertySetInfo();

                ::rtl::OUString sImageSourceProperty = GetPropertyName( BASEPROPERTY_IMAGEURL );
                if ( xPropInfo.get()->hasPropertyByName(  sImageSourceProperty ))
                {
                    Any aUrl = xProps.get()->getPropertyValue(  sImageSourceProperty );

                    ::rtl::OUString absoluteUrl =
                        getPhysicalLocation( getPropertyValue( GetPropertyName( BASEPROPERTY_DIALOGSOURCEURL ) ), aUrl );

                    aUrl <<= absoluteUrl;

                    xProps.get()->setPropertyValue(  sImageSourceProperty , aUrl );
                }
            }
    }



    if ( !aName.getLength() || !xM.is() )
        lcl_throwIllegalArgumentException();

    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() != aElementPos )
        lcl_throwElementExistException();

    maModels.push_back( UnoControlModelHolder( xM, aName ) );
    mbGroupsUpToDate = sal_False;
    startControlListening( xM );

    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element <<= aElement;
    aEvent.Accessor <<= aName;
    maContainerListeners.elementInserted( aEvent );

    // our "tab controller model" has potentially changed -> notify this
    implNotifyTabModelChange( aName );
}

void UnoControlDialogModel::removeByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() == aElementPos )
        lcl_throwNoSuchElementException();

    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element <<= aElementPos->first;
    aEvent.Accessor <<= aName;
    maContainerListeners.elementRemoved( aEvent );

    stopControlListening( aElementPos->first );
    Reference< XPropertySet > xPS( aElementPos->first, UNO_QUERY );
    maModels.erase( aElementPos );
    mbGroupsUpToDate = sal_False;

    if ( xPS.is() )
        try
        {
            xPS->setPropertyValue( PROPERTY_RESOURCERESOLVER, makeAny( Reference< resource::XStringResourceResolver >() ) );
        }
        catch( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }

    // our "tab controller model" has potentially changed -> notify this
    implNotifyTabModelChange( aName );
}

// ----------------------------------------------------------------------------
sal_Bool SAL_CALL UnoControlDialogModel::getGroupControl(  ) throw (RuntimeException)
{
    return sal_True;
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::setGroupControl( sal_Bool ) throw (RuntimeException)
{
    DBG_ERROR( "UnoControlDialogModel::setGroupControl: explicit grouping not supported" );
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::setControlModels( const Sequence< Reference< XControlModel > >& _rControls ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    // set the tab indexes according to the order of models in the sequence
    const Reference< XControlModel >* pControls = _rControls.getConstArray( );
    const Reference< XControlModel >* pControlsEnd = _rControls.getConstArray( ) + _rControls.getLength();

    sal_Int16 nTabIndex = 1;

    for ( ; pControls != pControlsEnd; ++pControls )
    {
        // look up the control in our own structure. This is to prevent invalid arguments
        UnoControlModelHolderList::const_iterator aPos =
            ::std::find_if(
                maModels.begin(), maModels.end(),
                CompareControlModel( *pControls )
            );
        if ( maModels.end() != aPos )
        {
            // okay, this is an existent model
            // now set the TabIndex property (if applicable)
            Reference< XPropertySet > xProps( aPos->first, UNO_QUERY );
            Reference< XPropertySetInfo > xPSI;
            if ( xProps.is() )
                xPSI = xProps->getPropertySetInfo();
            if ( xPSI.is() && xPSI->hasPropertyByName( getTabIndexPropertyName() ) )
                xProps->setPropertyValue( getTabIndexPropertyName(), makeAny( nTabIndex++ ) );
        }
        mbGroupsUpToDate = sal_False;
    }
}


typedef ::std::multimap< sal_Int32, Reference< XControlModel >, ::std::less< sal_Int32 > > MapIndexToModel;

// ----------------------------------------------------------------------------
Sequence< Reference< XControlModel > > SAL_CALL UnoControlDialogModel::getControlModels(  ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    MapIndexToModel aSortedModels;
        // will be the sorted container of all models which have a tab index property
    ::std::vector< Reference< XControlModel > > aUnindexedModels;
        // will be the container of all models which do not have a tab index property

    UnoControlModelHolderList::const_iterator aLoop = maModels.begin();
    for ( ; aLoop != maModels.end(); ++aLoop )
    {
        Reference< XControlModel > xModel( aLoop->first );

        // see if the model has a TabIndex property
        Reference< XPropertySet > xControlProps( xModel, UNO_QUERY );
        Reference< XPropertySetInfo > xPSI;
        if ( xControlProps.is() )
            xPSI = xControlProps->getPropertySetInfo( );
        DBG_ASSERT( xPSI.is(), "UnoControlDialogModel::getControlModels: invalid child model!" );

        // has it?
        if ( xPSI.is() && xPSI->hasPropertyByName( getTabIndexPropertyName() ) )
        {   // yes
            sal_Int32 nTabIndex = -1;
            xControlProps->getPropertyValue( getTabIndexPropertyName() ) >>= nTabIndex;

            aSortedModels.insert( MapIndexToModel::value_type( nTabIndex, xModel ) );
        }
        else if ( xModel.is() )
            // no, it hasn't, but we have to include it, anyway
            aUnindexedModels.push_back( xModel );
    }

    // okay, here we have a container of all our models, sorted by tab index,
    // plus a container of "unindexed" models
    // -> merge them
    Sequence< Reference< XControlModel > > aReturn( aUnindexedModels.size() + aSortedModels.size() );
    ::std::transform(
            aSortedModels.begin(), aSortedModels.end(),
            ::std::copy( aUnindexedModels.begin(), aUnindexedModels.end(), aReturn.getArray() ),
            ::std::select2nd< MapIndexToModel::value_type >( )
        );

    return aReturn;
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::setGroup( const Sequence< Reference< XControlModel > >&, const ::rtl::OUString& ) throw (RuntimeException)
{
    // not supported. We have only implicit grouping:
    // We only have a sequence of control models, and we _know_ (yes, that's a HACK relying on
    // implementation details) that VCL does grouping according to the order of controls automatically
    // At least VCL does this for all we're interested in: Radio buttons.
    DBG_ERROR( "UnoControlDialogModel::setGroup: grouping not supported" );
}

// ----------------------------------------------------------------------------
namespace
{
    enum GroupingMachineState
    {
        eLookingForGroup,
        eExpandingGroup
    };

    // ........................................................................
    static sal_Int32 lcl_getDialogStep( const Reference< XControlModel >& _rxModel )
    {
        sal_Int32 nStep = 0;
        try
        {
            Reference< XPropertySet > xModelProps( _rxModel, UNO_QUERY );
            xModelProps->getPropertyValue( getStepPropertyName() ) >>= nStep;
        }
        catch( const Exception& )
        {
            DBG_ERROR( "lcl_getDialogStep: caught an exception while determining the dialog page!" );
        }
        return nStep;
    }
}

// ----------------------------------------------------------------------------
sal_Int32 SAL_CALL UnoControlDialogModel::getGroupCount(  ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    implUpdateGroupStructure();

    return maGroups.size();
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::getGroup( sal_Int32 _nGroup, Sequence< Reference< XControlModel > >& _rGroup, ::rtl::OUString& _rName ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    implUpdateGroupStructure();

    if ( ( _nGroup < 0 ) || ( _nGroup >= (sal_Int32)maGroups.size() ) )
    {
        DBG_ERROR( "UnoControlDialogModel::getGroup: invalid argument and I am not allowed to throw an exception!" );
        _rGroup.realloc( 0 );
        _rName = ::rtl::OUString();
    }
    else
    {
        AllGroups::const_iterator aGroupPos = maGroups.begin() + _nGroup;
        _rGroup.realloc( aGroupPos->size() );
        // copy the models
        ::std::copy( aGroupPos->begin(), aGroupPos->end(), _rGroup.getArray() );
        // give the group a name
        _rName = ::rtl::OUString::valueOf( _nGroup );
    }
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::getGroupByName( const ::rtl::OUString& _rName, Sequence< Reference< XControlModel > >& _rGroup ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    ::rtl::OUString sDummyName;
    getGroup( _rName.toInt32( ), _rGroup, sDummyName );
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::addChangesListener( const Reference< XChangesListener >& _rxListener ) throw (RuntimeException)
{
    maChangeListeners.addInterface( _rxListener );
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::removeChangesListener( const Reference< XChangesListener >& _rxListener ) throw (RuntimeException)
{
    maChangeListeners.removeInterface( _rxListener );
}

// ----------------------------------------------------------------------------
void UnoControlDialogModel::implNotifyTabModelChange( const ::rtl::OUString& _rAccessor )
{
    // multiplex to our change listeners:
    // the changes event
    ChangesEvent aEvent;
    aEvent.Source = *this;
    aEvent.Base <<= aEvent.Source;  // the "base of the changes root" is also ourself
    aEvent.Changes.realloc( 1 );    // exactly one change
    aEvent.Changes[ 0 ].Accessor <<= _rAccessor;


    Sequence< Reference< XInterface > > aChangeListeners( maChangeListeners.getElements() );
    const Reference< XInterface >* pListener = aChangeListeners.getConstArray();
    const Reference< XInterface >* pListenerEnd = aChangeListeners.getConstArray() + aChangeListeners.getLength();
    for ( ; pListener != pListenerEnd; ++pListener )
    {
        if ( pListener->is() )
            static_cast< XChangesListener* >( pListener->get() )->changesOccurred( aEvent );
    }
}


// ----------------------------------------------------------------------------
void UnoControlDialogModel::implUpdateGroupStructure()
{
    if ( mbGroupsUpToDate )
        // nothing to do
        return;

    // conditions for a group:
    // * all elements of the group are radio buttons
    // * all elements of the group are on the same dialog page
    // * in the overall control order (determined by the tab index), all elements are subsequent

    maGroups.clear();

    Sequence< Reference< XControlModel > > aControlModels = getControlModels();
    const Reference< XControlModel >* pControlModels = aControlModels.getConstArray();
    const Reference< XControlModel >* pControlModelsEnd = pControlModels + aControlModels.getLength();

    // in extreme we have as much groups as controls
    maGroups.reserve( aControlModels.getLength() );

    GroupingMachineState eState = eLookingForGroup;     // the current state of our machine
    Reference< XServiceInfo > xModelSI;                 // for checking for a radion button
    AllGroups::iterator aCurrentGroup = maGroups.end(); // the group which we're currently building
    sal_Int32   nCurrentGroupStep = -1;                 // the step which all controls of the current group belong to
    sal_Bool    bIsRadioButton;                         // is it a radio button?

#if OSL_DEBUG_LEVEL > 1
    ::std::vector< ::rtl::OUString > aCurrentGroupLabels;
#endif

    for ( ; pControlModels != pControlModelsEnd; ++pControlModels )
    {
        // we'll need this in every state
        xModelSI = xModelSI.query( *pControlModels );
        bIsRadioButton = xModelSI.is() && xModelSI->supportsService( ::rtl::OUString::createFromAscii( szServiceName2_UnoControlRadioButtonModel ) );

        switch ( eState )
        {
            case eLookingForGroup:
            {
                if ( !bIsRadioButton )
                    // this is no radio button -> still looking for the beginning of a group
                    continue;
                // the current model is a radio button
                // -> we found the beginning of a new group
                // create the place for this group
                size_t nGroups = maGroups.size();
                maGroups.resize( nGroups + 1 );
                aCurrentGroup = maGroups.begin() + nGroups;
                // and add the (only, til now) member
                aCurrentGroup->push_back( *pControlModels );

                // get the step which all controls of this group now have to belong to
                nCurrentGroupStep = lcl_getDialogStep( *pControlModels );
                // new state: looking for further members
                eState = eExpandingGroup;

#if OSL_DEBUG_LEVEL > 1
                Reference< XPropertySet > xModelProps( *pControlModels, UNO_QUERY );
                ::rtl::OUString sLabel;
                if ( xModelProps.is() && xModelProps->getPropertySetInfo().is() && xModelProps->getPropertySetInfo()->hasPropertyByName( ::rtl::OUString::createFromAscii( "Label" ) ) )
                    xModelProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Label" ) ) >>= sLabel;
                aCurrentGroupLabels.push_back( sLabel );
#endif
            }
            break;

            case eExpandingGroup:
            {
                if ( !bIsRadioButton )
                {   // no radio button -> the group is done
                    aCurrentGroup = maGroups.end();
                    eState = eLookingForGroup;
#if OSL_DEBUG_LEVEL > 1
                    aCurrentGroupLabels.clear();
#endif
                    continue;
                }

                // it is a radio button - is it on the proper page?
                const sal_Int32 nThisModelStep = lcl_getDialogStep( *pControlModels );
                if  (   ( nThisModelStep == nCurrentGroupStep ) // the current button is on the same dialog page
                    ||  ( 0 == nThisModelStep )                 // the current button appears on all pages
                    )
                {
                    // -> it belongs to the same group
                    aCurrentGroup->push_back( *pControlModels );
                    // state still is eExpandingGroup - we're looking for further elements
                    eState = eExpandingGroup;

#if OSL_DEBUG_LEVEL > 1
                    Reference< XPropertySet > xModelProps( *pControlModels, UNO_QUERY );
                    ::rtl::OUString sLabel;
                    if ( xModelProps.is() && xModelProps->getPropertySetInfo().is() && xModelProps->getPropertySetInfo()->hasPropertyByName( ::rtl::OUString::createFromAscii( "Label" ) ) )
                        xModelProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Label" ) ) >>= sLabel;
                    aCurrentGroupLabels.push_back( sLabel );
#endif
                    continue;
                }

                // it's a radio button, but on a different page
                // -> we open a new group for it

                // close the old group
                aCurrentGroup = maGroups.end();
#if OSL_DEBUG_LEVEL > 1
                aCurrentGroupLabels.clear();
#endif

                // open a new group
                size_t nGroups = maGroups.size();
                maGroups.resize( nGroups + 1 );
                aCurrentGroup = maGroups.begin() + nGroups;
                // and add the (only, til now) member
                aCurrentGroup->push_back( *pControlModels );

                nCurrentGroupStep = nThisModelStep;

                // state is the same: we still are looking for further elements of the current group
                eState = eExpandingGroup;
#if OSL_DEBUG_LEVEL > 1
                Reference< XPropertySet > xModelProps( *pControlModels, UNO_QUERY );
                ::rtl::OUString sLabel;
                if ( xModelProps.is() && xModelProps->getPropertySetInfo().is() && xModelProps->getPropertySetInfo()->hasPropertyByName( ::rtl::OUString::createFromAscii( "Label" ) ) )
                    xModelProps->getPropertyValue( ::rtl::OUString::createFromAscii( "Label" ) ) >>= sLabel;
                aCurrentGroupLabels.push_back( sLabel );
#endif
            }
            break;
        }
    }

    mbGroupsUpToDate = sal_True;
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::propertyChange( const PropertyChangeEvent& _rEvent ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    DBG_ASSERT( 0 == _rEvent.PropertyName.compareToAscii( "TabIndex" ),
        "UnoControlDialogModel::propertyChange: not listening for this property!" );

    // the accessor for the changed element
    ::rtl::OUString sAccessor;
    UnoControlModelHolderList::const_iterator aPos =
        ::std::find_if(
            maModels.begin(), maModels.end(),
            CompareControlModel( Reference< XControlModel >( _rEvent.Source, UNO_QUERY ) )
        );
    OSL_ENSURE( maModels.end() != aPos, "UnoControlDialogModel::propertyChange: don't know this model!" );
    if ( maModels.end() != aPos )
        sAccessor = aPos->second;

    // our groups are not up-to-date
    mbGroupsUpToDate = sal_False;

    // notify
    implNotifyTabModelChange( sAccessor );
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::disposing( const EventObject& /*rEvent*/ ) throw (RuntimeException)
{
}

// ----------------------------------------------------------------------------
void UnoControlDialogModel::startControlListening( const Reference< XControlModel >& _rxChildModel )
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    Reference< XPropertySet > xModelProps( _rxChildModel, UNO_QUERY );
    Reference< XPropertySetInfo > xPSI;
    if ( xModelProps.is() )
        xPSI = xModelProps->getPropertySetInfo();

    if ( xPSI.is() && xPSI->hasPropertyByName( getTabIndexPropertyName() ) )
        xModelProps->addPropertyChangeListener( getTabIndexPropertyName(), this );
}

// ----------------------------------------------------------------------------
void UnoControlDialogModel::stopControlListening( const Reference< XControlModel >& _rxChildModel )
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    Reference< XPropertySet > xModelProps( _rxChildModel, UNO_QUERY );
    Reference< XPropertySetInfo > xPSI;
    if ( xModelProps.is() )
        xPSI = xModelProps->getPropertySetInfo();

    if ( xPSI.is() && xPSI->hasPropertyByName( getTabIndexPropertyName() ) )
        xModelProps->removePropertyChangeListener( getTabIndexPropertyName(), this );
}

// ============================================================================
// = class ResourceListener
// ============================================================================

ResourceListener::ResourceListener(
    const Reference< util::XModifyListener >& rListener ) :
    OWeakObject(),
    m_xListener( rListener ),
    m_bListening( false )
{
}

ResourceListener::~ResourceListener()
{
}

// XInterface
Any SAL_CALL ResourceListener::queryInterface( const Type& rType )
throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                static_cast< XModifyListener* >( this ),
                static_cast< XEventListener* >( this ));

    if ( a.hasValue() )
        return a;

    return OWeakObject::queryInterface( rType );
}

void SAL_CALL ResourceListener::acquire() throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL ResourceListener::release() throw ()
{
    OWeakObject::release();
}

void ResourceListener::startListening(
    const Reference< resource::XStringResourceResolver  >& rResource )
{
    Reference< util::XModifyBroadcaster > xModifyBroadcaster( rResource, UNO_QUERY );

    {
        // --- SAFE ---
        ::osl::ResettableGuard < ::osl::Mutex > aGuard( m_aMutex );
        bool bListening( m_bListening );
        bool bResourceSet( m_xResource.is() );
        aGuard.clear();
        // --- SAFE ---

        if ( bListening && bResourceSet )
            stopListening();

        // --- SAFE ---
        aGuard.reset();
        m_xResource = rResource;
        aGuard.clear();
        // --- SAFE ---
    }

    Reference< util::XModifyListener > xThis( static_cast<OWeakObject*>( this ), UNO_QUERY );
    if ( xModifyBroadcaster.is() )
    {
        try
        {
            xModifyBroadcaster->addModifyListener( xThis );

            // --- SAFE ---
            ::osl::ResettableGuard < ::osl::Mutex > aGuard( m_aMutex );
            m_bListening = true;
            // --- SAFE ---
        }
        catch ( RuntimeException& )
        {
            throw;
        }
        catch ( Exception& )
        {
        }
    }
}

void ResourceListener::stopListening()
{
    Reference< util::XModifyBroadcaster > xModifyBroadcaster;

    // --- SAFE ---
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( m_aMutex );
    if ( m_bListening && m_xResource.is() )
        xModifyBroadcaster = Reference< util::XModifyBroadcaster >( m_xResource, UNO_QUERY );
    aGuard.clear();
    // --- SAFE ---

    Reference< util::XModifyListener > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
    if ( xModifyBroadcaster.is() )
    {
        try
        {
            // --- SAFE ---
            aGuard.reset();
            m_bListening = false;
            m_xResource.clear();
            aGuard.clear();
            // --- SAFE ---

            xModifyBroadcaster->removeModifyListener( xThis );
        }
        catch ( RuntimeException& )
        {
            throw;
        }
        catch ( Exception& )
        {
        }
    }
}

// XModifyListener
void SAL_CALL ResourceListener::modified(
    const lang::EventObject& aEvent )
throw ( RuntimeException )
{
    Reference< util::XModifyListener > xListener;

    // --- SAFE ---
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( m_aMutex );
    xListener = m_xListener;
    aGuard.clear();
    // --- SAFE ---

    if ( xListener.is() )
    {
        try
        {
            xListener->modified( aEvent );
        }
        catch ( RuntimeException& )
        {
            throw;
        }
        catch ( Exception& )
        {
        }
    }
}

// XEventListener
void SAL_CALL ResourceListener::disposing(
    const EventObject& Source )
throw ( RuntimeException )
{
    Reference< lang::XEventListener > xListener;
    Reference< resource::XStringResourceResolver > xResource;

    // --- SAFE ---
    ::osl::ResettableGuard < ::osl::Mutex > aGuard( m_aMutex );
    Reference< XInterface > xIfacRes( m_xResource, UNO_QUERY );
    Reference< XInterface > xIfacList( m_xListener, UNO_QUERY );
    aGuard.clear();
    // --- SAFE ---

    if ( Source.Source == xIfacRes )
    {
        // --- SAFE ---
        aGuard.reset();
        m_bListening = false;
        xResource = m_xResource;
        xListener = Reference< lang::XEventListener >( m_xListener, UNO_QUERY );
        m_xResource.clear();
        aGuard.clear();
        // --- SAFE ---

        if ( xListener.is() )
        {
            try
            {
                xListener->disposing( Source );
            }
            catch ( RuntimeException& )
            {
                throw;
            }
            catch ( Exception& )
            {
            }
        }
    }
    else if ( Source.Source == xIfacList )
    {
        // --- SAFE ---
        aGuard.reset();
        m_bListening = false;
        xListener = Reference< lang::XEventListener >( m_xListener, UNO_QUERY );
        xResource = m_xResource;
        m_xResource.clear();
        m_xListener.clear();
        aGuard.clear();
        // --- SAFE ---

        // Remove ourself as listener from resource resolver
        Reference< util::XModifyBroadcaster > xModifyBroadcaster( xResource, UNO_QUERY );
        Reference< util::XModifyListener > xThis( static_cast< OWeakObject* >( this ), UNO_QUERY );
        if ( xModifyBroadcaster.is() )
        {
            try
            {
                xModifyBroadcaster->removeModifyListener( xThis );
            }
            catch ( RuntimeException& )
            {
                throw;
            }
            catch ( Exception& )
            {
            }
        }
    }
}

// ============================================================================
// = class UnoDialogControl
// ============================================================================

UnoDialogControl::UnoDialogControl() :
    maTopWindowListeners( *this ),
    mbWindowListener(false),
    mbSizeModified(false),
    mbPosModified(false)
{
    maComponentInfos.nWidth = 300;
    maComponentInfos.nHeight = 450;
    mxListener = new ResourceListener( Reference< util::XModifyListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
}

::rtl::OUString UnoDialogControl::GetComponentServiceName()
{

    sal_Bool bDecoration( sal_True );
    ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DECORATION )) >>= bDecoration;
    if ( bDecoration )
        return ::rtl::OUString::createFromAscii( "Dialog" );
    else
        return ::rtl::OUString::createFromAscii( "TabPage" );
}

// XInterface
Any UnoDialogControl::queryAggregation( const Type & rType ) throw(RuntimeException)
{
    Any aRet( UnoDialogControl_IBase::queryInterface( rType ) );
    return (aRet.hasValue() ? aRet : UnoControlContainer::queryAggregation( rType ));
}

// XTypeProvider
IMPL_IMPLEMENTATION_ID( UnoDialogControl )
Sequence< Type > UnoDialogControl::getTypes() throw(RuntimeException)
{
    return ::comphelper::concatSequences(
        UnoDialogControl_IBase::getTypes(),
        UnoControlContainer::getTypes()
    );
}

void UnoDialogControl::ImplInsertControl( Reference< XControlModel >& rxModel, const ::rtl::OUString& rName )
{
    Reference< XPropertySet > xP( rxModel, UNO_QUERY );

    ::rtl::OUString aDefCtrl;
    xP->getPropertyValue( GetPropertyName( BASEPROPERTY_DEFAULTCONTROL ) ) >>= aDefCtrl;

    // Add our own resource resolver to a newly created control
    Reference< resource::XStringResourceResolver > xStringResourceResolver;
    rtl::OUString aPropName( PROPERTY_RESOURCERESOLVER );

    Any aAny;
    ImplGetPropertyValue( aPropName ) >>= xStringResourceResolver;

    aAny <<= xStringResourceResolver;
    xP->setPropertyValue( aPropName, aAny );

    Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
    Reference < XControl > xCtrl( xMSF->createInstance( aDefCtrl ), UNO_QUERY );

    DBG_ASSERT( xCtrl.is(), "UnoDialogControl::ImplInsertControl: could not create the control!" );
    if ( xCtrl.is() )
    {
        xCtrl->setModel( rxModel );
        addControl( rName, xCtrl );
            // will implicitly call addingControl, where we can add the PropertiesChangeListener to the model
            // (which we formerly did herein)
            // 08.01.2001 - 96008 - fs@openoffice.org

        ImplSetPosSize( xCtrl );
    }
}

void UnoDialogControl::ImplRemoveControl( Reference< XControlModel >& rxModel )
{
    Sequence< Reference< XControl > > aControls = getControls();
    Reference< XControl > xCtrl = StdTabController::FindControl( aControls, rxModel );
    if ( xCtrl.is() )
        removeControl( xCtrl );
}

void UnoDialogControl::ImplSetPosSize( Reference< XControl >& rxCtrl )
{
    Reference< XPropertySet > xP( rxCtrl->getModel(), UNO_QUERY );

    sal_Int32 nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ) ) >>= nX;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ) ) >>= nY;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nWidth;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nHeight;

    // Currentley we are simply using MAP_APPFONT
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    DBG_ASSERT( pOutDev, "Missing Default Device!" );
    if ( pOutDev )
    {
        ::Size aTmp( nX, nY );
        aTmp = pOutDev->LogicToPixel( aTmp, MAP_APPFONT );
        nX = aTmp.Width();
        nY = aTmp.Height();
        aTmp = ::Size( nWidth, nHeight );
        aTmp = pOutDev->LogicToPixel( aTmp, MAP_APPFONT );
        nWidth = aTmp.Width();
        nHeight = aTmp.Height();
    }
    else
    {
        Reference< XWindowPeer > xPeer = ImplGetCompatiblePeer( sal_True );
        Reference< XDevice > xD( xPeer, UNO_QUERY );

        SimpleFontMetric aFM;
        FontDescriptor aFD;
        Any aVal = ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_FONTDESCRIPTOR ) );
        aVal >>= aFD;
        if ( aFD.StyleName.getLength() )
        {
            Reference< XFont > xFont = xD->getFont( aFD );
            aFM = xFont->getFontMetric();
        }
        else
        {
            Reference< XGraphics > xG = xD->createGraphics();
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
    Reference < XWindow > xW( rxCtrl, UNO_QUERY );
    xW->setPosSize( nX, nY, nWidth, nHeight, PosSize::POSSIZE );
}

void UnoDialogControl::dispose() throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >( this );
    maTopWindowListeners.disposeAndClear( aEvt );

    // Notify our listener helper about dispose
    // --- SAFE ---
    ::osl::ResettableGuard< ::osl::Mutex > aGuard( GetMutex() );
    Reference< XEventListener > xListener( mxListener, UNO_QUERY );
    mxListener.clear();
    aGuard.clear();
    // --- SAFE ---

    if ( xListener.is() )
        xListener->disposing( aEvt );

    UnoControlContainer::dispose();
}

void SAL_CALL UnoDialogControl::disposing(
    const EventObject& Source )
throw(RuntimeException)
{
    rtl::OUString aPropName( PROPERTY_RESOURCERESOLVER );
    Reference< resource::XStringResourceResolver > xStringResourceResolver;

    ImplGetPropertyValue( aPropName ) >>= xStringResourceResolver;
    Reference< XInterface > xIfac( xStringResourceResolver, UNO_QUERY );

    if ( Source.Source == xIfac )
    {
        Any aAny;

        // Reset resource resolver reference
        ImplSetPropertyValue( aPropName, aAny, sal_True );
        ImplUpdateResourceResolver();
    }
    else
    {
        UnoControlContainer::disposing( Source );
    }
}

sal_Bool UnoDialogControl::setModel( const Reference< XControlModel >& rxModel ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    // destroy the old tab controller, if existent
    if ( mxTabController.is() )
    {
        mxTabController->setModel( NULL );                  // just to be sure, should not be necessary
        removeTabController( mxTabController );
        ::comphelper::disposeComponent( mxTabController );  // just to be sure, should not be necessary
        mxTabController.clear();
    }

    if ( getModel().is() )
    {
        Sequence< Reference< XControl > > aControls = getControls();
        const Reference< XControl >* pCtrls = aControls.getConstArray();
        const Reference< XControl >* pCtrlsEnd = pCtrls + aControls.getLength();

        for ( ; pCtrls < pCtrlsEnd; ++pCtrls )
            removeControl( *pCtrls );
                // will implicitly call removingControl, which will remove the PropertyChangeListener
                // (which we formerly did herein)
                // 08.01.2001 - 96008 - fs@openoffice.org

        Reference< XContainer > xC( getModel(), UNO_QUERY );
        if ( xC.is() )
            xC->removeContainerListener( this );

        Reference< XChangesNotifier > xChangeNotifier( getModel(), UNO_QUERY );
        if ( xChangeNotifier.is() )
            xChangeNotifier->removeChangesListener( this );
    }

    sal_Bool bRet = UnoControl::setModel( rxModel );

    if ( getModel().is() )
    {
        Reference< XNameAccess > xNA( getModel(), UNO_QUERY );
        if ( xNA.is() )
        {
            Sequence< ::rtl::OUString > aNames = xNA->getElementNames();
            const ::rtl::OUString* pNames = aNames.getConstArray();
            sal_uInt32 nCtrls = aNames.getLength();

            Reference< XControlModel > xCtrlModel;
            for( sal_uInt32 n = 0; n < nCtrls; ++n, ++pNames )
            {
                xNA->getByName( *pNames ) >>= xCtrlModel;
                ImplInsertControl( xCtrlModel, *pNames );
            }
        }

        Reference< XContainer > xC( getModel(), UNO_QUERY );
        if ( xC.is() )
            xC->addContainerListener( this );

        Reference< XChangesNotifier > xChangeNotifier( getModel(), UNO_QUERY );
        if ( xChangeNotifier.is() )
            xChangeNotifier->addChangesListener( this );
    }

    Reference< XTabControllerModel > xTabbing( getModel(), UNO_QUERY );
    if ( xTabbing.is() )
    {
        mxTabController = new StdTabController;
        mxTabController->setModel( xTabbing );
        addTabController( mxTabController );
    }
    ImplStartListingForResourceEvents();

    return bRet;
}

void UnoDialogControl::setDesignMode( sal_Bool bOn ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    UnoControl::setDesignMode( bOn );

    Sequence< Reference< XControl > > xCtrls = getControls();
    sal_Int32 nControls = xCtrls.getLength();
    Reference< XControl >* pControls = xCtrls.getArray();
    for ( sal_Int32 n = 0; n < nControls; n++ )
        pControls[n]->setDesignMode( bOn );

    // #109067# in design mode the tab controller is not notified about
    // tab index changes, therefore the tab order must be activated
    // when switching from design mode to live mode
    if ( mxTabController.is() && !bOn )
        mxTabController->activateTabOrder();
}

void UnoDialogControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    UnoControlContainer::createPeer( rxToolkit, rParentPeer );

    Reference < XTopWindow > xTW( getPeer(), UNO_QUERY );
    if ( xTW.is() )
    {
        xTW->setMenuBar( mxMenuBar );

        if ( !mbWindowListener )
        {
            Reference< XWindowListener > xWL( static_cast< cppu::OWeakObject*>( this ), UNO_QUERY );
            addWindowListener( xWL );
            mbWindowListener = true;
        }

        if ( maTopWindowListeners.getLength() )
            xTW->addTopWindowListener( &maTopWindowListeners );
    }
}

void UnoDialogControl::PrepareWindowDescriptor( ::com::sun::star::awt::WindowDescriptor& rDesc )
{
    sal_Bool bDecoration( sal_True );
    ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DECORATION )) >>= bDecoration;
    if ( !bDecoration )
    {
        // Now we have to manipulate the WindowDescriptor
        rDesc.WindowAttributes = rDesc.WindowAttributes | ::com::sun::star::awt::WindowAttribute::NODECORATION;
    }

    // We have to set the graphic property before the peer
    // will be created. Otherwise the properties will be copied
    // into the peer via propertiesChangeEvents. As the order of
    // can lead to overwrites we have to set the graphic property
    // before the propertiesChangeEvents are sent!
    ::rtl::OUString aImageURL;
    Reference< graphic::XGraphic > xGraphic;
    if (( ImplGetPropertyValue( PROPERTY_IMAGEURL ) >>= aImageURL ) &&
        ( aImageURL.getLength() > 0 ))
    {
        ::rtl::OUString absoluteUrl =
            getPhysicalLocation( ImplGetPropertyValue( PROPERTY_DIALOGSOURCEURL ),
                                 ImplGetPropertyValue( PROPERTY_IMAGEURL ));

        xGraphic = lcl_getGraphicFromURL_nothrow( absoluteUrl );
        ImplSetPropertyValue( PROPERTY_GRAPHIC, uno::makeAny( xGraphic ), sal_True );
    }
}

void UnoDialogControl::elementInserted( const ContainerEvent& Event ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    Reference< XControlModel > xModel;
    ::rtl::OUString aName;

    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ImplInsertControl( xModel, aName );
}

void UnoDialogControl::elementRemoved( const ContainerEvent& Event ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    Reference< XControlModel > xModel;
    Event.Element >>= xModel;
    if ( xModel.is() )
        ImplRemoveControl( xModel );
}

void UnoDialogControl::elementReplaced( const ContainerEvent& Event ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );

    Reference< XControlModel > xModel;
    Event.ReplacedElement >>= xModel;
    if ( xModel.is() )
        ImplRemoveControl( xModel );

    ::rtl::OUString aName;
    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ImplInsertControl( xModel, aName );
}

void UnoDialogControl::addTopWindowListener( const Reference< XTopWindowListener >& rxListener ) throw (RuntimeException)
{
    maTopWindowListeners.addInterface( rxListener );
    if( getPeer().is() && maTopWindowListeners.getLength() == 1 )
    {
        Reference < XTopWindow >  xTW( getPeer(), UNO_QUERY );
        xTW->addTopWindowListener( &maTopWindowListeners );
    }
}

void UnoDialogControl::removeTopWindowListener( const Reference< XTopWindowListener >& rxListener ) throw (RuntimeException)
{
    if( getPeer().is() && maTopWindowListeners.getLength() == 1 )
    {
        Reference < XTopWindow >  xTW( getPeer(), UNO_QUERY );
        xTW->removeTopWindowListener( &maTopWindowListeners );
    }
    maTopWindowListeners.removeInterface( rxListener );
}

void UnoDialogControl::toFront(  ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toFront();
    }
}

void UnoDialogControl::toBack(  ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toBack();
    }
}

void UnoDialogControl::setMenuBar( const Reference< XMenuBar >& rxMenuBar ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    mxMenuBar = rxMenuBar;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->setMenuBar( mxMenuBar );
    }
}

static ::Size ImplMapPixelToAppFont( OutputDevice* pOutDev, const ::Size& aSize )
{
    ::Size aTmp = pOutDev->PixelToLogic( aSize, MAP_APPFONT );
    return aTmp;
}

// ::com::sun::star::awt::XWindowListener
void SAL_CALL UnoDialogControl::windowResized( const ::com::sun::star::awt::WindowEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    DBG_ASSERT( pOutDev, "Missing Default Device!" );
    if ( pOutDev && !mbSizeModified )
    {
        // Currentley we are simply using MAP_APPFONT
        ::Size aAppFontSize( e.Width, e.Height );

        Reference< XControl > xDialogControl( *this, UNO_QUERY_THROW );
        Reference< XDevice > xDialogDevice( xDialogControl->getPeer(), UNO_QUERY );
        OSL_ENSURE( xDialogDevice.is(), "UnoDialogControl::windowResized: no peer, but a windowResized event?" );
        if ( xDialogDevice.is() )
        {
            DeviceInfo aDeviceInfo( xDialogDevice->getInfo() );
            aAppFontSize.Width() -= aDeviceInfo.LeftInset + aDeviceInfo.RightInset;
            aAppFontSize.Height() -= aDeviceInfo.TopInset + aDeviceInfo.BottomInset;
        }

        aAppFontSize = ImplMapPixelToAppFont( pOutDev, aAppFontSize );

        // Remember that changes have been done by listener. No need to
        // update the position because of property change event.
        mbSizeModified = true;
        Sequence< rtl::OUString > aProps( 2 );
        Sequence< Any > aValues( 2 );
        // Properties in a sequence must be sorted!
        aProps[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ));
        aProps[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width"  ));
        aValues[0] <<= aAppFontSize.Height();
        aValues[1] <<= aAppFontSize.Width();

        ImplSetPropertyValues( aProps, aValues, true );
        mbSizeModified = false;
    }
}

void SAL_CALL UnoDialogControl::windowMoved( const ::com::sun::star::awt::WindowEvent& e )
throw (::com::sun::star::uno::RuntimeException)
{
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    DBG_ASSERT( pOutDev, "Missing Default Device!" );
    if ( pOutDev && !mbPosModified )
    {
        // Currentley we are simply using MAP_APPFONT
        Any    aAny;
        ::Size aTmp( e.X, e.Y );
        aTmp = ImplMapPixelToAppFont( pOutDev, aTmp );

        // Remember that changes have been done by listener. No need to
        // update the position because of property change event.
        mbPosModified = true;
        Sequence< rtl::OUString > aProps( 2 );
        Sequence< Any > aValues( 2 );
        aProps[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX"  ));
        aProps[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ));
        aValues[0] <<= aTmp.Width();
        aValues[1] <<= aTmp.Height();

        ImplSetPropertyValues( aProps, aValues, true );
        mbPosModified = false;
    }
}

void SAL_CALL UnoDialogControl::windowShown( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
    (void)e;
}

void SAL_CALL UnoDialogControl::windowHidden( const ::com::sun::star::lang::EventObject& e )
throw (::com::sun::star::uno::RuntimeException)
{
    (void)e;
}

// XPropertiesChangeListener
void UnoDialogControl::ImplModelPropertiesChanged( const Sequence< PropertyChangeEvent >& rEvents ) throw(RuntimeException)
{
    if( !isDesignMode() && !mbCreatingCompatiblePeer )
    {
        ::rtl::OUString s1( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) );
        ::rtl::OUString s2( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) );
        ::rtl::OUString s3( RTL_CONSTASCII_USTRINGPARAM( "Width" ) );
        ::rtl::OUString s4( RTL_CONSTASCII_USTRINGPARAM( "Height" ) );

        sal_Int32 nLen = rEvents.getLength();
        for( sal_Int32 i = 0; i < nLen; i++ )
        {
            const PropertyChangeEvent& rEvt = rEvents.getConstArray()[i];
            Reference< XControlModel > xModel( rEvt.Source, UNO_QUERY );
            sal_Bool bOwnModel = (XControlModel*)xModel.get() == (XControlModel*)getModel().get();
            if ( ( rEvt.PropertyName == s1 ) ||
                 ( rEvt.PropertyName == s2 ) ||
                 ( rEvt.PropertyName == s3 ) ||
                 ( rEvt.PropertyName == s4 ) )
            {
                if ( bOwnModel )
                {
                    if ( !mbPosModified && !mbSizeModified )
                    {
                        // Don't set new pos/size if we get new values from window listener
                        Reference< XControl > xThis( (XAggregation*)(::cppu::OWeakAggObject*)this, UNO_QUERY );
                        ImplSetPosSize( xThis );
                    }
                }
                else
                {
                    Sequence<Reference<XControl> > aControlSequence(getControls());
                    Reference<XControl> aControlRef( StdTabController::FindControl( aControlSequence, xModel ) );
                    ImplSetPosSize( aControlRef );
                }
                break;
            }
            else if ( bOwnModel && rEvt.PropertyName.equalsAsciiL( "ResourceResolver", 16 ))
            {
                ImplStartListingForResourceEvents();
            }
        }
    }

    sal_Int32 nLen = rEvents.getLength();
    for( sal_Int32 i = 0; i < nLen; i++ )
    {
        const PropertyChangeEvent& rEvt = rEvents.getConstArray()[i];
        Reference< XControlModel > xModel( rEvt.Source, UNO_QUERY );
        sal_Bool bOwnModel = (XControlModel*)xModel.get() == (XControlModel*)getModel().get();
        if ( bOwnModel && rEvt.PropertyName.equalsAsciiL( "ImageURL", 8 ))
        {
            ::rtl::OUString aImageURL;
            Reference< graphic::XGraphic > xGraphic;
            if (( ImplGetPropertyValue( PROPERTY_IMAGEURL ) >>= aImageURL ) &&
                ( aImageURL.getLength() > 0 ))
            {
                ::rtl::OUString absoluteUrl =
                    getPhysicalLocation( ImplGetPropertyValue( PROPERTY_DIALOGSOURCEURL ),
                                         ImplGetPropertyValue( PROPERTY_IMAGEURL ));

                xGraphic = lcl_getGraphicFromURL_nothrow( absoluteUrl );
            }

            ImplSetPropertyValue( PROPERTY_GRAPHIC, uno::makeAny( xGraphic ), sal_True );
            break;
        }
    }

    UnoControlContainer::ImplModelPropertiesChanged( rEvents );
}

void UnoDialogControl::ImplStartListingForResourceEvents()
{
    Reference< resource::XStringResourceResolver > xStringResourceResolver;

    ImplGetPropertyValue( PROPERTY_RESOURCERESOLVER ) >>= xStringResourceResolver;

    // Add our helper as listener to retrieve notifications about changes
    Reference< util::XModifyListener > rListener( mxListener );
    ResourceListener* pResourceListener = static_cast< ResourceListener* >( rListener.get() );

    // resource listener will stop listening if resolver reference is empty
    if ( pResourceListener )
        pResourceListener->startListening( xStringResourceResolver );
    ImplUpdateResourceResolver();
}

void UnoDialogControl::ImplUpdateResourceResolver()
{
    rtl::OUString aPropName( PROPERTY_RESOURCERESOLVER );
    Reference< resource::XStringResourceResolver > xStringResourceResolver;

    ImplGetPropertyValue( aPropName ) >>= xStringResourceResolver;
    if ( !xStringResourceResolver.is() )
        return;

    Any xNewStringResourceResolver; xNewStringResourceResolver <<= xStringResourceResolver;

    Sequence< rtl::OUString > aPropNames(1);
    aPropNames[0] = aPropName;

    const Sequence< Reference< awt::XControl > > aSeq = getControls();
    for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
    {
        Reference< XControl > xControl( aSeq[i] );
        Reference< XPropertySet > xPropertySet;

        if ( xControl.is() )
            xPropertySet = Reference< XPropertySet >( xControl->getModel(), UNO_QUERY );

        if ( !xPropertySet.is() )
            continue;

        try
        {
            Reference< resource::XStringResourceResolver > xCurrStringResourceResolver;
            Any aOldValue = xPropertySet->getPropertyValue( aPropName );
            if  (   ( aOldValue >>= xCurrStringResourceResolver )
                &&  ( xStringResourceResolver == xCurrStringResourceResolver )
                )
            {
                Reference< XMultiPropertySet >  xMultiPropSet( xPropertySet, UNO_QUERY );
                Reference< XPropertiesChangeListener > xListener( xPropertySet, UNO_QUERY );
                xMultiPropSet->firePropertiesChangeEvent( aPropNames, xListener );
            }
            else
                xPropertySet->setPropertyValue( aPropName, xNewStringResourceResolver );
        }
        /*catch ( NoSuchElementException& )*/ // that's nonsense, this is never thrown above ...
        catch ( const Exception& )
        {
        }
    }

    // propagate resource resolver changes to language dependent props of the dialog
    Reference< XPropertySet > xPropertySet( getModel(), UNO_QUERY );
    if ( xPropertySet.is() )
    {
        Reference< XMultiPropertySet >  xMultiPropSet( xPropertySet, UNO_QUERY );
        Reference< XPropertiesChangeListener > xListener( xPropertySet, UNO_QUERY );
        xMultiPropSet->firePropertiesChangeEvent( lcl_getLanguageDependentProperties(), xListener );
    }
}

void UnoDialogControl::setTitle( const ::rtl::OUString& Title ) throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    Any aAny;
    aAny <<= Title;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ), aAny, sal_True );
}

::rtl::OUString UnoDialogControl::getTitle() throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    return ImplGetPropertyValue_UString( BASEPROPERTY_TITLE );
}

sal_Int16 UnoDialogControl::execute() throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    sal_Int16 nDone = -1;
    if ( getPeer().is() )
    {
        Reference< XDialog > xDlg( getPeer(), UNO_QUERY );
        if( xDlg.is() )
        {
            GetComponentInfos().bVisible = sal_True;
            nDone = xDlg->execute();
            GetComponentInfos().bVisible = sal_False;
        }
    }
    return nDone;
}

void UnoDialogControl::endExecute() throw(RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    if ( getPeer().is() )
    {
        Reference< XDialog > xDlg( getPeer(), UNO_QUERY );
        if( xDlg.is() )
        {
            xDlg->endExecute();
            GetComponentInfos().bVisible = sal_False;
        }
    }
}

void UnoDialogControl::addingControl( const Reference< XControl >& _rxControl )
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    UnoControlContainer::addingControl( _rxControl );

    if ( _rxControl.is() )
    {
        Reference< XMultiPropertySet > xProps( _rxControl->getModel(), UNO_QUERY );
        if ( xProps.is() )
        {
            Sequence< ::rtl::OUString > aNames( 4 );
            ::rtl::OUString* pNames = aNames.getArray();
            *pNames++ = ::rtl::OUString::createFromAscii( "PositionX" );
            *pNames++ = ::rtl::OUString::createFromAscii( "PositionY" );
            *pNames++ = ::rtl::OUString::createFromAscii( "Width" );
            *pNames++ = ::rtl::OUString::createFromAscii( "Height" );

            xProps->addPropertiesChangeListener( aNames, this );
        }
    }
}

void UnoDialogControl::removingControl( const Reference< XControl >& _rxControl )
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    UnoControlContainer::removingControl( _rxControl );

    if ( _rxControl.is() )
    {
        Reference< XMultiPropertySet > xProps( _rxControl->getModel(), UNO_QUERY );
        if ( xProps.is() )
            xProps->removePropertiesChangeListener( this );
    }

}

void SAL_CALL UnoDialogControl::changesOccurred( const ChangesEvent& ) throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    // a tab controller model may have changed

    // #109067# in design mode don't notify the tab controller
    // about tab index changes
    if ( mxTabController.is() && !mbDesignMode )
        mxTabController->activateTabOrder();
}

// XModifyListener
void SAL_CALL UnoDialogControl::modified(
    const lang::EventObject& /*rEvent*/ )
throw (RuntimeException)
{
    ImplUpdateResourceResolver();
}

//  ----------------------------------------------------
//  Helper Method to convert relative url to physical location
//  ----------------------------------------------------

::rtl::OUString getPhysicalLocation( const ::com::sun::star::uno::Any& rbase, const ::com::sun::star::uno::Any& rUrl )
{


    ::rtl::OUString ret;

    ::rtl::OUString baseLocation;
    ::rtl::OUString url;

    rbase  >>= baseLocation;
    rUrl  >>= url;

    if ( url.getLength() > 0 )
    {
        INetURLObject urlObj(baseLocation);
        urlObj.removeSegment();
        baseLocation = urlObj.GetMainURL( INetURLObject::NO_DECODE );
        ::osl::FileBase::getAbsoluteFileURL( baseLocation, url, ret );
    }

    return ret;
}

