/*************************************************************************
 *
 *  $RCSfile: DrawViewWrapper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:27 $
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
#ifndef _CHART2_DRAW_VIEW_WRAPPER_HXX
#define _CHART2_DRAW_VIEW_WRAPPER_HXX

#ifndef _E3D_VIEW3D_HXX
#include <svx/view3d.hxx>
#endif

class SdrModel;

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/** The DrawViewWrapper should help us to reduce effort if the underlying DrawingLayer changes.
Another task is to hide functionality we do not need, for example more than one page.
*/

class MarkHandleProvider
{
public:
    virtual bool getMarkHandles( SdrHdlList& rHdlList ) =0;
    virtual bool getFrameDragSingles() =0;
};

class DrawViewWrapper : public E3dView
{
public:
    DrawViewWrapper(SdrModel* pModel, OutputDevice* pOut);
    virtual ~DrawViewWrapper();

    //fill list of selection handles 'aHdl'
    virtual void SetMarkHandles();

    //SdrPageView*    GetPageView() { return m_pWrappedDLPageView; };

    SdrObject* getHitObject( const Point& rPnt ) const;
    //BOOL PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, ULONG nOptions, SdrObject** ppRootObj, ULONG* pnMarkNum=NULL, USHORT* pnPassNum=NULL) const;
    //BOOL PickObj(const Point& rPnt, short nTol, SdrObject*& rpObj, SdrPageView*& rpPV, ULONG nOptions=0) const;
    //BOOL PickObj(const Point& rPnt, SdrObject*& rpObj, SdrPageView*& rpPV, ULONG nOptions=0) const { return PickObj(rPnt,nHitTolLog,rpObj,rpPV,nOptions); }

    //void MarkObj(SdrObject* pObj, SdrPageView* pPV, BOOL bUnmark=FALSE, BOOL bImpNoSetMarkHdl=FALSE);
    void MarkObject( SdrObject* pObj );

    //----------------------
    //pMarkHandleProvider can be NULL; ownership is not taken
    void setMarkHandleProvider( MarkHandleProvider* pMarkHandleProvider );

    void InitRedraw( OutputDevice* pOut, const Region& rReg );

private:
    mutable SdrPageView*            m_pWrappedDLPageView;
    mutable MarkHandleProvider*     m_pMarkHandleProvider;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif

