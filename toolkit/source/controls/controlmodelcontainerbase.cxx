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


#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wall.hxx>
#include <osl/mutex.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <toolkit/controls/unocontrols.hxx>
#include <toolkit/controls/formattedcontrol.hxx>
#include <toolkit/controls/roadmapcontrol.hxx>
#include <toolkit/controls/tkscrollbar.hxx>
#include <toolkit/controls/tabpagemodel.hxx>
#include <toolkit/controls/stdtabcontroller.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/awt/WindowAttribute.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakagg.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/outdev.hxx>
#include <comphelper/types.hxx>

#include <vcl/graph.hxx>
#include <vcl/image.hxx>

#include "tree/treecontrol.hxx"
#include "grid/gridcontrol.hxx"
#include <toolkit/controls/tabpagecontainer.hxx>

#include <map>
#include <algorithm>
#include <tools/urlobj.hxx>
#include <osl/file.hxx>
#include <sal/log.hxx>
#include <toolkit/controls/dialogcontrol.hxx>

#include <helper/tkresmgr.hxx>
#include <helper/unopropertyarrayhelper.hxx>
#include "controlmodelcontainerbase_internal.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace toolkit;

#define PROPERTY_RESOURCERESOLVER "ResourceResolver"


namespace
{
    const Sequence< OUString >& lcl_getLanguageDependentProperties()
    {
        // note: properties must be sorted
        static Sequence<OUString> s_aLanguageDependentProperties{ "HelpText", "Title" };
        return s_aLanguageDependentProperties;
    }
}


// functor for disposing a control model
struct DisposeControlModel
{
    void operator()( Reference< XControlModel >& _rxModel )
    {
        try
        {
            ::comphelper::disposeComponent( _rxModel );
        }
        catch (const Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("toolkit", "caught an exception while disposing a component! " << exceptionToString(ex) );
        }
    }
};


// functor for searching control model by name
struct FindControlModel
{
private:
    const OUString& m_rName;

public:
    explicit FindControlModel( const OUString& _rName ) : m_rName( _rName ) { }

    bool operator()( const ControlModelContainerBase::UnoControlModelHolder& _rCompare )
    {
        return _rCompare.second == m_rName;
    }
};


// functor for cloning a control model, and insertion into a target list
struct CloneControlModel
{
private:
    ControlModelContainerBase::UnoControlModelHolderVector&   m_rTargetVector;

public:
    explicit CloneControlModel( ControlModelContainerBase::UnoControlModelHolderVector& _rTargetVector )
        :m_rTargetVector( _rTargetVector )
    {
    }

    void operator()( const ControlModelContainerBase::UnoControlModelHolder& _rSource )
    {
        // clone the source object
        Reference< XCloneable > xCloneSource( _rSource.first, UNO_QUERY );
        Reference< XControlModel > xClone( xCloneSource->createClone(), UNO_QUERY );
        // add to target list
        m_rTargetVector.emplace_back( xClone, _rSource.second );
    }
};


// functor for comparing a XControlModel with a given reference
struct CompareControlModel
{
private:
    Reference< XControlModel > m_xReference;
public:
    explicit CompareControlModel( const Reference< XControlModel >& _rxReference ) : m_xReference( _rxReference ) { }

    bool operator()( const ControlModelContainerBase::UnoControlModelHolder& _rCompare )
    {
        return _rCompare.first.get() == m_xReference.get();
    }
};


static void lcl_throwIllegalArgumentException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw IllegalArgumentException();
}


static void lcl_throwNoSuchElementException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw NoSuchElementException();
}


static void lcl_throwElementExistException( )
{   // throwing is expensive (in terms of code size), thus we hope the compiler does not inline this ....
    throw ElementExistException();
}


static OUString getTabIndexPropertyName( )
{
    return OUString( "TabIndex" );
}


static OUString getStepPropertyName( )
{
    return OUString( "Step" );
}


//  class ControlModelContainerBase

ControlModelContainerBase::ControlModelContainerBase( const Reference< XComponentContext >& rxContext )
    :ControlModelContainer_IBase( rxContext )
    ,maContainerListeners( *this )
    ,maChangeListeners ( GetMutex() )
    ,mbGroupsUpToDate( false )
    ,m_bEnabled( true )
    ,m_nTabPageId(0)
{
}

ControlModelContainerBase::ControlModelContainerBase( const ControlModelContainerBase& rModel )
    : ControlModelContainer_IBase( rModel )
    , maContainerListeners( *this )
    , maChangeListeners ( GetMutex() )
    , mbGroupsUpToDate( false )
    , m_bEnabled( rModel.m_bEnabled )
    , m_nTabPageId( rModel.m_nTabPageId )
{
}

ControlModelContainerBase::~ControlModelContainerBase()
{
    maModels.clear();
    mbGroupsUpToDate = false;
}

Any ControlModelContainerBase::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    Any aAny;

    switch ( nPropId )
    {
        case BASEPROPERTY_DEFAULTCONTROL:
            aAny <<= OUString::createFromAscii( szServiceName_UnoControlDialog );
            break;
        default:
            aAny = UnoControlModel::ImplGetDefaultValue( nPropId );
    }

    return aAny;
}

