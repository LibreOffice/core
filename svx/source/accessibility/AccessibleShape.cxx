/*************************************************************************
 *
 *  $RCSfile: AccessibleShape.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: af $ $Date: 2002-03-18 10:14:56 $
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

namespace accessibility {

//=====  internal  ============================================================

AccessibleShape::AccessibleShape (const uno::Reference<drawing::XShape>& rxShape,
    const uno::Reference<XAccessible>& rxParent,
    AccessibleShapeTreeInfo& rShapeTreeInfo,
    long nIndex)
    : AccessibleContextBase (rxParent,AccessibleRole::SHAPE),
      mxShape (rxShape),
      mrShapeTreeInfo (rShapeTreeInfo),
      mnIndex (nIndex)
{
    // Create a children manager when this shape has children of its own.
    uno::Reference<drawing::XShapes> xShapes (mxShape, uno::UNO_QUERY);
    if (xShapes.is() && xShapes->getCount() > 0)
    {
        mpChildrenManager = new ChildrenManager (this, rShapeTreeInfo, *this);
        Rectangle aBBox (mxShape->getPosition().X, mxShape->getPosition().Y,
            mxShape->getSize().Width, mxShape->getSize().Height);
        mpChildrenManager->Update (xShapes, aBBox);

    }
    else
        mpChildrenManager = NULL;
    mxChildrenManager = uno::Reference<uno::XInterface> (
        static_cast<uno::XWeak*>(mpChildrenManager));

    // Register as accessible event listener at document window.
    uno::Reference<XAccessibleEventBroadcaster> xBroadcaster (
        mrShapeTreeInfo.GetDocumentWindow(), uno::UNO_QUERY);
    if (xBroadcaster.is())
        xBroadcaster->addEventListener (uno::Reference<XAccessibleEventListener>(this));

    // Register at shape as property change listener.
    uno::Reference<beans::XPropertySet> xShapeProperties (mxShape, uno::UNO_QUERY);
    if (xShapeProperties.is())
        xShapeProperties->addPropertyChangeListener (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("")),
            uno::Reference<beans::XPropertyChangeListener>(this));

    // Register at controller as property change listener.
    uno::Reference<beans::XPropertySet> xControllerProperties (
        mrShapeTreeInfo.GetControllerProperties(), uno::UNO_QUERY);
    if (xControllerProperties.is())
        xControllerProperties->addPropertyChangeListener (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("VisibleArea")),
            uno::Reference<beans::XPropertyChangeListener>(this));
}




AccessibleShape::~AccessibleShape (void)
{
    // mpChildrenManager is not deleted because that is handled by the
    // mxChildrenManager reference.
    OSL_TRACE ("~AccessibleShape");

    // Unregister from the various broadcasters.
    uno::Reference<XAccessibleEventBroadcaster> xBroadcaster (
        mrShapeTreeInfo.GetDocumentWindow(), uno::UNO_QUERY);
    if (xBroadcaster.is())
        xBroadcaster->removeEventListener (uno::Reference<XAccessibleEventListener>(this));

    uno::Reference<beans::XPropertySet> xShapeProperties (mxShape, uno::UNO_QUERY);
    if (xShapeProperties.is())
        xShapeProperties->removePropertyChangeListener (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("")),
            uno::Reference<beans::XPropertyChangeListener>(this));

    uno::Reference<beans::XPropertySet> xControllerProperties (
        mrShapeTreeInfo.GetControllerProperties(), uno::UNO_QUERY);
    if (xControllerProperties.is())
        xControllerProperties->removePropertyChangeListener (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("VisibleArea")),
            uno::Reference<beans::XPropertyChangeListener>(this));
}




bool AccessibleShape::operator== (const AccessibleShape& rShape)
{
    return this==&rShape;
}




//=====  XAccessibleContext  ==================================================

uno::Reference<XAccessible> SAL_CALL
    AccessibleShape::getAccessibleParent (void)
    throw (uno::RuntimeException)
{
    return AccessibleContextBase::getAccessibleParent();
}




/** Return the number of children of the original shape.
*/
sal_Int32 SAL_CALL
       AccessibleShape::getAccessibleChildCount (void)
    throw ()
{
    if (mpChildrenManager != NULL)
        return mpChildrenManager->GetChildCount ();
    else
        return 0;
}




/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
uno::Reference<XAccessible> SAL_CALL
    AccessibleShape::getAccessibleChild (long nIndex)
    throw (::com::sun::star::uno::RuntimeException)
{
    if (mpChildrenManager != NULL)
        return mpChildrenManager->GetChild (nIndex);
    else
        throw lang::IndexOutOfBoundsException (
            ::rtl::OUString::createFromAscii ("shape has no child"),
            NULL);
}




//=====  XAccessibleComponent  ================================================

