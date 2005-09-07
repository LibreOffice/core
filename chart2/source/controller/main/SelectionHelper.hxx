/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SelectionHelper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 00:35:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CHART2_SELECTIONHELPER_HXX
#define _CHART2_SELECTIONHELPER_HXX

#include "DrawViewWrapper.hxx"

class SdrObject;
// header for enum SdrDragMode
#ifndef _SVDTYPES_HXX
#include <svx/svdtypes.hxx>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/

class SelectionHelper : public MarkHandleProvider
{
public:
    static rtl::OUString    getObjectName( SdrObject* pObj );
    static bool             findNamedParent( SdrObject*& pInOutObject
                                , rtl::OUString& rOutName
                                , bool bGivenObjectMayBeResult );
    static SdrObject*       getObjectToSelect( const Point& rMPos
                                , rtl::OUString& rNameOfLastSelectedObject
                                , const DrawViewWrapper& rDrawViewWrapper
                                , bool bAllowMultiClickSelectionChange );
    static bool             isDragableObjectHitTwice( const Point& rMPos
                                , const rtl::OUString& rNameOfLastSelectedObject
                                , const DrawViewWrapper& rDrawViewWrapper );

    static void             changeSelection( const Point& rMPos
                                , rtl::OUString& rNameOfLastSelectedObject
                                , DrawViewWrapper& rDrawViewWrapper
                                , bool bAllowMultiClickSelectionChange );


    SelectionHelper( SdrObject* pSelectedObj, const rtl::OUString& rSelectedObjectCID );
    virtual ~SelectionHelper();


    //MarkHandleProvider:
    virtual bool getMarkHandles( SdrHdlList& rHdlList );
    virtual bool getFrameDragSingles();

    SdrObject*   getObjectToMark();//sets also internally the mark object
    //-> getMarkHandles will behave different if this method has found a Mark Object different from m_pSelectedObj

private:
    SdrObject*      m_pSelectedObj;//hit and logically selected object
    rtl::OUString   m_aSelectedObjectCID;

    SdrObject*      m_pMarkObj;//object that is marked instead to have more pretty handles
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
