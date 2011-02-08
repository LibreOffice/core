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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_toolkit.hxx"

#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/wall.hxx>
#include <osl/mutex.hxx>
#include <toolkit/controls/dialogcontrol.hxx>
#include <toolkit/helper/property.hxx>
#include <toolkit/helper/unopropertyarrayhelper.hxx>
#include <toolkit/controls/geometrycontrolmodel.hxx>
#include <toolkit/controls/unocontrols.hxx>
#include "toolkit/controls/formattedcontrol.hxx"
#include "toolkit/controls/roadmapcontrol.hxx"
#include "toolkit/controls/tkscrollbar.hxx"
#include <toolkit/controls/stdtabcontroller.hxx>
#include <toolkit/controls/tksimpleanimation.hxx>
#include <toolkit/controls/tkthrobber.hxx>

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
#include "grid/gridcontrol.hxx"

#include <map>
#include <boost/unordered_map.hpp>
#include <algorithm>
#include <o3tl/compat_functional.hxx>
#include "tools/urlobj.hxx"
#include "osl/file.hxx"
#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>
#include <vcl/button.hxx>
#include <toolkit/awt/vclxwindows.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::script;
using namespace toolkit;

#define PROPERTY_RESOURCERESOLVER ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ResourceResolver" ))
#define PROPERTY_DIALOGSOURCEURL ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DialogSourceURL" ))
#define PROPERTY_IMAGEURL ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ImageURL" ))
#define PROPERTY_GRAPHIC ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Graphic" ))

//HELPER
::rtl::OUString getPhysicalLocation( const ::com::sun::star::uno::Any& rbase, const ::com::sun::star::uno::Any& rUrl );

uno::Reference< graphic::XGraphic > getGraphicFromURL_nothrow( uno::Reference< graphic::XGraphicObject >& rxGrfObj, const ::rtl::OUString& _rURL );

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

    static ::rtl::OUString lcl_GetStringProperty( const ::rtl::OUString& sProperty, const Reference< XPropertySet >& xSet )
    {
        ::rtl::OUString sValue;
        Reference< XPropertySetInfo > xPSI;
        if (xSet.is() && (xPSI = xSet->getPropertySetInfo()).is() &&
                xPSI->hasPropertyByName( sProperty ) )
        {
            xSet->getPropertyValue( sProperty ) >>= sValue;
        }
        return sValue;
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

// we probably will need both a hash of control models and hash of controls
// => use some template magic

typedef ::cppu::WeakImplHelper1< container::XNameContainer > SimpleNameContainer_BASE;

template< typename T >
class SimpleNamedThingContainer : public SimpleNameContainer_BASE
{
    typedef boost::unordered_map< rtl::OUString, Reference< T >, ::rtl::OUStringHash,
       ::std::equal_to< ::rtl::OUString > > NamedThingsHash;
    NamedThingsHash things;
    ::osl::Mutex m_aMutex;
public:
    // ::com::sun::star::container::XNameContainer, XNameReplace, XNameAccess
    virtual void SAL_CALL replaceByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw NoSuchElementException();
        Reference< T > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw IllegalArgumentException();
        things[ aName ] = xElement;
    }
    virtual Any SAL_CALL getByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw NoSuchElementException();
        return uno::makeAny( things[ aName ] );
    }
    virtual Sequence< ::rtl::OUString > SAL_CALL getElementNames(  ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Sequence< ::rtl::OUString > aResult( things.size() );
        typename NamedThingsHash::iterator it = things.begin();
        typename NamedThingsHash::iterator it_end = things.end();
        rtl::OUString* pName = aResult.getArray();
        for (; it != it_end; ++it, ++pName )
            *pName = it->first;
        return aResult;
    }
    virtual sal_Bool SAL_CALL hasByName( const ::rtl::OUString& aName ) throw(RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( things.find( aName ) != things.end() );
    }
    virtual void SAL_CALL insertByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( hasByName( aName ) )
            throw ElementExistException();
        Reference< T > xElement;
        if ( ! ( aElement >>= xElement ) )
            throw IllegalArgumentException();
        things[ aName ] = xElement;
    }
    virtual void SAL_CALL removeByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if ( !hasByName( aName ) )
            throw NoSuchElementException();
        things.erase( things.find( aName ) );
    }
    virtual Type SAL_CALL getElementType(  ) throw (RuntimeException)
    {
        return T::static_type( NULL );
    }
    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        return ( things.size() > 0 );
    }
};


//  ----------------------------------------------------
//  class UnoControlDialogModel
//  ----------------------------------------------------
UnoControlDialogModel::UnoControlDialogModel( bool regProps )
    :maContainerListeners( *this )
    ,maChangeListeners ( GetMutex() )
    ,mbGroupsUpToDate( sal_False ), mbAdjustingGraphic( false )
{
    if ( !regProps ) // Register properties elsewhere ( probably in class derived from this )
        return;

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
    // #TODO separate class for 'UserForm' ( instead of re-using Dialog ? )
    uno::Reference< XNameContainer > xNameCont = new SimpleNamedThingContainer< XControlModel >();
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES, uno::makeAny( xNameCont ) );
}

