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

#include <sal/config.h>
#include <sal/log.hxx>

#include <memory>
#include <utility>

#include "accessiblecell.hxx"

#include <svx/DescriptionGenerator.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <vcl/svapp.hxx>

#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/string.hxx>
#include <editeng/outlobj.hxx>
#include <svx/IAccessibleViewForwarder.hxx>
#include <svx/unoshtxt.hxx>
#include <svx/svdotext.hxx>

using namespace sdr::table;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

namespace accessibility {

AccessibleCell::AccessibleCell( const css::uno::Reference< css::accessibility::XAccessible>& rxParent, const sdr::table::CellRef& rCell, sal_Int32 nIndex, const AccessibleShapeTreeInfo& rShapeTreeInfo )
: AccessibleCellBase( rxParent, AccessibleRole::TABLE_CELL )
, maShapeTreeInfo( rShapeTreeInfo )
, mnIndexInParent( nIndex )
, mxCell( rCell )
{
    //Init the pAccTable var
    pAccTable = dynamic_cast <AccessibleTableShape *> (rxParent.get());
}


AccessibleCell::~AccessibleCell()
{
    DBG_ASSERT( mpText == nullptr, "svx::AccessibleCell::~AccessibleCell(), not disposed!?" );
}


void AccessibleCell::Init()
{
    SdrView* pView = maShapeTreeInfo.GetSdrView();
    const vcl::Window* pWindow = maShapeTreeInfo.GetWindow ();
    if( (pView != nullptr) && (pWindow != nullptr) && mxCell.is())
    {
        OutlinerParaObject* pOutlinerParaObject = mxCell->GetEditOutlinerParaObject().release(); // Get the OutlinerParaObject if text edit is active

        bool bOwnParaObject = pOutlinerParaObject != nullptr;

        if( !pOutlinerParaObject )
            pOutlinerParaObject = mxCell->GetOutlinerParaObject();

        // create AccessibleTextHelper to handle this shape's text
        if( pOutlinerParaObject )
        {
            // non-empty text -> use full-fledged edit source right away

            mpText.reset( new AccessibleTextHelper( std::make_unique<SvxTextEditSource>(mxCell->GetObject(), mxCell.get(), *pView, *pWindow) ) );
            if( mxCell.is() && mxCell.get()->IsActiveCell() )
                mpText->SetFocus();
            mpText->SetEventSource(this);
        }

        if( bOwnParaObject)
            delete pOutlinerParaObject;
    }
}


bool AccessibleCell::SetState (sal_Int16 aState)
{
    bool bStateHasChanged = false;

    if (aState == AccessibleStateType::FOCUSED && mpText != nullptr)
    {
        // Offer FOCUSED state to edit engine and detect whether the state
        // changes.
        bool bIsFocused = mpText->HaveFocus ();
        mpText->SetFocus();
        bStateHasChanged = (bIsFocused != mpText->HaveFocus ());
    }
    else
        bStateHasChanged = AccessibleContextBase::SetState (aState);

    return bStateHasChanged;
}


bool AccessibleCell::ResetState (sal_Int16 aState)
{
    bool bStateHasChanged = false;

    if (aState == AccessibleStateType::FOCUSED && mpText != nullptr)
    {
        // Try to remove FOCUSED state from the edit engine and detect
        // whether the state changes.
        bool bIsFocused = mpText->HaveFocus ();
        mpText->SetFocus (false);
        bStateHasChanged = (bIsFocused != mpText->HaveFocus ());
    }
    else
        bStateHasChanged = AccessibleContextBase::ResetState (aState);

    return bStateHasChanged;
}


// XInterface


Any SAL_CALL AccessibleCell::queryInterface( const Type& aType )
{
    return AccessibleCellBase::queryInterface( aType );
}


void SAL_CALL AccessibleCell::acquire(  ) throw ()
{
    AccessibleCellBase::acquire();
}


void SAL_CALL AccessibleCell::release(  ) throw ()
{
    AccessibleCellBase::release();
}


// XAccessibleContext


/** The children of this cell come from the paragraphs of text.
*/
sal_Int32 SAL_CALL AccessibleCell::getAccessibleChildCount()
{
    SolarMutexGuard aSolarGuard;
    ThrowIfDisposed ();
    return mpText != nullptr ? mpText->GetChildCount () : 0;
}


/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
Reference<XAccessible> SAL_CALL AccessibleCell::getAccessibleChild (sal_Int32 nIndex)
{
    SolarMutexGuard aSolarGuard;
    ThrowIfDisposed ();

    // todo: does GetChild throw IndexOutOfBoundsException?
    return mpText->GetChild (nIndex);
}


/** Return a copy of the state set.
    Possible states are:
        ENABLED
        SHOWING
        VISIBLE
*/
Reference<XAccessibleStateSet> SAL_CALL AccessibleCell::getAccessibleStateSet()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);
    Reference<XAccessibleStateSet> xStateSet;

    if (rBHelper.bDisposed || mpText == nullptr)
    {
        // Return a minimal state set that only contains the DEFUNC state.
        xStateSet = AccessibleContextBase::getAccessibleStateSet ();
    }
    else
    {
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if(pStateSet)
        {
            // Merge current FOCUSED state from edit engine.
            if (mpText != nullptr)
            {
                if (mpText->HaveFocus())
                    pStateSet->AddState (AccessibleStateType::FOCUSED);
                else
                    pStateSet->RemoveState (AccessibleStateType::FOCUSED);
            }
            // Set the invisible state for merged cell
            if (mxCell.is() && mxCell->isMerged())
                pStateSet->RemoveState(AccessibleStateType::VISIBLE);
            else
                pStateSet->AddState(AccessibleStateType::VISIBLE);


            //Just when the parent table is not read-only,set states EDITABLE,RESIZABLE,MOVEABLE
            css::uno::Reference<XAccessible> xTempAcc = getAccessibleParent();
            if( xTempAcc.is() )
            {
                css::uno::Reference<XAccessibleContext>
                                        xTempAccContext = xTempAcc->getAccessibleContext();
                if( xTempAccContext.is() )
                {
                    css::uno::Reference<XAccessibleStateSet> rState =
                        xTempAccContext->getAccessibleStateSet();
                    if( rState.is() )           {
                        css::uno::Sequence<short> aStates = rState->getStates();
                        int count = aStates.getLength();
                        for( int iIndex = 0;iIndex < count;iIndex++ )
                        {
                            if( aStates[iIndex] == AccessibleStateType::EDITABLE )
                            {
                                pStateSet->AddState (AccessibleStateType::EDITABLE);
                                pStateSet->AddState (AccessibleStateType::RESIZABLE);
                                pStateSet->AddState (AccessibleStateType::MOVEABLE);
                                break;
                            }
                        }
                    }
                }
            }
            // Create a copy of the state set that may be modified by the
            // caller without affecting the current state set.
            xStateSet.set(new ::utl::AccessibleStateSetHelper (*pStateSet));
        }
    }

    return xStateSet;
}


