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


#include <comphelper/uno3.hxx>
#include <comphelper/stl_types.hxx>
#include <svtools/unoevent.hxx>
#include <svtools/unoimap.hxx>
#include <svx/svdobj.hxx>
#include <vcl/svapp.hxx>
#include <svx/unoshape.hxx>
#include <editeng/unofield.hxx>
#include <svx/shapepropertynotifier.hxx>
#include <toolkit/helper/convert.hxx>
#include <cppuhelper/implbase2.hxx>

#include <com/sun/star/drawing/XShape.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

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

//------------------------------------------------------------------------

DECLARE_STL_USTRINGACCESS_MAP( uno::Sequence< sal_Int8 > *,  ScShapeImplementationIdMap );

static ScShapeImplementationIdMap aImplementationIdMap;

static const SfxItemPropertyMapEntry* lcl_GetShapeMap()
{
    static SfxItemPropertyMapEntry aShapeMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ANCHOR), 0, &getCppuType((uno::Reference<uno::XInterface>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_HORIPOS), 0, &getCppuType((sal_Int32*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_IMAGEMAP), 0, &getCppuType((uno::Reference<container::XIndexContainer>*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_VERTPOS), 0, &getCppuType((sal_Int32*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_MOVEPROTECT), 0, &getCppuType((sal_Bool*)0), 0, 0 },
        // #i66550 HLINK_FOR_SHAPES
        {MAP_CHAR_LEN(SC_UNONAME_HYPERLINK), 0, &getCppuType((rtl::OUString*)0), 0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_URL), 0, &getCppuType((rtl::OUString*)0), 0, 0 },

        {0,0,0,0,0,0}
    };
    return aShapeMap_Impl;
}

const SvEventDescription* ScShapeObj::GetSupportedMacroItems()
{
    static const SvEventDescription aMacroDescriptionsImpl[] =
    {
        { 0, NULL }
    };
    return aMacroDescriptionsImpl;
}
// #i66550 HLINK_FOR_SHAPES
ScMacroInfo* ScShapeObj_getShapeHyperMacroInfo( ScShapeObj* pShape, sal_Bool bCreate = false )
{
        if( pShape )
            if( SdrObject* pObj = pShape->GetSdrObject() )
                return ScDrawLayer::GetMacroInfo( pObj, bCreate );
        return 0;
}

//------------------------------------------------------------------------

namespace
{
    void lcl_initializeNotifier( SdrObject& _rSdrObj, ::cppu::OWeakObject& _rShape )
    {
        ::svx::PPropertyValueProvider pProvider( new ::svx::PropertyValueProvider( _rShape, "Anchor" ) );
        _rSdrObj.getShapePropertyChangeNotifier().registerProvider( ::svx::eSpreadsheetAnchor, pProvider );
    }
}

//------------------------------------------------------------------------

ScShapeObj::ScShapeObj( uno::Reference<drawing::XShape>& xShape ) :
      pShapePropertySet(NULL),
      pShapePropertyState(NULL),
      pImplementationId(NULL),
      bIsTextShape(false),
      bInitializedNotifier(false)
{
    comphelper::increment( m_refCount );

    {
        mxShapeAgg = uno::Reference<uno::XAggregation>( xShape, uno::UNO_QUERY );
        // extra block to force deletion of the temporary before setDelegator
    }

    if (mxShapeAgg.is())
    {
        xShape = NULL;      // during setDelegator, mxShapeAgg must be the only ref

        mxShapeAgg->setDelegator( (cppu::OWeakObject*)this );

        xShape.set(uno::Reference<drawing::XShape>( mxShapeAgg, uno::UNO_QUERY ));

        bIsTextShape = ( SvxUnoTextBase::getImplementation( mxShapeAgg ) != NULL );
    }

    {
        SdrObject* pObj = GetSdrObject();
        if ( pObj )
        {
            lcl_initializeNotifier( *pObj, *this );
            bInitializedNotifier = true;
        }
    }

    comphelper::decrement( m_refCount );
}

ScShapeObj::~ScShapeObj()
{
//  if (mxShapeAgg.is())
//      mxShapeAgg->setDelegator(uno::Reference<uno::XInterface>());
}

// XInterface