UnoControlDialogModel::UnoControlDialogModel( const UnoControlDialogModel& rModel )
    : UnoControlDialogModel_IBase( rModel )
    , UnoControlDialogModel_Base( rModel )
    , maContainerListeners( *this )
    , maChangeListeners ( GetMutex() )
    , mbGroupsUpToDate( sal_False ), mbAdjustingGraphic( false )
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

void SAL_CALL UnoControlDialogModel::setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue ) throw (::com::sun::star::uno::Exception)
{
    UnoControlModel::setFastPropertyValue_NoBroadcast( nHandle, rValue );
    try
    {
        switch ( nHandle )
        {
        case BASEPROPERTY_IMAGEURL:
            if ( !mbAdjustingGraphic && ImplHasProperty( BASEPROPERTY_GRAPHIC ) )
            {
                mbAdjustingGraphic = true;
                ::rtl::OUString sImageURL;
                OSL_VERIFY( rValue >>= sImageURL );
                setPropertyValue( GetPropertyName( BASEPROPERTY_GRAPHIC ), uno::makeAny( ImageHelper::getGraphicAndGraphicObjectFromURL_nothrow( mxGrfObj, sImageURL ) ) );
                mbAdjustingGraphic = false;
            }
            break;

        case BASEPROPERTY_GRAPHIC:
            if ( !mbAdjustingGraphic && ImplHasProperty( BASEPROPERTY_IMAGEURL ) )
            {
                mbAdjustingGraphic = true;
                setPropertyValue( GetPropertyName( BASEPROPERTY_IMAGEURL ), uno::makeAny( ::rtl::OUString() ) );
                mbAdjustingGraphic = false;
            }
            break;
    }
    }
    catch( const ::com::sun::star::uno::Exception& )
    {
        OSL_ENSURE( sal_False, "UnoControlDialogModel::setFastPropertyValue_NoBroadcast: caught an exception while setting Graphic/ImageURL properties!" );
    }
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
        ::o3tl::select1st< UnoControlModelHolder >( )   // operation to apply -> select the XControlModel part
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
    SolarMutexGuard aSolarGuard;

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
    else if ( aServiceSpecifier.compareToAscii( szServiceName_GridControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoGridModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoMultiPageModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoMultiPageModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName_UnoFrameModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoFrameModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoSimpleAnimationControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoSimpleAnimationControlModel >;
    else if ( aServiceSpecifier.compareToAscii( szServiceName2_UnoThrobberControlModel ) == 0 )
        pNewModel = new OGeometryControlModel< UnoThrobberControlModel >;

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
                if ( xSI->supportsService( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.awt.UnoControlModel")) ) )
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
        pNames[22] = ::rtl::OUString::createFromAscii( szServiceName2_UnoSimpleAnimationControlModel );
        pNames[23] = ::rtl::OUString::createFromAscii( szServiceName2_UnoThrobberControlModel );
        pNames[24] = ::rtl::OUString::createFromAscii( szServiceName_UnoMultiPageModel );
        pNames[25] = ::rtl::OUString::createFromAscii( szServiceName_UnoFrameModel );
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

void UnoControlDialogModel::updateUserFormChildren( const Reference< XNameContainer >& xAllChildren, const rtl::OUString& aName, ChildOperation Operation, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xTarget ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
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

sal_Bool UnoControlDialogModel::hasElements() throw(RuntimeException)
{
    return !maModels.empty();
}

// XNameContainer, XNameReplace, XNameAccess
void UnoControlDialogModel::replaceByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;

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
        ::o3tl::select2nd< UnoControlModelHolder >()        // operator to apply: select the second element (the name)
    );

    return aNames;
}

sal_Bool UnoControlDialogModel::hasByName( const ::rtl::OUString& aName ) throw(RuntimeException)
{
    return maModels.end() != ImplFindElement( aName );
}

void UnoControlDialogModel::insertByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;

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

                    ::rtl::OUString absoluteUrl;
                    aUrl >>= absoluteUrl;
                    if ( absoluteUrl.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPREFIX ) ) == 0 )
                        xProps.get()->setPropertyValue(  sImageSourceProperty , aUrl );
                    // Now we inherit from this class, no all containers have
                    // DialogSourceURL
                    else if ( getPropertySetInfo()->hasPropertyByName( GetPropertyName( BASEPROPERTY_DIALOGSOURCEURL ) ) )
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

void UnoControlDialogModel::removeByName( const ::rtl::OUString& aName ) throw(NoSuchElementException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aSolarGuard;

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
    SolarMutexGuard aSolarGuard;

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
    SolarMutexGuard aSolarGuard;

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
            ::o3tl::select2nd< MapIndexToModel::value_type >( )
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
    SolarMutexGuard aSolarGuard;

    implUpdateGroupStructure();

    return maGroups.size();
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::getGroup( sal_Int32 _nGroup, Sequence< Reference< XControlModel > >& _rGroup, ::rtl::OUString& _rName ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

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
    SolarMutexGuard aSolarGuard;

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
void UnoControlDialogModel::AddRadioButtonGroup (
        ::std::map< ::rtl::OUString, ModelGroup >& rNamedGroups )
{
    if ( rNamedGroups.size() == 0 )
        return;

    size_t nGroups = maGroups.size();
    maGroups.reserve( nGroups + rNamedGroups.size() );
    ::std::map< ::rtl::OUString, ModelGroup >::const_iterator i = rNamedGroups.begin(), e = rNamedGroups.end();
    for( ; i != e; ++i)
    {
            maGroups.push_back( i->second );
    }

    rNamedGroups.clear();
}

void UnoControlDialogModel::AddRadioButtonToGroup (
        const Reference< XControlModel >& rControlModel,
        const ::rtl::OUString& rPropertyName,
        ::std::map< ::rtl::OUString, ModelGroup >& rNamedGroups,
        ModelGroup*& rpCurrentGroup )
{
    Reference< XPropertySet > xCurProps( rControlModel, UNO_QUERY );
    ::rtl::OUString sGroup = lcl_GetStringProperty( rPropertyName, xCurProps );
    const sal_Int32 nControlModelStep = lcl_getDialogStep( rControlModel );

    if ( sGroup.getLength() == 0 )
    {
        // Create a new group if:
        if ( maGroups.size() == 0 ||                // no groups
                rpCurrentGroup == NULL ||           // previous group was closed
                (nControlModelStep != 0 &&          // control step matches current group
                 maGroups.back().size() > 0 &&      //  (group 0 == display everywhere)
                 nControlModelStep != lcl_getDialogStep( maGroups.back().back() ) ) )
        {
            size_t nGroups = maGroups.size();
            maGroups.resize( nGroups + 1 );
        }
        rpCurrentGroup = &maGroups.back();
    }
    else
    {
        // Different steps get different sets of named groups
        if ( rNamedGroups.size() > 0 &&
                rNamedGroups.begin()->second.size() > 0 )
        {
            const sal_Int32 nPrevStep = lcl_getDialogStep( rNamedGroups.begin()->second.front() );
            if ( nControlModelStep != nPrevStep )
                AddRadioButtonGroup( rNamedGroups );
        }

        rpCurrentGroup = &rNamedGroups[ sGroup ];
    }
    rpCurrentGroup->push_back( rControlModel );
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
    ModelGroup* aCurrentGroup = NULL;                   // the group which we're currently building
    sal_Bool    bIsRadioButton;                         // is it a radio button?

    const ::rtl::OUString GROUP_NAME( RTL_CONSTASCII_USTRINGPARAM( "GroupName" ) );

    ::std::map< ::rtl::OUString, ModelGroup > aNamedGroups;

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
                AddRadioButtonToGroup( *pControlModels, GROUP_NAME, aNamedGroups, aCurrentGroup );

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
                    aCurrentGroup = NULL;
                    eState = eLookingForGroup;
#if OSL_DEBUG_LEVEL > 1
                    aCurrentGroupLabels.clear();
#endif
                    continue;
                }

                AddRadioButtonToGroup( *pControlModels, GROUP_NAME, aNamedGroups, aCurrentGroup );

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

    AddRadioButtonGroup( aNamedGroups );
    mbGroupsUpToDate = sal_True;
}

// ----------------------------------------------------------------------------
void SAL_CALL UnoControlDialogModel::propertyChange( const PropertyChangeEvent& _rEvent ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;

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
    SolarMutexGuard aSolarGuard;

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
    SolarMutexGuard aSolarGuard;

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
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Dialog"));
    else
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("TabPage"));
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
    SolarMutexGuard aSolarGuard;

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
    SolarMutexGuard aSolarGuard;

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
    SolarMutexGuard aSolarGuard;
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
    SolarMutexGuard aSolarGuard;

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
    if (( ImplGetPropertyValue( PROPERTY_IMAGEURL ) >>= aImageURL ) &&
        ( aImageURL.getLength() > 0 ))
    {
        rtl::OUString absoluteUrl( aImageURL );
        if ( aImageURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPREFIX ) ) != 0 )
            absoluteUrl =
                getPhysicalLocation( ImplGetPropertyValue( PROPERTY_DIALOGSOURCEURL ),
                                 ImplGetPropertyValue( PROPERTY_IMAGEURL ));
        ImplSetPropertyValue( PROPERTY_IMAGEURL, uno::makeAny( absoluteUrl ), sal_True );
    }
}