// XAccessibleComponent


sal_Bool SAL_CALL AccessibleCell::containsPoint( const css::awt::Point& aPoint)
{
    return AccessibleComponentBase::containsPoint( aPoint );
}

/** The implementation below is at the moment straightforward.  It iterates
    over all children (and thereby instances all children which have not
    been already instantiated) until a child covering the specified point is
    found.
    This leaves room for improvement.  For instance, first iterate only over
    the already instantiated children and only if no match is found
    instantiate the remaining ones.
*/
Reference<XAccessible > SAL_CALL  AccessibleCell::getAccessibleAtPoint ( const css::awt::Point& aPoint)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    sal_Int32 nChildCount = getAccessibleChildCount ();
    for (sal_Int32 i=0; i<nChildCount; ++i)
    {
        Reference<XAccessible> xChild (getAccessibleChild (i));
        if (xChild.is())
        {
            Reference<XAccessibleComponent> xChildComponent (xChild->getAccessibleContext(), uno::UNO_QUERY);
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


css::awt::Rectangle SAL_CALL AccessibleCell::getBounds()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    ThrowIfDisposed ();
    css::awt::Rectangle aBoundingBox;
    if( mxCell.is() )
    {
        // Get the cell's bounding box in internal coordinates (in 100th of mm)
        const ::tools::Rectangle aCellRect( mxCell->getCellRect() );

        // Transform coordinates from internal to pixel.
        if (maShapeTreeInfo.GetViewForwarder() == nullptr)
            throw uno::RuntimeException ("AccessibleCell has no valid view forwarder",static_cast<uno::XWeak*>(this));

        ::Size aPixelSize( maShapeTreeInfo.GetViewForwarder()->LogicToPixel(::Size(aCellRect.GetWidth(), aCellRect.GetHeight())) );
        ::Point aPixelPosition( maShapeTreeInfo.GetViewForwarder()->LogicToPixel( aCellRect.TopLeft() ));

        // Clip the shape's bounding box with the bounding box of its parent.
        Reference<XAccessibleComponent> xParentComponent ( getAccessibleParent(), uno::UNO_QUERY);
        if (xParentComponent.is())
        {
            // Make the coordinates relative to the parent.
            awt::Point aParentLocation (xParentComponent->getLocationOnScreen());
            int x = aPixelPosition.getX() - aParentLocation.X;
            int y = aPixelPosition.getY() - aParentLocation.Y;

            // Clip with parent (with coordinates relative to itself).
            ::tools::Rectangle aBBox ( x, y, x + aPixelSize.getWidth(), y + aPixelSize.getHeight());
            awt::Size aParentSize (xParentComponent->getSize());
            ::tools::Rectangle aParentBBox (0,0, aParentSize.Width, aParentSize.Height);
            aBBox = aBBox.GetIntersection (aParentBBox);
            aBoundingBox = awt::Rectangle ( aBBox.getX(), aBBox.getY(), aBBox.getWidth(), aBBox.getHeight());
        }
        else
        {
            SAL_INFO("svx", "parent does not support component");
            aBoundingBox = awt::Rectangle (aPixelPosition.getX(), aPixelPosition.getY(),aPixelSize.getWidth(), aPixelSize.getHeight());
        }
    }

    return aBoundingBox;
}


css::awt::Point SAL_CALL AccessibleCell::getLocation()
{
    ThrowIfDisposed ();
    css::awt::Rectangle aBoundingBox(getBounds());
    return css::awt::Point(aBoundingBox.X, aBoundingBox.Y);
}


css::awt::Point SAL_CALL AccessibleCell::getLocationOnScreen()
{
    ThrowIfDisposed ();

    // Get relative position...
    css::awt::Point aLocation(getLocation ());

    // ... and add absolute position of the parent.
    Reference<XAccessibleComponent> xParentComponent( getAccessibleParent(), uno::UNO_QUERY);
    if(xParentComponent.is())
    {
        css::awt::Point aParentLocation(xParentComponent->getLocationOnScreen());
        aLocation.X += aParentLocation.X;
        aLocation.Y += aParentLocation.Y;
    }
    else
    {
        SAL_WARN("svx", "parent does not support XAccessibleComponent");
    }

    return aLocation;
}


awt::Size SAL_CALL AccessibleCell::getSize()
{
    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Size (aBoundingBox.Width, aBoundingBox.Height);
}


void SAL_CALL AccessibleCell::grabFocus()
{
    AccessibleComponentBase::grabFocus();
}


sal_Int32 SAL_CALL AccessibleCell::getForeground()
{
    ThrowIfDisposed ();

    // todo
    return sal_Int32(0x0ffffffL);
}


sal_Int32 SAL_CALL AccessibleCell::getBackground()
{
    ThrowIfDisposed ();

    // todo
    return 0;
}


// XAccessibleExtendedComponent


css::uno::Reference< css::awt::XFont > SAL_CALL AccessibleCell::getFont()
{
//todo
    return AccessibleComponentBase::getFont();
}


OUString SAL_CALL AccessibleCell::getTitledBorderText()
{
    return AccessibleComponentBase::getTitledBorderText();
}


OUString SAL_CALL AccessibleCell::getToolTipText()
{
    return AccessibleComponentBase::getToolTipText();
}


// XAccessibleEventBroadcaster


void SAL_CALL AccessibleCell::addAccessibleEventListener( const Reference<XAccessibleEventListener >& rxListener)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);
    if (rBHelper.bDisposed || rBHelper.bInDispose)
    {
        Reference<XInterface> xSource( static_cast<XComponent *>(this) );
        lang::EventObject aEventObj(xSource);
        rxListener->disposing(aEventObj);
    }
    else
    {
        AccessibleContextBase::addAccessibleEventListener (rxListener);
        if (mpText != nullptr)
            mpText->AddEventListener (rxListener);
    }
}


