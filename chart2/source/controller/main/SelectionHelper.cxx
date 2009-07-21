/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SelectionHelper.cxx,v $
 * $Revision: 1.11.68.1 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "SelectionHelper.hxx"
#include "ObjectIdentifier.hxx"
//for C2U
#include "macros.hxx"
#include "DiagramHelper.hxx"
#include "ChartModelHelper.hxx"

// header for class SdrObjList
#include <svx/svdpage.hxx>
#include <svx/svditer.hxx>
#include "svx/obj3d.hxx"
// header for class SdrPathObj
#include <svx/svdopath.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
//using namespace ::com::sun::star::chart2;

namespace
{

rtl::OUString lcl_getObjectName( SdrObject* pObj )
{
    if(pObj)
       return pObj->GetName();
    return rtl::OUString();
}

void impl_selectObject( SdrObject* pObjectToSelect, DrawViewWrapper& rDrawViewWrapper )
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());

    if(pObjectToSelect)
    {
        SelectionHelper aSelectionHelper( pObjectToSelect );
        SdrObject* pMarkObj = aSelectionHelper.getObjectToMark();
        rDrawViewWrapper.setMarkHandleProvider(&aSelectionHelper);
        rDrawViewWrapper.MarkObject(pMarkObj);
        rDrawViewWrapper.setMarkHandleProvider(NULL);
    }
}

}//anonymous namespace

bool Selection::hasSelection()
{
    return m_aSelectedObjectCID.getLength() || m_xSelectAdditionalShape.is();
}

rtl::OUString Selection::getSelectedCID()
{
    return m_aSelectedObjectCID;
}

uno::Reference< drawing::XShape > Selection::getSelectedAdditionalShape()
{
    return m_xSelectAdditionalShape;
}

ObjectIdentifier Selection::getSelectedOID() const
{
    ObjectIdentifier aReturn;
    if ( m_aSelectedObjectCID.getLength() > 0 )
    {
        aReturn = ObjectIdentifier( m_aSelectedObjectCID );
    }
    else if ( m_xSelectAdditionalShape.is() )
    {
        aReturn = ObjectIdentifier( m_xSelectAdditionalShape );
    }
    return aReturn;
}

bool Selection::setSelection( const ::rtl::OUString& rCID )
{
    if( !rCID.equals( m_aSelectedObjectCID ) )
    {
        m_aSelectedObjectCID = rCID;
        m_xSelectAdditionalShape.set(0);
        return true;
    }
    return false;
}

bool Selection::setSelection( const uno::Reference< drawing::XShape >& xShape )
{
    if( !(m_xSelectAdditionalShape==xShape) )
    {
        clearSelection();
        m_xSelectAdditionalShape = xShape;
        return true;
    }
    return false;
}

void Selection::clearSelection()
{
    m_aSelectedObjectCID = m_aSelectedObjectCID_beforeMouseDown
        = m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing = rtl::OUString();
    m_xSelectAdditionalShape.set(0);
}

bool Selection::maybeSwitchSelectionAfterSingleClickWasEnsured()
{
    if( m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing.getLength()
        && !m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing.equals(m_aSelectedObjectCID) )
    {
        m_aSelectedObjectCID = m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing;
        m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing = C2U("");
        return true;
    }
    return false;
}

void Selection::resetPossibleSelectionAfterSingleClickWasEnsured()
{
    if( m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing.getLength() )
        m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing = C2U("");
}

void Selection::remindSelectionBeforeMouseDown()
{
    m_aSelectedObjectCID_beforeMouseDown = m_aSelectedObjectCID;
}

bool Selection::isSelectionDifferentFromBeforeMouseDown()
{
    return !ObjectIdentifier::areIdenticalObjects( m_aSelectedObjectCID, m_aSelectedObjectCID_beforeMouseDown );
}

void Selection::applySelection( DrawViewWrapper* pDrawViewWrapper )
{
    if( pDrawViewWrapper )
    {
        {
            ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
            pDrawViewWrapper->UnmarkAll();
        }
        SdrObject* pObjectToSelect = 0;
        if( m_aSelectedObjectCID.getLength() )
            pObjectToSelect = pDrawViewWrapper->getNamedSdrObject( m_aSelectedObjectCID );
        else if( m_xSelectAdditionalShape.is() )
            pObjectToSelect = DrawViewWrapper::getSdrObject( m_xSelectAdditionalShape );

        impl_selectObject( pObjectToSelect, *pDrawViewWrapper );
    }
}