awt::Rectangle SAL_CALL AccessibleShape::getBounds (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Point aP = mxShape->getPosition ();
    awt::Point aPosition (mrShapeTreeInfo.PointToPixel (aP));
    awt::Size aSize (mrShapeTreeInfo.SizeToPixel (mxShape->getSize ()));
    return awt::Rectangle (
        aPosition.X,
        aPosition.Y,
        aSize.Width,
        aSize.Height);
}




awt::Point SAL_CALL AccessibleShape::getLocation (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Rectangle aBBox = getBounds();
    return awt::Point (aBBox.X, aBBox.Y);
}




awt::Point SAL_CALL AccessibleShape::getLocationOnScreen (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Point aLocation (getLocation ());

    // Get the screen location of the parent and add it to this object's
    // location.
    if (mrShapeTreeInfo.GetDocumentWindow().is())
    {
        awt::Point aParentLocation (mrShapeTreeInfo.GetDocumentWindow()->getLocationOnScreen ());
        aLocation.X += aParentLocation.X;
        aLocation.Y += aParentLocation.Y;
    }

    return aLocation;
}




::com::sun::star::awt::Size SAL_CALL AccessibleShape::getSize (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    awt::Rectangle aBBox = getBounds();
    return awt::Size (aBBox.Width, aBBox.Height);
}




//=====  XAccessibleExtendedComponent  ========================================

sal_Int32 SAL_CALL AccessibleShape::getForeground (void)
    throw (::com::sun::star::uno::RuntimeException)
{
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
            static_cast<XAccessibleEventListener*>(this),
            static_cast<beans::XPropertyChangeListener*>(this));
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
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("AccessibleShape"));
}




::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL
    AccessibleShape::getSupportedServiceNames (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    return AccessibleContextBase::getSupportedServiceNames();
}





//=====  XTypeProvider  ===================================================

uno::Sequence<uno::Type> SAL_CALL
    AccessibleShape::getTypes (void)
    throw (uno::RuntimeException)
{
    // Get list of types from the context base implementation, ...
    uno::Sequence<uno::Type> aTypeList (AccessibleContextBase::getTypes());
    // ... get list of types from component base implementation, ...
    uno::Sequence<uno::Type> aComponentTypeList (AccessibleComponentBase::getTypes());
    // ... define local types, ...
    const uno::Type aEventListenerType =
        ::getCppuType((const uno::Reference<XAccessibleEventListener>*)0);
    const uno::Type aPropertyChangeListenerType =
        ::getCppuType((const uno::Reference<beans::XPropertyChangeListener>*)0);

    // ... and merge them all into one list.
    sal_Int32 nTypeCount (aTypeList.getLength()),
        nComponentTypeCount (aComponentTypeList.getLength());
    aTypeList.realloc (nTypeCount + nComponentTypeCount + 2);
    int i;
    for (i=0; i<nComponentTypeCount; i++)
    {
        aTypeList[nTypeCount + i] = aComponentTypeList[i];
    }
    aTypeList[nTypeCount + i] = aEventListenerType;
    aTypeList[nTypeCount + i + 1] = aPropertyChangeListenerType;

    return aTypeList;
}




//=====  lang::XEventListener  ================================================

void SAL_CALL
    AccessibleShape::disposing (const ::com::sun::star::lang::EventObject& aEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE ("AccessibleShape::disposing");

    // Either the document window, the shape, or the controller has been
    // disposed.  Each one leads at least to not being able to determine the
    // accessible shapes coordinates.  Therefore dispose this object.

    dispose ();
}




//=====  XComponent  ==========================================================

void AccessibleShape::dispose (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE ("AccessibleShape::dispose");

    // Unregister listeners.
    uno::Reference<XAccessibleEventBroadcaster> xBroadcaster (
        mrShapeTreeInfo.GetDocumentWindow(), uno::UNO_QUERY);
    xBroadcaster->removeEventListener (uno::Reference<XAccessibleEventListener>(this));

    uno::Reference<beans::XPropertySet> xShapeProperties (mxShape, uno::UNO_QUERY);
    if (xShapeProperties.is())
        xShapeProperties->removePropertyChangeListener (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("")),
            uno::Reference<beans::XPropertyChangeListener>(this));

    uno::Reference<beans::XPropertySet> xControllerProperties (
        mrShapeTreeInfo.GetControllerProperties(), uno::UNO_QUERY);
    if (xControllerProperties.is())
        xControllerProperties->removePropertyChangeListener (
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("VisibleArea")),
            uno::Reference<beans::XPropertyChangeListener>(this));

    // Cleanup.
    mxShape = NULL;

    // Call disposing on all listeners.
    uno::Reference<XAccessibleEventListener>* aListeners;
    int nListenerCount;
    EventListenerListType::iterator I;

    {
        // Guarded by the mutex copy list of listeners into a local list.
        // This is necessary to be able to call the listeners outside the
        // scope of the mutex and to not to be disturbed by outside changes
        // to the list of listeners.
        ::vos::OGuard aGuard (maMutex);
        nListenerCount = mxEventListeners.size();
        aListeners = new uno::Reference<XAccessibleEventListener>[nListenerCount];
        int i=0;
        for (I=mxEventListeners.begin(); I!=mxEventListeners.end(); I++)
        {
            aListeners[i++] = *I;
        }
    }

    // Now call the listeners.
    lang::EventObject aEvent (static_cast<uno::XWeak*>(this));
    for (int i=0; i<nListenerCount; i++)
        if (aListeners[i].is())
            aListeners[i]->disposing (aEvent);

    delete [] aListeners;
}