void UnoDialogControl::elementInserted( const ContainerEvent& Event ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    Reference< XControlModel > xModel;
    ::rtl::OUString aName;

    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ImplInsertControl( xModel, aName );
}

void UnoDialogControl::elementRemoved( const ContainerEvent& Event ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    Reference< XControlModel > xModel;
    Event.Element >>= xModel;
    if ( xModel.is() )
        ImplRemoveControl( xModel );
}

void UnoDialogControl::elementReplaced( const ContainerEvent& Event ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

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
    SolarMutexGuard aSolarGuard;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toFront();
    }
}

void UnoDialogControl::toBack(  ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    if ( getPeer().is() )
    {
        Reference< XTopWindow > xTW( getPeer(), UNO_QUERY );
        if( xTW.is() )
            xTW->toBack();
    }
}

void UnoDialogControl::setMenuBar( const Reference< XMenuBar >& rxMenuBar ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
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
            ::rtl::OUString absoluteUrl( aImageURL );
            // Ignore GraphicObject urls
            if (( ImplGetPropertyValue( PROPERTY_IMAGEURL ) >>= aImageURL ) &&
                ( aImageURL.getLength() > 0 ))
            {
                absoluteUrl = aImageURL;
                if ( aImageURL.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPREFIX ) ) != 0 )
                    absoluteUrl =
                        getPhysicalLocation( ImplGetPropertyValue( PROPERTY_DIALOGSOURCEURL ),
                                         ImplGetPropertyValue( PROPERTY_IMAGEURL ));

                ImplSetPropertyValue( PROPERTY_IMAGEURL, uno::makeAny( absoluteUrl ), sal_True );
            }
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