void Selection::adaptSelectionToNewPos( const Point& rMousePos, DrawViewWrapper* pDrawViewWrapper
                                       , bool bIsRightMouse, bool bWaitingForDoubleClick )
{
    if( pDrawViewWrapper )
    {
        //do not toggel multiclick selection if right clicked on the selected object or waiting for double click
        bool bAllowMultiClickSelectionChange = !bIsRightMouse && !bWaitingForDoubleClick;

        const rtl::OUString aNameOfLastSelectedObject( m_aSelectedObjectCID );

        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());

        //bAllowMultiClickSelectionChange==true -> a second click on the same object can lead to a changed selection (e.g. series -> single data point)

        //get object to select:
        SdrObject* pNewObj = 0;
        {
            m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing = C2U("");
            m_xSelectAdditionalShape.set(0);

            //the search for the object to select starts with the hit object deepest in the grouping hierarchy (a leaf in the tree)
            //further we travel along the grouping hierarchy from child to parent
            pNewObj = pDrawViewWrapper->getHitObject(rMousePos);
            m_aSelectedObjectCID = lcl_getObjectName( pNewObj );//name of pNewObj
            rtl::OUString aTestFirstHit = m_aSelectedObjectCID;

            //ignore handle only objects for hit test
            while( pNewObj && m_aSelectedObjectCID.match(C2U("HandlesOnly")) )
            {
                pNewObj->SetMarkProtect(true);
                pNewObj = pDrawViewWrapper->getHitObject(rMousePos);
                m_aSelectedObjectCID = lcl_getObjectName( pNewObj );
            }

            //accept only named objects while searching for the object to select
            //this call may change m_aSelectedObjectCID
            if( SelectionHelper::findNamedParent( pNewObj, m_aSelectedObjectCID, true ) )
            {
                //if the so far found object is a multi click object further steps are necessary
                while( ObjectIdentifier::isMultiClickObject( m_aSelectedObjectCID ) )
                {
                    bool bSameObjectAsLastSelected = ObjectIdentifier::areIdenticalObjects( aNameOfLastSelectedObject, m_aSelectedObjectCID );
                    if( bSameObjectAsLastSelected )
                    {
                        //if the same child is clicked again don't go up further
                        break;
                    }
                    if( ObjectIdentifier::areSiblings(aNameOfLastSelectedObject,m_aSelectedObjectCID) )
                    {
                        //if a sibling of the last selected object is clicked don't go up further
                        break;
                    }
                    SdrObject*    pLastChild     = pNewObj;
                    rtl::OUString aLastChildName = m_aSelectedObjectCID;
                    if(!SelectionHelper::findNamedParent( pNewObj, m_aSelectedObjectCID, false ))
                    {
                        //take the one found so far
                        break;
                    }
                    //if the last selected object is found don't go up further
                    //but take the last child if selection change is allowed
                    if( ObjectIdentifier::areIdenticalObjects( aNameOfLastSelectedObject, m_aSelectedObjectCID ) )
                    {
                        if( bAllowMultiClickSelectionChange )
                        {
                            pNewObj  = pLastChild;
                            m_aSelectedObjectCID = aLastChildName;
                        }
                        else
                            m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing = aLastChildName;

                        break;
                    }
                }

                DBG_ASSERT(pNewObj && m_aSelectedObjectCID.getLength(),"somehow lost selected object");
            }
            else
            {
                //maybe an additional shape was hit
                m_aSelectedObjectCID = rtl::OUString();
                if( pNewObj )
                {
                    m_xSelectAdditionalShape = uno::Reference< drawing::XShape >( pNewObj->getUnoShape(), uno::UNO_QUERY);
                }
            }

            if(!m_xSelectAdditionalShape.is())
            {
                rtl::OUString aPageCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, rtl::OUString() ) );//@todo read CID from model

                if( !m_aSelectedObjectCID.getLength() )
                    m_aSelectedObjectCID = aPageCID;

                //check wether the diagram was hit but not selected (e.g. because it has no filling):
                rtl::OUString aWallCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, rtl::OUString() ) );//@todo read CID from model
                if( m_aSelectedObjectCID.equals( aPageCID ) || m_aSelectedObjectCID.equals( aWallCID ) || !m_aSelectedObjectCID.getLength() )
                {
                    rtl::OUString aDiagramCID = ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM, rtl::OUString::valueOf( sal_Int32(0) ) );
                    //todo: if more than one diagram is available in future do chack the list of all diagrams here
                    SdrObject* pDiagram = pDrawViewWrapper->getNamedSdrObject( aDiagramCID );
                    if( pDiagram )
                    {
                        if( pDrawViewWrapper->IsObjectHit( pDiagram, rMousePos ) )
                        {
                            m_aSelectedObjectCID = aDiagramCID;
                            pNewObj = pDiagram;
                        }
                    }
                }
            }
        }

        if( bIsRightMouse && m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing.getLength() )
            m_aSelectedObjectCID_selectOnlyIfNoDoubleClickIsFollowing = C2U("");
    }
}

bool Selection::isResizeableObjectSelected()
{
    // #i12587# support for shapes in chart
    if ( m_xSelectAdditionalShape.is() )
    {
        return true;
    }

    ObjectType eObjectType = ObjectIdentifier::getObjectType( m_aSelectedObjectCID );
    switch( eObjectType )
    {
        case OBJECTTYPE_DIAGRAM:
        case OBJECTTYPE_DIAGRAM_WALL:
            return true;
        default:
            return false;
    }
    return false;
}

bool Selection::isRotateableObjectSelected( const uno::Reference< frame::XModel >& xChartModel )
{
    return SelectionHelper::isRotateableObject( m_aSelectedObjectCID, xChartModel );
}

bool Selection::isDragableObjectSelected()
{
    if( m_aSelectedObjectCID.getLength() )
        return ObjectIdentifier::isDragableObject( m_aSelectedObjectCID );
    return m_xSelectAdditionalShape.is();
}

bool Selection::isNonGraphicObjectShapeSelected() const
{
    if ( m_xSelectAdditionalShape.is() )
    {
        uno::Reference< drawing::XShapeDescriptor > xShapeDescriptor( m_xSelectAdditionalShape, uno::UNO_QUERY );
        if ( xShapeDescriptor.is() )
        {
            ::rtl::OUString aShapeType = xShapeDescriptor->getShapeType();
            if ( !aShapeType.equals( C2U( "com.sun.star.drawing.GraphicObjectShape" ) ) )
            {
                return true;
            }
        }
    }
    return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//static
bool SelectionHelper::findNamedParent( SdrObject*& pInOutObject
                                      , rtl::OUString& rOutName
                                      , bool bGivenObjectMayBeResult )
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    //find the deepest named group
    SdrObject* pObj = pInOutObject;
    rtl::OUString aName;
    if( bGivenObjectMayBeResult )
        aName = lcl_getObjectName( pObj );

    while( pObj && !ObjectIdentifier::isCID( aName  )  )
    {
        SdrObjList* pObjList = pObj->GetObjList();
        if( !pObjList )
            return false;;
        SdrObject* pOwner = pObjList->GetOwnerObj();
        if( !pOwner )
            return false;
        pObj = pOwner;
        aName = lcl_getObjectName( pObj );
    }

    if(!pObj)
        return false;
    if(!aName.getLength())
        return false;

    pInOutObject = pObj;
    rOutName = aName;
    return true;
}

//static
bool SelectionHelper::isDragableObjectHitTwice( const Point& rMPos
                    , const rtl::OUString& rNameOfSelectedObject
                    , const DrawViewWrapper& rDrawViewWrapper )
{
    if(!rNameOfSelectedObject.getLength())
        return false;
    if( !ObjectIdentifier::isDragableObject(rNameOfSelectedObject) )
        return false;
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    SdrObject* pObj = rDrawViewWrapper.getNamedSdrObject( rNameOfSelectedObject );
    if( !rDrawViewWrapper.IsObjectHit( pObj, rMPos ) )
        return false;
    return true;
}

