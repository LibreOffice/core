/*************************************************************************
 *
 *  $RCSfile: AccessibleShape.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: af $ $Date: 2002-04-22 14:32:17 $
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

#ifndef _SVX_UNOEDSRC_HXX
#include "unoedsrc.hxx"
#endif
#ifndef SVX_UNOSHTXT_HXX
#include "unoshtxt.hxx"
#endif
#include "svdobj.hxx"
#include "svdmodel.hxx"
#include "unoapi.hxx"
#include <com/sun/star/uno/Exception.hpp>

#include "ShapeTypeHandler.hxx"
#include "SvxShapeTypes.hxx"

#ifndef _SVX_ACCESSIBILITY_HRC
#include "accessibility.hrc"
#endif
#ifndef _SVX_DIALMGR_HXX
#include "dialmgr.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;
using ::com::sun::star::uno::Reference;

namespace accessibility {

//=====  internal  ============================================================

AccessibleShape::AccessibleShape (const uno::Reference<drawing::XShape>& rxShape,
    const uno::Reference<XAccessible>& rxParent,
    const AccessibleShapeTreeInfo& rShapeTreeInfo,
    long nIndex)
    : AccessibleContextBase (rxParent,AccessibleRole::SHAPE),
      mpChildrenManager(NULL),
      mxShape (rxShape),
      mrShapeTreeInfo (rShapeTreeInfo),
      mnIndex (nIndex),
      mpText (NULL)
{
    // The main part of the initialization is done in the init method which
    // has to be called from this constructor's caller.
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
    // Create a children manager when this shape has children of its own.
    Reference<drawing::XShapes> xShapes (mxShape, uno::UNO_QUERY);
    if (xShapes.is() && xShapes->getCount() > 0)
        mpChildrenManager = new ChildrenManager (
            this, xShapes, mrShapeTreeInfo, *this);
    if (mpChildrenManager != NULL)
        mpChildrenManager->Update();

    // Register at shape as dispose listener.
    Reference<lang::XComponent> xComponent (mxShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener (this);

    // Beware! Here we leave the paths of the UNO API and descend into the
    // depths of the core.  Necessary for makeing the edit engine accessible.
    /*
    SdrView* pView = mrShapeTreeInfo.GetSdrView ();
    const Window* pWindow = mrShapeTreeInfo.GetWindow ();
    if (pView != NULL && pWindow != NULL)
    {
        SvxEditSource* pEditSource = new SvxTextEditSource (
            *GetSdrObjectFromXShape (mxShape), *pView, *pWindow);
        mpText = new SvxAccessibleTextHelper (
            this,
            ::std::auto_ptr<SvxEditSource>(pEditSource));
    }*/
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
    AccessibleShape::getAccessibleChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    CheckDisposedState ();
    if (nIndex >= 0)
    {
        if (mpChildrenManager != NULL)
            if (nIndex < mpChildrenManager->GetChildCount())
                return mpChildrenManager->GetChild (nIndex);
            else
            {
                // Adapt index to children (paragraphs) of the edit engine.
                nIndex -= mpChildrenManager->GetChildCount();
            }
        if (mpText != NULL)
        {
            try
            {
                if (nIndex < mpText->GetChildCount())
                    return mpText->GetChild (nIndex);
            }
            catch (::com::sun::star::lang::IndexOutOfBoundsException e)
            {
                return NULL;
            }
        }
    }

    throw lang::IndexOutOfBoundsException (
        ::rtl::OUString::createFromAscii ("shape has no child with index ")+nIndex,
        static_cast<uno::XWeak*>(this));
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

    // Get the shape's bounding box in internal coordinates.  Use the
    // property BoundRect.  Only if that is not supported ask the shape for
    // its position and size directly.
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
            OSL_TRACE ("unknown property BoundingBox");
            awt::Point aPosition (mxShape->getPosition());
            awt::Size aSize (mxShape->getSize());
            aBoundingBox = awt::Rectangle (
                aPosition.X, aPosition.Y,
                aSize.Width, aSize.Height);
        }
    }

    // Transform coordinates from internal to pixel.
    ::Size aPixelSize = mrShapeTreeInfo.GetViewForwarder()->LogicToPixel (
        ::Size (aBoundingBox.Width, aBoundingBox.Height));
    ::Point aPixelPosition = mrShapeTreeInfo.GetViewForwarder()->LogicToPixel (
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
    sal_Int32 nColor (0x0ffffffL);

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
            static_cast<lang::XEventListener*>(this)
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
    //    const uno::Type aStateSetType =
    //      ::getCppuType((const uno::Reference<XAccessibleStateSet>*)0);

    // ... and merge them all into one list.
    sal_Int32 nTypeCount (aTypeList.getLength()),
        nComponentTypeCount (aComponentTypeList.getLength());
    aTypeList.realloc (nTypeCount + nComponentTypeCount + 1);
    int i;
    for (i=0; i<nComponentTypeCount; i++)
        aTypeList[nTypeCount + i] = aComponentTypeList[i];
    aTypeList[nTypeCount + i] = aEventListenerType;

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
        }
    }
    catch (uno::RuntimeException e)
    {
        OSL_TRACE ("caught exception while disposing");
    }
}




