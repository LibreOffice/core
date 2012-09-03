/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wall.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <toolkit/controls/unocontrols.hxx>
#include "toolkit/controls/formattedcontrol.hxx"
#include "toolkit/controls/roadmapcontrol.hxx"
#include "toolkit/controls/tkscrollbar.hxx"
#include <toolkit/controls/stdtabcontroller.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <comphelper/processfactory.hxx>
#include <vcl/svapp.hxx>
#include <vcl/outdev.hxx>
#include <comphelper/types.hxx>

#include <comphelper/componentcontext.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/helper/tkresmgr.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/graph.hxx>
#include <vcl/image.hxx>

#include "tree/treecontrol.hxx"
#include "grid/gridcontrol.hxx"
#include <toolkit/controls/tabpagecontainer.hxx>

#include <boost/bind.hpp>

#include <map>
#include <algorithm>
#include <functional>
#include "tools/urlobj.hxx"
#include "osl/file.hxx"
#include "toolkit/controls/dialogcontrol.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace toolkit;

#define PROPERTY_RESOURCERESOLVER ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceResolver" ))

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
        catch (const Exception&)
        {
            OSL_TRACE( "DisposeControlModel::(): caught an exception while disposing a component!" );
        }
    }
};

// ----------------------------------------------------------------------------
// functor for searching control model by name
struct FindControlModel : public ::std::unary_function< ControlModelContainerBase::UnoControlModelHolder, bool >
{
private:
    const ::rtl::OUString& m_rName;

public:
    FindControlModel( const ::rtl::OUString& _rName ) : m_rName( _rName ) { }

    bool operator()( const ControlModelContainerBase::UnoControlModelHolder& _rCompare )
    {
        return ( _rCompare.second == m_rName ) ? true : false;
    }
};

// ----------------------------------------------------------------------------
// functor for cloning a control model, and insertion into a target list
struct CloneControlModel : public ::std::unary_function< ControlModelContainerBase::UnoControlModelHolder, void >
{
private:
    ControlModelContainerBase::UnoControlModelHolderList&   m_rTargetList;

public:
    CloneControlModel( ControlModelContainerBase::UnoControlModelHolderList& _rTargetList )
        :m_rTargetList( _rTargetList )
    {
    }

    void operator()( const ControlModelContainerBase::UnoControlModelHolder& _rSource )
    {
        // clone the source object
        Reference< XCloneable > xCloneSource( _rSource.first, UNO_QUERY );
        Reference< XControlModel > xClone( xCloneSource->createClone(), UNO_QUERY );
        // add to target list
        m_rTargetList.push_back( ControlModelContainerBase::UnoControlModelHolder( xClone, _rSource.second ) );
    }
};

// ----------------------------------------------------------------------------
// functor for comparing a XControlModel with a given reference
struct CompareControlModel : public ::std::unary_function< ControlModelContainerBase::UnoControlModelHolder, bool >
{
private:
    Reference< XControlModel > m_xReference;
public:
    CompareControlModel( const Reference< XControlModel >& _rxReference ) : m_xReference( _rxReference ) { }

    bool operator()( const ControlModelContainerBase::UnoControlModelHolder& _rCompare )
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
//  class ControlModelContainerBase
//  ----------------------------------------------------
ControlModelContainerBase::ControlModelContainerBase( const Reference< XMultiServiceFactory >& i_factory )
    :ControlModelContainer_IBase( i_factory )
    ,maContainerListeners( *this )
    ,maChangeListeners ( GetMutex() )
    ,mbGroupsUpToDate( sal_False )
{
}

ControlModelContainerBase::ControlModelContainerBase( const ControlModelContainerBase& rModel )
    : ControlModelContainer_IBase( rModel )
    , maContainerListeners( *this )
    , maChangeListeners ( GetMutex() )
    , mbGroupsUpToDate( sal_False )
{
}

ControlModelContainerBase::~ControlModelContainerBase()
{
    maModels.clear();
    mbGroupsUpToDate = sal_False;
}

Any ControlModelContainerBase::ImplGetDefaultValue( sal_uInt16 nPropId ) const
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

::cppu::IPropertyArrayHelper& ControlModelContainerBase::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

void SAL_CALL ControlModelContainerBase::dispose(  ) throw(RuntimeException)
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
        ::boost::bind( &UnoControlModelHolder::first, _1 ) // operation to apply -> select the XControlModel part
    );

    // now dispose
    ::std::for_each( aChildModels.begin(), aChildModels.end(), DisposeControlModel() );
    aChildModels.clear();

    mbGroupsUpToDate = sal_False;
}