/*
rtl::OUString lcl_getObjectCIDToSelect( const Point& rMPos
                    , const rtl::OUString& rNameOfLastSelectedObject
                    , DrawViewWrapper& rDrawViewWrapper
                    , bool bAllowMultiClickSelectionChange
                    , rtl::OUString& rObjectToSelectIfNoDoubleClickIsFollowing //out parameter only
                    )
{
    rtl::OUString aRet;

    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());

    //bAllowMultiClickSelectionChange==true -> a second click on the same object can lead to a changed selection (e.g. series -> single data point)

    //get object to select:
    SdrObject* pNewObj = 0;
    {
        rObjectToSelectIfNoDoubleClickIsFollowing = C2U("");

        //the search for the object to select starts with the hit object deepest in the grouping hierarchy (a leaf in the tree)
        //further we travel along the grouping hierarchy from child to parent
        pNewObj = rDrawViewWrapper.getHitObject(rMPos);
        aRet = lcl_getObjectName( pNewObj );//name of pNewObj
        rtl::OUString aTestFirstHit = aRet;

        //ignore handle only objects for hit test
        while( pNewObj && aRet.match(C2U("HandlesOnly")) )
        {
            pNewObj->SetMarkProtect(true);
            pNewObj = rDrawViewWrapper.getHitObject(rMPos);
            aRet = lcl_getObjectName( pNewObj );
        }

        //accept only named objects while searching for the object to select
        //this call may change aRet
        if( !findNamedParent( pNewObj, aRet, true ) )
        {
            return C2U("");
        }
        //if the so far found object is a multi click object further steps are necessary
        while( ObjectIdentifier::isMultiClickObject( aRet ) )
        {
            bool bSameObjectAsLastSelected = ObjectIdentifier::areIdenticalObjects( rNameOfLastSelectedObject, aRet );
            if( bSameObjectAsLastSelected )
            {
                //if the same child is clicked again don't go up further
                break;
            }
            if( ObjectIdentifier::areSiblings(rNameOfLastSelectedObject,aRet) )
            {
                //if a sibling of the last selected object is clicked don't go up further
                break;
            }
            SdrObject*    pLastChild     = pNewObj;
            rtl::OUString aLastChildName = aRet;
            if(!findNamedParent( pNewObj, aRet, false ))
            {
                //take the one found so far
                break;
            }
            //if the last selected object is found don't go up further
            //but take the last child if selection change is allowed
            if( ObjectIdentifier::areIdenticalObjects( rNameOfLastSelectedObject, aRet ) )
            {
                if( bAllowMultiClickSelectionChange )
                {
                    pNewObj  = pLastChild;
                    aRet = aLastChildName;
                }
                else
                    rObjectToSelectIfNoDoubleClickIsFollowing = aLastChildName;

                break;
            }
        }

        //check wether the diagram was hit but not selected:
        rtl::OUString aPageCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, rtl::OUString() ) );//@todo read CID from model
        rtl::OUString aWallCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, rtl::OUString() ) );//@todo read CID from model
        if( aRet.equals( aPageCID ) || aRet.equals( aWallCID ) || !aRet.getLength() )
        {
            rtl::OUString aDiagramCID = ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM, rtl::OUString::valueOf( sal_Int32(0) ) );
            //todo: if more than one diagram is available in future do chack the list of all diagrams here
            SdrObject* pDiagram = rDrawViewWrapper.getNamedSdrObject( aDiagramCID );
            if( pDiagram )
            {
                if( rDrawViewWrapper.IsObjectHit( pDiagram, rMPos ) )
                {
                    aRet = aDiagramCID;
                    pNewObj = pDiagram;
                }
            }
        }

        DBG_ASSERT(pNewObj && aRet.getLength(),"somehow lost selected object");
    }

    return aRet;
}
*/