::cppu::IPropertyArrayHelper& ControlModelContainerBase::getInfoHelper()
{
    static UnoPropertyArrayHelper aHelper( ImplGetPropertyIds() );
    return aHelper;
}

void SAL_CALL ControlModelContainerBase::dispose(  )
{

    // tell our listeners
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = static_cast< XAggregation* >( static_cast< ::cppu::OWeakAggObject* >( this ) );

        maContainerListeners.disposeAndClear( aDisposeEvent );
        maChangeListeners.disposeAndClear( aDisposeEvent );
    }


    // call the base class
    UnoControlModel::dispose();


    // dispose our child models
    // for this, collect the models (we collect them from maModels, and this is modified when disposing children)
    ::std::vector< Reference< XControlModel > > aChildModels( maModels.size() );

    ::std::transform(
        maModels.begin(), maModels.end(),               // source range
        aChildModels.begin(),                           // target location
        []( const UnoControlModelHolder& rUnoControlModelHolder )
        { return rUnoControlModelHolder.first; }        // operation to apply -> select the XControlModel part
    );

    // now dispose
    ::std::for_each( aChildModels.begin(), aChildModels.end(), DisposeControlModel() );
    aChildModels.clear();

    mbGroupsUpToDate = false;
}

// XMultiPropertySet
Reference< XPropertySetInfo > ControlModelContainerBase::getPropertySetInfo(  )
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
rtl::Reference<UnoControlModel> ControlModelContainerBase::Clone() const
{
    // clone the container itself
    ControlModelContainerBase* pClone = new ControlModelContainerBase( *this );
    Clone_Impl(*pClone);

    return pClone;
}

ControlModelContainerBase::UnoControlModelHolderVector::iterator ControlModelContainerBase::ImplFindElement( const OUString& rName )
{
    return ::std::find_if( maModels.begin(), maModels.end(), FindControlModel( rName ) );
}

// ::XMultiServiceFactory
Reference< XInterface > ControlModelContainerBase::createInstance( const OUString& aServiceSpecifier )
{
    SolarMutexGuard aGuard;

    OGeometryControlModel_Base* pNewModel = nullptr;

    if ( aServiceSpecifier == "com.sun.star.awt.UnoControlEditModel" )
        pNewModel = new OGeometryControlModel< UnoControlEditModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlFormattedFieldModel" )
        pNewModel = new OGeometryControlModel< UnoControlFormattedFieldModel >( m_xContext);
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlFileControlModel" )
        pNewModel = new OGeometryControlModel< UnoControlFileControlModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlButtonModel" )
        pNewModel = new OGeometryControlModel< UnoControlButtonModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlImageControlModel" )
        pNewModel = new OGeometryControlModel< UnoControlImageControlModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlRadioButtonModel" )
        pNewModel = new OGeometryControlModel< UnoControlRadioButtonModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlCheckBoxModel" )
        pNewModel = new OGeometryControlModel< UnoControlCheckBoxModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlFixedHyperlinkModel" )
        pNewModel = new OGeometryControlModel< UnoControlFixedHyperlinkModel >( m_xContext );
    else if ( aServiceSpecifier == "stardiv.vcl.controlmodel.FixedText" )
        pNewModel = new OGeometryControlModel< UnoControlFixedTextModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlGroupBoxModel" )
        pNewModel = new OGeometryControlModel< UnoControlGroupBoxModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlListBoxModel" )
        pNewModel = new OGeometryControlModel< UnoControlListBoxModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlComboBoxModel" )
        pNewModel = new OGeometryControlModel< UnoControlComboBoxModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlDateFieldModel" )
        pNewModel = new OGeometryControlModel< UnoControlDateFieldModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlTimeFieldModel" )
        pNewModel = new OGeometryControlModel< UnoControlTimeFieldModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlNumericFieldModel" )
        pNewModel = new OGeometryControlModel< UnoControlNumericFieldModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlCurrencyFieldModel" )
        pNewModel = new OGeometryControlModel< UnoControlCurrencyFieldModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlPatternFieldModel" )
        pNewModel = new OGeometryControlModel< UnoControlPatternFieldModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlProgressBarModel" )
        pNewModel = new OGeometryControlModel< UnoControlProgressBarModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlScrollBarModel" )
        pNewModel = new OGeometryControlModel< UnoControlScrollBarModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlFixedLineModel" )
        pNewModel = new OGeometryControlModel< UnoControlFixedLineModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoControlRoadmapModel" )
        pNewModel = new OGeometryControlModel< UnoControlRoadmapModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.tree.TreeControlModel" )
        pNewModel = new OGeometryControlModel< UnoTreeModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.grid.UnoControlGridModel" )
        pNewModel = new OGeometryControlModel< UnoGridModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.tab.UnoControlTabPageContainerModel" )
        pNewModel = new OGeometryControlModel< UnoControlTabPageContainerModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoMultiPageModel" )
        pNewModel = new OGeometryControlModel< UnoMultiPageModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.tab.UnoControlTabPageModel" )
        pNewModel = new OGeometryControlModel< UnoControlTabPageModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoPageModel" )
        pNewModel = new OGeometryControlModel< UnoPageModel >( m_xContext );
    else if ( aServiceSpecifier == "com.sun.star.awt.UnoFrameModel" )
        pNewModel = new OGeometryControlModel< UnoFrameModel >( m_xContext );

    if ( !pNewModel )
    {
        Reference< XInterface > xObject = m_xContext->getServiceManager()->createInstanceWithContext(aServiceSpecifier, m_xContext);
        Reference< XServiceInfo > xSI( xObject, UNO_QUERY );
        Reference< XCloneable > xCloneAccess( xSI, UNO_QUERY );
        Reference< XAggregation > xAgg( xCloneAccess, UNO_QUERY );
        if ( xAgg.is() )
        {
            if ( xSI->supportsService("com.sun.star.awt.UnoControlModel") )
            {
                // release 3 of the 4 references we have to the object
                xAgg.clear();
                xSI.clear();
                xObject.clear();

                pNewModel = new OCommonGeometryControlModel( xCloneAccess, aServiceSpecifier );
            }
        }
    }

    Reference< XInterface > xNewModel = static_cast<cppu::OWeakObject*>(pNewModel);
    return xNewModel;
}

