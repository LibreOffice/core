/*************************************************************************
 *
 *  $RCSfile: AccessibleShape.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: af $ $Date: 2002-05-08 09:44:07 $
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

#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include "AccessibleShape.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_DESCRIPTION_GENERATOR_HXX
#include "DescriptionGenerator.hxx"
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_INFO_HXX
#include "AccessibleShapeInfo.hxx"
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLE_ROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLE_STATE_TYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPES_HPP_
#include <com/sun/star/drawing/XShapes.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_XSHAPEDESCRIPTOR_HPP_
#include <com/sun/star/drawing/XShapeDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_INDEXOUTOFBOUNDSEXCEPTION_HPP_
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_FILLSTYLE_HPP_
#include <com/sun/star/drawing/FillStyle.hpp>
#endif
#ifndef _SVX_UNOEDSRC_HXX
#include "unoedsrc.hxx"
#endif
#ifndef SVX_UNOSHTXT_HXX
#include "unoshtxt.hxx"
#endif
#include "svdobj.hxx"
#include "svdmodel.hxx"
#ifndef _SVX_UNOAPI_HXX_
#include "unoapi.hxx"
#endif
#include <com/sun/star/uno/Exception.hpp>
#include "ShapeTypeHandler.hxx"
#include "SvxShapeTypes.hxx"

#ifndef _SVX_ACCESSIBILITY_HRC
#include "accessibility.hrc"
#endif
#include "svdstr.hrc"

#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif
#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX_
#include <unotools/accessiblestatesethelper.hxx>
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

//=====  internal  ============================================================

AccessibleShape::AccessibleShape (
    const AccessibleShapeInfo& rShapeInfo,
    const AccessibleShapeTreeInfo& rShapeTreeInfo)
    : AccessibleContextBase (rShapeInfo.mxParent,AccessibleRole::SHAPE),
      mpChildrenManager(NULL),
      mxShape (rShapeInfo.mxShape),
      mShapeTreeInfo (rShapeTreeInfo),
      mnIndex (rShapeInfo.mnIndex),
      mpText (NULL),
      mpParent (rShapeInfo.mpChildrenManager)
{
}




AccessibleShape::~AccessibleShape (void)
{
    if (mpChildrenManager != NULL)
        delete mpChildrenManager;
    if (mpText != NULL)
        delete mpText;
    OSL_TRACE ("~AccessibleShape");

    // Unregistering from the various broadcasters should be unnecessary
    // since this destructor would not have been called if one of the
    // broadcasters would still hold a strong reference to this object.
}




void AccessibleShape::Init (void)
{
    // Set the opaque state for certain shape types when their fill style is
    // solid.
    ::utl::AccessibleStateSetHelper* pStateSet =
        static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet != NULL)
        switch (ShapeTypeHandler::Instance().GetTypeId (mxShape))
        {
            case DRAWING_PAGE:
            case DRAWING_RECTANGLE:
            case DRAWING_TEXT:
            {
                uno::Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY);
                if (xSet.is())
                {
                    try
                    {
                        uno::Any aValue = xSet->getPropertyValue (
                            OUString::createFromAscii ("FillStyle"));
                        drawing::FillStyle aFillStyle;
                        aValue >>= aFillStyle;
                        if (aFillStyle == drawing::FillStyle_SOLID)
                            pStateSet->AddState (AccessibleStateType::OPAQUE);
                    }
                    catch (::com::sun::star::beans::UnknownPropertyException)
                    {
                        // Ignore.
                    }
                }
            }
        }

    // Create a children manager when this shape has children of its own.
    Reference<drawing::XShapes> xShapes (mxShape, uno::UNO_QUERY);
    if (xShapes.is() && xShapes->getCount() > 0)
        mpChildrenManager = new ChildrenManager (
            this, xShapes, mShapeTreeInfo, *this);
    if (mpChildrenManager != NULL)
        mpChildrenManager->Update();

    // Register at shape as dispose listener.
    Reference<lang::XComponent> xComponent (mxShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener (this);

    // Beware! Here we leave the paths of the UNO API and descend into the
    // depths of the core.  Necessary for makeing the edit engine
    // accessible.
#if 0
    SdrView* pView = mShapeTreeInfo.GetSdrView ();
    const Window* pWindow = mShapeTreeInfo.GetWindow ();
    if (pView != NULL && pWindow != NULL)
    {
        SvxEditSource* pEditSource = new SvxTextEditSource (
            *GetSdrObjectFromXShape (mxShape), *pView, *pWindow);
        mpText = new SvxAccessibleTextHelper (
            this,
            ::std::auto_ptr<SvxEditSource>(pEditSource));
    }
#endif
}




bool AccessibleShape::operator== (const AccessibleShape& rShape)
{
    return this==&rShape;
}




//=====  XAccessibleContext  ==================================================

/** The children of this shape come from two sources: The children from
    group or scene shapes and the paragraphs of text.
*/
sal_Int32 SAL_CALL
       AccessibleShape::getAccessibleChildCount (void)
    throw ()
{
    CheckDisposedState ();
    sal_Int32 nChildCount = 0;

    // Add the number of shapes that are children of this shape.
    if (mpChildrenManager != NULL)
        nChildCount += mpChildrenManager->GetChildCount ();
    // Add the number text paragraphs.
    if (mpText != NULL)
        nChildCount += mpText->GetChildCount ();

    return nChildCount;
}