uno::Any SAL_CALL ScShapeObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    uno::Any aRet = ScShapeObj_Base::queryInterface( rType );

    if ( !aRet.hasValue() && bIsTextShape )
        aRet = ScShapeObj_TextBase::queryInterface( rType );

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
            mxShapeAgg->queryAggregation( getCppuType((uno::Reference<beans::XPropertySet>*) 0) ) >>= xProp;
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
            mxShapeAgg->queryAggregation( getCppuType((uno::Reference<beans::XPropertyState>*) 0) ) >>= xState;
        pShapePropertyState = xState.get();
    }
}

static uno::Reference<lang::XComponent> lcl_GetComponent( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<lang::XComponent> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<lang::XComponent>*) 0) ) >>= xRet;
    return xRet;
}

static uno::Reference<text::XText> lcl_GetText( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<text::XText> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<text::XText>*) 0) ) >>= xRet;
    return xRet;
}

static uno::Reference<text::XSimpleText> lcl_GetSimpleText( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<text::XSimpleText> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<text::XSimpleText>*) 0) ) >>= xRet;
    return xRet;
}

static uno::Reference<text::XTextRange> lcl_GetTextRange( const uno::Reference<uno::XAggregation>& xAgg )
{
    uno::Reference<text::XTextRange> xRet;
    if ( xAgg.is() )
        xAgg->queryAggregation( getCppuType((uno::Reference<text::XTextRange>*) 0) ) >>= xRet;
    return xRet;
}

//  XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScShapeObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
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

static sal_Bool lcl_GetPageNum( SdrPage* pPage, SdrModel& rModel, SCTAB& rNum )
{
    sal_uInt16 nCount = rModel.GetPageCount();
    for (sal_uInt16 i=0; i<nCount; i++)
        if ( rModel.GetPage(i) == pPage )
        {
            rNum = static_cast<SCTAB>(i);
            return sal_True;
        }

    return false;
}

static sal_Bool lcl_GetCaptionPoint( uno::Reference< drawing::XShape >& xShape, awt::Point& rCaptionPoint )
{
    sal_Bool bReturn = false;
    rtl::OUString sType(xShape->getShapeType());
    sal_Bool bCaptionShape( sType == "com.sun.star.drawing.CaptionShape" );
    if (bCaptionShape)
    {
        uno::Reference < beans::XPropertySet > xShapeProp (xShape, uno::UNO_QUERY);
        if (xShapeProp.is())
        {
            xShapeProp->getPropertyValue( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "CaptionPoint" )) ) >>= rCaptionPoint;
            bReturn = sal_True;
        }
    }
    return bReturn;
}

static ScRange lcl_GetAnchorCell( uno::Reference< drawing::XShape >& xShape, ScDocument* pDoc, SCTAB nTab,
                          awt::Point& rUnoPoint, awt::Size& rUnoSize, awt::Point& rCaptionPoint )
{
    ScRange aReturn;
    rUnoPoint = xShape->getPosition();
    rtl::OUString sType(xShape->getShapeType());
    sal_Bool bCaptionShape(lcl_GetCaptionPoint(xShape, rCaptionPoint));
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
        aReturn = pDoc->GetRange( nTab, Rectangle( VCLPoint(rUnoPoint), VCLPoint(rUnoPoint) ));
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
        aReturn = pDoc->GetRange( nTab, Rectangle( VCLPoint(rUnoPoint), VCLPoint(rUnoPoint) ));
    }

    return aReturn;
}

static awt::Point lcl_GetRelativePos( uno::Reference< drawing::XShape >& xShape, ScDocument* pDoc, SCTAB nTab, ScRange& rRange,
                              awt::Size& rUnoSize, awt::Point& rCaptionPoint)
{
    awt::Point aUnoPoint;
    rRange = lcl_GetAnchorCell(xShape, pDoc, nTab, aUnoPoint, rUnoSize, rCaptionPoint);
    Rectangle aRect(pDoc->GetMMRect( rRange.aStart.Col(), rRange.aStart.Row(), rRange.aEnd.Col(), rRange.aEnd.Row(), rRange.aStart.Tab() ));
    Point aPoint = pDoc->IsNegativePage(nTab) ? aRect.TopRight() : aRect.TopLeft();
    aUnoPoint.X -= aPoint.X();
    aUnoPoint.Y -= aPoint.Y();
    return aUnoPoint;
}