Reference< XInterface > ControlModelContainerBase::createInstanceWithArguments( const OUString& ServiceSpecifier, const Sequence< Any >& i_arguments )
{
    const Reference< XInterface > xInstance( createInstance( ServiceSpecifier ) );
    const Reference< XInitialization > xInstanceInit( xInstance, UNO_QUERY );
    ENSURE_OR_RETURN( xInstanceInit.is(), "ControlModelContainerBase::createInstanceWithArguments: can't pass the arguments!", xInstance );
    xInstanceInit->initialize( i_arguments );
    return xInstance;
}

Sequence< OUString > ControlModelContainerBase::getAvailableServiceNames()
{
    return { "com.sun.star.awt.UnoControlEditModel",
            "com.sun.star.awt.UnoControlFormattedFieldModel",
            "com.sun.star.awt.UnoControlFileControlModel",
            "com.sun.star.awt.UnoControlButtonModel",
            "com.sun.star.awt.UnoControlImageControlModel",
            "com.sun.star.awt.UnoControlRadioButtonModel",
            "com.sun.star.awt.UnoControlCheckBoxModel",
            "com.sun.star.awt.UnoControlFixedTextModel",
            "com.sun.star.awt.UnoControlGroupBoxModel",
            "com.sun.star.awt.UnoControlListBoxModel",
            "com.sun.star.awt.UnoControlComboBoxModel",
            "com.sun.star.awt.UnoControlDateFieldModel",
            "com.sun.star.awt.UnoControlTimeFieldModel",
            "com.sun.star.awt.UnoControlNumericFieldModel",
            "com.sun.star.awt.UnoControlCurrencyFieldModel",
            "com.sun.star.awt.UnoControlPatternFieldModel",
            "com.sun.star.awt.UnoControlProgressBarModel",
            "com.sun.star.awt.UnoControlScrollBarModel",
            "com.sun.star.awt.UnoControlFixedLineModel",
            "com.sun.star.awt.UnoControlRoadmapModel",
            "com.sun.star.awt.tree.TreeControlModel",
            "com.sun.star.awt.grid.UnoControlGridModel",
            "com.sun.star.awt.UnoControlFixedHyperlinkModel",
            "com.sun.star.awt.tab.UnoControlTabPageContainerModel",
            "com.sun.star.awt.tab.UnoControlTabPageModel",
            "com.sun.star.awt.UnoMultiPageModel",
            "com.sun.star.awt.UnoFrameModel"
    };
}

// XContainer
void ControlModelContainerBase::addContainerListener( const Reference< XContainerListener >& l )
{
    maContainerListeners.addInterface( l );
}

void ControlModelContainerBase::removeContainerListener( const Reference< XContainerListener >& l )
{
    maContainerListeners.removeInterface( l );
}

// XElementAccess
Type ControlModelContainerBase::getElementType()
{
    Type aType = cppu::UnoType<XControlModel>::get();
    return aType;
}

sal_Bool ControlModelContainerBase::hasElements()
{
    return !maModels.empty();
}