/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessibleShape::getAccessibleChild (sal_Int32 nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposedState ();

    uno::Reference<XAccessible> xChild;

    if ((mpChildrenManager != NULL) && (nIndex < mpChildrenManager->GetChildCount()))
    {
        xChild = mpChildrenManager->GetChild (nIndex);
    }
    else if (mpText != NULL)
    {
        sal_Int32 nI = nIndex;
        if (mpChildrenManager != NULL)
            nI -= mpChildrenManager->GetChildCount();
        xChild = mpText->GetChild (nI);
    }
    else
        throw lang::IndexOutOfBoundsException (
            ::rtl::OUString::createFromAscii ("shape has no child with index ")+nIndex,
            static_cast<uno::XWeak*>(this));

    return xChild;
}




//=====  XAccessibleComponent  ================================================

/** The implementation below is at the moment straightforward.  It iterates
    over all children (and thereby instances all children which have not
    been already instatiated) until a child covering the specifed point is
    found.
    This leaves room for improvement.  For instance, first iterate only over
    the already instantiated children and only if no match is found
    instantiate the remaining ones.
*/
uno::Reference<XAccessible > SAL_CALL
    AccessibleShape::getAccessibleAt (
        const awt::Point& aPoint)
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);

    sal_Int32 nChildCount = getAccessibleChildCount ();
    for (sal_Int32 i=0; i<nChildCount; ++i)
    {
        Reference<XAccessible> xChild (getAccessibleChild (i));
        if (xChild.is())
        {
            Reference<XAccessibleComponent> xChildComponent (
                xChild->getAccessibleContext(), uno::UNO_QUERY);
            if (xChildComponent.is())
            {
                awt::Rectangle aBBox (xChildComponent->getBounds());
                if ( (aPoint.X >= aBBox.X)
                    && (aPoint.Y >= aBBox.Y)
                    && (aPoint.X < aBBox.X+aBBox.Width)
                    && (aPoint.Y < aBBox.Y+aBBox.Height) )
                    return xChild;
            }
        }
    }

    // Have not found a child under the given point.  Returning empty
    // reference to indicate this.
    return uno::Reference<XAccessible>();
}