void SAL_CALL UnoDialogControl::endDialog( ::sal_Int32 i_result ) throw (RuntimeException)
{
    Reference< XDialog2 > xPeerDialog( getPeer(), UNO_QUERY );
    if ( xPeerDialog.is() )
        xPeerDialog->endDialog( i_result );
}

void SAL_CALL UnoDialogControl::setHelpId( ::sal_Int32 i_id ) throw (RuntimeException)
{
    Reference< XDialog2 > xPeerDialog( getPeer(), UNO_QUERY );
    if ( xPeerDialog.is() )
        xPeerDialog->setHelpId( i_id );
}

void UnoDialogControl::setTitle( const ::rtl::OUString& Title ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    Any aAny;
    aAny <<= Title;
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ), aAny, sal_True );
}

::rtl::OUString UnoDialogControl::getTitle() throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    return ImplGetPropertyValue_UString( BASEPROPERTY_TITLE );
}

sal_Int16 UnoDialogControl::execute() throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;
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
    SolarMutexGuard aSolarGuard;
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
    SolarMutexGuard aSolarGuard;
    UnoControlContainer::addingControl( _rxControl );

    if ( _rxControl.is() )
    {
        Reference< XMultiPropertySet > xProps( _rxControl->getModel(), UNO_QUERY );
        if ( xProps.is() )
        {
            Sequence< ::rtl::OUString > aNames( 4 );
            ::rtl::OUString* pNames = aNames.getArray();
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionX"));
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionY"));
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width"));
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height"));

            xProps->addPropertiesChangeListener( aNames, this );
        }
    }
}

void UnoDialogControl::removingControl( const Reference< XControl >& _rxControl )
{
    SolarMutexGuard aSolarGuard;
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
    SolarMutexGuard aSolarGuard;
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

    ::rtl::OUString absoluteURL( url );
    if ( url.getLength() > 0 )
    {
        // Don't adjust GraphicObject url(s)
        if ( url.compareToAscii( UNO_NAME_GRAPHOBJ_URLPREFIX, RTL_CONSTASCII_LENGTH( UNO_NAME_GRAPHOBJ_URLPREFIX ) ) != 0 )
        {
            INetURLObject urlObj(baseLocation);
            urlObj.removeSegment();
            baseLocation = urlObj.GetMainURL( INetURLObject::NO_DECODE );
            ::osl::FileBase::getAbsoluteFileURL( baseLocation, url, ret );
        }
        else
            ret = url;

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

UnoParentControl::UnoParentControl() :
    mbSizeModified(false),
    mbPosModified(false)
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
    // #TODO Do we need the ResourceListener foo ?
}

UnoParentControl::~UnoParentControl()
{
}

// XInterface
Any UnoParentControl::queryAggregation( const Type & rType ) throw(RuntimeException)
{
    Any aRet( UnoParentControl_IBase::queryInterface( rType ) );
    return (aRet.hasValue() ? aRet : UnoControlContainer::queryAggregation( rType ));
}

// XTypeProvider
IMPL_IMPLEMENTATION_ID( UnoParentControl )
Sequence< Type >
UnoParentControl::getTypes() throw(RuntimeException)
{
    return ::comphelper::concatSequences(
        UnoParentControl_IBase::getTypes(),
        UnoControlContainer::getTypes()
    );
}

void UnoParentControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    UnoControlContainer::createPeer( rxToolkit, rParentPeer );
}

void UnoParentControl::ImplInsertControl( Reference< XControlModel >& rxModel, const ::rtl::OUString& rName )
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