// static
::rtl::OUString SelectionHelper::getHitObjectCID(
    const Point& rMPos,
    DrawViewWrapper& rDrawViewWrapper,
    bool bGetDiagramInsteadOf_Wall )
{
    // //- solar mutex
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    rtl::OUString aRet;

    SdrObject* pNewObj = rDrawViewWrapper.getHitObject(rMPos);
    aRet = lcl_getObjectName( pNewObj );//name of pNewObj

    //ignore handle only objects for hit test
    while( pNewObj && aRet.match(C2U("HandlesOnly")) )
    {
        pNewObj->SetMarkProtect(true);
        pNewObj = rDrawViewWrapper.getHitObject(rMPos);
        aRet = lcl_getObjectName( pNewObj );
    }

    //accept only named objects while searching for the object to select
    if( !findNamedParent( pNewObj, aRet, true ) )
    {
        aRet = ::rtl::OUString();
    }

    rtl::OUString aPageCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_PAGE, rtl::OUString() ) );//@todo read CID from model
    //get page when nothing was hit
    if( aRet.getLength() == 0  && !pNewObj )
    {
        aRet = aPageCID;
    }

    //get diagram instead wall or page if hit inside diagram
    if( aRet.getLength() != 0  )
    {
        if( aRet.equals( aPageCID ) )
        {
            rtl::OUString aDiagramCID = ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM, rtl::OUString::valueOf( sal_Int32(0) ) );
            //todo: if more than one diagram is available in future do chack the list of all diagrams here
            SdrObject* pDiagram = rDrawViewWrapper.getNamedSdrObject( aDiagramCID );
            if( pDiagram )
            {
                if( rDrawViewWrapper.IsObjectHit( pDiagram, rMPos ) )
                {
                    aRet = aDiagramCID;
                }
            }
        }
        else if( bGetDiagramInsteadOf_Wall )
        {
            rtl::OUString aWallCID( ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM_WALL, rtl::OUString() ) );//@todo read CID from model

            if( aRet.equals( aWallCID ) )
            {
                rtl::OUString aDiagramCID = ObjectIdentifier::createClassifiedIdentifier( OBJECTTYPE_DIAGRAM, rtl::OUString::valueOf( sal_Int32(0) ) );
                aRet = aDiagramCID;
            }
        }
    }

    return aRet;
    // \\- solar mutex
}

//static
bool SelectionHelper::isRotateableObject( const ::rtl::OUString& rCID
                    , const uno::Reference< frame::XModel >& xChartModel )
{
    if( !ObjectIdentifier::isRotateableObject( rCID ) )
        return false;

    sal_Int32 nDimensionCount = DiagramHelper::getDimension( ChartModelHelper::findDiagram( xChartModel ) );

    if( nDimensionCount == 3 )
        return true;
    return false;
}

SelectionHelper::SelectionHelper( SdrObject* pSelectedObj )
                      : m_pSelectedObj( pSelectedObj )
{

}
SelectionHelper::~SelectionHelper()
{
}

//virtual
bool SelectionHelper::getFrameDragSingles()
{
    bool bFrameDragSingles = true;//true == green == surrounding handles
    if( m_pSelectedObj && m_pSelectedObj->ISA(E3dObject) )
        bFrameDragSingles = false;
    return bFrameDragSingles;
}

//static
SdrObject* SelectionHelper::getMarkHandlesObject( SdrObject* pObj )
{
    if(!pObj)
        return 0;
    rtl::OUString aName( lcl_getObjectName( pObj ) );
    if( aName.match(C2U("MarkHandles")) || aName.match(C2U("HandlesOnly")) )
        return pObj;
    if( aName.getLength() )//dont't get the markhandles of a different object
        return 0;

    //search for a child with name "MarkHandles" or "HandlesOnly"
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
    SdrObjList* pSubList = pObj->GetSubList();
    if(pSubList)
    {
        SdrObjListIter aIterator(*pSubList, IM_FLAT);
        while (aIterator.IsMore())
        {
            SdrObject* pMarkHandles = SelectionHelper::getMarkHandlesObject( aIterator.Next() );
            if( pMarkHandles )
                return pMarkHandles;
        }
    }
    return 0;
}

SdrObject* SelectionHelper::getObjectToMark()
{
    //return the selected object itself
    //or a specific other object if that exsists
    SdrObject* pObj = m_pSelectedObj;
    m_pMarkObj = pObj;

    //search for a child with name "MarkHandles" or "HandlesOnly"
    if(pObj)
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        SdrObjList* pSubList = pObj->GetSubList();
        if(pSubList)
        {
            SdrObjListIter aIterator(*pSubList, IM_FLAT);
            while (aIterator.IsMore())
            {
                SdrObject* pMarkHandles = SelectionHelper::getMarkHandlesObject( aIterator.Next() );
                if( pMarkHandles )
                {
                    m_pMarkObj = pMarkHandles;
                    break;
                }
            }
        }
    }
    return m_pMarkObj;
}

