/*************************************************************************
 *
 *  $RCSfile: AccessibleControlShape.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: af $ $Date: 2002-06-27 12:02:00 $
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

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_CONTROL_SHAPE_HXX
#include "AccessibleControlShape.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#include "AccessibleShapeInfo.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_DESCRIPTION_GENERATOR_HXX
#include "DescriptionGenerator.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_REFLECTION_XPROXYFACTORY_HPP_
#include <com/sun/star/reflection/XProxyFactory.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _SVDOUNO_HXX
#include "svdouno.hxx"
#endif
#ifndef _SVX_UNOAPI_HXX_
#include "unoapi.hxx"
#endif
#include "ShapeTypeHandler.hxx"
#include "SvxShapeTypes.hxx"
#include "svdstr.hrc"

using namespace accessibility;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::reflection::XProxyFactory;
using ::com::sun::star::uno::XAggregation;
using ::com::sun::star::lang::XComponent;

//=====  internal  ============================================================

AccessibleControlShape::AccessibleControlShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    :      AccessibleShape (rShapeInfo, rShapeTreeInfo)
    ,   mbListeningForName( sal_False )
    ,   mbListeningForDesc( sal_False )
    ,   mbDisposeNativeContext( sal_False )
{
}




AccessibleControlShape::~AccessibleControlShape (void)
{
    if ( m_xControlContextProxy.is() )
        m_xControlContextProxy->setDelegator( NULL );
    m_xControlContextProxy.clear();
        // this should remove the _one_and_only_ "real" reference (means not delegated to
        // ourself) to this proxy, and thus delete it
}

//=============================================================================
SdrObject* AccessibleControlShape::getSdrObject() const
{
    return GetSdrObjectFromXShape (mxShape);
}

//-----------------------------------------------------------------------------
void AccessibleControlShape::Init()
{
    AccessibleShape::Init();

    OSL_ENSURE( !m_xControlContextProxy.is(), "AccessibleControlShape::Init: already initialized!" );
    try
    {
        // What we need to do here is merge the functionality of the AccessibleContext of our UNO control
        // with our own AccessibleContext-related functionality.
        //
        // The problem is that we do not know the interfaces our "inner" context supports - this may be any
        // XAccessibleXXX interface (or even any other) which makes sense for it.
        //
        // In theory, we could implement all possible interfaces ourself, and re-route all functionality to
        // the inner context (except those we implement ourself, like XAccessibleComponent). But this is in no
        // way future-proof - as soon as an inner context appears which implements an additional interface,
        // we would need to adjust our implementation to support this new interface, too. Bad idea.
        //
        // The usual solution for such a problem is aggregation. Aggregation means using UNO's own meachnisms
        // for merging an inner with an outer component, an get a component which behaves as it is exactly one.
        // This is what XAggregation is for. Unfortunately, aggregation requires _exact_ control over the ref count
        // of the inner object, which we do not have at all.
        // Bad, too.
        //
        // But there is a solution: com.sun.star.reflection.ProxyFactory. This service is able to create a proxy
        // for any component, which supports _exactly_ the same interfaces as the component. In addition, it can
        // be aggregated, as by definition the proxy's ref count is exactly one when returned from the factory.
        // Sounds better. Though this yields the problem of slightly degraded performance, it's the only solution
        // I'm aware of at the moment .....
        //
        // 98750 - 30.04.2002 - fs@openoffice.org
        //

        // get the control which belongs to our model (relative to our view)
        const Window* pViewWindow = maShapeTreeInfo.GetWindow();
        SdrUnoObj* pUnoObjectImpl = PTR_CAST( SdrUnoObj, getSdrObject() );
        OSL_ENSURE( pViewWindow && pUnoObjectImpl, "AccessibleControlShape::Init: no view, or no SdrUnoObj!" );

        if ( pViewWindow && pUnoObjectImpl )
        {
            // get the context of the control - it will be our "inner" context
            Reference< XAccessible > xControlAccessible( pUnoObjectImpl->GetUnoControl( pViewWindow ), uno::UNO_QUERY );
            Reference< XAccessibleContext > xNativeControlContext;
            if ( xControlAccessible.is() )
                xNativeControlContext = xControlAccessible->getAccessibleContext();
            DBG_ASSERT( xNativeControlContext.is(), "AccessibleControlShape::Init: no AccessibleContext for the control!" );

            // get a proxy for this context
            // first a factory for the proxy
            Reference< XProxyFactory > xFactory;
            xFactory = xFactory.query( ::comphelper::createProcessComponent(
                ::rtl::OUString::createFromAscii( "com.sun.star.reflection.ProxyFactory" ) ) );
            OSL_ENSURE( xFactory.is(), "AccessibleControlShape::Init: could not create a proxy factory!" );
            // then the proxy itself
            if ( xFactory.is() && xNativeControlContext.is() )
            {
                m_xControlContextProxy = xFactory->createProxy( xNativeControlContext );

                // aggregate the proxy
                osl_incrementInterlockedCount( &m_refCount );
                if ( m_xControlContextProxy.is() )
                {
                    // At this point in time, the proxy has a ref count of exactly one - in m_xControlContextProxy.
                    // Remember to _not_ reset this member unles the delegator of the proxy has been reset, too!
                    m_xControlContextProxy->setDelegator( *this );
                }
                osl_decrementInterlockedCount( &m_refCount );

                mbDisposeNativeContext = sal_True;

                // Finally, we need to add ourself as dispose listener to the native context. In case it is disposed,
                // we need to dispose ourself, too. With a "real" aggregation (i.e. not using a proxy), this would be
                // done automatically, as every access to the XComponent interface of the aggregated object would
                // be re-routed to ourself. But with aggregating a proxy only we have to take care for this ourself.
                mxNativeContextComponent = mxNativeContextComponent.query( xNativeControlContext );
                if ( mxNativeContextComponent.is() )
                    mxNativeContextComponent->addEventListener( static_cast< AccessibleControlShape_Base* >( this ) );
            }
        }
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( sal_False, "AccessibleControlShape::Init: could not \"aggregate\" the controls XAccessibleContext!" );
    }
}

//=====  XAccessible  =========================================================

Reference<XAccessibleContext> SAL_CALL
    AccessibleControlShape::getAccessibleContext (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleShape::getAccessibleContext ();
}


//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleControlShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleControlShape"));
}




/// Set this object's name if is different to the current name.
::rtl::OUString
    AccessibleControlShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_CONTROL:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ControlShape"));
            break;
        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleControlShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}




//--------------------------------------------------------------------
namespace
{
    //................................................................
    const ::rtl::OUString& lcl_getNamePropertyName( )
    {
        static ::rtl::OUString s_sNamePropertyName( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );
        return s_sNamePropertyName;
    }
    //................................................................
    const ::rtl::OUString& lcl_getDescPropertyName( )
    {
        static ::rtl::OUString s_sDescPropertyDesc( RTL_CONSTASCII_USTRINGPARAM( "HelpText" ) );
        return s_sDescPropertyDesc;
    }
}

//--------------------------------------------------------------------
::rtl::OUString
    AccessibleControlShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_CONTROL:
        {
            // check if we can obtain the "Desc" property from the model
            ::rtl::OUString sDesc( getControlModelStringProperty( lcl_getDescPropertyName() ) );
            if ( !sDesc.getLength() )
            {   // no -> use the default
                aDG.Initialize (STR_ObjNameSingulUno);
                aDG.AddProperty (OUString::createFromAscii ("ControlBackground"),
                    DescriptionGenerator::COLOR,
                    OUString());
                aDG.AddProperty (OUString::createFromAscii ("ControlBorder"),
                    DescriptionGenerator::INTEGER,
                    OUString());
            }
            // ensure that we are listening to the Name property
            mbListeningForDesc = ensureListeningState( mbListeningForDesc, sal_True, lcl_getDescPropertyName() );
        }
        break;

        default:
            aDG.Initialize (::rtl::OUString::createFromAscii (
                "Unknown accessible control shape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}

//--------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2( AccessibleControlShape, AccessibleShape, AccessibleControlShape_Base )
IMPLEMENT_FORWARD_XTYPEPROVIDER2( AccessibleControlShape, AccessibleShape, AccessibleControlShape_Base )
    // order matters in both cases: the second argument is the base class doing the ref counting

//--------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::propertyChange( const beans::PropertyChangeEvent& _rEvent ) throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( maMutex );

    // check if it is the name or the description
    if ( _rEvent.PropertyName.equals( lcl_getNamePropertyName() ) )
    {
        SetAccessibleName (CreateAccessibleName());
    }
    else if ( _rEvent.PropertyName.equals( lcl_getDescPropertyName() ) )
    {
        SetAccessibleDescription (CreateAccessibleDescription());
    }
#ifdef _DEBUG
    else
    {
        OSL_ENSURE( sal_False, "AccessibleControlShape::propertyChange: where did this come from?" );
    }
#endif
}

//--------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::disposing (const lang::EventObject& _rSource) throw (uno::RuntimeException)
{
    // did it come from our inner context (the real one, not it's proxy!)?
    OSL_TRACE ("AccessibleControlShape::disposing");
    if ( _rSource.Source == mxNativeContextComponent )
    {
        // If our "pseudo-aggregated" inner context does not live anymore,
        // we don't want to live, too.  This is accomplished by asking our
        // parent to replace this object with a new one.  Disposing this
        // object and sending notifications about the replacement are the
        // task of our parent.
        AccessibleShape* pShape =
            ShapeTypeHandler::Instance().CreateAccessibleObject (
                AccessibleShapeInfo (
                    mxShape, getAccessibleParent(), mpParent, mnIndex),
                maShapeTreeInfo);
        Reference<XAccessible> xShape (pShape);
        // Now that there is a reference to the new accessible shape we
        // can safely call its Init() method.  The following call transfers
        // owenership to our parent so that at the end of the scope we can
        // safely release the reference.
        mpParent->ReplaceChild (this, pShape);
    }
    else
        AccessibleShape::disposing( _rSource );
}

//--------------------------------------------------------------------
sal_Bool AccessibleControlShape::ensureListeningState(
        const sal_Bool _bCurrentlyListening, const sal_Bool _bNeedNewListening,
        const ::rtl::OUString& _rPropertyName )
{
    if ( ( _bCurrentlyListening == _bNeedNewListening ) || !ensureControlModelAccess() )
        //  nothing to do
        return _bCurrentlyListening;

    try
    {
        if ( !mxModelPropsMeta.is() || mxModelPropsMeta->hasPropertyByName( _rPropertyName ) )
        {
            // add or revoke as listener
            if ( _bNeedNewListening )
                mxControlModel->addPropertyChangeListener( _rPropertyName, static_cast< beans::XPropertyChangeListener* >( this ) );
            else
                mxControlModel->removePropertyChangeListener( _rPropertyName, static_cast< beans::XPropertyChangeListener* >( this ) );
        }
    }
    catch( const uno::Exception& e )
    {
        e;  // make compiler happy
        OSL_ENSURE( sal_False, "AccessibleControlShape::ensureListeningState: could not change the listening state!" );
    }

    return _bNeedNewListening;
}

//--------------------------------------------------------------------
::rtl::OUString AccessibleControlShape::CreateAccessibleName (void) throw (uno::RuntimeException)
{
    // check if we can obtain the "Name" property from the model
    ::rtl::OUString sName( getControlModelStringProperty( lcl_getNamePropertyName() ) );
    if ( !sName.getLength() )
    {   // no -> use the default
        sName = AccessibleShape::CreateAccessibleName();
    }

    // now that somebody first asked us for our name, ensure that we are listening to name changes on the model
    mbListeningForName = ensureListeningState( mbListeningForName, sal_True, lcl_getNamePropertyName() );

    return sName;
}

//--------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::disposing (void)
{
    // ensure we're not listening
    mbListeningForName = ensureListeningState( mbListeningForName, sal_False, lcl_getNamePropertyName() );
    mbListeningForDesc = ensureListeningState( mbListeningForDesc, sal_False, lcl_getDescPropertyName() );

    // release the model
    mxControlModel.clear();
    mxModelPropsMeta.clear();

    // forward the disposel to our inner context
    if ( mbDisposeNativeContext )
    {
        // don't listen for disposals anymore
        if ( mxNativeContextComponent.is() )
            mxNativeContextComponent->removeEventListener( static_cast< AccessibleControlShape_Base* >( this ) );

        Reference< XComponent > xInnerComponent;
        if ( ::comphelper::query_aggregation( m_xControlContextProxy, xInnerComponent ) )
            xInnerComponent->dispose();
        // do _not_ clear m_xControlContextProxy! This has to be done in the dtor for correct ref-count handling

        // no need to dispose the proxy/inner context anymore
        mbDisposeNativeContext = sal_False;
    }
    mxNativeContextComponent.clear();

    // let the base do it's stuff
    AccessibleShape::disposing();
}

//--------------------------------------------------------------------
sal_Bool AccessibleControlShape::ensureControlModelAccess() SAL_THROW(())
{
    if ( mxControlModel.is() )
        return sal_True;

    try
    {
        Reference< drawing::XControlShape > xShape( mxShape, uno::UNO_QUERY );
        if ( xShape.is() )
            mxControlModel = mxControlModel.query( xShape->getControl() );

        if ( mxControlModel.is() )
            mxModelPropsMeta = mxControlModel->getPropertySetInfo();
    }
    catch( const uno::Exception& e )
    {
        e;  // make compiler happy
        OSL_ENSURE( sal_False, "AccessibleControlShape::ensureControlModelAccess: caught an exception!" );
    }

    return mxControlModel.is();
}

//--------------------------------------------------------------------
::rtl::OUString AccessibleControlShape::getControlModelStringProperty( const ::rtl::OUString& _rPropertyName ) const SAL_THROW(())
{
    ::rtl::OUString sReturn;
    try
    {
        if ( const_cast< AccessibleControlShape* >( this ) ->ensureControlModelAccess() )
        {
            if ( !mxModelPropsMeta.is() ||  mxModelPropsMeta->hasPropertyByName( _rPropertyName ) )
                // ask only if a) the control does not have a PropertySetInfo object or b) it has, and the
                // property in question is available
                mxControlModel->getPropertyValue( _rPropertyName ) >>= sReturn;
        }
    }
    catch( const uno::Exception& )
    {
        OSL_ENSURE( sal_False, "OAccessibleControlContext::getModelStringProperty: caught an exception!" );
    }
    return sReturn;
}