// XMultiPropertySet
Reference< XPropertySetInfo > ControlModelContainerBase::getPropertySetInfo(  ) throw(RuntimeException)
{
    static Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
void ControlModelContainerBase::Clone_Impl(ControlModelContainerBase& _rClone) const
{
    // clone all children
    ::std::for_each(
        maModels.begin(), maModels.end(),
        CloneControlModel( _rClone.maModels )
    );
}
UnoControlModel* ControlModelContainerBase::Clone() const
{
    // clone the container itself
    ControlModelContainerBase* pClone = new ControlModelContainerBase( *this );
    Clone_Impl(*pClone);

    return pClone;
}

ControlModelContainerBase::UnoControlModelHolderList::iterator ControlModelContainerBase::ImplFindElement( const ::rtl::OUString& rName )
{
    return ::std::find_if( maModels.begin(), maModels.end(), FindControlModel( rName ) );
}

// ::XMultiServiceFactory
Reference< XInterface > ControlModelContainerBase::createInstance( const ::rtl::OUString& aServiceSpecifier ) throw(Exception, RuntimeException)
{
    SolarMutexGuard aGuard;

    OGeometryControlModel_Base* pNewModel = NULL;

    const Reference< XMultiServiceFactory > xFactory( maContext.getLegacyServiceFactory() );
    if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlEditModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlEditModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlFormattedFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFormattedFieldModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlFileControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFileControlModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlButtonModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlButtonModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlImageControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlImageControlModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlRadioButtonModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlRadioButtonModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlCheckBoxModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlCheckBoxModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoControlFixedHyperlinkModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFixedHyperlinkModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoControlFixedTextModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFixedTextModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlGroupBoxModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlGroupBoxModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlListBoxModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlListBoxModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlComboBoxModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlComboBoxModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlDateFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlDateFieldModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlTimeFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlTimeFieldModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlNumericFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlNumericFieldModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlCurrencyFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlCurrencyFieldModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlPatternFieldModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlPatternFieldModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlProgressBarModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlProgressBarModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlScrollBarModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlScrollBarModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlFixedLineModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFixedLineModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlRoadmapModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlRoadmapModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName_TreeControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoTreeModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName_GridControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoGridModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoControlTabPageContainerModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlTabPageContainerModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoMultiPageModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoMultiPageModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoPageModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoPageModel >( xFactory );
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoFrameModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoFrameModel >( xFactory );

    if ( !pNewModel )
    {
        if ( xFactory.is() )
        {
            Reference< XInterface > xObject = xFactory->createInstance( aServiceSpecifier );
            Reference< XServiceInfo > xSI( xObject, UNO_QUERY );
            Reference< XCloneable > xCloneAccess( xSI, UNO_QUERY );
            Reference< XAggregation > xAgg( xCloneAccess, UNO_QUERY );
            if ( xAgg.is() )
            {
                if ( xSI->supportsService(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlModel"))) )
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

Reference< XInterface > ControlModelContainerBase::createInstanceWithArguments( const ::rtl::OUString& ServiceSpecifier, const Sequence< Any >& /* Arguments */ ) throw(Exception, RuntimeException)
{
    return createInstance( ServiceSpecifier );
}

Sequence< ::rtl::OUString > ControlModelContainerBase::getAvailableServiceNames() throw(RuntimeException)
{
    static Sequence< ::rtl::OUString >* pNamesSeq = NULL;
    if ( !pNamesSeq )
    {
        pNamesSeq = new Sequence< ::rtl::OUString >( 26 );
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
        pNames[21] = ::rtl::OUString::createFromAscii( szServiceName_GridControlModel );
        pNames[22] = ::rtl::OUString::createFromAscii( szServiceName_UnoControlTabPageContainerModel );
        pNames[23] = ::rtl::OUString::createFromAscii( szServiceName_UnoControlTabPageModel );
        pNames[24] = ::rtl::OUString::createFromAscii( szServiceName_UnoMultiPageModel );
        pNames[25] = ::rtl::OUString::createFromAscii( szServiceName_UnoFrameModel );
    }
    return *pNamesSeq;
}

// XContainer
void ControlModelContainerBase::addContainerListener( const Reference< XContainerListener >& l ) throw(RuntimeException)
{
    maContainerListeners.addInterface( l );
}

void ControlModelContainerBase::removeContainerListener( const Reference< XContainerListener >& l ) throw(RuntimeException)
{
    maContainerListeners.removeInterface( l );
}

// XElementAcces
Type ControlModelContainerBase::getElementType() throw(RuntimeException)
{
    Type aType = getCppuType( ( Reference< XControlModel>* ) NULL );
    return aType;
}

sal_Bool ControlModelContainerBase::hasElements() throw(RuntimeException)
{
    return !maModels.empty();
}

// XNameContainer, XNameReplace, XNameAccess
void ControlModelContainerBase::replaceByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XControlModel > xNewModel;
    aElement >>= xNewModel;
    if ( !xNewModel.is() )
        lcl_throwIllegalArgumentException();

    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() == aElementPos )
        lcl_throwNoSuchElementException();
    // Dialog behaviour is to have all containee names unique ( MSO Userform is the same )
    // With container controls you could have constructed an existing hierachy and are now
    // add this to an existing container, in this case a name nested in the containment
    // hierachy of the added control could contain a name clash, if we have access to the
    // list of global names then recursively check for previously existing names ( we need
    // to do this obviously before the 'this' objects container is updated
    Reference< XNameContainer > xAllChildren( getPropertyValue( GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ) ), UNO_QUERY );
    if ( xAllChildren.is() )
    {
        // remove old control ( and children ) from global list of containees
        updateUserFormChildren( xAllChildren, aName, Remove, uno::Reference< XControlModel >() );
        // Add new control ( and containees if they exist )
        updateUserFormChildren( xAllChildren, aName, Insert, xNewModel );
    }
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

Any ControlModelContainerBase::getByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() == aElementPos )
        lcl_throwNoSuchElementException();

    return makeAny( aElementPos->first );
}

Sequence< ::rtl::OUString > ControlModelContainerBase::getElementNames() throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aNames( maModels.size() );

    ::std::transform(
        maModels.begin(), maModels.end(),               // source range
        aNames.getArray(),                              // target range
        ::boost::bind( &UnoControlModelHolder::second, _1 ) // operator to apply: select the second element (the name)
    );

    return aNames;
}

sal_Bool ControlModelContainerBase::hasByName( const ::rtl::OUString& aName ) throw(RuntimeException)
{
    return maModels.end() != ImplFindElement( aName );
}

void ControlModelContainerBase::insertByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XControlModel > xM;
    aElement >>= xM;

    if ( xM.is() )
    {
        Reference< beans::XPropertySet > xProps( xM, UNO_QUERY );
            if ( xProps.is() )
            {

                Reference< beans::XPropertySetInfo > xPropInfo = xProps.get()->getPropertySetInfo();

                ::rtl::OUString sImageSourceProperty = GetPropertyName( BASEPROPERTY_IMAGEURL );
                if ( xPropInfo.get()->hasPropertyByName(  sImageSourceProperty ) && ImplHasProperty(BASEPROPERTY_DIALOGSOURCEURL) )
                {
                    Any aUrl = xProps.get()->getPropertyValue(  sImageSourceProperty );

                    ::rtl::OUString absoluteUrl =
                        getPhysicalLocation( getPropertyValue( GetPropertyName( BASEPROPERTY_DIALOGSOURCEURL ) ), aUrl );

                    aUrl <<= absoluteUrl;

                    xProps.get()->setPropertyValue(  sImageSourceProperty , aUrl );
                }
            }
    }



    if ( aName.isEmpty() || !xM.is() )
        lcl_throwIllegalArgumentException();

    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() != aElementPos )
        lcl_throwElementExistException();

    // Dialog behaviour is to have all containee names unique ( MSO Userform is the same )
    // With container controls you could have constructed an existing hierachy and are now
    // add this to an existing container, in this case a name nested in the containment
    // hierachy of the added control could contain a name clash, if we have access to the
    // list of global names then we need to recursively check for previously existing
    // names ( we need to do this obviously before the 'this' objects container is updated
    // remove old control ( and children ) from global list of containees
    Reference< XNameContainer > xAllChildren( getPropertyValue( GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ) ), UNO_QUERY );

    if ( xAllChildren.is() )
        updateUserFormChildren( xAllChildren, aName, Insert, xM );
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