// XNameContainer, XNameReplace, XNameAccess
void ControlModelContainerBase::replaceByName( const OUString& aName, const Any& aElement )
{
    SolarMutexGuard aGuard;

    Reference< XControlModel > xNewModel;
    aElement >>= xNewModel;
    if ( !xNewModel.is() )
        lcl_throwIllegalArgumentException();

    UnoControlModelHolderVector::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() == aElementPos )
        lcl_throwNoSuchElementException();
    // Dialog behaviour is to have all containee names unique (MSO Userform is the same)
    // With container controls you could have constructed an existing hierarchy and are now
    // add this to an existing container, in this case a name nested in the containment
    // hierarchy of the added control could contain a name clash, if we have access to the
    // list of global names then recursively check for previously existing names (we need
    // to do this obviously before the 'this' objects container is updated)
    Reference< XNameContainer > xAllChildren( getPropertyValue( GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ) ), UNO_QUERY );
    if ( xAllChildren.is() )
    {
        // remove old control (and children) from global list of containers
        updateUserFormChildren( xAllChildren, aName, Remove, uno::Reference< XControlModel >() );
        // Add new control (and containers if they exist)
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

Any ControlModelContainerBase::getByName( const OUString& aName )
{
    UnoControlModelHolderVector::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() == aElementPos )
        lcl_throwNoSuchElementException();

    return makeAny( aElementPos->first );
}

Sequence< OUString > ControlModelContainerBase::getElementNames()
{
    Sequence< OUString > aNames( maModels.size() );

    ::std::transform(
        maModels.begin(), maModels.end(),               // source range
        aNames.getArray(),                              // target range
        []( const UnoControlModelHolder& rUnoControlModelHolder )
        { return rUnoControlModelHolder.second; }        // operator to apply: select the second element (the name)
    );

    return aNames;
}

sal_Bool ControlModelContainerBase::hasByName( const OUString& aName )
{
    return maModels.end() != ImplFindElement( aName );
}

void ControlModelContainerBase::insertByName( const OUString& aName, const Any& aElement )
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

                const OUString& sImageSourceProperty = GetPropertyName( BASEPROPERTY_IMAGEURL );
                if ( xPropInfo.get()->hasPropertyByName(  sImageSourceProperty ) && ImplHasProperty(BASEPROPERTY_DIALOGSOURCEURL) )
                {
                    Any aUrl = xProps.get()->getPropertyValue(  sImageSourceProperty );

                    OUString absoluteUrl =
                        getPhysicalLocation( getPropertyValue( GetPropertyName( BASEPROPERTY_DIALOGSOURCEURL ) ), aUrl );

                    aUrl <<= absoluteUrl;

                    xProps.get()->setPropertyValue(  sImageSourceProperty , aUrl );
                }
            }
    }


    if ( aName.isEmpty() || !xM.is() )
        lcl_throwIllegalArgumentException();

    UnoControlModelHolderVector::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() != aElementPos )
        lcl_throwElementExistException();

    // Dialog behaviour is to have all containee names unique (MSO Userform is the same)
    // With container controls you could have constructed an existing hierarchy and are now
    // add this to an existing container, in this case a name nested in the containment
    // hierarchy of the added control could contain a name clash, if we have access to the
    // list of global names then we need to recursively check for previously existing
    // names (we need to do this obviously before the 'this' objects container is updated)
    // remove old control (and children) from global list of containers
    Reference< XNameContainer > xAllChildren( getPropertyValue( GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ) ), UNO_QUERY );

    if ( xAllChildren.is() )
        updateUserFormChildren( xAllChildren, aName, Insert, xM );
    maModels.emplace_back( xM, aName );
    mbGroupsUpToDate = false;
    startControlListening( xM );

    ContainerEvent aEvent;
    aEvent.Source = *this;
    aEvent.Element = aElement;
    aEvent.Accessor <<= aName;
    maContainerListeners.elementInserted( aEvent );

    // our "tab controller model" has potentially changed -> notify this
    implNotifyTabModelChange( aName );
}

void ControlModelContainerBase::removeByName( const OUString& aName )
{
    SolarMutexGuard aGuard;

    UnoControlModelHolderVector::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() == aElementPos )
        lcl_throwNoSuchElementException();

    // Dialog behaviour is to have all containee names unique (MSO Userform is the same)
    // With container controls you could have constructed an existing hierarchy and are now
    // removing this control from an existing container, in this case all nested names in
    // the containment hierarchy of the control to be removed need to be removed from the global
    // names cache (we need to do this obviously before the 'this' objects container is updated)
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
    mbGroupsUpToDate = false;

    if ( xPS.is() )
    {
        try
        {
            xPS->setPropertyValue( PROPERTY_RESOURCERESOLVER, makeAny( Reference< resource::XStringResourceResolver >() ) );
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("toolkit.controls");
        }
    }

    // our "tab controller model" has potentially changed -> notify this
    implNotifyTabModelChange( aName );
}


sal_Bool SAL_CALL ControlModelContainerBase::getGroupControl(  )
{
    return true;
}


void SAL_CALL ControlModelContainerBase::setGroupControl( sal_Bool )
{
    SAL_WARN("toolkit", "explicit grouping not supported" );
}