awt::Rectangle SAL_CALL AccessibleShape::getBounds (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposedState ();
    awt::Rectangle aBoundingBox;

    // Get the shape's bounding box in internal coordinates (in 100th of
    // mm).  Use the property BoundRect.  Only if that is not supported ask
    // the shape for its position and size directly.
    Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY);
    if (xSet.is())
    {
        try
        {
            uno::Any aValue = xSet->getPropertyValue (
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("BoundRect")));
            aValue >>= aBoundingBox;
        }
        catch (beans::UnknownPropertyException e)
        {
            // Fallback when there is no BoundRect Property.
            OSL_TRACE ("unknown property BoundRect");
            awt::Point aPosition (mxShape->getPosition());
            awt::Size aSize (mxShape->getSize());
            aBoundingBox = awt::Rectangle (
                aPosition.X, aPosition.Y,
                aSize.Width, aSize.Height);
        }
    }

    // Transform coordinates from internal to pixel.
    ::Size aPixelSize = mShapeTreeInfo.GetViewForwarder()->LogicToPixel (
        ::Size (aBoundingBox.Width, aBoundingBox.Height));
    ::Point aPixelPosition = mShapeTreeInfo.GetViewForwarder()->LogicToPixel (
        ::Point (aBoundingBox.X, aBoundingBox.Y));

    // Clip the shape's bounding box with the bounding box of its parent.
    Reference<XAccessibleComponent> xParentComponent (getAccessibleParent(), uno::UNO_QUERY);
    if (xParentComponent.is())
    {
        ::Rectangle aBBox (
            aPixelPosition.getX(),
            aPixelPosition.getY(),
            aPixelPosition.getX() + aPixelSize.getWidth(),
            aPixelPosition.getY() + aPixelSize.getHeight());
        awt::Rectangle aParentBoundingBox (xParentComponent->getBounds());
        ::Rectangle aParentBBox (
            aParentBoundingBox.X,
            aParentBoundingBox.Y,
            aParentBoundingBox.X + aParentBoundingBox.Width,
            aParentBoundingBox.Y + aParentBoundingBox.Height);
        aBBox = aBBox.GetIntersection (aParentBBox);
        aBoundingBox = awt::Rectangle (
            aBBox.getX(),
            aBBox.getY(),
            aBBox.getWidth(),
            aBBox.getHeight());
    }
    else
        aBoundingBox = awt::Rectangle (
            aPixelPosition.getX(), aPixelPosition.getY(),
            aPixelSize.getWidth(), aPixelSize.getHeight());

    return aBoundingBox;
}




awt::Point SAL_CALL AccessibleShape::getLocation (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposedState ();
    // Get absolute position...
    awt::Point aLocation (getLocationOnScreen ());

    // ... and subtract absolute position of the parent.
    uno::Reference<XAccessibleComponent> xParentComponent (
        getAccessibleParent(), uno::UNO_QUERY);
    if (xParentComponent.is())
    {
        awt::Point aParentLocation (xParentComponent->getLocationOnScreen());
        aLocation.X -= aParentLocation.X;
        aLocation.Y -= aParentLocation.Y;
    }
    else
        OSL_TRACE ("getLocation: parent does not support XAccessibleComponent");

    return aLocation;
}




awt::Point SAL_CALL AccessibleShape::getLocationOnScreen (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposedState ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Point (aBoundingBox.X, aBoundingBox.Y);
}




awt::Size SAL_CALL AccessibleShape::getSize (void)
    throw (uno::RuntimeException)
{
    CheckDisposedState ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Size (aBoundingBox.Width, aBoundingBox.Height);
}




sal_Bool SAL_CALL AccessibleShape::isShowing (void)
    throw (uno::RuntimeException)
{
    return mxStateSet->contains (AccessibleStateType::SHOWING);
}




sal_Bool SAL_CALL AccessibleShape::isVisible (void)
    throw (uno::RuntimeException)
{
    return mxStateSet->contains (AccessibleStateType::VISIBLE);
}




//=====  XAccessibleExtendedComponent  ========================================

sal_Int32 SAL_CALL AccessibleShape::getForeground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposedState ();
    sal_Int32 nColor (0x0ffffffL);

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxShape, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue (OUString::createFromAscii ("LineColor"));
            aColor >>= nColor;
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        // Ignore exception and return default color.
    }
    return nColor;
}




sal_Int32 SAL_CALL AccessibleShape::getBackground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposedState ();
    sal_Int32 nColor (0L);

    try
    {
        uno::Reference<beans::XPropertySet> aSet (mxShape, uno::UNO_QUERY);
        if (aSet.is())
        {
            uno::Any aColor;
            aColor = aSet->getPropertyValue (OUString::createFromAscii ("FillColor"));
            aColor >>= nColor;
        }
    }
    catch (::com::sun::star::beans::UnknownPropertyException)
    {
        // Ignore exception and return default color.
    }
    return nColor;
}




//=====  XInterface  ==========================================================