//=====  XComponent  ==========================================================

void AccessibleShape::dispose (void)
    throw (uno::RuntimeException)
{
    CheckDisposedState ();
    OSL_TRACE ("AccessibleShape::dispose");

    // Unregister listeners.
    Reference<lang::XComponent> xComponent (mxShape, uno::UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener (this);

    // Cleanup.
    mxShape = NULL;

        // Call base classes.
    AccessibleContextBase::dispose ();
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
            aDG.Initialize (OUString::createFromAscii("3D Cube"));
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_EXTRUDE:
            aDG.Initialize (OUString::createFromAscii("3D Extrusion Object"));
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_LATHE:
            aDG.Initialize (OUString::createFromAscii("3D Lathe Object"));
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_POLYGON:
            aDG.Initialize (OUString::createFromAscii("3D Polygon"));
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_SCENE:
            aDG.Initialize (OUString::createFromAscii("3D Scene"));
            break;
        case DRAWING_3D_SPHERE:
            aDG.Initialize (OUString::createFromAscii("3D Sphere"));
            aDG.Add3DProperties ();
            break;
        case DRAWING_CAPTION:
            aDG.Initialize (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM("CaptionShape")));
            break;
        case DRAWING_CLOSED_BEZIER:
            aDG.Initialize (OUString::createFromAscii("Closed Bezier Curve"));
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_CLOSED_FREEHAND:
            aDG.Initialize (OUString::createFromAscii("Closed Freehand Curve"));
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_CONNECTOR:
            aDG.Initialize (OUString::createFromAscii("Connector"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_CONTROL:
            aDG.Initialize (OUString::createFromAscii("Control"));
            aDG.AddProperty (OUString::createFromAscii ("ControlBackground"),
                DescriptionGenerator::COLOR,
                OUString());
            aDG.AddProperty (OUString::createFromAscii ("ControlBorder"),
                DescriptionGenerator::INTEGER,
                OUString());
            break;
        case DRAWING_ELLIPSE:
            aDG.Initialize (RID_SVXSTR_A11Y_ST_ELLIPSE);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_GROUP:
            aDG.Initialize (OUString::createFromAscii("Group"));
            break;
        case DRAWING_LINE:
            aDG.Initialize (OUString::createFromAscii("Line"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_MEASURE:
            aDG.Initialize (OUString::createFromAscii("Dimension Line"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_OPEN_BEZIER:
            aDG.Initialize (RID_SVXSTR_A11Y_ST_OPEN_BEZIER_CURVE);
            aDG.AddLineProperties ();
            break;
        case DRAWING_OPEN_FREEHAND:
            aDG.Initialize (OUString::createFromAscii("Open Freehand Curve"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_PAGE:
            aDG.Initialize (OUString::createFromAscii("Page"));
            break;
        case DRAWING_POLY_LINE:
            aDG.Initialize (OUString::createFromAscii("Poly Line"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_POLY_LINE_PATH:
            aDG.Initialize (OUString::createFromAscii("Poly Line Path"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_POLY_POLYGON:
            aDG.Initialize (OUString::createFromAscii("Poly Polygon"));
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_POLY_POLYGON_PATH:
            aDG.Initialize (OUString::createFromAscii("Poly Polygon Path"));
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_RECTANGLE:
            aDG.Initialize (RID_SVXSTR_A11Y_ST_RECTANGLE);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_TEXT:
            aDG.Initialize (OUString::createFromAscii("Text"));
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


} // end of namespace accessibility