void ControlModelContainerBase::removeByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;

    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() == aElementPos )
        lcl_throwNoSuchElementException();

    // Dialog behaviour is to have all containee names unique ( MSO Userform is the same )
    // With container controls you could have constructed an existing hierachy and are now
    // removing this control from an existing container, in this case all nested names in
    // the containment hierachy of the control to be removed need to be removed from the global
    // names cache ( we need to do this obviously before the 'this' objects container is updated )
    Reference< XNameContainer > xAllChildren( getPropertyValue( GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ) ), UNO_QUERY );
    if ( xAllChildren.is() )
        updateUserFormChildren( xAllChildren, aName, Remove, uno::Reference< XControlModel >() );

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
    {
        try
        {
            xPS->setPropertyValue( PROPERTY_RESOURCERESOLVER, makeAny( Reference< resource::XStringResourceResolver >() ) );
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    // our "tab controller model" has potentially changed -> notify this
    implNotifyTabModelChange( aName );
}

// ----------------------------------------------------------------------------
sal_Bool SAL_CALL ControlModelContainerBase::getGroupControl(  ) throw (RuntimeException)
{
    return sal_True;
}

// ----------------------------------------------------------------------------
void SAL_CALL ControlModelContainerBase::setGroupControl( sal_Bool ) throw (RuntimeException)
{
    OSL_TRACE( "UnoControlDialogModel::setGroupControl: explicit grouping not supported" );
}

// ----------------------------------------------------------------------------
void SAL_CALL ControlModelContainerBase::setControlModels( const Sequence< Reference< XControlModel > >& _rControls ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

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
Sequence< Reference< XControlModel > > SAL_CALL ControlModelContainerBase::getControlModels(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

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
            ::boost::bind( &MapIndexToModel::value_type::second, _1 )
        );

    return aReturn;
}

// ----------------------------------------------------------------------------
void SAL_CALL ControlModelContainerBase::setGroup( const Sequence< Reference< XControlModel > >&, const ::rtl::OUString& ) throw (RuntimeException)
{
    // not supported. We have only implicit grouping:
    // We only have a sequence of control models, and we _know_ (yes, that's a HACK relying on
    // implementation details) that VCL does grouping according to the order of controls automatically
    // At least VCL does this for all we're interested in: Radio buttons.
    OSL_TRACE( "UnoControlDialogModel::setGroup: grouping not supported" );
}

////----- XInitialization -------------------------------------------------------------------
void SAL_CALL ControlModelContainerBase::initialize (const Sequence<Any>& rArguments) throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException)
{
    if ( rArguments.getLength() == 1 )
    {
        sal_Int16 nPageId = -1;
        if ( !( rArguments[ 0 ] >>= nPageId ))
            throw lang::IllegalArgumentException();
        m_nTabPageId = nPageId;
    }
    else
        m_nTabPageId = -1;
}
::sal_Int16 SAL_CALL ControlModelContainerBase::getTabPageID() throw (::com::sun::star::uno::RuntimeException)
{
    return m_nTabPageId;
}
::sal_Bool SAL_CALL ControlModelContainerBase::getEnabled() throw (::com::sun::star::uno::RuntimeException)
{
    return m_bEnabled;
}
void SAL_CALL ControlModelContainerBase::setEnabled( ::sal_Bool _enabled ) throw (::com::sun::star::uno::RuntimeException)
{
    m_bEnabled = _enabled;
}
::rtl::OUString SAL_CALL ControlModelContainerBase::getTitle() throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference<XPropertySet> xThis(*this,UNO_QUERY);
    ::rtl::OUString sTitle;
    xThis->getPropertyValue(GetPropertyName(BASEPROPERTY_TITLE)) >>= sTitle;
    return sTitle;
    //return m_sTitle;
}
void SAL_CALL ControlModelContainerBase::setTitle( const ::rtl::OUString& _title ) throw (::com::sun::star::uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    Reference<XPropertySet> xThis(*this,UNO_QUERY);
    xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_TITLE),makeAny(_title));
}
::rtl::OUString SAL_CALL ControlModelContainerBase::getImageURL() throw (::com::sun::star::uno::RuntimeException)
{
    return m_sImageURL;
}
void SAL_CALL ControlModelContainerBase::setImageURL( const ::rtl::OUString& _imageurl ) throw (::com::sun::star::uno::RuntimeException)
{
    m_sImageURL = _imageurl;
}
::rtl::OUString SAL_CALL ControlModelContainerBase::getToolTip() throw (::com::sun::star::uno::RuntimeException)
{
    return m_sTooltip;
}
void SAL_CALL ControlModelContainerBase::setToolTip( const ::rtl::OUString& _tooltip ) throw (::com::sun::star::uno::RuntimeException)
{
    m_sTooltip = _tooltip;
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
        catch (const Exception&)
        {
            OSL_TRACE( "lcl_getDialogStep: caught an exception while determining the dialog page!" );
        }
        return nStep;
    }
}

