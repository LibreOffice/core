/*************************************************************************
 *
 *  $RCSfile: SelectionHelper.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:28 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "SelectionHelper.hxx"
#include "chartview/ObjectIdentifier.hxx"
//for C2U
#include "macros.hxx"

// header for class SdrObjList
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif

#ifndef _SVDITER_HXX
#include <svx/svditer.hxx>
#endif
#ifndef _E3D_OBJ3D_HXX
#include "svx/obj3d.hxx"
#endif
// header for class SdrPathObj
#ifndef _SVDOPATH_HXX
#include <svx/svdopath.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................
using namespace ::com::sun::star;
//using namespace ::drafts::com::sun::star::chart2;

//static
rtl::OUString SelectionHelper::getObjectName( SdrObject* pObj )
{
    rtl::OUString aName;
    if(pObj)
    {
        try
        {
            uno::Reference< uno::XInterface > xShape = pObj->getUnoShape();
            uno::Reference< beans::XPropertySet > xShapeProps(
                uno::Reference< beans::XPropertySet >::query( xShape ) );
            uno::Any aAName = xShapeProps->getPropertyValue( C2U( "Name" ) ); //UNO_NAME_MISC_OBJ_NAME
            aAName >>= aName;
        }
        catch( uno::Exception &e)
        {
            e;
        }
    }
    return aName;
}

//static
bool SelectionHelper::findNamedParent( SdrObject*& pInOutObject
                                      , rtl::OUString& rOutName
                                      , bool bGivenObjectMayBeResult )
{
    //find the deepest named group
    SdrObject* pObj = pInOutObject;
    rtl::OUString aName;
    if( bGivenObjectMayBeResult )
        aName = getObjectName( pObj );

    while( pObj && !ObjectIdentifier::isCID( aName  )  )
    {
        SdrObjList* pObjList = pObj->GetObjList();
        if( !pObjList )
            return false;;
        SdrObject* pOwner = pObjList->GetOwnerObj();
        if( !pOwner )
            return false;
        pObj = pOwner;
        aName = getObjectName( pObj );
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
SdrObject* SelectionHelper::getObjectToSelect( const Point& rMPos
                    , rtl::OUString& rNameOfLastSelectedObject
                    , const DrawViewWrapper& rDrawViewWrapper )
{
    //the search for the object to select starts with the hit object deepest in the grouping hierarchy (a leaf in the tree)
    //further we travel along the grouping hierarchy from child to parent
    SdrObject*    pNewObj = rDrawViewWrapper.getHitObject(rMPos);
    rtl::OUString aNewName = getObjectName( pNewObj );//name of pNewObj
    rtl::OUString aTestFirstHit = aNewName;

    //ignore handle only objects for hit test
    while( pNewObj && aNewName.match(C2U("HandlesOnly")) )
    {
        pNewObj->SetMarkProtect(true);
        pNewObj = rDrawViewWrapper.getHitObject(rMPos);
        aNewName = getObjectName( pNewObj );
    }

    //accept only named objects while searching for the object to select
    if( !findNamedParent( pNewObj, aNewName, true ) )
    {
        rNameOfLastSelectedObject = C2U("");
        return NULL;
    }
    //if the so far found object is a multi click object further steps are necessary
    while( ObjectIdentifier::isMultiClickObject( aNewName ) )
    {
        if( rNameOfLastSelectedObject.getLength() && aNewName.match( rNameOfLastSelectedObject ) )
        {
            //if the same child is clicked again don't go up further
            break;
        }
        if( ObjectIdentifier::areSiblings(rNameOfLastSelectedObject,aNewName) )
        {
            //if a sibling of the last selected object is clicked don't go up further
            break;
        }
        SdrObject*    pLastChild     = pNewObj;
        rtl::OUString aLastChildName = aNewName;
        if(!findNamedParent( pNewObj, aNewName, false ))
        {
            //take the one found so far
            break;
        }
        //if the last selected object is found don't go up further but take the last child
        if( rNameOfLastSelectedObject.getLength() && aNewName.match( rNameOfLastSelectedObject ) )
        {
            pNewObj  = pLastChild;
            aNewName = aLastChildName;
            break;
        }
    }
    DBG_ASSERT(pNewObj && aNewName.getLength(),"somehow lost selected object");
    rNameOfLastSelectedObject = aNewName;
    return pNewObj;
}


SelectionHelper::SelectionHelper( SdrObject* pSelectedObj
                      , const rtl::OUString& rSelectedObjectCID )
                      : m_pSelectedObj( pSelectedObj )
                      , m_aSelectedObjectCID( rSelectedObjectCID )
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

SdrObject* SelectionHelper::getObjectToMark()
{
    //return the selected object itself
    //or a sub object if one with name "Handles" exsists

    SdrObject* pObj = m_pSelectedObj;

    m_pMarkObj = pObj;
    if(pObj)
    {
        SdrObjList* pSubList = pObj->GetSubList();
        if(pSubList)
        {
            SdrObjListIter aIterator(*pSubList, IM_DEEPWITHGROUPS);
            while (aIterator.IsMore())
            {
                SdrObject* pSubObj = aIterator.Next();
                rtl::OUString aName = SelectionHelper::getObjectName( pSubObj );
                if( aName.match(C2U("Handles")) )
                {
                    m_pMarkObj = pSubObj;
                    break;
                }
            }
        }
    }
    return m_pMarkObj;
}

//virtual
bool SelectionHelper::getMarkHandles( SdrHdlList& rHdlList )
{
    //@todo -> more flexible handle creation
    //2 scenarios possible:
    //1. add an additional invisible shape as a child to the selected object
    //this child needs to be named somehow and handles need to be generated therefrom ...
    //or 2. offer a central service per view where renderer and so can register for handle creation for a special shape
    //.. or 3. feature from drawinglayer to create handles for each shape ... (bad performance ... ?) ?

    //scenario 1 is now used:
    //if a child with name Handles exsists
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
            const XPolyPolygon& rPolyPolygon = ((SdrPathObj*)m_pMarkObj)->GetPathPoly();
            for( sal_Int32 nN = 0; nN < rPolyPolygon.Count(); nN++)
            {
                const XPolygon& rPolygon = rPolyPolygon[nN];
                for( sal_Int32 nM = 0; nM < rPolygon.GetPointCount(); nM++)
                {
                    SdrHdl* pHdl = new SdrHdl(rPolygon[nM],HDL_POLY);
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

    SdrObjListIter aIterator(*pSubList, IM_DEEPWITHGROUPS);

    if( pObj->ISA(E3dObject) )
    {
        /*
        E3dObject* p3DObj = (E3dObject*) pObj;
        USHORT nGroup = p3DObj->GetLogicalGroup();
        if(nGroup>0)
            aIterator = SdrObjListIter(*((SdrObject*)p3DObj->GetScene()), IM_DEEPWITHGROUPS);
        else
        */
        return false;
    }

    while (aIterator.IsMore())
    {
        SdrObject* pSubObj = aIterator.Next();
        Point aPos = pSubObj->GetBoundRect().Center();
        SdrHdl* pHdl = new SdrHdl(aPos,HDL_POLY);
        rHdlList.AddHdl(pHdl);
    }
    return true;
}

//.............................................................................
} //namespace chart
//.............................................................................