com::sun::star::uno::Any SAL_CALL
    AccessibleShape::queryInterface (const com::sun::star::uno::Type & rType)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Any aReturn = AccessibleContextBase::queryInterface (rType);
    if ( ! aReturn.hasValue())
        aReturn = ::cppu::queryInterface (rType,
            static_cast<XAccessibleComponent*>(this),
            static_cast<XAccessibleExtendedComponent*>(this),
            static_cast<lang::XEventListener*>(this),
            static_cast<lang::XUnoTunnel*>(this)
            );
    return aReturn;
}




void SAL_CALL
    AccessibleShape::acquire (void)
    throw ()
{
    AccessibleContextBase::acquire ();
}




void SAL_CALL
    AccessibleShape::release (void)
    throw ()
{
    AccessibleContextBase::release ();
}




//=====  XServiceInfo  ========================================================

::rtl::OUString SAL_CALL
    AccessibleShape::getImplementationName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposedState ();
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleShape"));
}




::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    AccessibleShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposedState ();
    return AccessibleContextBase::getSupportedServiceNames();
}





//=====  XTypeProvider  ===================================================

uno::Sequence<uno::Type> SAL_CALL
    AccessibleShape::getTypes (void)
    throw (uno::RuntimeException)
{
    CheckDisposedState ();
    // Get list of types from the context base implementation, ...
    uno::Sequence<uno::Type> aTypeList (AccessibleContextBase::getTypes());
    // ... get list of types from component base implementation, ...
    uno::Sequence<uno::Type> aComponentTypeList (AccessibleComponentBase::getTypes());
    // ... define local types, ...
    const uno::Type aEventListenerType =
        ::getCppuType((const uno::Reference<lang::XEventListener>*)0);
    const uno::Type aUnoTunnelType =
        ::getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);
    //    const uno::Type aStateSetType =
    //      ::getCppuType((const uno::Reference<XAccessibleStateSet>*)0);

    // ... and merge them all into one list.
    sal_Int32   nTypeCount (aTypeList.getLength()), nComponentTypeCount (aComponentTypeList.getLength());
    int         i;

    aTypeList.realloc (nTypeCount + nComponentTypeCount + 2);

    for (i=0; i<nComponentTypeCount; i++)
        aTypeList[nTypeCount + i] = aComponentTypeList[i];

    aTypeList[nTypeCount + i++ ] = aEventListenerType;
    aTypeList[nTypeCount + i ] = aUnoTunnelType;

    return aTypeList;
}




//=====  lang::XEventListener  ================================================

void SAL_CALL
    AccessibleShape::disposing (const lang::EventObject& aEvent)
    throw (uno::RuntimeException)
{
    try
    {
        CheckDisposedState ();
        OSL_TRACE ("AccessibleShape::disposing");

        if (aEvent.Source == mxShape)
        {
            uno::Reference<beans::XPropertySet> xShapeProperties (mxShape, uno::UNO_QUERY);
            SetState (AccessibleStateType::DEFUNC);
            mxShape = NULL;

            // Release the child containers.
            if (mpChildrenManager != NULL)
            {
                delete mpChildrenManager;
                mpChildrenManager = NULL;
            }
            if (mpText != NULL)
            {
                delete mpText;
                mpText = NULL;
            }
        }
    }
    catch (uno::RuntimeException e)
    {
        OSL_TRACE ("caught exception while disposing");
    }
}

//=====  lang::XUnoTunnel  ================================================

const uno::Sequence< sal_Int8 >&
    AccessibleShape::getUnoTunnelImplementationId()
    throw()
{
    static uno::Sequence< sal_Int8 >* pSeq = 0;

    if( !pSeq )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );

        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*) aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }

    return( *pSeq );
}

//------------------------------------------------------------------------------
AccessibleShape*
    AccessibleShape::getImplementation( const uno::Reference< uno::XInterface >& rxIFace )
    throw()
{
    uno::Reference< lang::XUnoTunnel >  xTunnel( rxIFace, uno::UNO_QUERY );
    AccessibleShape*                    pReturn = NULL;

    if( xTunnel.is() )
        pReturn = reinterpret_cast< AccessibleShape* >( xTunnel->getSomething( getUnoTunnelImplementationId() ) );

    return( pReturn );
}