void UnoParentControl::ImplRemoveControl( Reference< XControlModel >& rxModel )
{
    Sequence< Reference< XControl > > aControls = getControls();
    Reference< XControl > xCtrl = StdTabController::FindControl( aControls, rxModel );
    if ( xCtrl.is() )
        removeControl( xCtrl );
}

void UnoParentControl::ImplSetPosSize( Reference< XControl >& rxCtrl )
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

void UnoParentControl::dispose() throw(RuntimeException)
{
    UnoControlContainer::dispose();
}

void SAL_CALL UnoParentControl::disposing(
    const EventObject& Source )
throw(RuntimeException)
{
    UnoControlContainer::disposing( Source );
}

sal_Bool UnoParentControl::setModel( const Reference< XControlModel >& rxModel ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

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
//    ImplStartListingForResourceEvents();

    return bRet;
}
void UnoParentControl::setDesignMode( sal_Bool bOn ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

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

void UnoParentControl::elementInserted( const ContainerEvent& Event ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    Reference< XControlModel > xModel;
    ::rtl::OUString aName;

    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ImplInsertControl( xModel, aName );
}

void UnoParentControl::elementRemoved( const ContainerEvent& Event ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    Reference< XControlModel > xModel;
    Event.Element >>= xModel;
    if ( xModel.is() )
        ImplRemoveControl( xModel );
}

void UnoParentControl::elementReplaced( const ContainerEvent& Event ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    Reference< XControlModel > xModel;
    Event.ReplacedElement >>= xModel;
    if ( xModel.is() )
        ImplRemoveControl( xModel );

    ::rtl::OUString aName;
    Event.Accessor >>= aName;
    Event.Element >>= xModel;
    ImplInsertControl( xModel, aName );
}

// XPropertiesChangeListener
void UnoParentControl::ImplModelPropertiesChanged( const Sequence< PropertyChangeEvent >& rEvents ) throw(RuntimeException)
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

void UnoParentControl::addingControl( const Reference< XControl >& _rxControl )
{
    SolarMutexGuard aSolarGuard;
    UnoControlContainer::addingControl( _rxControl );

    if ( _rxControl.is() )
    {
        Reference< XMultiPropertySet > xProps( _rxControl->getModel(), UNO_QUERY );
        if ( xProps.is() )
        {
            Sequence< ::rtl::OUString > aNames( 4 );
            ::rtl::OUString* pNames = aNames.getArray();
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionX"));
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("PositionY"));
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Width"));
            *pNames++ = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Height"));

            xProps->addPropertiesChangeListener( aNames, this );
        }
    }
}

void UnoParentControl::removingControl( const Reference< XControl >& _rxControl )
{
    SolarMutexGuard aSolarGuard;
    UnoControlContainer::removingControl( _rxControl );

    if ( _rxControl.is() )
    {
        Reference< XMultiPropertySet > xProps( _rxControl->getModel(), UNO_QUERY );
        if ( xProps.is() )
            xProps->removePropertiesChangeListener( this );
    }

}

void SAL_CALL UnoParentControl::changesOccurred( const ChangesEvent& ) throw (RuntimeException)
{
    SolarMutexGuard aSolarGuard;
    // a tab controller model may have changed

    // #109067# in design mode don't notify the tab controller
    // about tab index changes
    if ( mxTabController.is() && !mbDesignMode )
        mxTabController->activateTabOrder();
}

//  ----------------------------------------------------
//  class MultiPageControl
//  ----------------------------------------------------
UnoMultiPageControl::UnoMultiPageControl() : maTabListeners( *this )
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}

UnoMultiPageControl::~UnoMultiPageControl()
{
}
// XTabListener

void SAL_CALL UnoMultiPageControl::inserted( ::sal_Int32 /*ID*/ ) throw (RuntimeException)
{
}
void SAL_CALL UnoMultiPageControl::removed( ::sal_Int32 /*ID*/ ) throw (RuntimeException)
{
}
void SAL_CALL UnoMultiPageControl::changed( ::sal_Int32 /*ID*/, const Sequence< NamedValue >& /*Properties*/ ) throw (RuntimeException)
{
}
void SAL_CALL UnoMultiPageControl::activated( ::sal_Int32 ID ) throw (RuntimeException)
{
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ), uno::makeAny( ID ), sal_False );

}
void SAL_CALL UnoMultiPageControl::deactivated( ::sal_Int32 /*ID*/ ) throw (RuntimeException)
{
}
void SAL_CALL UnoMultiPageControl::disposing(const EventObject&) throw (RuntimeException)
{
}

void SAL_CALL UnoMultiPageControl::dispose() throw (RuntimeException)
{
    lang::EventObject aEvt;
    aEvt.Source = (::cppu::OWeakObject*)this;
    maTabListeners.disposeAndClear( aEvt );
    UnoParentControl::dispose();
}

// com::sun::star::awt::XSimpleTabController
::sal_Int32 SAL_CALL UnoMultiPageControl::insertTab() throw (RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    return xMultiPage->insertTab();
}