// ----------------------------------------------------------------------------
sal_Int32 SAL_CALL ControlModelContainerBase::getGroupCount(  ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    implUpdateGroupStructure();

    return maGroups.size();
}

// ----------------------------------------------------------------------------
void SAL_CALL ControlModelContainerBase::getGroup( sal_Int32 _nGroup, Sequence< Reference< XControlModel > >& _rGroup, ::rtl::OUString& _rName ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    implUpdateGroupStructure();

    if ( ( _nGroup < 0 ) || ( _nGroup >= (sal_Int32)maGroups.size() ) )
    {
        OSL_TRACE( "UnoControlDialogModel::getGroup: invalid argument and I am not allowed to throw an exception!" );
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
void SAL_CALL ControlModelContainerBase::getGroupByName( const ::rtl::OUString& _rName, Sequence< Reference< XControlModel > >& _rGroup ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

    ::rtl::OUString sDummyName;
    getGroup( _rName.toInt32( ), _rGroup, sDummyName );
}

// ----------------------------------------------------------------------------
void SAL_CALL ControlModelContainerBase::addChangesListener( const Reference< XChangesListener >& _rxListener ) throw (RuntimeException)
{
    maChangeListeners.addInterface( _rxListener );
}

// ----------------------------------------------------------------------------
void SAL_CALL ControlModelContainerBase::removeChangesListener( const Reference< XChangesListener >& _rxListener ) throw (RuntimeException)
{
    maChangeListeners.removeInterface( _rxListener );
}

// ----------------------------------------------------------------------------
void ControlModelContainerBase::implNotifyTabModelChange( const ::rtl::OUString& _rAccessor )
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
void ControlModelContainerBase::implUpdateGroupStructure()
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
                if ( xModelProps.is() && xModelProps->getPropertySetInfo().is() && xModelProps->getPropertySetInfo()->hasPropertyByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")) ) )
                    xModelProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")) ) >>= sLabel;
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
                    if ( xModelProps.is() && xModelProps->getPropertySetInfo().is() && xModelProps->getPropertySetInfo()->hasPropertyByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")) ) )
                        xModelProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")) ) >>= sLabel;
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
                if ( xModelProps.is() && xModelProps->getPropertySetInfo().is() && xModelProps->getPropertySetInfo()->hasPropertyByName( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")) ) )
                    xModelProps->getPropertyValue( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Label")) ) >>= sLabel;
                aCurrentGroupLabels.push_back( sLabel );
#endif
            }
            break;
        }
    }

    mbGroupsUpToDate = sal_True;
}

// ----------------------------------------------------------------------------
void SAL_CALL ControlModelContainerBase::propertyChange( const PropertyChangeEvent& _rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;

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
void SAL_CALL ControlModelContainerBase::disposing( const EventObject& /*rEvent*/ ) throw (RuntimeException)
{
}

// ----------------------------------------------------------------------------
void ControlModelContainerBase::startControlListening( const Reference< XControlModel >& _rxChildModel )
{
    SolarMutexGuard aGuard;

    Reference< XPropertySet > xModelProps( _rxChildModel, UNO_QUERY );
    Reference< XPropertySetInfo > xPSI;
    if ( xModelProps.is() )
        xPSI = xModelProps->getPropertySetInfo();

    if ( xPSI.is() && xPSI->hasPropertyByName( getTabIndexPropertyName() ) )
        xModelProps->addPropertyChangeListener( getTabIndexPropertyName(), this );
}

// ----------------------------------------------------------------------------
void ControlModelContainerBase::stopControlListening( const Reference< XControlModel >& _rxChildModel )
{
    SolarMutexGuard aGuard;

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
        catch (const RuntimeException&)
        {
            throw;
        }
        catch (const Exception&)
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
        catch (const RuntimeException&)
        {
            throw;
        }
        catch (const Exception&)
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
        catch (const RuntimeException&)
        {
            throw;
        }
        catch (const Exception&)
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
            catch (const RuntimeException&)
            {
                throw;
            }
            catch (const Exception&)
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
            catch (const RuntimeException&)
            {
                throw;
            }
            catch (const Exception&)
            {
            }
        }
    }
}

//===============================================================
//  ----------------------------------------------------
//  class DialogContainerControl
//  ----------------------------------------------------
ControlContainerBase::ControlContainerBase( const Reference< XMultiServiceFactory >& i_factory )
    :ContainerControl_IBase( i_factory )
    ,mbSizeModified(false)
    ,mbPosModified(false)
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
    mxListener = new ResourceListener( Reference< util::XModifyListener >(
                        static_cast< OWeakObject* >( this ), UNO_QUERY ));
}

ControlContainerBase::~ControlContainerBase()
{
}

void ControlContainerBase::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    UnoControlContainer::createPeer( rxToolkit, rParentPeer );
}

