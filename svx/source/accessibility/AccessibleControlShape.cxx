/*************************************************************************
 *
 *  $RCSfile: AccessibleControlShape.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: fs $ $Date: 2002-04-29 16:51:40 $
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
#ifndef _SVX_ACCESSIBILITY_DESCRIPTION_GENERATOR_HXX
#include "DescriptionGenerator.hxx"
#endif
#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>

#include "ShapeTypeHandler.hxx"
#include "SvxShapeTypes.hxx"

using namespace accessibility;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

//=====  internal  ============================================================

AccessibleControlShape::AccessibleControlShape (const ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XShape>& rxShape,
    const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    long nIndex)
    :      AccessibleShape (rxShape, rxParent, rShapeTreeInfo, nIndex)
    ,   mbListeningForName( sal_False )
    ,   mbListeningForDesc( sal_False )
{
}




AccessibleControlShape::~AccessibleControlShape (void)
{
}




//=====  XAccessible  =========================================================

Reference<XAccessibleContext> SAL_CALL
    AccessibleControlShape::getAccessibleContext (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    // Here will later be the control shape asked for its control and that
    // for its accessibility object.
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
                aDG.Initialize (OUString::createFromAscii("Control"));
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

    ::rtl::OUString sNewValue;
    sal_Int16 nEventId = -1;
    // check if it is the name or the description
    if ( _rEvent.PropertyName.equals( lcl_getNamePropertyName() ) )
    {
        nEventId = AccessibleEventId::ACCESSIBLE_NAME_EVENT;
        sNewValue = CreateAccessibleName();
    }
    else if ( _rEvent.PropertyName.equals( lcl_getDescPropertyName() ) )
    {
        nEventId = AccessibleEventId::ACCESSIBLE_DESCRIPTION_EVENT;
        sNewValue = CreateAccessibleDescription();
    }
#ifdef _DEBUG
    else
    {
        OSL_ENSURE( sal_False, "AccessibleControlShape::propertyChange: where did this come from?" );
    }
#endif
    if ( -1 != nEventId )
    {   // notify the AccessibilityListeners
        CommitChange( nEventId, uno::makeAny( sNewValue ), uno::Any() );
            // TODO: check this in reality: If the name is changed from non-empty to empty, this means
            // that we remove ourself as listener _while_we_are_within_the_listener_call_. Don't know
            // if the common implementation in cppuhelper can correctly handle this!
    }
}

//--------------------------------------------------------------------
void SAL_CALL AccessibleControlShape::disposing (const lang::EventObject& _rSource) throw (uno::RuntimeException)
{
    // simply disambiguate
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

    // let the base do it's stuff
    AccessibleShape::dispose();
}

//--------------------------------------------------------------------
sal_Bool AccessibleControlShape::ensureControlModelAccess() const SAL_THROW(())
{
    if ( mxControlModel.is() )
        return sal_True;

    try
    {
        Reference< drawing::XControlShape > xShape( mxShape, uno::UNO_QUERY );
        if ( xShape.is() )
            const_cast< AccessibleControlShape* >( this )->mxControlModel = mxControlModel.query( xShape->getControl() );

        if ( mxControlModel.is() )
            const_cast< AccessibleControlShape* >( this )->mxModelPropsMeta = mxControlModel->getPropertySetInfo();
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
        if ( ensureControlModelAccess() )
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

