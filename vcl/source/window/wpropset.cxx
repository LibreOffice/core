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

#include "precompiled_vcl.hxx"

#include "vcl/wpropset.hxx"
#include "vcl/window.hxx"
#include "vcl/vclevent.hxx"
#include "vcl/svdata.hxx"

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/beans/PropertyValue.hpp"
#include "com/sun/star/beans/PropertyAttribute.hpp"
#include "com/sun/star/beans/XPropertySet.hpp"
#include "com/sun/star/beans/XPropertyContainer.hpp"
#include "com/sun/star/beans/XPropertyAccess.hpp"

#include "cppuhelper/basemutex.hxx"
#include "cppuhelper/compbase1.hxx"

#include <map>

using namespace vcl;
using namespace com::sun::star;

/*

TODO:
- release solarmutex during outside UNO calls
- in ChildEventListener protect against reentry by using PostUserEvent

*/

class vcl::WindowPropertySetListener :
    public cppu::BaseMutex,
    public cppu::WeakComponentImplHelper1< com::sun::star::beans::XPropertyChangeListener >,
    private boost::noncopyable
{
    WindowPropertySet*      mpParent;
    bool                    mbSuspended;
public:
    WindowPropertySetListener( WindowPropertySet* pParent )
    : cppu::WeakComponentImplHelper1< com::sun::star::beans::XPropertyChangeListener >( m_aMutex )
    , mpParent( pParent )
    , mbSuspended( false )
    {}

    virtual ~WindowPropertySetListener()
    {
    }

    using cppu::WeakComponentImplHelperBase::disposing;
    virtual void SAL_CALL disposing( const lang::EventObject& ) throw()
    {
    }

    virtual void SAL_CALL propertyChange( const beans::PropertyChangeEvent& i_rEvent ) throw()
    {
        if( ! mbSuspended )
            mpParent->propertyChange( i_rEvent );
    }

    void suspend( bool i_bSuspended )
    {
        mbSuspended = i_bSuspended;
    }
};

class vcl::WindowPropertySetData
{
public:

    struct PropertyMapEntry
    {
        Window*                                 mpWindow;
        boost::shared_ptr<WindowArranger>       mpLayout;
        uno::Sequence< beans::PropertyValue >   maSavedValues;

        PropertyMapEntry( Window* i_pWindow = NULL,
                         const boost::shared_ptr<WindowArranger>& i_pLayout = boost::shared_ptr<WindowArranger>() )
        : mpWindow( i_pWindow )
        , mpLayout( i_pLayout )
        {}

        uno::Sequence< beans::PropertyValue > getProperties() const
        {
            if( mpWindow )
                return mpWindow->getProperties();
            else if( mpLayout.get() )
                return mpLayout->getProperties();
            return uno::Sequence< beans::PropertyValue >();
        }

        void setProperties( const uno::Sequence< beans::PropertyValue >& i_rProps ) const
        {
            if( mpWindow )
                mpWindow->setProperties( i_rProps );
            else if( mpLayout.get() )
                mpLayout->setProperties( i_rProps );
        }
    };

    Window*                                                         mpTopWindow;
    bool                                                            mbOwner;
    std::map< rtl::OUString, PropertyMapEntry >                     maProperties;
    uno::Reference< beans::XPropertySet >                           mxPropSet;
    uno::Reference< beans::XPropertyAccess >                        mxPropSetAccess;
    uno::Reference< beans::XPropertyChangeListener >                mxListener;
    vcl::WindowPropertySetListener*                                 mpListener;

    WindowPropertySetData()
    : mpTopWindow( NULL )
    , mbOwner( false )
    , mpListener( NULL )
    {}

    ~WindowPropertySetData()
    {
        // release layouters, possibly interface properties before destroying
        // the involved parent to be on the safe side
        maProperties.clear();
        if( mbOwner )
            delete mpTopWindow;
    }
};

static rtl::OUString getIdentifiedPropertyName( const rtl::OUString& i_rIdentifier, const rtl::OUString& i_rName )
{
    rtl::OUStringBuffer aBuf( i_rIdentifier.getLength() + 1 + i_rName.getLength() );
    aBuf.append( i_rIdentifier );
    aBuf.append( sal_Unicode( '#' ) );
    aBuf.append( i_rName );
    return aBuf.makeStringAndClear();
}

