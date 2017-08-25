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

#include <comphelper/sequence.hxx>
#include <svtools/unoevent.hxx>
#include <svtools/unoimap.hxx>
#include <svx/svdobj.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/event.hxx>
#include <svx/unoshape.hxx>
#include <editeng/unofield.hxx>
#include <svx/shapepropertynotifier.hxx>
#include <toolkit/helper/convert.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>

#include "shapeuno.hxx"
#include "miscuno.hxx"
#include "cellsuno.hxx"
#include "textuno.hxx"
#include "fielduno.hxx"
#include "docsh.hxx"
#include "drwlayer.hxx"
#include "userdat.hxx"
#include "unonames.hxx"

using namespace ::com::sun::star;

static const SfxItemPropertyMapEntry* lcl_GetShapeMap()
{
    static const SfxItemPropertyMapEntry aShapeMap_Impl[] =
    {
        {OUString(SC_UNONAME_ANCHOR), 0, cppu::UnoType<uno::XInterface>::get(), 0, 0 },
        {OUString(SC_UNONAME_HORIPOS), 0, cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {OUString(SC_UNONAME_IMAGEMAP), 0, cppu::UnoType<container::XIndexContainer>::get(), 0, 0 },
        {OUString(SC_UNONAME_VERTPOS), 0, cppu::UnoType<sal_Int32>::get(), 0, 0 },
        {OUString(SC_UNONAME_MOVEPROTECT), 0, cppu::UnoType<sal_Bool>::get(), 0, 0 },
        // #i66550 HLINK_FOR_SHAPES
        {OUString(SC_UNONAME_HYPERLINK), 0, cppu::UnoType<OUString>::get(), 0, 0 },
        {OUString(SC_UNONAME_URL), 0, cppu::UnoType<OUString>::get(), 0, 0 },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    return aShapeMap_Impl;
}

const SvEventDescription* ScShapeObj::GetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { SvMacroItemId::NONE, nullptr }
    };
    return aMacroDescriptionsImpl;
}
// #i66550 HLINK_FOR_SHAPES
ScMacroInfo* ScShapeObj_getShapeHyperMacroInfo( const ScShapeObj* pShape, bool bCreate = false )
{
        if( pShape )
            if( SdrObject* pObj = pShape->GetSdrObject() )
                return ScDrawLayer::GetMacroInfo( pObj, bCreate );
        return nullptr;
}

namespace
{
    void lcl_initializeNotifier( SdrObject& _rSdrObj, ::cppu::OWeakObject& _rShape )
    {
        std::shared_ptr<svx::IPropertyValueProvider> pProvider( new svx::PropertyValueProvider( _rShape, "Anchor" ) );
        _rSdrObj.getShapePropertyChangeNotifier().registerProvider( svx::ShapeProperty::CalcDocAnchor, pProvider );
    }
}

ScShapeObj::ScShapeObj( uno::Reference<drawing::XShape>& xShape ) :
      pShapePropertySet(nullptr),
      pShapePropertyState(nullptr),
      bIsTextShape(false),
      bIsNoteCaption(false),
      bInitializedNotifier(false)
{
    osl_atomic_increment( &m_refCount );

    {
        mxShapeAgg.set( xShape, uno::UNO_QUERY );
        // extra block to force deletion of the temporary before setDelegator
    }

    if (mxShapeAgg.is())
    {
        xShape = nullptr;      // during setDelegator, mxShapeAgg must be the only ref

        mxShapeAgg->setDelegator( static_cast<cppu::OWeakObject*>(this) );

        xShape.set(uno::Reference<drawing::XShape>( mxShapeAgg, uno::UNO_QUERY ));

        bIsTextShape = ( SvxUnoTextBase::getImplementation( mxShapeAgg ) != nullptr );
    }

    {
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            bIsNoteCaption = ScDrawLayer::IsNoteCaption( pObj );
            lcl_initializeNotifier( *pObj, *this );
            bInitializedNotifier = true;
        }
    }

    osl_atomic_decrement( &m_refCount );
}

ScShapeObj::~ScShapeObj()
{
//  if (mxShapeAgg.is())
//      mxShapeAgg->setDelegator(uno::Reference<uno::XInterface>());
}

// XInterface

uno::Any SAL_CALL ScShapeObj::queryInterface( const uno::Type& rType )
{
    uno::Any aRet = ScShapeObj_Base::queryInterface( rType );

    if ( !aRet.hasValue() && bIsTextShape )
        aRet = ScShapeObj_TextBase::queryInterface( rType );

    if ( !aRet.hasValue() && bIsNoteCaption )
        aRet = ScShapeObj_ChildBase::queryInterface( rType );

    if ( !aRet.hasValue() && mxShapeAgg.is() )
        aRet = mxShapeAgg->queryAggregation( rType );

    return aRet;
}

void SAL_CALL ScShapeObj::acquire() throw()
{
        OWeakObject::acquire();
}

void SAL_CALL ScShapeObj::release() throw()
{
        OWeakObject::release();
}

void ScShapeObj::GetShapePropertySet()
{
    // #i61908# Store the result of queryAggregation in a member.
    // The reference in mxShapeAgg is kept for this object's lifetime, so the pointer is always valid.

    if (!pShapePropertySet)
    {
        uno::Reference<beans::XPropertySet> xProp;
        if ( mxShapeAgg.is() )
            mxShapeAgg->queryAggregation( cppu::UnoType<beans::XPropertySet>::get()) >>= xProp;
        pShapePropertySet = xProp.get();
    }
}

void ScShapeObj::GetShapePropertyState()
{
    // #i61908# Store the result of queryAggregation in a member.
    // The reference in mxShapeAgg is kept for this object's lifetime, so the pointer is always valid.

    if (!pShapePropertyState)
    {
        uno::Reference<beans::XPropertyState> xState;
        if ( mxShapeAgg.is() )
            mxShapeAgg->queryAggregation( cppu::UnoType<beans::XPropertyState>::get()) >>= xState;
        pShapePropertyState = xState.get();
    }
}