void SAL_CALL UnoMultiPageControl::removeTab( ::sal_Int32 ID ) throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    xMultiPage->removeTab( ID );
}

void SAL_CALL UnoMultiPageControl::setTabProps( ::sal_Int32 ID, const Sequence< NamedValue >& Properties ) throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    xMultiPage->setTabProps( ID, Properties );
}

Sequence< NamedValue > SAL_CALL UnoMultiPageControl::getTabProps( ::sal_Int32 ID ) throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    return xMultiPage->getTabProps( ID );
}

void SAL_CALL UnoMultiPageControl::activateTab( ::sal_Int32 ID ) throw (IndexOutOfBoundsException, RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    xMultiPage->activateTab( ID );
    ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ), uno::makeAny( ID ), sal_True );

}

::sal_Int32 SAL_CALL UnoMultiPageControl::getActiveTabID() throw (RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( !xMultiPage.is() )
        throw RuntimeException();
    return xMultiPage->getActiveTabID();
}

void SAL_CALL UnoMultiPageControl::addTabListener( const Reference< XTabListener >& Listener ) throw (RuntimeException)
{
    maTabListeners.addInterface( Listener );
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( xMultiPage.is()  && maTabListeners.getLength() == 1 )
        xMultiPage->addTabListener( &maTabListeners );
}

void SAL_CALL UnoMultiPageControl::removeTabListener( const Reference< XTabListener >& Listener ) throw (RuntimeException)
{
    Reference< XSimpleTabController > xMultiPage( getPeer(), UNO_QUERY );
    if ( xMultiPage.is()  && maTabListeners.getLength() == 1 )
        xMultiPage->removeTabListener( &maTabListeners );
    maTabListeners.removeInterface( Listener );
}


// lang::XTypeProvider
IMPL_XTYPEPROVIDER_START( UnoMultiPageControl )
    getCppuType( ( uno::Reference< awt::XSimpleTabController>* ) NULL ),
    getCppuType( ( uno::Reference< awt::XTabListener>* ) NULL ),
    UnoParentControl::getTypes()
IMPL_XTYPEPROVIDER_END

// uno::XInterface
uno::Any UnoMultiPageControl::queryAggregation( const uno::Type & rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = ::cppu::queryInterface( rType,
                                        SAL_STATIC_CAST( awt::XTabListener*, this ), SAL_STATIC_CAST( awt::XSimpleTabController*, this ) );
    return (aRet.hasValue() ? aRet : UnoParentControl::queryAggregation( rType ));
}

::rtl::OUString UnoMultiPageControl::GetComponentServiceName()
{
    sal_Bool bDecoration( sal_True );
    ImplGetPropertyValue( GetPropertyName( BASEPROPERTY_DECORATION )) >>= bDecoration;
    if ( bDecoration )
        return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tabcontrol"));
    // Hopefully we can tweak the tabcontrol to display without tabs
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tabcontrolnotabs"));
}

void UnoMultiPageControl::bindPage( const uno::Reference< awt::XControl >& _rxControl )
{
    uno::Reference< awt::XWindowPeer > xPage( _rxControl->getPeer() );
    uno::Reference< awt::XSimpleTabController > xTabCntrl( getPeer(), uno::UNO_QUERY );
    uno::Reference< beans::XPropertySet > xProps( _rxControl->getModel(), uno::UNO_QUERY );

   VCLXTabPage* pXPage = dynamic_cast< VCLXTabPage* >( xPage.get() );
   TabPage* pPage = pXPage ? pXPage->getTabPage() : NULL;
    if ( xTabCntrl.is() && pPage )
    {
        VCLXMultiPage* pXTab = dynamic_cast< VCLXMultiPage* >( xTabCntrl.get() );
        if ( pXTab )
        {
            rtl::OUString sTitle;
            xProps->getPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ) ) >>= sTitle;
            pXTab->insertTab( pPage, sTitle);
        }
    }

}

void UnoMultiPageControl::createPeer( const Reference< XToolkit > & rxToolkit, const Reference< XWindowPeer >  & rParentPeer ) throw(RuntimeException)
{
    SolarMutexGuard aSolarGuard;

    UnoControlContainer::createPeer( rxToolkit, rParentPeer );

    uno::Sequence< uno::Reference< awt::XControl > > aCtrls = getControls();
    sal_uInt32 nCtrls = aCtrls.getLength();
    for( sal_uInt32 n = 0; n < nCtrls; n++ )
       bindPage( aCtrls[ n ] );
    sal_Int32 nActiveTab(0);
    Reference< XPropertySet > xMultiProps( getModel(), UNO_QUERY );
    xMultiProps->getPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ) ) >>= nActiveTab;

    uno::Reference< awt::XSimpleTabController > xTabCntrl( getPeer(), uno::UNO_QUERY );
    if ( xTabCntrl.is() )
    {
        xTabCntrl->addTabListener( this );
        if ( nActiveTab && nCtrls ) // Ensure peer is initialise with correct activated tab
        {
            xTabCntrl->activateTab( nActiveTab );
            ImplSetPropertyValue( GetPropertyName( BASEPROPERTY_MULTIPAGEVALUE ), uno::makeAny( nActiveTab ), sal_True );
        }
    }
}

