/*************************************************************************
 *
 *  $RCSfile: dialogcontrol.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-27 17:03:19 $
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

#ifndef TOOLKIT_DIALOG_CONTROL_HXX
#include <toolkit/controls/dialogcontrol.hxx>
#endif
#ifndef _TOOLKIT_HELPER_PROPERTY_HXX_
#include <toolkit/helper/property.hxx>
#endif
#ifndef _TOOLKIT_HELPER_UNOPROPERTYARRAYHELPER_HXX_
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#endif
#ifndef _TOOLKIT_HELPERS_GEOMETRYCONTROLMODEL_HXX_
#include <toolkit/controls/geometrycontrolmodel.hxx>
#endif
#ifndef _TOOLKIT_HELPER_UNOCONTROLS_HXX_
#include <toolkit/controls/unocontrols.hxx>
#endif
#include "toolkit/controls/formattedcontrol.hxx"
#ifndef _TOOLKIT_CONTROLS_STDTABCONTROLLER_HXX_
#include <toolkit/controls/stdtabcontroller.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <vcl/outdev.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif

#include <map>
#include <algorithm>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace toolkit;

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

    Any aBool;
    aBool <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
}

UnoControlDialogModel::UnoControlDialogModel( const UnoControlDialogModel& rModel )
    :UnoControlModel( rModel )
    ,maContainerListeners( *this )
    ,maChangeListeners ( GetMutex() )
    ,mbGroupsUpToDate( sal_False )
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
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        Any aAny;
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
        Sequence<sal_Int32> aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

void SAL_CALL UnoControlDialogModel::dispose(  ) throw(RuntimeException)
{
    // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
    // tell our listeners
    {
        ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

        EventObject aDisposeEvent;
        aDisposeEvent.Source = static_cast< XAggregation* >( static_cast< ::cppu::OWeakAggObject* >( this ) );

        maContainerListeners.disposeAndClear( aDisposeEvent );
        maChangeListeners.disposeAndClear( aDisposeEvent );
    }

    // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
    // call the base class
    UnoControlModel::dispose();

    // 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
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
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlProgressBarModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlProgressBarModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlScrollBarModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlScrollBarModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoControlFixedLineModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoControlFixedLineModel >;

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
        pNamesSeq = new Sequence< ::rtl::OUString >( 19 );
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
    Any aElement;
    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() != aElementPos )
        aElement <<= aElementPos->first;
    return aElement;
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
    Reference< XControlModel > xM;
    aElement >>= xM;

    if ( !aName.getLength() || !xM.is() )
        lcl_throwIllegalArgumentException();

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
    UnoControlModelHolderList::iterator aElementPos = ImplFindElement( aName );
    if ( maModels.end() != aElementPos )
    {
        ContainerEvent aEvent;
        aEvent.Source = *this;
        aEvent.Element <<= aElementPos->first;
        aEvent.Accessor <<= aName;
        maContainerListeners.elementRemoved( aEvent );

        stopControlListening( aElementPos->first );
        maModels.erase( aElementPos );
        mbGroupsUpToDate = sal_False;
    }

    // our "tab controller model" has potentially changed -> notify this
    implNotifyTabModelChange( aName );
}

// ----------------------------------------------------------------------------
sal_Bool SAL_CALL UnoControlDialogModel::getGroupControl(  ) throw (RuntimeException)
{
    return sal_True;
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::setGroupControl( sal_Bool GroupControl ) throw (RuntimeException)
{
    DBG_ERROR( "UnoControlDialogModel::setGroupControl: explicit grouping not supported" );
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::setControlModels( const Sequence< Reference< XControlModel > >& _rControls ) throw (RuntimeException)
{
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
void SAL_CALL UnoControlDialogModel::setGroup( const Sequence< Reference< XControlModel > >& Group, const ::rtl::OUString& GroupName ) throw (RuntimeException)
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
    implUpdateGroupStructure();

    return maGroups.size();
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::getGroup( sal_Int32 _nGroup, Sequence< Reference< XControlModel > >& _rGroup, ::rtl::OUString& _rName ) throw (RuntimeException)
{
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

#ifdef DEBUG
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

#ifdef DEBUG
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
#ifdef DEBUG
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

#ifdef DEBUG
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
#ifdef DEBUG
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
#ifdef DEBUG
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
    DBG_ASSERT( 0 == _rEvent.PropertyName.compareToAscii( "TabIndex" ),
        "UnoControlDialogModel::propertyChange: not listening for this property!" );

    // the accessor for the changed element
    ::rtl::OUString sAccessor;
    UnoControlModelHolderList::const_iterator aPos =
        ::std::find_if(
            maModels.begin(), maModels.end(),
            CompareControlModel( Reference< XControlModel >( _rEvent.Source, UNO_QUERY ) )
        );
    DBG_ASSERT( maModels.end() != aPos, "UnoControlDialogModel::propertyChange: don't know this model!" );
    if ( maModels.end() != aPos )
        sAccessor = aPos->second;

    // our groups are not up-to-date
    mbGroupsUpToDate = sal_False;

    // notify
    implNotifyTabModelChange( sAccessor );
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::disposing( const EventObject& evt ) throw (RuntimeException)
{
    // TODO
}

// ----------------------------------------------------------------------------
void UnoControlDialogModel::startControlListening( const Reference< XControlModel >& _rxChildModel )
{
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
    Reference< XPropertySet > xModelProps( _rxChildModel, UNO_QUERY );
    Reference< XPropertySetInfo > xPSI;
    if ( xModelProps.is() )
        xPSI = xModelProps->getPropertySetInfo();

    if ( xPSI.is() && xPSI->hasPropertyByName( getTabIndexPropertyName() ) )
        xModelProps->removePropertyChangeListener( getTabIndexPropertyName(), this );
}

// ============================================================================
// = class UnoDialogControl
// ============================================================================
UnoDialogControl::UnoDialogControl() : maTopWindowListeners( *this )
{
    maComponentInfos.nWidth = 300;
    maComponentInfos.nHeight = 450;
}

::rtl::OUString UnoDialogControl::GetComponentServiceName()
{
    return ::rtl::OUString::createFromAscii( "Dialog" );
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
    EventObject aEvt;
    aEvt.Source = static_cast< ::cppu::OWeakObject* >( this );
    maTopWindowListeners.disposeAndClear( aEvt );

    UnoControlContainer::dispose();
}

sal_Bool UnoDialogControl::setModel( const Reference< XControlModel >& rxModel ) throw(RuntimeException)
{
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

    return bRet;
}

void UnoDialogControl::setDesignMode( sal_Bool bOn ) throw(RuntimeException)
{
    ::osl::Guard< ::osl::Mutex > aGuard( GetMutex() );

    UnoControl::setDesignMode( bOn );

    Sequence< Reference< XControl > > xCtrls = getControls();
    sal_Int32 nControls = xCtrls.getLength();
    Reference< XControl >* pControls = xCtrls.getArray();
    for ( sal_Int32 n = 0; n < nControls; n++ )
        pControls[n]->setDesignMode( bOn );
}

void UnoDialogControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
{
    UnoControlContainer::createPeer( rxToolkit, rParentPeer );

    Reference < XTopWindow > xTW( getPeer(), UNO_QUERY );
    xTW->setMenuBar( mxMenuBar );
    if ( maTopWindowListeners.getLength() )
        xTW->addTopWindowListener( &maTopWindowListeners );
}

void UnoDialogControl::elementInserted( const ContainerEvent& Event ) throw(RuntimeException)
{
    Reference< XControlModel > xModel;
    ::rtl::OUString aName;

    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ImplInsertControl( xModel, aName );
}

void UnoDialogControl::elementRemoved( const ContainerEvent& Event ) throw(RuntimeException)
{
    Reference< XControlModel > xModel;
    Event.Element >>= xModel;
    if ( xModel.is() )
        ImplRemoveControl( xModel );
}

void UnoDialogControl::elementReplaced( const ContainerEvent& Event ) throw(RuntimeException)
{
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
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toFront();
    }
}

void UnoDialogControl::toBack(  ) throw (RuntimeException)
{
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toBack();
    }
}

void UnoDialogControl::setMenuBar( const Reference< XMenuBar >& rxMenuBar ) throw (RuntimeException)
{
    mxMenuBar = rxMenuBar;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->setMenuBar( mxMenuBar );
    }
}

// XPropertiesChangeListener
void UnoDialogControl::propertiesChange( const Sequence< PropertyChangeEvent >& rEvents ) throw(RuntimeException)
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
            const PropertyChangeEvent& rEvt = rEvents.getConstArray()[i];
            Reference< XControlModel > xModel( rEvt.Source, UNO_QUERY );
            sal_Bool bOwnModel = (XControlModel*)xModel.get() == (XControlModel*)getModel().get();
            if ( ( rEvt.PropertyName == s1 ) || ( rEvt.PropertyName == s2 ) || ( rEvt.PropertyName == s3 ) || ( rEvt.PropertyName == s4 ) )
            {
                if ( bOwnModel )
                {
                    Reference< XControl > xThis( (XAggregation*)(::cppu::OWeakAggObject*)this, UNO_QUERY );
                    ImplSetPosSize( xThis );
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

    UnoControlContainer::propertiesChange( rEvents );
}

void UnoDialogControl::setTitle( const ::rtl::OUString& Title ) throw(RuntimeException)
{
    Any aAny;
    aAny <<= Title;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ), aAny, sal_True );
}

::rtl::OUString UnoDialogControl::getTitle() throw(RuntimeException)
{
    return ImplGetPropertyValue_UString( BASEPROPERTY_TITLE );
}

sal_Int16 UnoDialogControl::execute() throw(RuntimeException)
{
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
    UnoControlContainer::removingControl( _rxControl );

    if ( _rxControl.is() )
    {
        Reference< XMultiPropertySet > xProps( _rxControl->getModel(), UNO_QUERY );
        if ( xProps.is() )
            xProps->removePropertiesChangeListener( this );
    }

}

void SAL_CALL UnoDialogControl::changesOccurred( const ChangesEvent& Event ) throw (RuntimeException)
{
    // a tab controller model may have changed
    if ( mxTabController.is() )
        mxTabController->activateTabOrder();
}