static void spliceIdentifiedPropertyName( const rtl::OUString& i_rIdentifiedPropName,
                                          rtl::OUString& o_rIdentifier,
                                          rtl::OUString& o_rPropName )
{
    sal_Int32 nIndex = 0;
    o_rIdentifier = i_rIdentifiedPropName.getToken( 0, sal_Unicode( '#' ), nIndex );
    if( nIndex != -1 )
        o_rPropName = i_rIdentifiedPropName.copy( nIndex );
    else
        o_rPropName = rtl::OUString();
}

WindowPropertySet::WindowPropertySet( Window* i_pTopWindow, bool i_bTakeOwnership )
: mpImpl( new vcl::WindowPropertySetData )
{
    mpImpl->mpTopWindow = i_pTopWindow;
    mpImpl->mbOwner = i_bTakeOwnership;

    mpImpl->mpTopWindow->AddChildEventListener( LINK( this, WindowPropertySet, ChildEventListener ) );

    mpImpl->mxPropSet = uno::Reference< beans::XPropertySet >(
        ImplGetSVData()->maAppData.mxMSF->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.beans.PropertyBag" ) ) ),
        uno::UNO_QUERY );
    OSL_ENSURE( mpImpl->mxPropSet.is(), "could not create instance of com.sun.star.beans.PropertyBag" );
    mpImpl->mxPropSetAccess = uno::Reference< beans::XPropertyAccess >( mpImpl->mxPropSet, uno::UNO_QUERY );
    OSL_ENSURE( mpImpl->mxPropSet.is(), "could not query XPropertyAccess interface" );
    if( ! mpImpl->mxPropSetAccess.is() )
        mpImpl->mxPropSet.clear();

    addWindowToSet( i_pTopWindow );

    setupProperties();

    if( mpImpl->mxPropSet.is() )
    {
        mpImpl->mxListener.set( mpImpl->mpListener = new WindowPropertySetListener( this ) );
    }
}

WindowPropertySet::~WindowPropertySet()
{
    mpImpl->mpTopWindow->RemoveChildEventListener( LINK( this, WindowPropertySet, ChildEventListener ) );

    delete mpImpl;
    mpImpl = NULL;
}

uno::Reference< beans::XPropertySet > WindowPropertySet::getPropertySet() const
{
    return mpImpl->mxPropSet;
}

void WindowPropertySet::addLayoutToSet( const boost::shared_ptr< WindowArranger >& i_pLayout )
{
    if( i_pLayout.get() )
    {
        if( i_pLayout->getIdentifier().getLength() )
        {
            WindowPropertySetData::PropertyMapEntry& rEntry = mpImpl->maProperties[ i_pLayout->getIdentifier() ];
            OSL_ENSURE( rEntry.mpWindow == 0 && rEntry.mpLayout.get() == 0, "inserted layout has duplicate name" );
            rEntry.mpWindow = NULL;
            rEntry.mpLayout = i_pLayout;
            rEntry.maSavedValues = i_pLayout->getProperties();
        }
        // insert child layouts
        size_t nChildren = i_pLayout->countElements();
        for( size_t i = 0; i < nChildren; i++ )
            addLayoutToSet( i_pLayout->getChild( i ) );
    }
}

void WindowPropertySet::addWindowToSet( Window* i_pWindow )
{
    if( i_pWindow->getIdentifier().getLength() ) // no name, no properties
    {
        WindowPropertySetData::PropertyMapEntry& rEntry = mpImpl->maProperties[ i_pWindow->getIdentifier() ];
        OSL_ENSURE( rEntry.mpWindow == 0 && rEntry.mpLayout.get() == 0, "inserted window has duplicate name" );
        rEntry.mpWindow = i_pWindow;
        rEntry.mpLayout.reset();
        rEntry.maSavedValues = i_pWindow->getProperties();
    }
    addLayoutToSet( i_pWindow->getLayout() );

    Window* pWin = i_pWindow->GetWindow( WINDOW_FIRSTCHILD );
    while( pWin )
    {
        addWindowToSet( pWin );
        pWin = pWin->GetWindow( WINDOW_NEXT );
    }
}