void SAL_CALL ControlModelContainerBase::setControlModels( const Sequence< Reference< XControlModel > >& _rControls )
{
    SolarMutexGuard aGuard;

    // set the tab indexes according to the order of models in the sequence

    sal_Int16 nTabIndex = 1;

    for ( auto const & control : _rControls )
    {
        // look up the control in our own structure. This is to prevent invalid arguments
        UnoControlModelHolderVector::const_iterator aPos =
            ::std::find_if(
                maModels.begin(), maModels.end(),
                CompareControlModel( control )
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
        mbGroupsUpToDate = false;
    }
}


typedef ::std::multimap< sal_Int32, Reference< XControlModel > > MapIndexToModel;


Sequence< Reference< XControlModel > > SAL_CALL ControlModelContainerBase::getControlModels(  )
{
    SolarMutexGuard aGuard;

    MapIndexToModel aSortedModels;
        // will be the sorted container of all models which have a tab index property
    ::std::vector< Reference< XControlModel > > aUnindexedModels;
        // will be the container of all models which do not have a tab index property

    for ( const auto& rModel : maModels )
    {
        Reference< XControlModel > xModel( rModel.first );

        // see if the model has a TabIndex property
        Reference< XPropertySet > xControlProps( xModel, UNO_QUERY );
        Reference< XPropertySetInfo > xPSI;
        if ( xControlProps.is() )
            xPSI = xControlProps->getPropertySetInfo( );
        DBG_ASSERT( xPSI.is(), "ControlModelContainerBase::getControlModels: invalid child model!" );

        // has it?
        if ( xPSI.is() && xPSI->hasPropertyByName( getTabIndexPropertyName() ) )
        {   // yes
            sal_Int32 nTabIndex = -1;
            xControlProps->getPropertyValue( getTabIndexPropertyName() ) >>= nTabIndex;

            aSortedModels.emplace( nTabIndex, xModel );
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
            [] ( const MapIndexToModel::value_type& entryIndexToModel )
            { return entryIndexToModel.second; }
        );

    return aReturn;
}


void SAL_CALL ControlModelContainerBase::setGroup( const Sequence< Reference< XControlModel > >&, const OUString& )
{
    // not supported. We have only implicit grouping:
    // We only have a sequence of control models, and we _know_ (yes, that's a HACK relying on
    // implementation details) that VCL does grouping according to the order of controls automatically
    // At least VCL does this for all we're interested in: Radio buttons.
    SAL_WARN("toolkit", "grouping not supported" );
}

////----- XInitialization -------------------------------------------------------------------
void SAL_CALL ControlModelContainerBase::initialize (const Sequence<Any>& rArguments)
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
::sal_Int16 SAL_CALL ControlModelContainerBase::getTabPageID()
{
    return m_nTabPageId;
}
sal_Bool SAL_CALL ControlModelContainerBase::getEnabled()
{
    return m_bEnabled;
}
void SAL_CALL ControlModelContainerBase::setEnabled( sal_Bool _enabled )
{
    m_bEnabled = _enabled;
}
OUString SAL_CALL ControlModelContainerBase::getTitle()
{
    SolarMutexGuard aGuard;
    Reference<XPropertySet> xThis(*this,UNO_QUERY);
    OUString sTitle;
    xThis->getPropertyValue(GetPropertyName(BASEPROPERTY_TITLE)) >>= sTitle;
    return sTitle;
}
void SAL_CALL ControlModelContainerBase::setTitle( const OUString& _title )
{
    SolarMutexGuard aGuard;
    Reference<XPropertySet> xThis(*this,UNO_QUERY);
    xThis->setPropertyValue(GetPropertyName(BASEPROPERTY_TITLE),makeAny(_title));
}
OUString SAL_CALL ControlModelContainerBase::getImageURL()
{
    return m_sImageURL;
}
void SAL_CALL ControlModelContainerBase::setImageURL( const OUString& _imageurl )
{
    m_sImageURL = _imageurl;
}
OUString SAL_CALL ControlModelContainerBase::getToolTip()
{
    return m_sTooltip;
}
void SAL_CALL ControlModelContainerBase::setToolTip( const OUString& _tooltip )
{
    m_sTooltip = _tooltip;
}


namespace
{
    enum GroupingMachineState
    {
        eLookingForGroup,
        eExpandingGroup
    };


    sal_Int32 lcl_getDialogStep( const Reference< XControlModel >& _rxModel )
    {
        sal_Int32 nStep = 0;
        try
        {
            Reference< XPropertySet > xModelProps( _rxModel, UNO_QUERY );
            xModelProps->getPropertyValue( getStepPropertyName() ) >>= nStep;
        }
        catch (const Exception&)
        {
            css::uno::Any ex( cppu::getCaughtException() );
            SAL_WARN("toolkit", "caught an exception while determining the dialog page! " << exceptionToString(ex) );
        }
        return nStep;
    }
}


sal_Int32 SAL_CALL ControlModelContainerBase::getGroupCount(  )
{
    SolarMutexGuard aGuard;

    implUpdateGroupStructure();

    return maGroups.size();
}


void SAL_CALL ControlModelContainerBase::getGroup( sal_Int32 _nGroup, Sequence< Reference< XControlModel > >& _rGroup, OUString& _rName )
{
    SolarMutexGuard aGuard;

    implUpdateGroupStructure();

    if ( ( _nGroup < 0 ) || ( _nGroup >= static_cast<sal_Int32>(maGroups.size()) ) )
    {
        SAL_WARN("toolkit", "invalid argument and I am not allowed to throw an exception!" );
        _rGroup.realloc( 0 );
        _rName.clear();
    }
    else
    {
        AllGroups::const_iterator aGroupPos = maGroups.begin() + _nGroup;
        _rGroup.realloc( aGroupPos->size() );
        // copy the models
        ::std::copy( aGroupPos->begin(), aGroupPos->end(), _rGroup.getArray() );
        // give the group a name
        _rName = OUString::number( _nGroup );
    }
}


void SAL_CALL ControlModelContainerBase::getGroupByName( const OUString& _rName, Sequence< Reference< XControlModel > >& _rGroup )
{
    SolarMutexGuard aGuard;

    OUString sDummyName;
    getGroup( _rName.toInt32( ), _rGroup, sDummyName );
}


void SAL_CALL ControlModelContainerBase::addChangesListener( const Reference< XChangesListener >& _rxListener )
{
    maChangeListeners.addInterface( _rxListener );
}


void SAL_CALL ControlModelContainerBase::removeChangesListener( const Reference< XChangesListener >& _rxListener )
{
    maChangeListeners.removeInterface( _rxListener );
}


void ControlModelContainerBase::implNotifyTabModelChange( const OUString& _rAccessor )
{
    // multiplex to our change listeners:
    // the changes event
    ChangesEvent aEvent;
    aEvent.Source = *this;
    aEvent.Base <<= aEvent.Source;  // the "base of the changes root" is also ourself
    aEvent.Changes.realloc( 1 );    // exactly one change
    aEvent.Changes[ 0 ].Accessor <<= _rAccessor;


    std::vector< Reference< XInterface > > aChangeListeners( maChangeListeners.getElements() );
    for ( const auto& rListener : aChangeListeners )
    {
        if ( rListener.is() )
            static_cast< XChangesListener* >( rListener.get() )->changesOccurred( aEvent );
    }
}


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
    Reference< XServiceInfo > xModelSI;                 // for checking for a radio button
    AllGroups::iterator aCurrentGroup = maGroups.end(); // the group which we're currently building
    sal_Int32   nCurrentGroupStep = -1;                 // the step which all controls of the current group belong to


    for ( ; pControlModels != pControlModelsEnd; ++pControlModels )
    {
        // we'll need this in every state
        xModelSI.set(*pControlModels, css::uno::UNO_QUERY);
        // is it a radio button?
        bool bIsRadioButton = xModelSI.is() && xModelSI->supportsService( "com.sun.star.awt.UnoControlRadioButtonModel" );

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

            }
            break;

            case eExpandingGroup:
            {
                if ( !bIsRadioButton )
                {   // no radio button -> the group is done
                    aCurrentGroup = maGroups.end();
                    eState = eLookingForGroup;
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

                    continue;
                }

                // it's a radio button, but on a different page
                // -> we open a new group for it


                // open a new group
                size_t nGroups = maGroups.size();
                maGroups.resize( nGroups + 1 );
                aCurrentGroup = maGroups.begin() + nGroups;
                // and add the (only, til now) member
                aCurrentGroup->push_back( *pControlModels );

                nCurrentGroupStep = nThisModelStep;

                // state is the same: we still are looking for further elements of the current group
                eState = eExpandingGroup;
            }
            break;
        }
    }

    mbGroupsUpToDate = true;
}