void SAL_CALL AccessibleCell::removeAccessibleEventListener( const Reference<XAccessibleEventListener >& rxListener)
{
    SolarMutexGuard aSolarGuard;
    AccessibleContextBase::removeAccessibleEventListener(rxListener);
    if (mpText != nullptr)
        mpText->RemoveEventListener (rxListener);
}


// XServiceInfo


OUString SAL_CALL AccessibleCell::getImplementationName()
{
    return OUString("AccessibleCell");
}


Sequence<OUString> SAL_CALL AccessibleCell::getSupportedServiceNames()
{
    ThrowIfDisposed ();

    // Get list of supported service names from base class...
    uno::Sequence<OUString> aServiceNames = AccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    // ...and add additional names.
    aServiceNames.realloc (nCount + 1);
    aServiceNames[nCount] = "com.sun.star.drawing.AccessibleCell";

    return aServiceNames;
}


// IAccessibleViewForwarderListener


void AccessibleCell::ViewForwarderChanged()
{
    // Inform all listeners that the graphical representation (i.e. size
    // and/or position) of the shape has changed.
    CommitChange(AccessibleEventId::VISIBLE_DATA_CHANGED, Any(), Any());

    // update our children that our screen position might have changed
    if( mpText )
        mpText->UpdateChildren();
}