void WindowPropertySet::setupProperties()
{
    uno::Reference< beans::XPropertyContainer > xCont( mpImpl->mxPropSet, uno::UNO_QUERY );
    OSL_ENSURE( xCont.is(), "could not get XPropertyContainer interface" );
    if( ! xCont.is() )
        return;

    for( std::map< rtl::OUString, WindowPropertySetData::PropertyMapEntry >::iterator it
         = mpImpl->maProperties.begin(); it != mpImpl->maProperties.end(); ++it )
    {
        uno::Sequence< beans::PropertyValue > aOutsideValues( it->second.maSavedValues );
        beans::PropertyValue* pVal = aOutsideValues.getArray();
        for( sal_Int32 i = 0; i <  aOutsideValues.getLength(); i++ )
        {
            pVal[i].Name = getIdentifiedPropertyName( it->first, pVal[i].Name );
            xCont->addProperty( pVal[i].Name,
                                beans::PropertyAttribute::BOUND | beans:: PropertyAttribute::CONSTRAINED,
                                pVal[i].Value
                                );
        }
    }
}

void WindowPropertySet::propertyChange( const beans::PropertyChangeEvent& i_rEvent )
{
    rtl::OUString aIdentifier, aProperty;
    spliceIdentifiedPropertyName( i_rEvent.PropertyName, aIdentifier, aProperty );
    std::map< rtl::OUString, WindowPropertySetData::PropertyMapEntry >::iterator it =
        mpImpl->maProperties.find( aIdentifier );
    if( it != mpImpl->maProperties.end() )
    {
        uno::Sequence< beans::PropertyValue > aSet( 1 );
        aSet[0].Name  = aProperty;
        aSet[0].Value = i_rEvent.NewValue;
        it->second.setProperties( aSet );
    }
}

IMPL_LINK( vcl::WindowPropertySet, ChildEventListener, VclWindowEvent*, pEvent )
{
    // find window in our properties
    std::map< rtl::OUString, WindowPropertySetData::PropertyMapEntry >::iterator it
    = mpImpl->maProperties.find( pEvent->GetWindow()->getIdentifier() );
    if( it != mpImpl->maProperties.end() ) // this is valid, some unnamed child may have sent an event
    {
        sal_uLong nId = pEvent->GetId();
        // check if anything interesting happened
        if(
           // general windowy things
           nId == VCLEVENT_WINDOW_SHOW                  ||
           nId == VCLEVENT_WINDOW_HIDE                  ||
           nId == VCLEVENT_WINDOW_ENABLED               ||
           nId == VCLEVENT_WINDOW_DISABLED              ||
           // button thingies
           nId == VCLEVENT_BUTTON_CLICK                 ||
           nId == VCLEVENT_PUSHBUTTON_TOGGLE            ||
           nId == VCLEVENT_RADIOBUTTON_TOGGLE           ||
           nId == VCLEVENT_CHECKBOX_TOGGLE              ||
           // listbox
           nId == VCLEVENT_LISTBOX_SELECT               ||
           // edit
           nId == VCLEVENT_EDIT_MODIFY
           )
        {
            WindowPropertySetData::PropertyMapEntry& rEntry = it->second;
            // collect changes
            uno::Sequence< beans::PropertyValue > aNewProps( rEntry.getProperties() );
            uno::Sequence< beans::PropertyValue > aNewPropsOut( aNewProps );

            // translate to identified properties
            beans::PropertyValue* pValues = aNewPropsOut.getArray();
            for( sal_Int32 i = 0; i < aNewPropsOut.getLength(); i++ )
                pValues[i].Name = getIdentifiedPropertyName( it->first, pValues[i].Name );

            // broadcast changes
            bool bWasVeto = false;
            mpImpl->mpListener->suspend( true );
            try
            {
                mpImpl->mxPropSetAccess->setPropertyValues( aNewPropsOut );
            }
            catch( beans::PropertyVetoException& )
            {
                bWasVeto = true;
            }
            mpImpl->mpListener->suspend( false );

            if( ! bWasVeto ) // changes accepted ?
                rEntry.maSavedValues = rEntry.getProperties();
            else // no, reset
                rEntry.setProperties( rEntry.maSavedValues );
        }
    }

    return 0;
}