static uno::Reference<lang::XComponent> lcl_GetComponent( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<lang::XComponent> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( cppu::UnoType<lang::XComponent>::get()) >>= xRet;
    return xRet;
}

static uno::Reference<text::XText> lcl_GetText( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<text::XText> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( cppu::UnoType<text::XText>::get()) >>= xRet;
    return xRet;
}

static uno::Reference<text::XSimpleText> lcl_GetSimpleText( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<text::XSimpleText> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( cppu::UnoType<text::XSimpleText>::get()) >>= xRet;
    return xRet;
}

static uno::Reference<text::XTextRange> lcl_GetTextRange( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<text::XTextRange> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( cppu::UnoType<text::XTextRange>::get()) >>= xRet;
    return xRet;
}

//  XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScShapeObj::getPropertySetInfo()
{
    SolarMutexGuard aGuard;

    // #i61527# cache property set info for this object
    if ( !mxPropSetInfo.is() )
    {
        //  mix own and aggregated properties:
        GetShapePropertySet();
        if (pShapePropertySet)
        {
            uno::Reference<beans::XPropertySetInfo> xAggInfo(pShapePropertySet->getPropertySetInfo());
            const uno::Sequence<beans::Property> aPropSeq(xAggInfo->getProperties());
            mxPropSetInfo.set(new SfxExtItemPropertySetInfo( lcl_GetShapeMap(), aPropSeq ));
        }
    }
    return mxPropSetInfo;
}

static bool lcl_GetPageNum( const SdrPage* pPage, SdrModel& rModel, SCTAB& rNum )
{
    sal_uInt16 nCount = rModel.GetPageCount();
    for (sal_uInt16 i=0; i<nCount; i++)
        if ( rModel.GetPage(i) == pPage )
        {
            rNum = static_cast<SCTAB>(i);
            return true;
        }

    return false;
}

static bool lcl_GetCaptionPoint( const uno::Reference< drawing::XShape >& xShape, awt::Point& rCaptionPoint )
{
    bool bReturn = false;
    OUString sType(xShape->getShapeType());
    bool bCaptionShape( sType == "com.sun.star.drawing.CaptionShape" );
    if (bCaptionShape)
    {
        uno::Reference < beans::XPropertySet > xShapeProp (xShape, uno::UNO_QUERY);
        if (xShapeProp.is())
        {
            xShapeProp->getPropertyValue("CaptionPoint") >>= rCaptionPoint;
            bReturn = true;
        }
    }
    return bReturn;
}

static ScRange lcl_GetAnchorCell( const uno::Reference< drawing::XShape >& xShape, const ScDocument* pDoc, SCTAB nTab,
                          awt::Point& rUnoPoint, awt::Size& rUnoSize, awt::Point& rCaptionPoint )
{
    ScRange aReturn;
    rUnoPoint = xShape->getPosition();
    bool bCaptionShape(lcl_GetCaptionPoint(xShape, rCaptionPoint));
    if (pDoc->IsNegativePage(nTab))
    {
        rUnoSize = xShape->getSize();
        rUnoPoint.X += rUnoSize.Width; // the right top point is base
        if (bCaptionShape)
        {
            if (rCaptionPoint.X > 0 && rCaptionPoint.X > rUnoSize.Width)
                rUnoPoint.X += rCaptionPoint.X - rUnoSize.Width;
            if (rCaptionPoint.Y < 0)
                rUnoPoint.Y += rCaptionPoint.Y;
        }
        aReturn = pDoc->GetRange( nTab, tools::Rectangle( VCLPoint(rUnoPoint), VCLPoint(rUnoPoint) ));
    }
    else
    {
        if (bCaptionShape)
        {
            if (rCaptionPoint.X < 0)
                rUnoPoint.X += rCaptionPoint.X;
            if (rCaptionPoint.Y < 0)
                rUnoPoint.Y += rCaptionPoint.Y;
        }
        aReturn = pDoc->GetRange( nTab, tools::Rectangle( VCLPoint(rUnoPoint), VCLPoint(rUnoPoint) ));
    }

    return aReturn;
}

static awt::Point lcl_GetRelativePos( const uno::Reference< drawing::XShape >& xShape, const ScDocument* pDoc, SCTAB nTab, ScRange& rRange,
                              awt::Size& rUnoSize, awt::Point& rCaptionPoint)
{
    awt::Point aUnoPoint;
    rRange = lcl_GetAnchorCell(xShape, pDoc, nTab, aUnoPoint, rUnoSize, rCaptionPoint);
    tools::Rectangle aRect(pDoc->GetMMRect( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aStart.Tab() ));
    Point aPoint = pDoc->IsNegativePage(nTab) ? aRect.TopRight() : aRect.TopLeft();
    aUnoPoint.X -= aPoint.X();
    aUnoPoint.Y -= aPoint.Y();
    return aUnoPoint;
}