void    UnoMultiPageControl::impl_createControlPeerIfNecessary( const uno::Reference< awt::XControl >& _rxControl)
{
    OSL_PRECOND( _rxControl.is(), "UnoMultiPageControl::impl_createControlPeerIfNecessary: invalid control, this will crash!" );

    // if the container already has a peer, then also create a peer for the control
    uno::Reference< awt::XWindowPeer > xMyPeer( getPeer() );

    if( xMyPeer.is() )
    {
        _rxControl->createPeer( NULL, xMyPeer );
        bindPage( _rxControl );
        ImplActivateTabControllers();
    }

}

// ------------- UnoMultiPageModel -----------------

UnoMultiPageModel::UnoMultiPageModel() : UnoControlDialogModel( false )
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );

    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_SIZEABLE );
    //ImplRegisterProperty( BASEPROPERTY_DIALOGSOURCEURL );
    ImplRegisterProperty( BASEPROPERTY_MULTIPAGEVALUE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES );

    Any aBool;
    aBool <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_DECORATION, aBool );
    // MultiPage Control has the tab stop property. And the default value is True.
    ImplRegisterProperty( BASEPROPERTY_TABSTOP, aBool );
}

UnoMultiPageModel::UnoMultiPageModel( const UnoMultiPageModel& rModel )
    : UnoControlDialogModel( rModel )
{
}

UnoMultiPageModel::~UnoMultiPageModel()
{
}

UnoControlModel*
UnoMultiPageModel::Clone() const
{
    // clone the container itself
    UnoMultiPageModel* pClone = new UnoMultiPageModel( *this );

    // clone all children
    ::std::for_each(
        maModels.begin(), maModels.end(),
        CloneControlModel( pClone->maModels )
    );

    return pClone;
}

::rtl::OUString UnoMultiPageModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoMultiPageModel );
}

uno::Any UnoMultiPageModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoMultiPageControl );
        return aAny;
    }
    return UnoControlDialogModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoMultiPageModel::getInfoHelper()
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
uno::Reference< beans::XPropertySetInfo > UnoMultiPageModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

void UnoMultiPageModel::insertByName( const ::rtl::OUString& aName, const Any& aElement ) throw(IllegalArgumentException, ElementExistException, WrappedTargetException, RuntimeException)
{
    Reference< XServiceInfo > xInfo;
    aElement >>= xInfo;

    if ( !xInfo.is() )
        throw IllegalArgumentException();

    // Only a Page model can be inserted into the multipage
    if ( !xInfo->supportsService( rtl::OUString::createFromAscii( szServiceName_UnoPageModel ) ) )
        throw IllegalArgumentException();

    return UnoControlDialogModel::insertByName( aName, aElement );
}

// ----------------------------------------------------------------------------
sal_Bool SAL_CALL UnoMultiPageModel::getGroupControl(  ) throw (RuntimeException)
{
    return sal_True;
}

//  ----------------------------------------------------
//  class UnoPageControl
//  ----------------------------------------------------
UnoPageControl::UnoPageControl()
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}

UnoPageControl::~UnoPageControl()
{
}

::rtl::OUString UnoPageControl::GetComponentServiceName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("tabpage"));
}


// ------------- UnoPageModel -----------------

UnoPageModel::UnoPageModel() : UnoControlDialogModel( false )
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );

    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_TITLE );
    ImplRegisterProperty( BASEPROPERTY_SIZEABLE );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES );
//    ImplRegisterProperty( BASEPROPERTY_DIALOGSOURCEURL );

    Any aBool;
    aBool <<= (sal_Bool) sal_True;
    ImplRegisterProperty( BASEPROPERTY_MOVEABLE, aBool );
    ImplRegisterProperty( BASEPROPERTY_CLOSEABLE, aBool );
    //ImplRegisterProperty( BASEPROPERTY_TABSTOP, aBool );
}

UnoPageModel::UnoPageModel( const UnoPageModel& rModel )
    : UnoControlDialogModel( rModel )
{
}

UnoPageModel::~UnoPageModel()
{
}

UnoControlModel*
UnoPageModel::Clone() const
{
    // clone the container itself
    UnoPageModel* pClone = new UnoPageModel( *this );

    // clone all children
    ::std::for_each(
        maModels.begin(), maModels.end(),
        CloneControlModel( pClone->maModels )
    );

    return pClone;
}

::rtl::OUString UnoPageModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoPageModel );
}

uno::Any UnoPageModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoPageControl );
        return aAny;
    }
    return UnoControlDialogModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoPageModel::getInfoHelper()
{
    static UnoPropertyArrayHelper* pHelper = NULL;
    if ( !pHelper )
    {
        uno::Sequence<sal_Int32>  aIDs = ImplGetPropertyIds();
        pHelper = new UnoPropertyArrayHelper( aIDs );
    }
    return *pHelper;
}