//=====  XAccessibleEventListener  ============================================

void SAL_CALL
    AccessibleShape::notifyEvent (const AccessibleEventObject& aEvent)
    throw (::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE ("AccessibleShape::notifyEvent %d", aEvent.EventId);
    if (aEvent.EventId == AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT)
    {
        // Visual appearance changed.  This includes size and position in
        // which we are interested.
        CommitChange (AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT,
            uno::Any(),uno::Any());
    }
}





//=====  XPropertyChangeListener  =============================================

void SAL_CALL
    AccessibleShape::propertyChange (const beans::PropertyChangeEvent& rEvent)
    throw (uno::RuntimeException)
{
    OSL_TRACE ("AccessibleShape::propertyChange");
    if (rEvent.Source == mxShape)
    {
        // Event came from the shape.  It therefore very likely affects the
        // appearance of the shape.  Send one VISUAL_DATA and a
        // DESCRIPTION event, because the description depends on some
        // of the graphical attributes.
        CommitChange (AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT,
            uno::Any(),uno::Any());

        SetAccessibleDescription (CreateAccessibleDescription());
        CommitChange (AccessibleEventId::ACCESSIBLE_DESCRIPTION_EVENT,
            uno::Any(),uno::Any());
    }
    else if (rEvent.Source == mrShapeTreeInfo.GetControllerProperties())
    {
        OSL_TRACE ("    controller property change of %s",
            rtl::OUStringToOString (rEvent.PropertyName, RTL_TEXTENCODING_ASCII_US).getStr());
        uno::Reference<beans::XPropertySet> xSet (rEvent.Source, uno::UNO_QUERY);
        awt::Rectangle aVisibleArea;
        try{
        if (xSet.is())
        {
            uno::Any aVA = xSet->getPropertyValue (
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("VisibleArea")));
            aVA >>= aVisibleArea;
        }
        }catch (uno::RuntimeException& e)
        {
            OSL_TRACE ("    VisibleArea property not known");
        }
        OSL_TRACE ("    visible area is now %d %d %d %d",
            aVisibleArea.X, aVisibleArea.Y,aVisibleArea.Width, aVisibleArea.Height);
        // Event came from the controller.  That means that the visible area
        // changed.
        mrShapeTreeInfo.CalcCSChangeTransformation();
        CommitChange (AccessibleEventId::ACCESSIBLE_VISIBLE_DATA_EVENT,
            uno::Any(),uno::Any());
    }
    else
        OSL_TRACE ("unknown property event change source");
}