//------------------------------------------------------------------------------
sal_Int64 SAL_CALL
    AccessibleShape::getSomething( const uno::Sequence< sal_Int8 >& rIdentifier )
    throw(uno::RuntimeException)
{
    sal_Int64 nReturn( 0 );

    if( ( rIdentifier.getLength() == 16 ) && ( 0 == rtl_compareMemory( getUnoTunnelImplementationId().getConstArray(), rIdentifier.getConstArray(), 16 ) ) )
        nReturn = reinterpret_cast< sal_Int64 >( this );

    return( nReturn );
}

//=====  IAccessibleViewForwarderListener  ====================================

void AccessibleShape::ViewForwarderChanged (ChangeType aChangeType,
        const IAccessibleViewForwarder* pViewForwarder)
{
    // Inform all listeners that the graphical representation (i.e. size
    // and/or position) of the shape has changed.
    CommitChange (AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT,
        uno::Any(),
        uno::Any());

    // Tell children manager of the modified view forwarder.
    if (mpChildrenManager != NULL)
        mpChildrenManager->ViewForwarderChanged (aChangeType, pViewForwarder);

}




//=====  protected internal  ==================================================
/// Set this object's name if is different to the current name.
::rtl::OUString
    AccessibleShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_3D_CUBE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("3DCubeShape"));
            break;
        case DRAWING_3D_EXTRUDE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("3DExtrudeShape"));
            break;
        case DRAWING_3D_LATHE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("3DLatheShape"));
            break;
        case DRAWING_3D_POLYGON:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("3DPolygonShape"));
            break;
        case DRAWING_3D_SCENE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("3DSceneShape"));
            break;
        case DRAWING_3D_SPHERE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("3DSphereShape"));
            break;
        case DRAWING_CAPTION:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("CaptionShape"));
            break;

        case DRAWING_CLOSED_BEZIER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ClosedBezierShape"));
            break;
        case DRAWING_CLOSED_FREEHAND:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ClosedFreehandShape"));
            break;
        case DRAWING_CONNECTOR:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("ConnectorShape"));
            break;
        case DRAWING_ELLIPSE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("EllipseShape"));
            break;
        case DRAWING_GROUP:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("GroupShape"));
            break;
        case DRAWING_LINE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("LineShape"));
            break;
        case DRAWING_MEASURE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("MeasureShape"));
            break;
        case DRAWING_OPEN_BEZIER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("OpenBezierShape"));
            break;
        case DRAWING_OPEN_FREEHAND:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("OpenFreehandShape"));
            break;
        case DRAWING_PAGE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("PageShape"));
            break;
        case DRAWING_POLY_LINE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("PolyLineShape"));
            break;
        case DRAWING_POLY_LINE_PATH:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("PolyLinePathShape"));
            break;
        case DRAWING_POLY_POLYGON:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("PolyPolygonShape"));
            break;
        case DRAWING_POLY_POLYGON_PATH:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("PolyPolygonPathShape"));
            break;
        case DRAWING_RECTANGLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("RectangleShape"));
            break;
        case DRAWING_TEXT:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("TextShape"));
            break;

        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("UnknownAccessibleShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM(": "))
                    + xDescriptor->getShapeType();
    }

    return sName;
}




::rtl::OUString
    AccessibleShape::CreateAccessibleName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OUString sName (CreateAccessibleBaseName());

    // Append the shape's index to the name to disambiguate between shapes
    // of the same type.  If such an index where not given to the
    // constructor then use the z-order instead.  If even that does not exist
    // we throw an exception.
    long nIndex = mnIndex;
    if (nIndex == -1)
    {
        try
        {
            uno::Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY);
            if (xSet.is())
            {
                uno::Any aZOrder (xSet->getPropertyValue (::rtl::OUString::createFromAscii ("ZOrder")));
                aZOrder >>= nIndex;
            }
        }
        catch (beans::UnknownPropertyException)
        {
            // We throw our own exception that is a bit more informative.
            throw uno::RuntimeException (::rtl::OUString (
                RTL_CONSTASCII_USTRINGPARAM("AccessibleShape has invalid index and no ZOrder property")),
                static_cast<uno::XWeak*>(this));
        }

    }
    sName += OUString::valueOf (nIndex);

    return sName;
}