// beans::XMultiPropertySet
uno::Reference< beans::XPropertySetInfo > UnoPageModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

// ----------------------------------------------------------------------------
sal_Bool SAL_CALL UnoPageModel::getGroupControl(  ) throw (RuntimeException)
{
    return sal_False;
}

// Frame control

//  ----------------------------------------------------
//  class UnoFrameControl
//  ----------------------------------------------------
UnoFrameControl::UnoFrameControl()
{
    maComponentInfos.nWidth = 280;
    maComponentInfos.nHeight = 400;
}

UnoFrameControl::~UnoFrameControl()
{
}

::rtl::OUString UnoFrameControl::GetComponentServiceName()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("frame"));
}

void UnoFrameControl::ImplSetPosSize( Reference< XControl >& rxCtrl )
{
    bool bOwnCtrl = false;
    rtl::OUString sTitle;
    if ( rxCtrl.get() == Reference<XControl>( this ).get() )
        bOwnCtrl = true;
    Reference< XPropertySet > xProps( getModel(), UNO_QUERY );
    //xProps->getPropertyValue( GetPropertyName( BASEPROPERTY_TITLE ) ) >>= sTitle;
    xProps->getPropertyValue( GetPropertyName( BASEPROPERTY_LABEL ) ) >>= sTitle;

    UnoParentControl::ImplSetPosSize( rxCtrl );
    Reference < XWindow > xW( rxCtrl, UNO_QUERY );
    if ( !bOwnCtrl && xW.is() && sTitle.getLength() )
    {
        awt::Rectangle aSizePos = xW->getPosSize();

        sal_Int32 nX = aSizePos.X, nY = aSizePos.Y, nWidth = aSizePos.Width, nHeight = aSizePos.Height;
        // Retrieve the values set by the base class
        OutputDevice*pOutDev = Application::GetDefaultDevice();
        if ( pOutDev )
        {
            if ( !bOwnCtrl && sTitle.getLength() )
            {
                // Adjust Y based on height of Title
                ::Rectangle aRect = pOutDev->GetTextRect( aRect, sTitle );
                nY = nY + ( aRect.GetHeight() / 2 );
            }
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
            if ( !bOwnCtrl && sTitle.getLength() )
                // offset y based on height of font ( not sure if my guess at the correct calculation is correct here )
                nY = nY + ( nH / 8); // how do I test this
        }
        xW->setPosSize( nX, nY, nWidth, nHeight, PosSize::POSSIZE );
    }
}

// ------------- UnoFrameModel -----------------

UnoFrameModel::UnoFrameModel() : UnoControlDialogModel( false )
{
    ImplRegisterProperty( BASEPROPERTY_DEFAULTCONTROL );
    ImplRegisterProperty( BASEPROPERTY_BACKGROUNDCOLOR );
    ImplRegisterProperty( BASEPROPERTY_ENABLED );
    ImplRegisterProperty( BASEPROPERTY_ENABLEVISIBLE );
    ImplRegisterProperty( BASEPROPERTY_FONTDESCRIPTOR );
    ImplRegisterProperty( BASEPROPERTY_HELPTEXT );
    ImplRegisterProperty( BASEPROPERTY_HELPURL );
    ImplRegisterProperty( BASEPROPERTY_PRINTABLE );
    ImplRegisterProperty( BASEPROPERTY_LABEL );
    ImplRegisterProperty( BASEPROPERTY_WRITING_MODE );
    ImplRegisterProperty( BASEPROPERTY_CONTEXT_WRITING_MODE );
    ImplRegisterProperty( BASEPROPERTY_USERFORMCONTAINEES );
}

UnoFrameModel::UnoFrameModel( const UnoFrameModel& rModel )
    : UnoControlDialogModel( rModel )
{
}

UnoFrameModel::~UnoFrameModel()
{
}

UnoControlModel*
UnoFrameModel::Clone() const
{
    // clone the container itself
    UnoFrameModel* pClone = new UnoFrameModel( *this );

    // clone all children
    ::std::for_each(
        maModels.begin(), maModels.end(),
        CloneControlModel( pClone->maModels )
    );

    return pClone;
}

::rtl::OUString UnoFrameModel::getServiceName() throw(::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii( szServiceName_UnoFrameModel );
}

uno::Any UnoFrameModel::ImplGetDefaultValue( sal_uInt16 nPropId ) const
{
    if ( nPropId == BASEPROPERTY_DEFAULTCONTROL )
    {
        uno::Any aAny;
        aAny <<= ::rtl::OUString::createFromAscii( szServiceName_UnoFrameControl );
        return aAny;
    }
    return UnoControlDialogModel::ImplGetDefaultValue( nPropId );
}

::cppu::IPropertyArrayHelper& UnoFrameModel::getInfoHelper()
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
uno::Reference< beans::XPropertySetInfo > UnoFrameModel::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    static uno::Reference< beans::XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