void SAL_CALL ScShapeObj::setPropertyValue(const OUString& aPropertyName, const uno::Any& aValue)
{
    SolarMutexGuard aGuard;

    if ( aPropertyName == SC_UNONAME_ANCHOR )
    {
        uno::Reference<sheet::XCellRangeAddressable> xRangeAdd(aValue, uno::UNO_QUERY);
        if (xRangeAdd.is())
        {
            SdrObject *pObj = GetSdrObject();
            if (pObj)
            {
                ScDrawLayer* pModel = static_cast<ScDrawLayer*>(pObj->GetModel());
                SdrPage* pPage = pObj->GetPage();
                if ( pModel && pPage )
                {
                    ScDocument* pDoc = pModel->GetDocument();
                    if ( pDoc )
                    {
                        SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                        if ( pObjSh && dynamic_cast<const ScDocShell*>( pObjSh) !=  nullptr )
                        {
                            ScDocShell* pDocSh = static_cast<ScDocShell*>(pObjSh);

                            SCTAB nTab = 0;
                            if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                            {
                                table::CellRangeAddress aAddress = xRangeAdd->getRangeAddress();
                                if (nTab == aAddress.Sheet)
                                {
                                    tools::Rectangle aRect(pDoc->GetMMRect( static_cast<SCCOL>(aAddress.StartColumn), static_cast<SCROW>(aAddress.StartRow),
                                        static_cast<SCCOL>(aAddress.EndColumn), static_cast<SCROW>(aAddress.EndRow), aAddress.Sheet ));
                                    awt::Point aRelPoint;
                                    uno::Reference<drawing::XShape> xShape( mxShapeAgg, uno::UNO_QUERY );
                                    if (xShape.is())
                                    {
                                        Point aPoint;
                                        Point aEndPoint;
                                        if (pDoc->IsNegativePage(nTab))
                                        {
                                            aPoint = aRect.TopRight();
                                            aEndPoint = aRect.BottomLeft();
                                        }
                                        else
                                        {
                                            aPoint = aRect.TopLeft();
                                            aEndPoint = aRect.BottomRight();
                                        }
                                        awt::Size aUnoSize;
                                        awt::Point aCaptionPoint;
                                        ScRange aRange;
                                        aRelPoint = lcl_GetRelativePos( xShape, pDoc, nTab, aRange, aUnoSize, aCaptionPoint );
                                        awt::Point aUnoPoint(aRelPoint);

                                        aUnoPoint.X += aPoint.X();
                                        aUnoPoint.Y += aPoint.Y();

                                        if ( aUnoPoint.Y > aEndPoint.Y() )
                                            aUnoPoint.Y = aEndPoint.Y() - 2;
                                        if (pDoc->IsNegativePage(nTab))
                                        {
                                            if ( aUnoPoint.X < aEndPoint.X() )
                                                aUnoPoint.X = aEndPoint.X() + 2;
                                            aUnoPoint.X -= aUnoSize.Width;
                                            // remove difference to caption point
                                            if (aCaptionPoint.X > 0 && aCaptionPoint.X > aUnoSize.Width)
                                                aUnoPoint.X -= aCaptionPoint.X - aUnoSize.Width;
                                        }
                                        else
                                        {
                                            if ( aUnoPoint.X > aEndPoint.X() )
                                                aUnoPoint.X = aEndPoint.X() - 2;
                                            if (aCaptionPoint.X < 0)
                                                aUnoPoint.X -= aCaptionPoint.X;
                                        }
                                        if (aCaptionPoint.Y < 0)
                                            aUnoPoint.Y -= aCaptionPoint.Y;

                                        xShape->setPosition(aUnoPoint);
                                        pDocSh->SetModified();
                                    }

                                    if (aAddress.StartRow != aAddress.EndRow) //should be a Spreadsheet
                                    {
                                        OSL_ENSURE(aAddress.StartRow == 0 && aAddress.EndRow == MAXROW &&
                                            aAddress.StartColumn == 0 && aAddress.EndColumn == MAXCOL, "here should be a XSpreadsheet");
                                        ScDrawLayer::SetPageAnchored(*pObj);
                                    }
                                    else
                                    {
                                        OSL_ENSURE(aAddress.StartRow == aAddress.EndRow &&
                                            aAddress.StartColumn == aAddress.EndColumn, "here should be a XCell");
                                        ScDrawObjData aAnchor;
                                        aAnchor.maStart = ScAddress(aAddress.StartColumn, aAddress.StartRow, aAddress.Sheet);
                                        aAnchor.maStartOffset = Point(aRelPoint.X, aRelPoint.Y);
                                        //Uno sets the Anchor in terms of the unrotated shape, not much we can do
                                        //about that since uno also displays the shape geometry in terms of the unrotated
                                        //shape. #TODO think about changing the anchoring behaviour here too
                                        //Currently we've only got a start anchor, not an end-anchor, so generate that now
                                        ScDrawLayer::UpdateCellAnchorFromPositionEnd(*pObj, aAnchor, *pDoc, aAddress.Sheet);
                                        ScDrawLayer::SetCellAnchored(*pObj, aAnchor);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
            throw lang::IllegalArgumentException("only XCell or XSpreadsheet objects allowed", static_cast<cppu::OWeakObject*>(this), 0);
    }
    else if ( aPropertyName == SC_UNONAME_IMAGEMAP )
    {
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            ImageMap aImageMap;
            uno::Reference< uno::XInterface > xImageMapInt(aValue, uno::UNO_QUERY);

            if( !xImageMapInt.is() || !SvUnoImageMap_fillImageMap( xImageMapInt, aImageMap ) )
                throw lang::IllegalArgumentException();

            ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo(pObj);
            if( pIMapInfo )
            {
                // replace existing image map
                pIMapInfo->SetImageMap( aImageMap );
            }
            else
            {
                // insert new user data with image map
                pObj->AppendUserData(new ScIMapInfo(aImageMap) );
            }
        }
    }
    else if ( aPropertyName == SC_UNONAME_HORIPOS )
    {
        sal_Int32 nPos = 0;
        if (aValue >>= nPos)
        {
            SdrObject *pObj = GetSdrObject();
            if (pObj)
            {
                ScDrawLayer* pModel = static_cast<ScDrawLayer*>(pObj->GetModel());
                SdrPage* pPage = pObj->GetPage();
                if ( pModel && pPage )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        ScDocument* pDoc = pModel->GetDocument();
                        if ( pDoc )
                        {
                            SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                            if ( pObjSh && dynamic_cast<const ScDocShell*>( pObjSh) !=  nullptr )
                            {
                                ScDocShell* pDocSh = static_cast<ScDocShell*>(pObjSh);
                                uno::Reference<drawing::XShape> xShape( mxShapeAgg, uno::UNO_QUERY );
                                if (xShape.is())
                                {
                                    if (ScDrawLayer::GetAnchorType(*pObj) == SCA_PAGE)
                                    {
                                        awt::Point aPoint(xShape->getPosition());
                                        awt::Size aSize(xShape->getSize());
                                        awt::Point aCaptionPoint;
                                        if (pDoc->IsNegativePage(nTab))
                                        {
                                            nPos *= -1;
                                            nPos -= aSize.Width;
                                        }
                                        if (lcl_GetCaptionPoint(xShape, aCaptionPoint))
                                        {
                                            if (pDoc->IsNegativePage(nTab))
                                            {
                                                if (aCaptionPoint.X > 0 && aCaptionPoint.X > aSize.Width)
                                                    nPos -= aCaptionPoint.X - aSize.Width;
                                            }
                                            else
                                            {
                                                if (aCaptionPoint.X < 0)
                                                    nPos -= aCaptionPoint.X;
                                            }
                                        }
                                        aPoint.X = nPos;
                                        xShape->setPosition(aPoint);
                                        pDocSh->SetModified();
                                    }
                                    else if (ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL)
                                    {
                                        awt::Size aUnoSize;
                                        awt::Point aCaptionPoint;
                                        ScRange aRange;
                                        awt::Point aUnoPoint(lcl_GetRelativePos( xShape, pDoc, nTab, aRange, aUnoSize, aCaptionPoint ));
                                        tools::Rectangle aRect(pDoc->GetMMRect( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() ));
                                        if (pDoc->IsNegativePage(nTab))
                                        {
                                            aUnoPoint.X = -nPos;
                                            Point aPoint(aRect.TopRight());
                                            Point aEndPoint(aRect.BottomLeft());
                                            aUnoPoint.X += aPoint.X();
                                            if (aUnoPoint.X < aEndPoint.X())
                                                aUnoPoint.X = aEndPoint.X() + 2;
                                            aUnoPoint.X -= aUnoSize.Width;
                                            if (aCaptionPoint.X > 0 && aCaptionPoint.X > aUnoSize.Width)
                                                aUnoPoint.X -= aCaptionPoint.X - aUnoSize.Width;
                                        }
                                        else
                                        {
                                            aUnoPoint.X = nPos;
                                            Point aPoint(aRect.TopLeft());
                                            Point aEndPoint(aRect.BottomRight());
                                            aUnoPoint.X += aPoint.X();
                                            if (aUnoPoint.X > aEndPoint.X())
                                                aUnoPoint.X = aEndPoint.X() - 2;
                                            if (aCaptionPoint.X < 0)
                                                aUnoPoint.X -= aCaptionPoint.X;
                                        }
                                        aUnoPoint.Y = xShape->getPosition().Y;
                                        xShape->setPosition(aUnoPoint);
                                        pDocSh->SetModified();
                                    }
                                    else
                                    {
                                        OSL_FAIL("unknown anchor type");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if ( aPropertyName == SC_UNONAME_VERTPOS )
    {
        sal_Int32 nPos = 0;
        if (aValue >>= nPos)
        {
            SdrObject *pObj = GetSdrObject();
            if (pObj)
            {
                ScDrawLayer* pModel = static_cast<ScDrawLayer*>(pObj->GetModel());
                SdrPage* pPage = pObj->GetPage();
                if ( pModel && pPage )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        ScDocument* pDoc = pModel->GetDocument();
                        if ( pDoc )
                        {
                            SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                            if ( pObjSh && dynamic_cast<const ScDocShell*>( pObjSh) !=  nullptr )
                            {
                                ScDocShell* pDocSh = static_cast<ScDocShell*>(pObjSh);
                                uno::Reference<drawing::XShape> xShape( mxShapeAgg, uno::UNO_QUERY );
                                if (xShape.is())
                                {
                                    if (ScDrawLayer::GetAnchorType(*pObj) == SCA_PAGE)
                                    {
                                        awt::Point aPoint = xShape->getPosition();
                                        awt::Point aCaptionPoint;
                                        if (lcl_GetCaptionPoint(xShape, aCaptionPoint))
                                        {
                                            if (aCaptionPoint.Y < 0)
                                                nPos -= aCaptionPoint.Y;
                                        }
                                        aPoint.Y = nPos;
                                        xShape->setPosition(aPoint);
                                        pDocSh->SetModified();
                                    }
                                    else if (ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL)
                                    {
                                        awt::Size aUnoSize;
                                        awt::Point aCaptionPoint;
                                        ScRange aRange;
                                        awt::Point aUnoPoint(lcl_GetRelativePos( xShape, pDoc, nTab, aRange, aUnoSize, aCaptionPoint ));
                                        tools::Rectangle aRect(pDoc->GetMMRect( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() ));
                                        Point aPoint(aRect.TopRight());
                                        Point aEndPoint(aRect.BottomLeft());
                                        aUnoPoint.Y = nPos;
                                        aUnoPoint.Y += aPoint.Y();
                                        if (aUnoPoint.Y > aEndPoint.Y())
                                            aUnoPoint.Y = aEndPoint.Y() - 2;
                                        if (aCaptionPoint.Y < 0)
                                            aUnoPoint.Y -= aCaptionPoint.Y;
                                        aUnoPoint.X = xShape->getPosition().X;
                                        xShape->setPosition(aUnoPoint);
                                        pDocSh->SetModified();
                                    }
                                    else
                                    {
                                        OSL_FAIL("unknown anchor type");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else if  ( aPropertyName == SC_UNONAME_HYPERLINK ||
               aPropertyName == SC_UNONAME_URL )
    {
        OUString sHlink;
        ScMacroInfo* pInfo = ScShapeObj_getShapeHyperMacroInfo(this, true);
        if ( ( aValue >>= sHlink ) && pInfo )
            pInfo->SetHlink( sHlink );
    }
    else if ( aPropertyName == SC_UNONAME_MOVEPROTECT )
    {
        if( SdrObject* pObj = GetSdrObject() )
        {
            bool aProt = false;
            if( aValue >>= aProt )
                pObj->SetMoveProtect( aProt );
        }
    }
    else
    {
        GetShapePropertySet();
        if (pShapePropertySet)
            pShapePropertySet->setPropertyValue( aPropertyName, aValue );
    }
}

uno::Any SAL_CALL ScShapeObj::getPropertyValue( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    uno::Any aAny;
    if ( aPropertyName == SC_UNONAME_ANCHOR )
    {
        SdrObject *pObj = GetSdrObject();
        if (pObj)
        {
            ScDrawLayer* pModel = static_cast<ScDrawLayer*>(pObj->GetModel());
            SdrPage* pPage = pObj->GetPage();
            if ( pModel && pPage )
            {
                ScDocument* pDoc = pModel->GetDocument();
                if ( pDoc )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                        if ( pObjSh && dynamic_cast<const ScDocShell*>( pObjSh) !=  nullptr )
                        {
                            ScDocShell* pDocSh = static_cast<ScDocShell*>(pObjSh);
                            uno::Reference< uno::XInterface > xAnchor;
                            if (ScDrawObjData *pAnchor = ScDrawLayer::GetObjDataTab(pObj, nTab))
                                xAnchor.set(static_cast<cppu::OWeakObject*>(new ScCellObj( pDocSh, pAnchor->maStart)));
                            else
                                xAnchor.set(static_cast<cppu::OWeakObject*>(new ScTableSheetObj( pDocSh, nTab )));
                            aAny <<= xAnchor;
                        }
                    }
                }
            }
        }
    }
    else if ( aPropertyName == SC_UNONAME_IMAGEMAP )
    {
        uno::Reference< uno::XInterface > xImageMap;
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo(GetSdrObject());
            if( pIMapInfo )
            {
                const ImageMap& rIMap = pIMapInfo->GetImageMap();
                xImageMap.set(SvUnoImageMap_createInstance( rIMap, GetSupportedMacroItems() ));
            }
            else
                xImageMap = SvUnoImageMap_createInstance();
        }
        aAny <<= uno::Reference< container::XIndexContainer >::query( xImageMap );
    }
    else if ( aPropertyName == SC_UNONAME_HORIPOS )
    {
        SdrObject *pObj = GetSdrObject();
        if (pObj)
        {
            ScDrawLayer* pModel = static_cast<ScDrawLayer*>(pObj->GetModel());
            SdrPage* pPage = pObj->GetPage();
            if ( pModel && pPage )
            {
                ScDocument* pDoc = pModel->GetDocument();
                if ( pDoc )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        uno::Reference<drawing::XShape> xShape( mxShapeAgg, uno::UNO_QUERY );
                        if (xShape.is())
                        {
                            if (ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL)
                            {
                                awt::Size aUnoSize;
                                awt::Point aCaptionPoint;
                                ScRange aRange;
                                awt::Point aUnoPoint(lcl_GetRelativePos( xShape, pDoc, nTab, aRange, aUnoSize, aCaptionPoint ));
                                if (pDoc->IsNegativePage(nTab))
                                    aUnoPoint.X *= -1;
                                aAny <<= aUnoPoint.X;
                            }
                            else
                            {
                                awt::Point aCaptionPoint;
                                awt::Point aUnoPoint(xShape->getPosition());
                                awt::Size aUnoSize(xShape->getSize());
                                if (pDoc->IsNegativePage(nTab))
                                {
                                    aUnoPoint.X *= -1;
                                    aUnoPoint.X -= aUnoSize.Width;
                                }
                                if (lcl_GetCaptionPoint(xShape, aCaptionPoint))
                                {
                                    if (pDoc->IsNegativePage(nTab))
                                    {
                                        if (aCaptionPoint.X > 0 && aCaptionPoint.X > aUnoSize.Width)
                                            aUnoPoint.X -= aCaptionPoint.X - aUnoSize.Width;
                                    }
                                    else
                                    {
                                        if (aCaptionPoint.X < 0)
                                            aUnoPoint.X += aCaptionPoint.X;
                                    }
                                }
                                aAny <<= aUnoPoint.X;
                            }
                        }
                    }
                }
            }
        }
    }
    else if ( aPropertyName ==  SC_UNONAME_VERTPOS )
    {
        SdrObject *pObj = GetSdrObject();
        if (pObj)
        {
            ScDrawLayer* pModel = static_cast<ScDrawLayer*>(pObj->GetModel());
            SdrPage* pPage = pObj->GetPage();
            if ( pModel && pPage )
            {
                ScDocument* pDoc = pModel->GetDocument();
                if ( pDoc )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        uno::Reference<drawing::XShape> xShape( mxShapeAgg, uno::UNO_QUERY );
                        if (xShape.is())
                        {
                            if (ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL)
                            {
                                awt::Size aUnoSize;
                                awt::Point aCaptionPoint;
                                ScRange aRange;
                                awt::Point aUnoPoint(lcl_GetRelativePos( xShape, pDoc, nTab, aRange, aUnoSize, aCaptionPoint ));

                                aAny <<= aUnoPoint.Y;
                            }
                            else
                            {
                                awt::Point aUnoPoint(xShape->getPosition());
                                awt::Point aCaptionPoint;
                                if (lcl_GetCaptionPoint(xShape, aCaptionPoint))
                                {
                                    if (aCaptionPoint.Y < 0)
                                        aUnoPoint.Y += aCaptionPoint.Y;
                                }
                                aAny <<= aUnoPoint.Y;
                            }
                        }
                    }
                }
            }
        }
    }
    else if ( aPropertyName == SC_UNONAME_HYPERLINK ||
              aPropertyName == SC_UNONAME_URL )
    {
        OUString sHlink;
        if ( ScMacroInfo* pInfo = ScShapeObj_getShapeHyperMacroInfo(this) )
            sHlink = pInfo->GetHlink();
        aAny <<= sHlink;
    }
    else if ( aPropertyName == SC_UNONAME_MOVEPROTECT )
    {
        bool aProt = false;
        if ( SdrObject* pObj = GetSdrObject() )
            aProt = pObj->IsMoveProtect();
        aAny <<= aProt;
    }
    else
    {
        if(!pShapePropertySet) //performance consideration
            GetShapePropertySet();
        if (pShapePropertySet)
            aAny = pShapePropertySet->getPropertyValue( aPropertyName );
    }

    return aAny;
}

void SAL_CALL ScShapeObj::addPropertyChangeListener( const OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
{
    SolarMutexGuard aGuard;

    GetShapePropertySet();
    if (pShapePropertySet)
        pShapePropertySet->addPropertyChangeListener( aPropertyName, aListener );

    if ( !bInitializedNotifier )
    {
        // here's the latest chance to initialize the property notification at the SdrObject
        // (in the ctor, where we also attempt to do this, we do not necessarily have
        // and SdrObject, yet)
        SdrObject* pObj = GetSdrObject();
        OSL_ENSURE( pObj, "ScShapeObj::addPropertyChangeListener: no SdrObject -> no property change notification!" );
        if ( pObj )
            lcl_initializeNotifier( *pObj, *this );
        bInitializedNotifier = true;
    }
}

void SAL_CALL ScShapeObj::removePropertyChangeListener( const OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
{
    SolarMutexGuard aGuard;

    GetShapePropertySet();
    if (pShapePropertySet)
        pShapePropertySet->removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::addVetoableChangeListener( const OUString& aPropertyName,
                            const uno::Reference<beans::XVetoableChangeListener>& aListener)
{
    SolarMutexGuard aGuard;

    GetShapePropertySet();
    if (pShapePropertySet)
        pShapePropertySet->addVetoableChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::removeVetoableChangeListener( const OUString& aPropertyName,
                            const uno::Reference<beans::XVetoableChangeListener>& aListener)
{
    SolarMutexGuard aGuard;

    GetShapePropertySet();
    if (pShapePropertySet)
        pShapePropertySet->removeVetoableChangeListener( aPropertyName, aListener );
}

//  XPropertyState

beans::PropertyState SAL_CALL ScShapeObj::getPropertyState( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    beans::PropertyState eRet = beans::PropertyState_DIRECT_VALUE;
    if ( aPropertyName == SC_UNONAME_IMAGEMAP )
    {
        // ImageMap is always "direct"
    }
    else if ( aPropertyName == SC_UNONAME_ANCHOR )
    {
        // Anchor is always "direct"
    }
    else if ( aPropertyName == SC_UNONAME_HORIPOS )
    {
        // HoriPos is always "direct"
    }
    else if ( aPropertyName == SC_UNONAME_VERTPOS )
    {
        // VertPos is always "direct"
    }
    else
    {
        GetShapePropertyState();
        if (pShapePropertyState)
            eRet = pShapePropertyState->getPropertyState( aPropertyName );
    }

    return eRet;
}

uno::Sequence<beans::PropertyState> SAL_CALL ScShapeObj::getPropertyStates(
                                const uno::Sequence<OUString>& aPropertyNames )
{
    SolarMutexGuard aGuard;

    //  simple loop to get own and aggregated states

    const OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScShapeObj::setPropertyToDefault( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    if ( aPropertyName == SC_UNONAME_IMAGEMAP )
    {
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            ScIMapInfo* pIMapInfo = ScDrawLayer::GetIMapInfo(pObj);
            if( pIMapInfo )
            {
                ImageMap aEmpty;
                pIMapInfo->SetImageMap( aEmpty );   // replace with empty image map
            }
            else
            {
                // nothing to do (no need to insert user data for an empty map)
            }
        }
    }
    else
    {
        GetShapePropertyState();
        if (pShapePropertyState)
            pShapePropertyState->setPropertyToDefault( aPropertyName );
    }
}

uno::Any SAL_CALL ScShapeObj::getPropertyDefault( const OUString& aPropertyName )
{
    SolarMutexGuard aGuard;

    uno::Any aAny;
    if ( aPropertyName == SC_UNONAME_IMAGEMAP )
    {
        //  default: empty ImageMap
        uno::Reference< uno::XInterface > xImageMap(SvUnoImageMap_createInstance());
        aAny <<= uno::Reference< container::XIndexContainer >::query( xImageMap );
    }
    else
    {
        GetShapePropertyState();
        if (pShapePropertyState)
            aAny = pShapePropertyState->getPropertyDefault( aPropertyName );
    }

    return aAny;
}

// XTextContent

void SAL_CALL ScShapeObj::attach( const uno::Reference<text::XTextRange>& /* xTextRange */ )
{
    SolarMutexGuard aGuard;

    throw lang::IllegalArgumentException();     // anchor cannot be changed
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getAnchor()
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xRet;

    SdrObject* pObj = GetSdrObject();
    if( pObj )
    {
        ScDrawLayer* pModel = static_cast<ScDrawLayer*>(pObj->GetModel());
        SdrPage* pPage = pObj->GetPage();
        if ( pModel )
        {
            ScDocument* pDoc = pModel->GetDocument();
            if ( pDoc )
            {
                SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                if ( pObjSh && dynamic_cast<const ScDocShell*>( pObjSh) !=  nullptr )
                {
                    ScDocShell* pDocSh = static_cast<ScDocShell*>(pObjSh);

                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        Point aPos(pObj->GetCurrentBoundRect().TopLeft());
                        ScRange aRange(pDoc->GetRange( nTab, tools::Rectangle( aPos, aPos ) ));

                        //  anchor is always the cell

                        xRet.set(new ScCellObj( pDocSh, aRange.aStart ));
                    }
                }
            }
        }
    }

    return xRet;
}

// XComponent

void SAL_CALL ScShapeObj::dispose()
{
    SolarMutexGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp(lcl_GetComponent(mxShapeAgg));
    if ( xAggComp.is() )
        xAggComp->dispose();
}

void SAL_CALL ScShapeObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
{
    SolarMutexGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp(lcl_GetComponent(mxShapeAgg));
    if ( xAggComp.is() )
        xAggComp->addEventListener(xListener);
}

void SAL_CALL ScShapeObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
{
    SolarMutexGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp(lcl_GetComponent(mxShapeAgg));
    if ( xAggComp.is() )
        xAggComp->removeEventListener(xListener);
}

// XText
// (special handling for ScCellFieldObj)

static void lcl_CopyOneProperty( beans::XPropertySet& rDest, beans::XPropertySet& rSource, const sal_Char* pName )
{
    OUString aNameStr(OUString::createFromAscii(pName));
    try
    {
        rDest.setPropertyValue( aNameStr, rSource.getPropertyValue( aNameStr ) );
    }
    catch (uno::Exception&)
    {
        OSL_FAIL("Exception in text field");
    }
}

void SAL_CALL ScShapeObj::insertTextContent( const uno::Reference<text::XTextRange>& xRange,
                                                const uno::Reference<text::XTextContent>& xContent,
                                                sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextContent> xEffContent;

    ScEditFieldObj* pCellField = ScEditFieldObj::getImplementation( xContent );
    if ( pCellField )
    {
        //  createInstance("TextField.URL") from the document creates a ScCellFieldObj.
        //  To insert it into drawing text, a SvxUnoTextField is needed instead.
        //  The ScCellFieldObj object is left in non-inserted state.

        SvxUnoTextField* pDrawField = new SvxUnoTextField( text::textfield::Type::URL );
        xEffContent.set(pDrawField);
        lcl_CopyOneProperty( *pDrawField, *pCellField, SC_UNONAME_URL );
        lcl_CopyOneProperty( *pDrawField, *pCellField, SC_UNONAME_REPR );
        lcl_CopyOneProperty( *pDrawField, *pCellField, SC_UNONAME_TARGET );
    }
    else
        xEffContent.set(xContent);

    uno::Reference<text::XText> xAggText(lcl_GetText(mxShapeAgg));
    if ( xAggText.is() )
        xAggText->insertTextContent( xRange, xEffContent, bAbsorb );
}

void SAL_CALL ScShapeObj::removeTextContent( const uno::Reference<text::XTextContent>& xContent )
{
    SolarMutexGuard aGuard;

    //  ScCellFieldObj can't be used here.

    uno::Reference<text::XText> xAggText(lcl_GetText(mxShapeAgg));
    if ( xAggText.is() )
        xAggText->removeTextContent( xContent );
}

// XSimpleText (parent of XText)
// Use own SvxUnoTextCursor subclass - everything is just passed to aggregated object

uno::Reference<text::XTextCursor> SAL_CALL ScShapeObj::createTextCursor()
{
    SolarMutexGuard aGuard;

    if ( mxShapeAgg.is() )
    {
        //  ScDrawTextCursor must be used to ensure the ScShapeObj is returned by getText

        SvxUnoTextBase* pText = SvxUnoTextBase::getImplementation( mxShapeAgg );
        if (pText)
            return new ScDrawTextCursor( this, *pText );
    }

    return uno::Reference<text::XTextCursor>();
}

uno::Reference<text::XTextCursor> SAL_CALL ScShapeObj::createTextCursorByRange(
                                    const uno::Reference<text::XTextRange>& aTextPosition )
{
    SolarMutexGuard aGuard;

    if ( mxShapeAgg.is() && aTextPosition.is() )
    {
        //  ScDrawTextCursor must be used to ensure the ScShapeObj is returned by getText

        SvxUnoTextBase* pText = SvxUnoTextBase::getImplementation( mxShapeAgg );
        SvxUnoTextRangeBase* pRange = SvxUnoTextRangeBase::getImplementation( aTextPosition );
        if ( pText && pRange )
        {
            SvxUnoTextCursor* pCursor = new ScDrawTextCursor( this, *pText );
            uno::Reference<text::XTextCursor> xCursor( pCursor );
            pCursor->SetSelection( pRange->GetSelection() );
            return xCursor;
        }
    }

    return uno::Reference<text::XTextCursor>();
}

void SAL_CALL ScShapeObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                        const OUString& aString, sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XSimpleText> xAggSimpleText(lcl_GetSimpleText(mxShapeAgg));
    if ( xAggSimpleText.is() )
        xAggSimpleText->insertString( xRange, aString, bAbsorb );
    else
        throw uno::RuntimeException();
}

void SAL_CALL ScShapeObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                                sal_Int16 nControlCharacter, sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XSimpleText> xAggSimpleText(lcl_GetSimpleText(mxShapeAgg));
    if ( xAggSimpleText.is() )
        xAggSimpleText->insertControlCharacter( xRange, nControlCharacter, bAbsorb );
    else
        throw uno::RuntimeException();
}

// XTextRange
// (parent of XSimpleText)

uno::Reference<text::XText> SAL_CALL ScShapeObj::getText()
{
    return this;
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getStart()
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( xAggTextRange.is() )
        return xAggTextRange->getStart();
    else
        throw uno::RuntimeException();

//    return uno::Reference<text::XTextRange>();
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getEnd()
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( xAggTextRange.is() )
        return xAggTextRange->getEnd();
    else
        throw uno::RuntimeException();

//    return uno::Reference<text::XTextRange>();
}

OUString SAL_CALL ScShapeObj::getString()
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( xAggTextRange.is() )
        return xAggTextRange->getString();
    else
        throw uno::RuntimeException();

//    return OUString();
}

void SAL_CALL ScShapeObj::setString( const OUString& aText )
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( xAggTextRange.is() )
        xAggTextRange->setString( aText );
    else
        throw uno::RuntimeException();
}

// XChild

uno::Reference< uno::XInterface > SAL_CALL ScShapeObj::getParent()
{
    SolarMutexGuard aGuard;

    // receive cell position from caption object (parent of a note caption is the note cell)
    SdrObject* pObj = GetSdrObject();
    if( pObj )
    {
        ScDrawLayer* pModel = static_cast<ScDrawLayer*>(pObj->GetModel());
        SdrPage* pPage = pObj->GetPage();
        if ( pModel )
        {
            ScDocument* pDoc = pModel->GetDocument();
            if ( pDoc )
            {
                SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                if ( pObjSh && dynamic_cast<const ScDocShell*>( pObjSh) !=  nullptr )
                {
                    ScDocShell* pDocSh = static_cast<ScDocShell*>(pObjSh);

                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        const ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObj, nTab );
                        if( pCaptData )
                            return static_cast< ::cppu::OWeakObject* >( new ScCellObj( pDocSh, pCaptData->maStart ) );
                    }
                }
            }
        }
    }

    return nullptr;
}

void SAL_CALL ScShapeObj::setParent( const uno::Reference< uno::XInterface >& )
{
    throw lang::NoSupportException();
}

// XTypeProvider

uno::Sequence<uno::Type> SAL_CALL ScShapeObj::getTypes()
{
    uno::Sequence< uno::Type > aBaseTypes( ScShapeObj_Base::getTypes() );

    uno::Sequence< uno::Type > aTextTypes;
    if ( bIsTextShape )
        aTextTypes = ScShapeObj_TextBase::getTypes();

    uno::Reference<lang::XTypeProvider> xBaseProvider;
    if ( mxShapeAgg.is() )
        mxShapeAgg->queryAggregation( cppu::UnoType<lang::XTypeProvider>::get()) >>= xBaseProvider;
    OSL_ENSURE( xBaseProvider.is(), "ScShapeObj: No XTypeProvider from aggregated shape!" );

    uno::Sequence< uno::Type > aAggTypes;
    if( xBaseProvider.is() )
        aAggTypes = xBaseProvider->getTypes();

    return ::comphelper::concatSequences( aBaseTypes, aTextTypes, aAggTypes );
}

uno::Sequence<sal_Int8> SAL_CALL ScShapeObj::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

SdrObject* ScShapeObj::GetSdrObject() const throw()
{
    if(mxShapeAgg.is())
    {
        SvxShape* pShape = SvxShape::getImplementation( mxShapeAgg );
        if(pShape)
            return pShape->GetSdrObject();
    }

    return nullptr;
}

#define SC_EVENTACC_ONCLICK     "OnClick"
#define SC_EVENTACC_SCRIPT      "Script"
#define SC_EVENTACC_EVENTTYPE   "EventType"

class ShapeUnoEventAccessImpl : public ::cppu::WeakImplHelper< container::XNameReplace >
{
private:
    ScShapeObj* mpShape;

    ScMacroInfo* getInfo( bool bCreate )
    {
        return ScShapeObj_getShapeHyperMacroInfo( mpShape, bCreate );
    }

public:
    explicit ShapeUnoEventAccessImpl( ScShapeObj* pShape ): mpShape( pShape )
    {
    }

    // XNameReplace
    virtual void SAL_CALL replaceByName( const OUString& aName, const uno::Any& aElement ) override
    {
        if ( !hasByName( aName ) )
            throw container::NoSuchElementException();
        uno::Sequence< beans::PropertyValue > aProperties;
        aElement >>= aProperties;
        const beans::PropertyValue* pProperties = aProperties.getConstArray();
        const sal_Int32 nCount = aProperties.getLength();
        sal_Int32 nIndex;
        bool isEventType = false;
        for( nIndex = 0; nIndex < nCount; nIndex++, pProperties++ )
        {
            if ( pProperties->Name == SC_EVENTACC_EVENTTYPE )
            {
                isEventType = true;
                continue;
            }
            if ( isEventType && (pProperties->Name == SC_EVENTACC_SCRIPT) )
            {
                OUString sValue;
                if ( pProperties->Value >>= sValue )
                {
                    ScMacroInfo* pInfo = getInfo( true );
                    OSL_ENSURE( pInfo, "shape macro info could not be created!" );
                    if ( !pInfo )
                        break;
                    if ( pProperties->Name == SC_EVENTACC_SCRIPT )
                        pInfo->SetMacro( sValue );
                    else
                        pInfo->SetHlink( sValue );
                }
            }
        }
    }

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        uno::Sequence< beans::PropertyValue > aProperties;
        ScMacroInfo* pInfo = getInfo(false);

        if ( aName == SC_EVENTACC_ONCLICK )
        {
            if ( pInfo && !pInfo->GetMacro().isEmpty() )
            {
                aProperties.realloc( 2 );
                aProperties[ 0 ].Name = SC_EVENTACC_EVENTTYPE;
                aProperties[ 0 ].Value <<= OUString(SC_EVENTACC_SCRIPT);
                aProperties[ 1 ].Name = SC_EVENTACC_SCRIPT;
                aProperties[ 1 ].Value <<= pInfo->GetMacro();
            }
        }
        else
        {
            throw container::NoSuchElementException();
        }

        return uno::Any( aProperties );
    }

    virtual uno::Sequence< OUString > SAL_CALL getElementNames() override
    {
        uno::Sequence<OUString> aSeq { SC_EVENTACC_ONCLICK };
        return aSeq;
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        return aName == SC_EVENTACC_ONCLICK;
    }

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() override
    {
        return cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get();
    }

    virtual sal_Bool SAL_CALL hasElements() override
    {
        // elements are always present (but contained property sequences may be empty)
        return true;
    }
};

::uno::Reference< container::XNameReplace > SAL_CALL
ScShapeObj::getEvents(  )
{
    return new ShapeUnoEventAccessImpl( this );
}

OUString SAL_CALL ScShapeObj::getImplementationName(  )
{
    return OUString( "com.sun.star.comp.sc.ScShapeObj" );
}

sal_Bool SAL_CALL ScShapeObj::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

uno::Sequence< OUString > SAL_CALL ScShapeObj::getSupportedServiceNames(  )
{
    uno::Reference<lang::XServiceInfo> xSI;
    if ( mxShapeAgg.is() )
        mxShapeAgg->queryAggregation( cppu::UnoType<lang::XServiceInfo>::get() ) >>= xSI;

    uno::Sequence< OUString > aSupported;
    if ( xSI.is() )
        aSupported = xSI->getSupportedServiceNames();

    aSupported.realloc( aSupported.getLength() + 1 );
    aSupported[ aSupported.getLength() - 1 ] = "com.sun.star.sheet.Shape";

    if( bIsNoteCaption )
    {
        aSupported.realloc( aSupported.getLength() + 1 );
        aSupported[ aSupported.getLength() - 1 ] = "com.sun.star.sheet.CellAnnotationShape";
    }

    return aSupported;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