void SAL_CALL ScShapeObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameString(aPropertyName);

    if ( aNameString.EqualsAscii( SC_UNONAME_ANCHOR ) )
    {
        uno::Reference<sheet::XCellRangeAddressable> xRangeAdd(aValue, uno::UNO_QUERY);
        if (xRangeAdd.is())
        {
            SdrObject *pObj = GetSdrObject();
            if (pObj)
            {
                ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
                SdrPage* pPage = pObj->GetPage();
                if ( pModel && pPage )
                {
                    ScDocument* pDoc = pModel->GetDocument();
                    if ( pDoc )
                    {
                        SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                        if ( pObjSh && pObjSh->ISA(ScDocShell) )
                        {
                            ScDocShell* pDocSh = (ScDocShell*)pObjSh;

                            SCTAB nTab = 0;
                            if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                            {
                                table::CellRangeAddress aAddress = xRangeAdd->getRangeAddress();
                                if (nTab == aAddress.Sheet)
                                {
                                    Rectangle aRect(pDoc->GetMMRect( static_cast<SCCOL>(aAddress.StartColumn), static_cast<SCROW>(aAddress.StartRow),
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
                                        ScDrawLayer::SetCellAnchored(*pObj, aAnchor);
                                        //Currently we've only got a start anchor, not an end-anchor, so generate that now
                                        ScDrawLayer::UpdateCellAnchorFromPositionEnd(*pObj, *pDoc, aAddress.Sheet);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
            throw lang::IllegalArgumentException(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("only XCell or XSpreadsheet objects allowed")), static_cast<cppu::OWeakObject*>(this), 0);
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
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
    else if ( aNameString.EqualsAscii( SC_UNONAME_HORIPOS ) )
    {
        sal_Int32 nPos = 0;
        if (aValue >>= nPos)
        {
            SdrObject *pObj = GetSdrObject();
            if (pObj)
            {
                ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
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
                            if ( pObjSh && pObjSh->ISA(ScDocShell) )
                            {
                                ScDocShell* pDocSh = (ScDocShell*)pObjSh;
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
                                        Rectangle aRect(pDoc->GetMMRect( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() ));
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
    else if ( aNameString.EqualsAscii( SC_UNONAME_VERTPOS ) )
    {
        sal_Int32 nPos = 0;
        if (aValue >>= nPos)
        {
            SdrObject *pObj = GetSdrObject();
            if (pObj)
            {
                ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
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
                            if ( pObjSh && pObjSh->ISA(ScDocShell) )
                            {
                                ScDocShell* pDocSh = (ScDocShell*)pObjSh;
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
                                        Rectangle aRect(pDoc->GetMMRect( aRange.aStart.Col(), aRange.aStart.Row(), aRange.aEnd.Col(), aRange.aEnd.Row(), aRange.aStart.Tab() ));
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
    else if  ( aNameString.EqualsAscii( SC_UNONAME_HYPERLINK ) ||
               aNameString.EqualsAscii( SC_UNONAME_URL) )
    {
        rtl::OUString sHlink;
        ScMacroInfo* pInfo = ScShapeObj_getShapeHyperMacroInfo(this, true);
        if ( ( aValue >>= sHlink ) && pInfo )
            pInfo->SetHlink( sHlink );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_MOVEPROTECT ) )
    {
        if( SdrObject* pObj = this->GetSdrObject() )
        {
            sal_Bool aProt = false;
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

uno::Any SAL_CALL ScShapeObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameString = aPropertyName;

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_ANCHOR ) )
    {
        SdrObject *pObj = GetSdrObject();
        if (pObj)
        {
            ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
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
                        if ( pObjSh && pObjSh->ISA(ScDocShell) )
                        {
                            ScDocShell* pDocSh = (ScDocShell*)pObjSh;
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
    else if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
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
                xImageMap = SvUnoImageMap_createInstance( GetSupportedMacroItems() );
        }
        aAny <<= uno::Reference< container::XIndexContainer >::query( xImageMap );
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_HORIPOS ) )
    {
        SdrObject *pObj = GetSdrObject();
        if (pObj)
        {
            ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
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
    else if ( aNameString.EqualsAscii( SC_UNONAME_VERTPOS ) )
    {
        SdrObject *pObj = GetSdrObject();
        if (pObj)
        {
            ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
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
                            uno::Reference< uno::XInterface > xAnchor;
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
    else if ( aNameString.EqualsAscii( SC_UNONAME_HYPERLINK ) ||
              aNameString.EqualsAscii( SC_UNONAME_URL ) )
    {
        rtl::OUString sHlink;
        if ( ScMacroInfo* pInfo = ScShapeObj_getShapeHyperMacroInfo(this) )
            sHlink = pInfo->GetHlink();
        aAny <<= sHlink;
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_MOVEPROTECT ) )
    {
        sal_Bool aProt = false;
        if ( SdrObject* pObj = this->GetSdrObject() )
            aProt = pObj->IsMoveProtect();
        aAny <<= aProt;
    }
    else
    {
        GetShapePropertySet();
        if (pShapePropertySet)
            aAny = pShapePropertySet->getPropertyValue( aPropertyName );
    }

    return aAny;
}

void SAL_CALL ScShapeObj::addPropertyChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
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

void SAL_CALL ScShapeObj::removePropertyChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XPropertyChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    GetShapePropertySet();
    if (pShapePropertySet)
        pShapePropertySet->removePropertyChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::addVetoableChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XVetoableChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    GetShapePropertySet();
    if (pShapePropertySet)
        pShapePropertySet->addVetoableChangeListener( aPropertyName, aListener );
}

void SAL_CALL ScShapeObj::removeVetoableChangeListener( const rtl::OUString& aPropertyName,
                            const uno::Reference<beans::XVetoableChangeListener>& aListener)
                            throw(beans::UnknownPropertyException,
                                lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    GetShapePropertySet();
    if (pShapePropertySet)
        pShapePropertySet->removeVetoableChangeListener( aPropertyName, aListener );
}

//  XPropertyState

beans::PropertyState SAL_CALL ScShapeObj::getPropertyState( const rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameString(aPropertyName);

    beans::PropertyState eRet = beans::PropertyState_DIRECT_VALUE;
    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
    {
        // ImageMap is always "direct"
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_ANCHOR ) )
    {
        // Anchor is always "direct"
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_HORIPOS ) )
    {
        // HoriPos is always "direct"
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_VERTPOS ) )
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
                                const uno::Sequence<rtl::OUString>& aPropertyNames )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //  simple loop to get own and aggregated states

    const rtl::OUString* pNames = aPropertyNames.getConstArray();
    uno::Sequence<beans::PropertyState> aRet(aPropertyNames.getLength());
    beans::PropertyState* pStates = aRet.getArray();
    for(sal_Int32 i = 0; i < aPropertyNames.getLength(); i++)
        pStates[i] = getPropertyState(pNames[i]);
    return aRet;
}

void SAL_CALL ScShapeObj::setPropertyToDefault( const rtl::OUString& aPropertyName )
                            throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameString(aPropertyName);

    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
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

uno::Any SAL_CALL ScShapeObj::getPropertyDefault( const rtl::OUString& aPropertyName )
                                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameString = aPropertyName;

    uno::Any aAny;
    if ( aNameString.EqualsAscii( SC_UNONAME_IMAGEMAP ) )
    {
        //  default: empty ImageMap
        uno::Reference< uno::XInterface > xImageMap(SvUnoImageMap_createInstance( GetSupportedMacroItems() ));
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
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    throw lang::IllegalArgumentException();     // anchor cannot be changed
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getAnchor() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xRet;

    SdrObject* pObj = GetSdrObject();
    if( pObj )
    {
        ScDrawLayer* pModel = (ScDrawLayer*)pObj->GetModel();
        SdrPage* pPage = pObj->GetPage();
        if ( pModel )
        {
            ScDocument* pDoc = pModel->GetDocument();
            if ( pDoc )
            {
                SfxObjectShell* pObjSh = pDoc->GetDocumentShell();
                if ( pObjSh && pObjSh->ISA(ScDocShell) )
                {
                    ScDocShell* pDocSh = (ScDocShell*)pObjSh;

                    SCTAB nTab = 0;
                    if ( lcl_GetPageNum( pPage, *pModel, nTab ) )
                    {
                        Point aPos(pObj->GetCurrentBoundRect().TopLeft());
                        ScRange aRange(pDoc->GetRange( nTab, Rectangle( aPos, aPos ) ));

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

void SAL_CALL ScShapeObj::dispose() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp(lcl_GetComponent(mxShapeAgg));
    if ( xAggComp.is() )
        xAggComp->dispose();
}

void SAL_CALL ScShapeObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<lang::XComponent> xAggComp(lcl_GetComponent(mxShapeAgg));
    if ( xAggComp.is() )
        xAggComp->addEventListener(xListener);
}

void SAL_CALL ScShapeObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
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
    rtl::OUString aNameStr(rtl::OUString::createFromAscii(pName));
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
                                    throw(lang::IllegalArgumentException, uno::RuntimeException)
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
                                throw(container::NoSuchElementException, uno::RuntimeException)
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
                                                    throw(uno::RuntimeException)
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
                                                    throw(uno::RuntimeException)
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
                                        const rtl::OUString& aString, sal_Bool bAbsorb )
                                    throw(uno::RuntimeException)
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
                                    throw(lang::IllegalArgumentException, uno::RuntimeException)
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

uno::Reference<text::XText> SAL_CALL ScShapeObj::getText() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return this;
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getStart() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( xAggTextRange.is() )
        return xAggTextRange->getStart();
    else
        throw uno::RuntimeException();

//    return uno::Reference<text::XTextRange>();
}

uno::Reference<text::XTextRange> SAL_CALL ScShapeObj::getEnd() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( xAggTextRange.is() )
        return xAggTextRange->getEnd();
    else
        throw uno::RuntimeException();

//    return uno::Reference<text::XTextRange>();
}

rtl::OUString SAL_CALL ScShapeObj::getString() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( xAggTextRange.is() )
        return xAggTextRange->getString();
    else
        throw uno::RuntimeException();

//    return rtl::OUString();
}

void SAL_CALL ScShapeObj::setString( const rtl::OUString& aText ) throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    uno::Reference<text::XTextRange> xAggTextRange(lcl_GetTextRange(mxShapeAgg));
    if ( xAggTextRange.is() )
        xAggTextRange->setString( aText );
    else
        throw uno::RuntimeException();
}

// XTypeProvider

uno::Sequence<uno::Type> SAL_CALL ScShapeObj::getTypes() throw(uno::RuntimeException)
{
    uno::Sequence< uno::Type > aBaseTypes( ScShapeObj_Base::getTypes() );

    uno::Sequence< uno::Type > aTextTypes;
    if ( bIsTextShape )
        aTextTypes = ScShapeObj_TextBase::getTypes();

    uno::Reference<lang::XTypeProvider> xBaseProvider;
    if ( mxShapeAgg.is() )
        mxShapeAgg->queryAggregation( getCppuType((uno::Reference<lang::XTypeProvider>*) 0) ) >>= xBaseProvider;
    OSL_ENSURE( xBaseProvider.is(), "ScShapeObj: No XTypeProvider from aggregated shape!" );

    uno::Sequence< uno::Type > aAggTypes;
    if( xBaseProvider.is() )
        aAggTypes = xBaseProvider->getTypes();

    return ::comphelper::concatSequences( aBaseTypes, aTextTypes, aAggTypes );
}

uno::Sequence<sal_Int8> SAL_CALL ScShapeObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    // do we need to compute the implementation id for this instance?
    if( !pImplementationId && mxShapeAgg.is())
    {
        uno::Reference< drawing::XShape > xAggShape;
        mxShapeAgg->queryAggregation( ::getCppuType((uno::Reference< drawing::XShape >*)0) ) >>= xAggShape;

        if( xAggShape.is() )
        {
            const rtl::OUString aShapeType( xAggShape->getShapeType() );
            // did we already compute an implementation id for the agregated shape type?
            ScShapeImplementationIdMap::iterator aIter( aImplementationIdMap.find(aShapeType ) );
            if( aIter == aImplementationIdMap.end() )
            {
                // we need to create a new implementation id for this
                // note: this memory is not free'd until application exits
                //       but since we have a fixed set of shapetypes and the
                //       memory will be reused this is ok.
                pImplementationId = new uno::Sequence< sal_Int8 >( 16 );
                rtl_createUuid( (sal_uInt8 *) pImplementationId->getArray(), 0, sal_True );
                aImplementationIdMap[ aShapeType ] = pImplementationId;
            }
            else
            {
                // use the already computed implementation id
                pImplementationId = (*aIter).second;
            }
        }
    }
    if( NULL == pImplementationId )
    {
        OSL_FAIL( "Could not create an implementation id for a ScXShape!" );
        return uno::Sequence< sal_Int8 > ();
    }
    else
    {
        return *pImplementationId;
    }
}

SdrObject* ScShapeObj::GetSdrObject() const throw()
{
    if(mxShapeAgg.is())
    {
        SvxShape* pShape = SvxShape::getImplementation( mxShapeAgg );
        if(pShape)
            return pShape->GetSdrObject();
    }

    return NULL;
}

#define SC_EVENTACC_ONCLICK     ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OnClick" ) )
#define SC_EVENTACC_SCRIPT      ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Script" ) )
#define SC_EVENTACC_EVENTTYPE   ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "EventType" ) )

typedef ::cppu::WeakImplHelper1< container::XNameReplace > ShapeUnoEventAcess_BASE;
class ShapeUnoEventAccessImpl : public ShapeUnoEventAcess_BASE
{
private:
    ScShapeObj* mpShape;

    ScMacroInfo* getInfo( sal_Bool bCreate = false )
    {
        return ScShapeObj_getShapeHyperMacroInfo( mpShape, bCreate );
    }

public:
    ShapeUnoEventAccessImpl( ScShapeObj* pShape ): mpShape( pShape )
    {
    }

    // XNameReplace
    virtual void SAL_CALL replaceByName( const rtl::OUString& aName, const uno::Any& aElement ) throw(lang::IllegalArgumentException, container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
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
            if ( pProperties->Name.equals( SC_EVENTACC_EVENTTYPE ) )
            {
                isEventType = true;
                continue;
            }
            if ( isEventType && (pProperties->Name == SC_EVENTACC_SCRIPT) )
            {
                rtl::OUString sValue;
                if ( pProperties->Value >>= sValue )
                {
                    ScMacroInfo* pInfo = getInfo( sal_True );
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
    virtual uno::Any SAL_CALL getByName( const rtl::OUString& aName ) throw(container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        uno::Sequence< beans::PropertyValue > aProperties;
        ScMacroInfo* pInfo = getInfo();

        if ( aName == SC_EVENTACC_ONCLICK )
        {
            if ( pInfo && !pInfo->GetMacro().isEmpty() )
            {
                aProperties.realloc( 2 );
                aProperties[ 0 ].Name = SC_EVENTACC_EVENTTYPE;
                aProperties[ 0 ].Value <<= SC_EVENTACC_SCRIPT;
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

    virtual uno::Sequence< rtl::OUString > SAL_CALL getElementNames() throw(uno::RuntimeException)
    {
        uno::Sequence< rtl::OUString > aSeq( 1 );
        aSeq[ 0 ] = SC_EVENTACC_ONCLICK;
        return aSeq;
    }

    virtual sal_Bool SAL_CALL hasByName( const rtl::OUString& aName ) throw(uno::RuntimeException)
    {
        return aName == SC_EVENTACC_ONCLICK;
    }

    // XElementAccess
    virtual uno::Type SAL_CALL getElementType() throw(uno::RuntimeException)
    {
        return *SEQTYPE(::getCppuType((const uno::Sequence< beans::PropertyValue >*)0));
    }

    virtual sal_Bool SAL_CALL hasElements() throw(uno::RuntimeException)
    {
        // elements are always present (but contained property sequences may be empty)
        return sal_True;
    }
};

::uno::Reference< container::XNameReplace > SAL_CALL
ScShapeObj::getEvents(  ) throw(uno::RuntimeException)
{
    return new ShapeUnoEventAccessImpl( this );
}

::rtl::OUString SAL_CALL ScShapeObj::getImplementationName(  ) throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.sc.ScShapeObj" ) );
}

::sal_Bool SAL_CALL ScShapeObj::supportsService( const ::rtl::OUString& _ServiceName ) throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSupported( getSupportedServiceNames() );
    for ( const ::rtl::OUString* pSupported = aSupported.getConstArray();
          pSupported != aSupported.getConstArray() + aSupported.getLength();
          ++pSupported
        )
        if ( _ServiceName == *pSupported )
            return sal_True;
    return false;
}

uno::Sequence< ::rtl::OUString > SAL_CALL ScShapeObj::getSupportedServiceNames(  ) throw (uno::RuntimeException)
{
    uno::Reference<lang::XServiceInfo> xSI;
    if ( mxShapeAgg.is() )
        mxShapeAgg->queryAggregation( lang::XServiceInfo::static_type() ) >>= xSI;

    uno::Sequence< ::rtl::OUString > aSupported;
    if ( xSI.is() )
        aSupported = xSI->getSupportedServiceNames();

    aSupported.realloc( aSupported.getLength() + 1 );
    aSupported[ aSupported.getLength() - 1 ] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.sheet.Shape" ) );
    return aSupported;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