::rtl::OUString
    AccessibleShape::CreateAccessibleDescription (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    DescriptionGenerator aDG (mxShape);
    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_3D_CUBE:
            aDG.Initialize (STR_ObjNameSingulCube3d);
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_EXTRUDE:
            aDG.Initialize (STR_ObjNameSingulExtrude3d);
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_LATHE:
            aDG.Initialize (STR_ObjNameSingulLathe3d);
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_POLYGON:
            aDG.Initialize (STR_ObjNameSingulPoly3d);
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_SCENE:
            aDG.Initialize (STR_ObjNameSingulScene3d);
            break;
        case DRAWING_3D_SPHERE:
            aDG.Initialize (STR_ObjNameSingulSphere3d);
            aDG.Add3DProperties ();
            break;
        case DRAWING_CAPTION:
            aDG.Initialize (STR_ObjNameSingulCAPTION);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_CLOSED_BEZIER:
            aDG.Initialize (STR_ObjNameSingulPATHFILL);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_CLOSED_FREEHAND:
            aDG.Initialize (STR_ObjNameSingulFREEFILL);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_CONNECTOR:
            aDG.Initialize (STR_ObjNameSingulEDGE);
            aDG.AddLineProperties ();
            break;
        case DRAWING_CONTROL:
            aDG.Initialize (STR_ObjNameSingulUno);
            aDG.AddProperty (OUString::createFromAscii ("ControlBackground"),
                DescriptionGenerator::COLOR,
                OUString());
            aDG.AddProperty (OUString::createFromAscii ("ControlBorder"),
                DescriptionGenerator::INTEGER,
                OUString());
            break;
        case DRAWING_ELLIPSE:
            aDG.Initialize (STR_ObjNameSingulCIRCE);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_GROUP:
            aDG.Initialize (STR_ObjNameSingulGRUP);
            break;
        case DRAWING_LINE:
            aDG.Initialize (STR_ObjNameSingulLINE);
            aDG.AddLineProperties ();
            break;
        case DRAWING_MEASURE:
            aDG.Initialize (STR_ObjNameSingulMEASURE);
            aDG.AddLineProperties ();
            break;
        case DRAWING_OPEN_BEZIER:
            aDG.Initialize (STR_ObjNameSingulPATHLINE);
            aDG.AddLineProperties ();
            break;
        case DRAWING_OPEN_FREEHAND:
            aDG.Initialize (STR_ObjNameSingulFREELINE);
            aDG.AddLineProperties ();
            break;
        case DRAWING_PAGE:
            aDG.Initialize (STR_ObjNameSingulPAGE);
            break;
        case DRAWING_POLY_LINE:
            aDG.Initialize (STR_ObjNameSingulPLIN);
            aDG.AddLineProperties ();
            break;
        case DRAWING_POLY_LINE_PATH:
            aDG.Initialize (STR_ObjNameSingulPLIN);
            aDG.AddLineProperties ();
            break;
        case DRAWING_POLY_POLYGON:
            aDG.Initialize (STR_ObjNameSingulPOLY);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_POLY_POLYGON_PATH:
            aDG.Initialize (STR_ObjNameSingulPOLY);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_RECTANGLE:
            aDG.Initialize (STR_ObjNameSingulRECT);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_TEXT:
            aDG.Initialize (STR_ObjNameSingulTEXT);
            aDG.AddTextProperties ();
            break;
        default:
            aDG.Initialize (::rtl::OUString::createFromAscii (
                "Unknown accessible shape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}

uno::Reference< drawing::XShape > AccessibleShape::GetXShape()
{
    return( mxShape );
}



// protected
void AccessibleShape::disposing (void)
{
    CheckDisposedState ();
    OSL_TRACE ("AccessibleShape::disposing()");

    // Unregister at broadcasters.
    Reference<lang::XComponent> xComponent (mxShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener (this);

    // Release the child containers.
    if (mpChildrenManager != NULL)
    {
        delete mpChildrenManager;
        mpChildrenManager = NULL;
    }
    if (mpText != NULL)
    {
        delete mpText;
        mpText = NULL;
    }

    // Cleanup.
    mxShape = NULL;

    // Call base classes.
    AccessibleContextBase::dispose ();
}





} // end of namespace accessibility