// protected


void AccessibleCell::disposing()
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard (maMutex);

    // Make sure to send an event that this object loses the focus in the
    // case that it has the focus.
    ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet != nullptr)
        pStateSet->RemoveState(AccessibleStateType::FOCUSED);

    if (mpText != nullptr)
    {
        mpText->Dispose();
        mpText.reset();
    }

    // Cleanup.  Remove references to objects to allow them to be
    // destroyed.
    mxCell.clear();
    maShapeTreeInfo.dispose();

    // Call base classes.
    AccessibleContextBase::dispose ();
}

sal_Int32 SAL_CALL AccessibleCell::getAccessibleIndexInParent()
{
    ThrowIfDisposed ();
    return mnIndexInParent;
}


OUString AccessibleCell::getCellName( sal_Int32 nCol, sal_Int32 nRow )
{
    OUStringBuffer aBuf;

    if (nCol < 26*26)
    {
        if (nCol < 26)
            aBuf.append( static_cast<sal_Unicode>( 'A' +
                        static_cast<sal_uInt16>(nCol)));
        else
        {
            aBuf.append( static_cast<sal_Unicode>( 'A' +
                        (static_cast<sal_uInt16>(nCol) / 26) - 1));
            aBuf.append( static_cast<sal_Unicode>( 'A' +
                        (static_cast<sal_uInt16>(nCol) % 26)));
        }
    }
    else
    {
        OUStringBuffer aStr;
        while (nCol >= 26)
        {
            sal_Int32 nC = nCol % 26;
            aStr.append(static_cast<sal_Unicode>( 'A' +
                    static_cast<sal_uInt16>(nC)));
            nCol = nCol - nC;
            nCol = nCol / 26 - 1;
        }
        aStr.append(static_cast<sal_Unicode>( 'A' +
                static_cast<sal_uInt16>(nCol)));
        aBuf.append(comphelper::string::reverseString(aStr.makeStringAndClear()));
    }
    aBuf.append( OUString::number(nRow+1) );
    return aBuf.makeStringAndClear();
}

OUString SAL_CALL AccessibleCell::getAccessibleName()
{
    ThrowIfDisposed ();
    SolarMutexGuard aSolarGuard;

    if( pAccTable )
    {
        try
        {
            sal_Int32 nRow = 0, nCol = 0;
            pAccTable->getColumnAndRow(mnIndexInParent, nCol, nRow);
            return getCellName( nCol, nRow );
        }
        catch(const Exception&)
        {
        }
    }

    return AccessibleCellBase::getAccessibleName();
}

void AccessibleCell::UpdateChildren()
{
    if (mpText)
        mpText->UpdateChildren();
}

/* MT: Above getAccessibleName was introduced with IA2 CWS, while below was introduce in 3.3 meanwhile. Check which one is correct
+If this is correct, we also don't need  sdr::table::CellRef getCellRef(), UpdateChildren(), getCellName( sal_Int32 nCol, sal_Int32 nRow ) above
+

OUString SAL_CALL AccessibleCell::getAccessibleName() throw (css::uno::RuntimeException)
{
    ThrowIfDisposed ();
    SolarMutexGuard aSolarGuard;

    if( mxCell.is() )
        return mxCell->getName();

    return AccessibleCellBase::getAccessibleName();
}
*/

} // end of namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