/// Set this object's name if is different to the current name.
::rtl::OUString
    AccessibleShape::CreateAccessibleBaseName (void)
    throw (::com::sun::star::uno::RuntimeException)
{
    ::rtl::OUString sName;

    ShapeTypeId nShapeType = ShapeTypeHandler::Instance().GetTypeId (mxShape);
    switch (nShapeType)
    {
        case DRAWING_RECTANGLE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("RectangleShape"));
            break;
        case DRAWING_ELLIPSE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("EllipseShape"));
            break;
        case DRAWING_CONTROL:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("ConrolShape"));
            break;
        case DRAWING_CONNECTOR:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("ConnectorShape"));
            break;
        case DRAWING_MEASURE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("MeasureShape"));
            break;
        case DRAWING_LINE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("LineShape"));
            break;
        case DRAWING_POLY_POLYGON:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PolyPolygonShape"));
            break;
        case DRAWING_POLY_LINE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PolyLineShape"));
            break;
        case DRAWING_OPEN_BEZIER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("OpenBezierShape"));
            break;
        case DRAWING_CLOSED_BEZIER:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("ClosedBezierShape"));
            break;
        case DRAWING_OPEN_FREEHAND:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("OpenFreehandShape"));
            break;
        case DRAWING_CLOSED_FREEHAND:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("ClosedFreehandShape"));
            break;
        case DRAWING_POLY_POLYGON_PATH:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PolyPolygonPathShape"));
            break;
        case DRAWING_POLY_LINE_PATH:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PolyLinePathShape"));
            break;
        case DRAWING_GROUP:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("GroupShape"));
            break;
        case DRAWING_TEXT:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("TextShape"));
            break;
        case DRAWING_PAGE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("PageShape"));
            break;
        case DRAWING_3D_SCENE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DSceneShape"));
            break;
        case DRAWING_3D_CUBE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DCubeShape"));
            break;
        case DRAWING_3D_SPHERE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DSphereShape"));
            break;
        case DRAWING_3D_LATHE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DLatheShape"));
            break;
        case DRAWING_3D_EXTRUDE:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DExtrudeShape"));
            break;
        case DRAWING_3D_POLYGON:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("3DPolygonShape"));
            break;
        default:
            sName = ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("UnknownAccessibleShape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
                sName += ::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM (": "))
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
    // constructor then use the z-order instead.
    long nIndex = mnIndex;
    /*    if (nIndex == -1)
    {
        uno::Reference<beans::XPropertySet> xSet (mxShape, uno::UNO_QUERY);
        if (xSet.is())
        {
            uno::Any aZOrder (xSet->getPropertyValue (::rtl::OUString::createFromAscii ("ZOrder")));
            aZOrder >>= nIndex;
        }
    }
    */    sName += OUString::valueOf (nIndex);

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
        case DRAWING_RECTANGLE:
            aDG.Initialize (RID_SVXSTR_A11Y_ST_RECTANGLE);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_ELLIPSE:
            aDG.Initialize (RID_SVXSTR_A11Y_ST_ELLIPSE);
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
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
        case DRAWING_CONNECTOR:
            aDG.Initialize (OUString::createFromAscii("Connector"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_MEASURE:
            aDG.Initialize (OUString::createFromAscii("Dimension Line"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_LINE:
            aDG.Initialize (OUString::createFromAscii("Line"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_POLY_POLYGON:
            aDG.Initialize (OUString::createFromAscii("Poly Polygon"));
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_POLY_LINE:
            aDG.Initialize (OUString::createFromAscii("Poly Line"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_OPEN_BEZIER:
            aDG.Initialize (RID_SVXSTR_A11Y_ST_OPEN_BEZIER_CURVE);
            aDG.AddLineProperties ();
            break;
        case DRAWING_CLOSED_BEZIER:
            aDG.Initialize (OUString::createFromAscii("Closed Bezier Curve"));
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_OPEN_FREEHAND:
            aDG.Initialize (OUString::createFromAscii("Open Freehand Curve"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_CLOSED_FREEHAND:
            aDG.Initialize (OUString::createFromAscii("Closed Freehand Curve"));
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_POLY_POLYGON_PATH:
            aDG.Initialize (OUString::createFromAscii("Poly Polygon Path"));
            aDG.AddLineProperties ();
            aDG.AddFillProperties ();
            break;
        case DRAWING_POLY_LINE_PATH:
            aDG.Initialize (OUString::createFromAscii("Poly Line Path"));
            aDG.AddLineProperties ();
            break;
        case DRAWING_GROUP:
            aDG.Initialize (OUString::createFromAscii("Group"));
            break;
        case DRAWING_TEXT:
            aDG.Initialize (OUString::createFromAscii("Text"));
            aDG.AddTextProperties ();
            break;
        case DRAWING_PAGE:
            aDG.Initialize (OUString::createFromAscii("Page"));
            break;
        case DRAWING_3D_SCENE:
            aDG.Initialize (OUString::createFromAscii("3D Scene"));
            break;
        case DRAWING_3D_CUBE:
            aDG.Initialize (OUString::createFromAscii("3D Cube"));
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_SPHERE:
            aDG.Initialize (OUString::createFromAscii("3D Sphere"));
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_LATHE:
            aDG.Initialize (OUString::createFromAscii("3D Lathe Object"));
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_EXTRUDE:
            aDG.Initialize (OUString::createFromAscii("3D Extrusion Object"));
            aDG.Add3DProperties ();
            break;
        case DRAWING_3D_POLYGON:
            aDG.Initialize (OUString::createFromAscii("3D Polygon"));
            aDG.Add3DProperties ();
            break;
        default:
            aDG.Initialize (::rtl::OUString::createFromAscii (
                "Unknown accessible shape"));
            uno::Reference<drawing::XShapeDescriptor> xDescriptor (mxShape, uno::UNO_QUERY);
            if (xDescriptor.is())
            {
                aDG.AppendString (::rtl::OUString (RTL_CONSTASCII_USTRINGPARAM ("service name=")));
                aDG.AppendString (xDescriptor->getShapeType());
            }
    }

    return aDG();
}


} // end of namespace accessibility
