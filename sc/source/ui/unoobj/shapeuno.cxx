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

#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <svtools/unoevent.hxx>
#include <svtools/unoimap.hxx>
#include <svx/svdobj.hxx>
#include <svx/ImageMapInfo.hxx>
#include <vcl/dropcache.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp.hxx>
#include <sfx2/event.hxx>
#include <editeng/unofield.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>

#include <shapeuno.hxx>
#include <cellsuno.hxx>
#include <textuno.hxx>
#include <fielduno.hxx>
#include <docsh.hxx>
#include <drwlayer.hxx>
#include <userdat.hxx>
#include <unonames.hxx>
#include <styleuno.hxx>

using namespace ::com::sun::star;

static std::span<const SfxItemPropertyMapEntry> lcl_GetShapeMap()
{
    static const SfxItemPropertyMapEntry aShapeMap_Impl[] =
    {
        { SC_UNONAME_ANCHOR, 0, cppu::UnoType<uno::XInterface>::get(), 0, 0 },
        { SC_UNONAME_RESIZE_WITH_CELL, 0, cppu::UnoType<sal_Bool>::get(), 0, 0 },
        { SC_UNONAME_HORIPOS, 0, cppu::UnoType<sal_Int32>::get(), 0, 0 },
        { SC_UNONAME_IMAGEMAP, 0, cppu::UnoType<container::XIndexContainer>::get(), 0, 0 },
        { SC_UNONAME_VERTPOS, 0, cppu::UnoType<sal_Int32>::get(), 0, 0 },
        { SC_UNONAME_MOVEPROTECT, 0, cppu::UnoType<sal_Bool>::get(), 0, 0 },
        { SC_UNONAME_HYPERLINK, 0, cppu::UnoType<OUString>::get(), 0, 0 },
        { SC_UNONAME_URL, 0, cppu::UnoType<OUString>::get(), 0, 0 },
        { SC_UNONAME_STYLE, 0, cppu::UnoType<style::XStyle>::get(), css::beans::PropertyAttribute::MAYBEVOID, 0 },
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
ScMacroInfo* ScShapeObj_getShapeHyperMacroInfo( const ScShapeObj* pShape, bool bCreate = false )
{
        if( pShape )
            if( SdrObject* pObj = pShape->GetSdrObject() )
                return ScDrawLayer::GetMacroInfo( pObj, bCreate );
        return nullptr;
}

ScShapeObj::ScShapeObj( uno::Reference<drawing::XShape>& xShape ) :
      pShapePropertySet(nullptr),
      pShapePropertyState(nullptr),
      bIsTextShape(false),
      bIsNoteCaption(false)
{
    osl_atomic_increment( &m_refCount );

    {
        mxShapeAgg.set( xShape, uno::UNO_QUERY );
        // extra block to force deletion of the temporary before setDelegator
    }

    if (mxShapeAgg.is())
    {
        xShape = nullptr;      // during setDelegator, mxShapeAgg must be the only ref

        mxShapeAgg->setDelegator( getXWeak() );

        xShape.set(uno::Reference<drawing::XShape>( mxShapeAgg, uno::UNO_QUERY ));

        bIsTextShape = ( comphelper::getFromUnoTunnel<SvxUnoTextBase>( mxShapeAgg ) != nullptr );
    }

    {
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            bIsNoteCaption = ScDrawLayer::IsNoteCaption( pObj );
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

void SAL_CALL ScShapeObj::acquire() noexcept
{
        OWeakObject::acquire();
}

void SAL_CALL ScShapeObj::release() noexcept
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

namespace {

struct PropertySetInfoCache : public CacheOwner
{
    uno::Reference<beans::XPropertySetInfo> getPropertySetInfo(const uno::Reference<beans::XPropertySetInfo>& rxPropSetInfo)
    {
        std::unique_lock l(gCacheMutex);
        // prevent memory leaks, possibly we could use an LRU map here.
        if (gCacheMap.size() > 100)
            gCacheMap.clear();
        auto it = gCacheMap.find(rxPropSetInfo);
        if (it != gCacheMap.end())
            return it->second;
        uno::Reference<beans::XPropertySetInfo> xCombined = new SfxExtItemPropertySetInfo( lcl_GetShapeMap(), rxPropSetInfo->getProperties() );
        gCacheMap.emplace(rxPropSetInfo, xCombined);
        return xCombined;
    }

private:
    virtual OUString getCacheName() const override
    {
        return "PropertySetInfoCache";
    }

    virtual bool dropCaches() override
    {
        std::unique_lock l(gCacheMutex);
        map_t(gCacheMap.get_allocator()).swap(gCacheMap);
        return true;
    }

    virtual void dumpState(rtl::OStringBuffer& rState) override
    {
        THREAD_UNSAFE_DUMP_BEGIN
        rState.append("\nPropertySetInfoCache:\t");
        rState.append(static_cast<sal_Int32>(gCacheMap.size()));
        THREAD_UNSAFE_DUMP_END
    }

    std::mutex gCacheMutex;
    typedef std::unordered_map<uno::Reference<beans::XPropertySetInfo>, uno::Reference<beans::XPropertySetInfo>> map_t;
    map_t gCacheMap;
};

}

/**
 * If there are lots of shapes, the cost of allocating the XPropertySetInfo structures adds up.
 * But we have a static set of properties, and most of the underlying types have one static
 * set per class. So we can cache the combination of them, which dramatically reduces the number
 * of these we need to allocate.
 */
static uno::Reference<beans::XPropertySetInfo> getPropertySetInfoFromCache(const uno::Reference<beans::XPropertySetInfo>& rxPropSetInfo)
{
    static PropertySetInfoCache aCache;
    return aCache.getPropertySetInfo(rxPropSetInfo);
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
            mxPropSetInfo = getPropertySetInfoFromCache(xAggInfo);
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
            xShapeProp->getPropertyValue(u"CaptionPoint"_ustr) >>= rCaptionPoint;
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
        aReturn
            = pDoc->GetRange(nTab, tools::Rectangle(vcl::unohelper::ConvertToVCLPoint(rUnoPoint),
                                                    vcl::unohelper::ConvertToVCLPoint(rUnoPoint)));
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
        aReturn
            = pDoc->GetRange(nTab, tools::Rectangle(vcl::unohelper::ConvertToVCLPoint(rUnoPoint),
                                                    vcl::unohelper::ConvertToVCLPoint(rUnoPoint)));
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
        if (!xRangeAdd.is())
            throw lang::IllegalArgumentException(u"only XCell or XSpreadsheet objects allowed"_ustr, getXWeak(), 0);

        SdrObject *pObj = GetSdrObject();
        if (pObj)
        {
            ScDrawLayer& rModel(static_cast< ScDrawLayer& >(pObj->getSdrModelFromSdrObject()));
            SdrPage* pPage(pObj->getSdrPageFromSdrObject());

            if ( pPage )
            {
                ScDocument* pDoc(rModel.GetDocument());

                if ( pDoc )
                {
                    if ( ScDocShell* pDocSh = pDoc->GetDocumentShell() )
                    {
                        SCTAB nTab = 0;
                        if ( lcl_GetPageNum( pPage, rModel, nTab ) )
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
                                    OSL_ENSURE(aAddress.StartRow == 0 && aAddress.EndRow == pDoc->MaxRow() &&
                                        aAddress.StartColumn == 0 && aAddress.EndColumn == pDoc->MaxCol(), "here should be a XSpreadsheet");
                                    ScDrawLayer::SetPageAnchored(*pObj);
                                }
                                else
                                {
                                    OSL_ENSURE(aAddress.StartRow == aAddress.EndRow &&
                                        aAddress.StartColumn == aAddress.EndColumn, "here should be a XCell");
                                    ScDrawObjData aAnchor;
                                    aAnchor.maStart = ScAddress(aAddress.StartColumn, aAddress.StartRow, aAddress.Sheet);
                                    aAnchor.maStartOffset = Point(aRelPoint.X, aRelPoint.Y);
                                    ScDrawObjData* pDrawObjData = ScDrawLayer::GetObjData(pObj);
                                    if (pDrawObjData)
                                        aAnchor.mbResizeWithCell = pDrawObjData->mbResizeWithCell;
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
    else if ( aPropertyName == SC_UNONAME_RESIZE_WITH_CELL )
    {
        SdrObject* pObj = GetSdrObject();
        if (!pObj)
            return;
        ScAnchorType aAnchorType = ScDrawLayer::GetAnchorType(*pObj);

        // Nothing to do if anchored to page
        if (aAnchorType == SCA_PAGE)
            return;

        ScDrawObjData* pDrawObjData = ScDrawLayer::GetObjData(pObj);
        if (!pDrawObjData)
            return;

        aValue >>= pDrawObjData->mbResizeWithCell;
        ScDrawLayer::SetCellAnchored(*pObj, *pDrawObjData);
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

            SvxIMapInfo* pIMapInfo = SvxIMapInfo::GetIMapInfo(pObj);
            if( pIMapInfo )
            {
                // replace existing image map
                pIMapInfo->SetImageMap( aImageMap );
            }
            else
            {
                // insert new user data with image map
                pObj->AppendUserData(std::unique_ptr<SdrObjUserData>(new SvxIMapInfo(aImageMap) ));
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
                ScDrawLayer& rModel(static_cast< ScDrawLayer& >(pObj->getSdrModelFromSdrObject()));
                SdrPage* pPage(pObj->getSdrPageFromSdrObject());

                if ( pPage )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, rModel, nTab ) )
                    {
                        ScDocument* pDoc = rModel.GetDocument();
                        if ( pDoc )
                        {
                            if ( ScDocShell* pDocSh = pDoc->GetDocumentShell() )
                            {
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
                                    else if (ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL
                                             || ScDrawLayer::GetAnchorType(*pObj)
                                                    == SCA_CELL_RESIZE)
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
                ScDrawLayer& rModel(static_cast< ScDrawLayer& >(pObj->getSdrModelFromSdrObject()));
                SdrPage* pPage(pObj->getSdrPageFromSdrObject());

                if ( pPage )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, rModel, nTab ) )
                    {
                        ScDocument* pDoc = rModel.GetDocument();
                        if ( pDoc )
                        {
                            if ( ScDocShell* pDocSh = pDoc->GetDocumentShell() )
                            {
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
                                    else if (ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL
                                             || ScDrawLayer::GetAnchorType(*pObj)
                                                    == SCA_CELL_RESIZE)
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
        OUString sHyperlink;
        SdrObject* pObj = GetSdrObject();
        if (pObj && (aValue >>= sHyperlink))
            pObj->setHyperlink(sHyperlink);
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
    else if ( aPropertyName == SC_UNONAME_STYLE )
    {
        if (SdrObject* pObj = GetSdrObject())
        {
            uno::Reference<style::XStyle> xStyle(aValue, uno::UNO_QUERY);
            auto pStyleSheetObj = dynamic_cast<ScStyleObj*>(xStyle.get());
            if (!pStyleSheetObj)
                throw lang::IllegalArgumentException();

            auto pStyleSheet = pStyleSheetObj->GetStyle_Impl();
            auto pOldStyleSheet = pObj->GetStyleSheet();

            if (pStyleSheet != pOldStyleSheet)
                pObj->SetStyleSheet(static_cast<SfxStyleSheet*>(pStyleSheet), false);
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
            ScDrawLayer& rModel(static_cast< ScDrawLayer& >(pObj->getSdrModelFromSdrObject()));
            SdrPage* pPage(pObj->getSdrPageFromSdrObject());

            if ( pPage )
            {
                ScDocument* pDoc = rModel.GetDocument();
                if ( pDoc )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, rModel, nTab ) )
                    {
                        if ( ScDocShell* pDocSh = pDoc->GetDocumentShell() )
                        {
                            uno::Reference< uno::XInterface > xAnchor;
                            if (ScDrawObjData *pAnchor = ScDrawLayer::GetObjDataTab(pObj, nTab))
                                xAnchor.set(cppu::getXWeak(new ScCellObj( pDocSh, pAnchor->maStart)));
                            else
                                xAnchor.set(cppu::getXWeak(new ScTableSheetObj( pDocSh, nTab )));
                            aAny <<= xAnchor;
                        }
                    }
                }
            }
        }
    }
    else if (aPropertyName == SC_UNONAME_RESIZE_WITH_CELL)
    {
        bool bIsResizeWithCell = false;
        SdrObject* pObj = GetSdrObject();
        if (pObj)
        {
            ScAnchorType anchorType = ScDrawLayer::GetAnchorType(*pObj);
            bIsResizeWithCell = (anchorType == SCA_CELL_RESIZE);
        }
        aAny <<= bIsResizeWithCell;
    }
    else if ( aPropertyName == SC_UNONAME_IMAGEMAP )
    {
        uno::Reference< uno::XInterface > xImageMap;
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            SvxIMapInfo* pIMapInfo = SvxIMapInfo::GetIMapInfo(GetSdrObject());
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
            ScDrawLayer& rModel(static_cast< ScDrawLayer& >(pObj->getSdrModelFromSdrObject()));
            SdrPage* pPage(pObj->getSdrPageFromSdrObject());

            if ( pPage )
            {
                ScDocument* pDoc = rModel.GetDocument();
                if ( pDoc )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, rModel, nTab ) )
                    {
                        uno::Reference<drawing::XShape> xShape( mxShapeAgg, uno::UNO_QUERY );
                        if (xShape.is())
                        {
                            if (ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL
                                || ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL_RESIZE)
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
            ScDrawLayer& rModel(static_cast< ScDrawLayer& >(pObj->getSdrModelFromSdrObject()));
            SdrPage* pPage(pObj->getSdrPageFromSdrObject());

            if ( pPage )
            {
                ScDocument* pDoc = rModel.GetDocument();
                if ( pDoc )
                {
                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, rModel, nTab ) )
                    {
                        uno::Reference<drawing::XShape> xShape( mxShapeAgg, uno::UNO_QUERY );
                        if (xShape.is())
                        {
                            if (ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL
                                || ScDrawLayer::GetAnchorType(*pObj) == SCA_CELL_RESIZE)
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
        if (SdrObject* pObj = GetSdrObject())
            sHlink = pObj->getHyperlink();
        aAny <<= sHlink;
    }
    else if ( aPropertyName == SC_UNONAME_MOVEPROTECT )
    {
        bool aProt = false;
        if ( SdrObject* pObj = GetSdrObject() )
            aProt = pObj->IsMoveProtect();
        aAny <<= aProt;
    }
    else if ( aPropertyName == SC_UNONAME_STYLE )
    {
        if (SdrObject* pObj = GetSdrObject())
        {
            if (auto pStyleSheet = pObj->GetStyleSheet())
            {
                ScDrawLayer& rModel(static_cast< ScDrawLayer& >(pObj->getSdrModelFromSdrObject()));
                ScDocument* pDoc = rModel.GetDocument();
                aAny <<= uno::Reference<style::XStyle>(new ScStyleObj(
                    pDoc ? pDoc->GetDocumentShell() : nullptr,
                    SfxStyleFamily::Frame, pStyleSheet->GetName()));
            }
        }
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

    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    std::transform(aPropertyNames.begin(), aPropertyNames.end(), aRet.getArray(),
        [this](const OUString& rName) -> beans::PropertyState { return getPropertyState(rName); });
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
            SvxIMapInfo* pIMapInfo = SvxIMapInfo::GetIMapInfo(pObj);
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
    throw lang::IllegalArgumentException();     // anchor cannot be changed
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getAnchor()
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xRet;

    SdrObject* pObj = GetSdrObject();
    if( pObj )
    {
        ScDrawLayer& rModel(static_cast< ScDrawLayer& >(pObj->getSdrModelFromSdrObject()));
        SdrPage* pPage(pObj->getSdrPageFromSdrObject());
        ScDocument* pDoc = rModel.GetDocument();

        if ( pPage && pDoc )
        {
            if ( ScDocShell* pDocSh = pDoc->GetDocumentShell() )
            {
                SCTAB nTab = 0;
                if ( lcl_GetPageNum( pPage, rModel, nTab ) )
                {
                    Point aPos(pObj->GetCurrentBoundRect().TopLeft());
                    ScRange aRange(pDoc->GetRange( nTab, tools::Rectangle( aPos, aPos ) ));

                    //  anchor is always the cell

                    xRet.set(new ScCellObj( pDocSh, aRange.aStart ));
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

static void lcl_CopyOneProperty( beans::XPropertySet& rDest, beans::XPropertySet& rSource, const OUString& aNameStr )
{
    try
    {
        rDest.setPropertyValue( aNameStr, rSource.getPropertyValue( aNameStr ) );
    }
    catch (uno::Exception&)
    {
        TOOLS_WARN_EXCEPTION( "sc", "Exception in text field");
    }
}

void SAL_CALL ScShapeObj::insertTextContent( const uno::Reference<text::XTextRange>& xRange,
                                                const uno::Reference<text::XTextContent>& xContent,
                                                sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextContent> xEffContent;

    ScEditFieldObj* pCellField = dynamic_cast<ScEditFieldObj*>( xContent.get() );
    if ( pCellField )
    {
        //  createInstance("TextField.URL") from the document creates a ScCellFieldObj.
        //  To insert it into drawing text, a SvxUnoTextField is needed instead.
        //  The ScCellFieldObj object is left in non-inserted state.

        rtl::Reference<SvxUnoTextField> pDrawField = new SvxUnoTextField( text::textfield::Type::URL );
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

        SvxUnoTextBase* pText = comphelper::getFromUnoTunnel<SvxUnoTextBase>( mxShapeAgg );
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

        SvxUnoTextBase* pText = comphelper::getFromUnoTunnel<SvxUnoTextBase>( mxShapeAgg );
        SvxUnoTextRangeBase* pRange = comphelper::getFromUnoTunnel<SvxUnoTextRangeBase>( aTextPosition );
        if ( pText && pRange )
        {
            rtl::Reference<SvxUnoTextCursor> pCursor = new ScDrawTextCursor( this, *pText );
            pCursor->SetSelection( pRange->GetSelection() );
            return pCursor;
        }
    }

    return uno::Reference<text::XTextCursor>();
}

void SAL_CALL ScShapeObj::insertString( const uno::Reference<text::XTextRange>& xRange,
                                        const OUString& aString, sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XSimpleText> xAggSimpleText(lcl_GetSimpleText(mxShapeAgg));
    if ( !xAggSimpleText.is() )
        throw uno::RuntimeException();

    xAggSimpleText->insertString( xRange, aString, bAbsorb );
}

void SAL_CALL ScShapeObj::insertControlCharacter( const uno::Reference<text::XTextRange>& xRange,
                                                sal_Int16 nControlCharacter, sal_Bool bAbsorb )
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XSimpleText> xAggSimpleText(lcl_GetSimpleText(mxShapeAgg));
    if ( !xAggSimpleText.is() )
        throw uno::RuntimeException();

    xAggSimpleText->insertControlCharacter( xRange, nControlCharacter, bAbsorb );
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
    if ( !xAggTextRange.is() )
        throw uno::RuntimeException();

    return xAggTextRange->getStart();
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getEnd()
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( !xAggTextRange.is() )
        throw uno::RuntimeException();

    return xAggTextRange->getEnd();
}

OUString SAL_CALL ScShapeObj::getString()
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( !xAggTextRange.is() )
        throw uno::RuntimeException();

    return xAggTextRange->getString();
}

void SAL_CALL ScShapeObj::setString( const OUString& aText )
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( !xAggTextRange.is() )
        throw uno::RuntimeException();

    xAggTextRange->setString( aText );
}

// XChild

uno::Reference< uno::XInterface > SAL_CALL ScShapeObj::getParent()
{
    SolarMutexGuard aGuard;

    // receive cell position from caption object (parent of a note caption is the note cell)
    SdrObject* pObj = GetSdrObject();
    if( pObj )
    {
        ScDrawLayer& rModel(static_cast< ScDrawLayer& >(pObj->getSdrModelFromSdrObject()));
        SdrPage* pPage(pObj->getSdrPageFromSdrObject());
        ScDocument* pDoc = rModel.GetDocument();

        if ( pPage && pDoc )
        {
            if ( ScDocShell* pDocSh = pDoc->GetDocumentShell() )
            {
                SCTAB nTab = 0;
                if ( lcl_GetPageNum( pPage, rModel, nTab ) )
                {
                    const ScDrawObjData* pCaptData = ScDrawLayer::GetNoteCaptionData( pObj, nTab );
                    if( pCaptData )
                        return cppu::getXWeak( new ScCellObj( pDocSh, pCaptData->maStart ) );
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

SdrObject* ScShapeObj::GetSdrObject() const noexcept
{
    if(mxShapeAgg.is())
        return SdrObject::getSdrObjectFromXShape( mxShapeAgg );
    return nullptr;
}

constexpr OUString SC_EVENTACC_ONCLICK = u"OnClick"_ustr;
constexpr OUString SC_EVENTACC_SCRIPT = u"Script"_ustr;
constexpr OUString SC_EVENTACC_EVENTTYPE = u"EventType"_ustr;

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
        bool isEventType = false;
        for (const beans::PropertyValue& rProperty : aProperties)
        {
            if ( rProperty.Name == SC_EVENTACC_EVENTTYPE )
            {
                isEventType = true;
                continue;
            }
            if ( isEventType && (rProperty.Name == SC_EVENTACC_SCRIPT) )
            {
                OUString sValue;
                if ( rProperty.Value >>= sValue )
                {
                    ScMacroInfo* pInfo = getInfo( true );
                    OSL_ENSURE( pInfo, "shape macro info could not be created!" );
                    if ( !pInfo )
                        break;
                    pInfo->SetMacro( sValue );
                }
            }
        }
    }

    // XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        uno::Sequence< beans::PropertyValue > aProperties;
        ScMacroInfo* pInfo = getInfo(false);

        if ( aName != SC_EVENTACC_ONCLICK )
        {
            throw container::NoSuchElementException();
        }

        if ( pInfo && !pInfo->GetMacro().isEmpty() )
        {
            aProperties = { comphelper::makePropertyValue(SC_EVENTACC_EVENTTYPE,
                                                          SC_EVENTACC_SCRIPT),
                            comphelper::makePropertyValue(SC_EVENTACC_SCRIPT, pInfo->GetMacro()) };
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
    return u"com.sun.star.comp.sc.ScShapeObj"_ustr;
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
    aSupported.getArray()[ aSupported.getLength() - 1 ] = "com.sun.star.sheet.Shape";

    if( bIsNoteCaption )
    {
        aSupported.realloc( aSupported.getLength() + 1 );
        aSupported.getArray()[ aSupported.getLength() - 1 ] = "com.sun.star.sheet.CellAnnotationShape";
    }

    return aSupported;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