void SAL_CALL ControlModelContainerBase::propertyChange( const PropertyChangeEvent& _rEvent )
{
    SolarMutexGuard aGuard;

    DBG_ASSERT( _rEvent.PropertyName == "TabIndex",
        "ControlModelContainerBase::propertyChange: not listening for this property!" );

    // the accessor for the changed element
    OUString sAccessor;
    UnoControlModelHolderVector::const_iterator aPos =
        ::std::find_if(
            maModels.begin(), maModels.end(),
            CompareControlModel( Reference< XControlModel >( _rEvent.Source, UNO_QUERY ) )
        );
    OSL_ENSURE( maModels.end() != aPos, "ControlModelContainerBase::propertyChange: don't know this model!" );
    if ( maModels.end() != aPos )
        sAccessor = aPos->second;

    // our groups are not up-to-date
    mbGroupsUpToDate = false;

    // notify
    implNotifyTabModelChange( sAccessor );
}


void SAL_CALL ControlModelContainerBase::disposing( const EventObject& /*rEvent*/ )
{
}


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


// = class ResourceListener


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
        xModifyBroadcaster.set( m_xResource, UNO_QUERY );
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
        xListener.set( m_xListener, UNO_QUERY );
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
        xListener.set( m_xListener, UNO_QUERY );
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


//  class DialogContainerControl

ControlContainerBase::ControlContainerBase( const Reference< XComponentContext >& rxContext )
    :ContainerControl_IBase()
    ,m_xContext(rxContext)
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