//static
E3dScene* SelectionHelper::getSceneToRotate( SdrObject* pObj )
{
    //search wether the object or one of its children is a 3D object
    //if so, return the accessory 3DScene

    E3dObject* pRotateable = 0;

    if(pObj)
    {
        pRotateable = dynamic_cast<E3dObject*>(pObj);
        if( !pRotateable )
        {
            ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
            SdrObjList* pSubList = pObj->GetSubList();
            if(pSubList)
            {
                SdrObjListIter aIterator(*pSubList, IM_DEEPWITHGROUPS);
                while( aIterator.IsMore() && !pRotateable )
                {
                    SdrObject* pSubObj = aIterator.Next();
                    pRotateable = dynamic_cast<E3dObject*>(pSubObj);
                }
            }
        }
    }

    E3dScene* pScene = 0;
    if(pRotateable)
    {
        ::vos::OGuard aSolarGuard( Application::GetSolarMutex());
        pScene = pRotateable->GetScene();
    }
    return pScene;

}

//virtual
bool SelectionHelper::getMarkHandles( SdrHdlList& rHdlList )
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex());

    //@todo -> more flexible handle creation
    //2 scenarios possible:
    //1. add an additional invisible shape as a child to the selected object
    //this child needs to be named somehow and handles need to be generated therefrom ...
    //or 2. offer a central service per view where renderer and so can register for handle creation for a special shape
    //.. or 3. feature from drawinglayer to create handles for each shape ... (bad performance ... ?) ?

    //scenario 1 is now used:
    //if a child with name MarkHandles exsists
    //this child is marked instead of the logical selected object

/*
    //if a special mark object was found
    //that object should be used for marking only
    if( m_pMarkObj != m_pSelectedObj)
        return false;
*/
    //if a special mark object was found
    //that object should be used to create handles from
    if( m_pMarkObj && m_pMarkObj != m_pSelectedObj)
    {
        rHdlList.Clear();
        if( m_pMarkObj->ISA(SdrPathObj) )
        {
            //if th object is a polygon
            //from each point a handle is generated
            const ::basegfx::B2DPolyPolygon& rPolyPolygon = ((SdrPathObj*)m_pMarkObj)->GetPathPoly();
            for( sal_uInt32 nN = 0L; nN < rPolyPolygon.count(); nN++)
            {
                const ::basegfx::B2DPolygon aPolygon(rPolyPolygon.getB2DPolygon(nN));
                for( sal_uInt32 nM = 0L; nM < aPolygon.count(); nM++)
                {
                    const ::basegfx::B2DPoint aPoint(aPolygon.getB2DPoint(nM));
                    SdrHdl* pHdl = new SdrHdl(Point(basegfx::fround(aPoint.getX()), basegfx::fround(aPoint.getY())), HDL_POLY);
                    rHdlList.AddHdl(pHdl);
                }
            }
            return true;
        }
        else
            return false; //use the special MarkObject for marking
    }

    //@todo:
    //add and document good marking defaults ...

    rHdlList.Clear();

    SdrObject* pObj = m_pSelectedObj;
    if(!pObj)
        return false;
    SdrObjList* pSubList = pObj->GetSubList();
    if( !pSubList )//no group object !pObj->IsGroupObject()
        return false;

    rtl::OUString aName( lcl_getObjectName( pObj ) );
    ObjectType eObjectType( ObjectIdentifier::getObjectType( aName ) );
    if( OBJECTTYPE_DATA_POINT == eObjectType
        || OBJECTTYPE_DATA_LABEL == eObjectType
        || OBJECTTYPE_LEGEND_ENTRY == eObjectType
        || OBJECTTYPE_AXIS_UNITLABEL == eObjectType )
    {
        return false;
    }

    SdrObjListIter aIterator(*pSubList, IM_FLAT);

    while (aIterator.IsMore())
    {
        SdrObject* pSubObj = aIterator.Next();
        if( OBJECTTYPE_DATA_SERIES == eObjectType )
        {
            rtl::OUString aSubName( lcl_getObjectName( pSubObj ) );
            ObjectType eSubObjectType( ObjectIdentifier::getObjectType( aSubName ) );
            if( eSubObjectType!=OBJECTTYPE_DATA_POINT  )
                return false;
        }

        Point aPos = pSubObj->GetCurrentBoundRect().Center();
        SdrHdl* pHdl = new SdrHdl(aPos,HDL_POLY);
        rHdlList.AddHdl(pHdl);
    }
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................