void ControlContainerBase::ImplInsertControl( Reference< XControlModel >& rxModel, const ::rtl::OUString& rName )
{
    Reference< XPropertySet > xP( rxModel, UNO_QUERY );

    ::rtl::OUString aDefCtrl;
    xP->getPropertyValue( GetPropertyName( BASEPROPERTY_DEFAULTCONTROL ) ) >>= aDefCtrl;
    Reference < XControl > xCtrl;
    maContext.createComponent( aDefCtrl, xCtrl );

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

void ControlContainerBase::ImplRemoveControl( Reference< XControlModel >& rxModel )
{
    Sequence< Reference< XControl > > aControls = getControls();
    Reference< XControl > xCtrl = StdTabController::FindControl( aControls, rxModel );
    if ( xCtrl.is() )
    {
        removeControl( xCtrl );
        try
        {
            Reference< XComponent > const xControlComp( xCtrl, UNO_QUERY_THROW );
            xControlComp->dispose();
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

void ControlContainerBase::ImplSetPosSize( Reference< XControl >& rxCtrl )
{
    Reference< XPropertySet > xP( rxCtrl->getModel(), UNO_QUERY );

    sal_Int32 nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionX" ) ) ) >>= nX;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "PositionY" ) ) ) >>= nY;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Width" ) ) ) >>= nWidth;
    xP->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Height" ) ) ) >>= nHeight;
    MapMode aMode( MAP_APPFONT );
    OutputDevice*pOutDev = Application::GetDefaultDevice();
    if ( pOutDev )
    {
        ::Size aTmp( nX, nY );
        aTmp = pOutDev->LogicToPixel( aTmp, aMode );
        nX = aTmp.Width();
        nY = aTmp.Height();
        aTmp = ::Size( nWidth, nHeight );
        aTmp = pOutDev->LogicToPixel( aTmp, aMode );
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
        if ( !aFD.StyleName.isEmpty() )
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

void ControlContainerBase::dispose() throw(RuntimeException)
{
    EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >( this );
    // Notify our listener helper about dispose
    // --- SAFE ---

    SolarMutexClearableGuard aGuard;
    Reference< XEventListener > xListener( mxListener, UNO_QUERY );
    mxListener.clear();
    aGuard.clear();
    // --- SAFE ---

    if ( xListener.is() )
        xListener->disposing( aEvt );
    UnoControlContainer::dispose();
}

void SAL_CALL ControlContainerBase::disposing(
    const EventObject& Source )
throw(RuntimeException)
{
    UnoControlContainer::disposing( Source );
}

sal_Bool ControlContainerBase::setModel( const Reference< XControlModel >& rxModel ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

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
void ControlContainerBase::setDesignMode( sal_Bool bOn ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

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

void ControlContainerBase::elementInserted( const ContainerEvent& Event ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XControlModel > xModel;
    ::rtl::OUString aName;

    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ENSURE_OR_RETURN_VOID( xModel.is(), "UnoDialogControl::elementInserted: illegal element!" );
    try
    {
        ImplInsertControl( xModel, aName );
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void ControlContainerBase::elementRemoved( const ContainerEvent& Event ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XControlModel > xModel;
    Event.Element >>= xModel;
    ENSURE_OR_RETURN_VOID( xModel.is(), "UnoDialogControl::elementRemoved: illegal element!" );
    try
    {
        ImplRemoveControl( xModel );
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void ControlContainerBase::elementReplaced( const ContainerEvent& Event ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;

    Reference< XControlModel > xModel;
    Event.ReplacedElement >>= xModel;
    try
    {
        OSL_ENSURE( xModel.is(), "UnoDialogControl::elementReplaced: invalid ReplacedElement!" );
        if ( xModel.is() )
            ImplRemoveControl( xModel );
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    ::rtl::OUString aName;
    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ENSURE_OR_RETURN_VOID( xModel.is(), "UnoDialogControl::elementReplaced: invalid new element!" );
    try
    {
        ImplInsertControl( xModel, aName );
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

// XPropertiesChangeListener
void ControlContainerBase::ImplModelPropertiesChanged( const Sequence< PropertyChangeEvent >& rEvents ) throw(RuntimeException)
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
        }
    }

    UnoControlContainer::ImplModelPropertiesChanged( rEvents );
}

void ControlContainerBase::addingControl( const Reference< XControl >& _rxControl )
{
    SolarMutexGuard aGuard;
    UnoControlContainer::addingControl( _rxControl );

    if ( _rxControl.is() )
    {
        Reference< XMultiPropertySet > xProps( _rxControl->getModel(), UNO_QUERY );
        if ( xProps.is() )
        {
            Sequence< ::rtl::OUString > aNames( 4 );
            ::rtl::OUString* pNames = aNames.getArray();
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionX") );
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionY") );
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width"));
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height"));

            xProps->addPropertiesChangeListener( aNames, this );
        }
    }
}

void ControlContainerBase::removingControl( const Reference< XControl >& _rxControl )
{
    SolarMutexGuard aGuard;
    UnoControlContainer::removingControl( _rxControl );

    if ( _rxControl.is() )
    {
        Reference< XMultiPropertySet > xProps( _rxControl->getModel(), UNO_QUERY );
        if ( xProps.is() )
            xProps->removePropertiesChangeListener( this );
    }

}

void SAL_CALL ControlContainerBase::changesOccurred( const ChangesEvent& ) throw (RuntimeException)
{
    SolarMutexGuard aGuard;
    // a tab controller model may have changed

    // #109067# in design mode don't notify the tab controller
    // about tab index changes
    if ( mxTabController.is() && !mbDesignMode )
        mxTabController->activateTabOrder();
}
void lcl_ApplyResolverToNestedContainees(  const Reference< resource::XStringResourceResolver >& xStringResourceResolver, const Reference< XControlContainer >& xContainer )
{
    rtl::OUString aPropName( PROPERTY_RESOURCERESOLVER );

    Any xNewStringResourceResolver; xNewStringResourceResolver <<= xStringResourceResolver;

    Sequence< rtl::OUString > aPropNames(1);
    aPropNames[0] = aPropName;

    const Sequence< Reference< awt::XControl > > aSeq = xContainer->getControls();
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
        catch (const Exception&)
        {
        }

        uno::Reference< XControlContainer > xNestedContainer( xControl, uno::UNO_QUERY );
        if ( xNestedContainer.is() )
            lcl_ApplyResolverToNestedContainees(  xStringResourceResolver, xNestedContainer );

    }

}
void ControlContainerBase::ImplStartListingForResourceEvents()
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

void ControlContainerBase::ImplUpdateResourceResolver()
{
    rtl::OUString aPropName( PROPERTY_RESOURCERESOLVER );
    Reference< resource::XStringResourceResolver > xStringResourceResolver;

    ImplGetPropertyValue( aPropName ) >>= xStringResourceResolver;
    if ( !xStringResourceResolver.is() )
        return;

    lcl_ApplyResolverToNestedContainees(  xStringResourceResolver, this );

    // propagate resource resolver changes to language dependent props of the dialog
    Reference< XPropertySet > xPropertySet( getModel(), UNO_QUERY );
    if ( xPropertySet.is() )
    {
        Reference< XMultiPropertySet >  xMultiPropSet( xPropertySet, UNO_QUERY );
        Reference< XPropertiesChangeListener > xListener( xPropertySet, UNO_QUERY );
        xMultiPropSet->firePropertiesChangeEvent( lcl_getLanguageDependentProperties(), xListener );
    }
}

////    ----------------------------------------------------
////    Helper Method to convert relative url to physical location
////    ----------------------------------------------------

::rtl::OUString getPhysicalLocation( const ::com::sun::star::uno::Any& rbase, const ::com::sun::star::uno::Any& rUrl )
{

    ::rtl::OUString baseLocation;
    ::rtl::OUString url;

    rbase  >>= baseLocation;
    rUrl  >>= url;

    ::rtl::OUString absoluteURL( url );
    if ( !url.isEmpty() )
    {
        INetURLObject urlObj(baseLocation);
        urlObj.removeSegment();
        baseLocation = urlObj.GetMainURL( INetURLObject::NO_DECODE );

        const INetURLObject protocolCheck( url );
        const INetProtocol protocol = protocolCheck.GetProtocol();
        if ( protocol == INET_PROT_NOT_VALID )
        {
            ::rtl::OUString testAbsoluteURL;
            if ( ::osl::FileBase::E_None == ::osl::FileBase::getAbsoluteFileURL( baseLocation, url, testAbsoluteURL ) )
                absoluteURL = testAbsoluteURL;
        }
    }

    return absoluteURL;
}

void
ControlModelContainerBase::updateUserFormChildren( const Reference< XNameContainer >& xAllChildren, const rtl::OUString& aName, ChildOperation Operation, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xTarget ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    if ( Operation < Insert || Operation > Remove )
        throw IllegalArgumentException();

    if ( xAllChildren.is() )
    {
        if ( Operation == Remove )
        {
            Reference< XControlModel > xOldModel( xAllChildren->getByName( aName ), UNO_QUERY );
            xAllChildren->removeByName( aName );

            Reference< XNameContainer > xChildContainer( xOldModel, UNO_QUERY );
            if ( xChildContainer.is() )
            {
                Reference< XPropertySet > xProps( xChildContainer, UNO_QUERY );
                // container control is being removed from this container, reset the
                // global list of containees
                if ( xProps.is() )
                    xProps->setPropertyValue(  GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ), uno::makeAny( uno::Reference< XNameContainer >() ) );
                Sequence< rtl::OUString > aChildNames = xChildContainer->getElementNames();
                for ( sal_Int32 index=0; index< aChildNames.getLength(); ++index )
                    updateUserFormChildren( xAllChildren, aChildNames[ index ], Operation,  Reference< XControlModel > () );
            }
        }
        else if ( Operation == Insert )
        {
            xAllChildren->insertByName( aName, uno::makeAny( xTarget ) );
            Reference< XNameContainer > xChildContainer( xTarget, UNO_QUERY );
            if ( xChildContainer.is() )
            {
                // container control is being added from this container, reset the
                // global list of containees to point to the correct global list
                Reference< XPropertySet > xProps( xChildContainer, UNO_QUERY );
                if ( xProps.is() )
                    xProps->setPropertyValue(  GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ), uno::makeAny( xAllChildren ) );
                Sequence< rtl::OUString > aChildNames = xChildContainer->getElementNames();
                for ( sal_Int32 index=0; index< aChildNames.getLength(); ++index )
                {
                    Reference< XControlModel > xChildTarget( xChildContainer->getByName( aChildNames[ index ] ), UNO_QUERY );
                    updateUserFormChildren( xAllChildren, aChildNames[ index ], Operation, xChildTarget );
                }
            }
        }
    }
    else
        throw IllegalArgumentException();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