void ControlContainerBase::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer )
{
    SolarMutexGuard aGuard;
    UnoControlContainer::createPeer( rxToolkit, rParentPeer );
}

void ControlContainerBase::ImplInsertControl( Reference< XControlModel > const & rxModel, const OUString& rName )
{
    Reference< XPropertySet > xP( rxModel, UNO_QUERY );

    OUString aDefCtrl;
    xP->getPropertyValue( GetPropertyName( BASEPROPERTY_DEFAULTCONTROL ) ) >>= aDefCtrl;
    Reference < XControl > xCtrl( m_xContext->getServiceManager()->createInstanceWithContext(aDefCtrl, m_xContext), UNO_QUERY );

    DBG_ASSERT( xCtrl.is(), "ControlContainerBase::ImplInsertControl: could not create the control!" );
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

void ControlContainerBase::ImplRemoveControl( Reference< XControlModel > const & rxModel )
{
    Sequence< Reference< XControl > > aControls = getControls();
    Reference< XControl > xCtrl = StdTabController::FindControl( aControls, rxModel );
    if ( xCtrl.is() )
    {
        removeControl( xCtrl );
        try
        {
            xCtrl->dispose();
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("toolkit.controls");
        }
    }
}

void ControlContainerBase::ImplSetPosSize( Reference< XControl >& rxCtrl )
{
    Reference< XPropertySet > xP( rxCtrl->getModel(), UNO_QUERY );

    sal_Int32 nX = 0, nY = 0, nWidth = 0, nHeight = 0;
    xP->getPropertyValue("PositionX") >>= nX;
    xP->getPropertyValue("PositionY") >>= nY;
    xP->getPropertyValue("Width") >>= nWidth;
    xP->getPropertyValue("Height") >>= nHeight;
    MapMode aMode( MapUnit::MapAppFont );
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
        Reference< XWindowPeer > xPeer = ImplGetCompatiblePeer();
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
        sal_Int16 nW = nH/2;    // calculate average width?!

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

void ControlContainerBase::dispose()
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
{
    UnoControlContainer::disposing( Source );
}

sal_Bool ControlContainerBase::setModel( const Reference< XControlModel >& rxModel )
{
    SolarMutexGuard aGuard;

    // destroy the old tab controller, if existent
    if ( mxTabController.is() )
    {
        mxTabController->setModel( nullptr );                  // just to be sure, should not be necessary
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

    bool bRet = UnoControl::setModel( rxModel );

    if ( getModel().is() )
    {
        Reference< XNameAccess > xNA( getModel(), UNO_QUERY );
        if ( xNA.is() )
        {
            Sequence< OUString > aNames = xNA->getElementNames();
            const OUString* pNames = aNames.getConstArray();
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
void ControlContainerBase::setDesignMode( sal_Bool bOn )
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

void ControlContainerBase::elementInserted( const ContainerEvent& Event )
{
    SolarMutexGuard aGuard;

    Reference< XControlModel > xModel;
    OUString aName;

    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ENSURE_OR_RETURN_VOID( xModel.is(), "ControlContainerBase::elementInserted: illegal element!" );
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
        DBG_UNHANDLED_EXCEPTION("toolkit.controls");
    }
}

void ControlContainerBase::elementRemoved( const ContainerEvent& Event )
{
    SolarMutexGuard aGuard;

    Reference< XControlModel > xModel;
    Event.Element >>= xModel;
    ENSURE_OR_RETURN_VOID( xModel.is(), "ControlContainerBase::elementRemoved: illegal element!" );
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
        DBG_UNHANDLED_EXCEPTION("toolkit.controls");
    }
}

void ControlContainerBase::elementReplaced( const ContainerEvent& Event )
{
    SolarMutexGuard aGuard;

    Reference< XControlModel > xModel;
    Event.ReplacedElement >>= xModel;
    try
    {
        OSL_ENSURE( xModel.is(), "ControlContainerBase::elementReplaced: invalid ReplacedElement!" );
        if ( xModel.is() )
            ImplRemoveControl( xModel );
    }
    catch (const RuntimeException&)
    {
        throw;
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("toolkit.controls");
    }

    OUString aName;
    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ENSURE_OR_RETURN_VOID( xModel.is(), "ControlContainerBase::elementReplaced: invalid new element!" );
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
        DBG_UNHANDLED_EXCEPTION("toolkit.controls");
    }
}

// XPropertiesChangeListener
void ControlContainerBase::ImplModelPropertiesChanged( const Sequence< PropertyChangeEvent >& rEvents )
{
    if( !isDesignMode() && !mbCreatingCompatiblePeer )
    {
        sal_Int32 nLen = rEvents.getLength();
        for( sal_Int32 i = 0; i < nLen; i++ )
        {
            const PropertyChangeEvent& rEvt = rEvents.getConstArray()[i];
            Reference< XControlModel > xModel( rEvt.Source, UNO_QUERY );
            bool bOwnModel = xModel.get() == getModel().get();
            if ( ( rEvt.PropertyName == "PositionX" ) ||
                 ( rEvt.PropertyName == "PositionY" ) ||
                 ( rEvt.PropertyName == "Width" ) ||
                 ( rEvt.PropertyName == "Height" ) )
            {
                if ( bOwnModel )
                {
                    if ( !mbPosModified && !mbSizeModified )
                    {
                        // Don't set new pos/size if we get new values from window listener
                        Reference< XControl > xThis( static_cast<XAggregation*>(static_cast<cppu::OWeakAggObject*>(this)), UNO_QUERY );
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
            Sequence< OUString > aNames( 4 );
            OUString* pNames = aNames.getArray();
            *pNames++ = "PositionX";
            *pNames++ = "PositionY";
            *pNames++ = "Width";
            *pNames++ = "Height";

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

void SAL_CALL ControlContainerBase::changesOccurred( const ChangesEvent& )
{
    SolarMutexGuard aGuard;
    // a tab controller model may have changed

    // #109067# in design mode don't notify the tab controller
    // about tab index changes
    if ( mxTabController.is() && !mbDesignMode )
        mxTabController->activateTabOrder();
}
static void lcl_ApplyResolverToNestedContainees(  const Reference< resource::XStringResourceResolver >& xStringResourceResolver, const Reference< XControlContainer >& xContainer )
{
    OUString aPropName( PROPERTY_RESOURCERESOLVER );

    Any aNewStringResourceResolver;
    aNewStringResourceResolver <<= xStringResourceResolver;

    Sequence< OUString > aPropNames { aPropName };

    const Sequence< Reference< awt::XControl > > aSeq = xContainer->getControls();
    for ( sal_Int32 i = 0; i < aSeq.getLength(); i++ )
    {
        Reference< XControl > xControl( aSeq[i] );
        Reference< XPropertySet > xPropertySet;

        if ( xControl.is() )
            xPropertySet.set( xControl->getModel(), UNO_QUERY );

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
                xPropertySet->setPropertyValue( aPropName, aNewStringResourceResolver );
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
    Reference< resource::XStringResourceResolver > xStringResourceResolver;

    ImplGetPropertyValue( PROPERTY_RESOURCERESOLVER ) >>= xStringResourceResolver;
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

OUString getPhysicalLocation( const css::uno::Any& rbase, const css::uno::Any& rUrl )
{

    OUString baseLocation;
    OUString url;

    rbase  >>= baseLocation;
    rUrl  >>= url;

    OUString absoluteURL( url );
    if ( !url.isEmpty() )
    {
        INetURLObject urlObj(baseLocation);
        urlObj.removeSegment();
        baseLocation = urlObj.GetMainURL( INetURLObject::DecodeMechanism::NONE );

        const INetURLObject protocolCheck( url );
        const INetProtocol protocol = protocolCheck.GetProtocol();
        if ( protocol == INetProtocol::NotValid )
        {
            OUString testAbsoluteURL;
            if ( ::osl::FileBase::E_None == ::osl::FileBase::getAbsoluteFileURL( baseLocation, url, testAbsoluteURL ) )
                absoluteURL = testAbsoluteURL;
        }
    }

    return absoluteURL;
}

void
ControlModelContainerBase::updateUserFormChildren( const Reference< XNameContainer >& xAllChildren, const OUString& aName, ChildOperation Operation, const css::uno::Reference< css::awt::XControlModel >& xTarget )
{
    if ( Operation < Insert || Operation > Remove )
        throw IllegalArgumentException();

    if ( !xAllChildren.is() )
        throw IllegalArgumentException();

    if ( Operation == Remove )
    {
        Reference< XControlModel > xOldModel( xAllChildren->getByName( aName ), UNO_QUERY );
        xAllChildren->removeByName( aName );

        Reference< XNameContainer > xChildContainer( xOldModel, UNO_QUERY );
        if ( xChildContainer.is() )
        {
            Reference< XPropertySet > xProps( xChildContainer, UNO_QUERY );
            // container control is being removed from this container, reset the
            // global list of containers
            if ( xProps.is() )
                xProps->setPropertyValue(  GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ), uno::makeAny( uno::Reference< XNameContainer >() ) );
            Sequence< OUString > aChildNames = xChildContainer->getElementNames();
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
            // global list of containers to point to the correct global list
            Reference< XPropertySet > xProps( xChildContainer, UNO_QUERY );
            if ( xProps.is() )
                xProps->setPropertyValue(  GetPropertyName( BASEPROPERTY_USERFORMCONTAINEES ), uno::makeAny( xAllChildren ) );
            Sequence< OUString > aChildNames = xChildContainer->getElementNames();
            for ( sal_Int32 index=0; index< aChildNames.getLength(); ++index )
            {
                Reference< XControlModel > xChildTarget( xChildContainer->getByName( aChildNames[ index ] ), UNO_QUERY );
                updateUserFormChildren( xAllChildren, aChildNames[ index ], Operation, xChildTarget );
            }
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
