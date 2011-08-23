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

#include "chtscene.hxx"
#include "datapoin.hxx"

#include <bf_svx/svxids.hrc>

#include "schattr.hxx"
#include "objid.hxx"

#include "axisobj.hxx"

namespace binfilter {

#define SCH_MIN(a, b) (((a) < (b))? (a): (b))

/*N*/ long ChartModel::GetChartShapeType()
/*N*/ {
/*N*/ 	long nShape   =CHART_SHAPE3D_IGNORE;   // BM: ANY instead of IGNORE
/*N*/ 	long nOldShape=CHART_SHAPE3D_IGNORE;
/*N*/ 
/*N*/     if( Is3DChart() &&
/*N*/         ( GetBaseType() == CHTYPE_BAR ||
/*N*/           GetBaseType() == CHTYPE_COLUMN ))
/*N*/     {
/*N*/         const SfxPoolItem *pPoolItem;
/*N*/         long nRowCnt=GetRowCount();
/*N*/         long nColCnt=GetColCount();
/*N*/         for(long nRow=0;nRow<nRowCnt;nRow++)
/*N*/         {
/*N*/             for(long nCol=0;nCol<nColCnt;nCol++)
/*N*/             {
/*N*/                 const SfxItemSet& rAttr=GetDataPointAttr(nCol,nRow);
/*N*/                 if(rAttr.GetItemState(SCHATTR_STYLE_SHAPE, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/                 {
/*N*/                     nShape=((const SfxInt32Item*) pPoolItem)->GetValue();
/*N*/                 }
/*N*/                 else
/*N*/                 {
/*?*/                     if(!GetDataPointObj(nCol,nRow))
/*?*/                     {
/*?*/                         nShape=nOldShape;
/*?*/                     }
/*?*/                     else
/*?*/                     {
/*?*/                         nShape=CHART_SHAPE3D_SQUARE;
/*?*/                     }
/*N*/                 }
/*N*/ 
/*N*/ 
/*N*/                 if( (nShape!=nOldShape) && nOldShape!=CHART_SHAPE3D_IGNORE)
/*?*/                     return CHART_SHAPE3D_ANY;
/*N*/ 
/*N*/                 nOldShape=nShape;
/*N*/             }
/*N*/         }
/*N*/     }
/*N*/ 
/*N*/     return nShape;
/*N*/ }
/*************************************************************************
|*                                                 |
|* 3D-Balken erzeugen; aPos: links, unten, hinten  |__
|*                                                /
\************************************************************************/
/*N*/ E3dCompoundObject* ChartModel::Create3DBar(Vector3D         aPos,
/*N*/ 										   Vector3D			aSizeVec,
/*N*/ 										   long				nCol,
/*N*/ 										   long				nRow,
/*N*/ 										   SfxItemSet		&rAttr,
/*N*/ 										   BOOL				bIsSimple,
/*N*/ 										   double			fMinPos,
/*N*/ 										   double			fOriPos,
/*N*/ 										   double			fMaxPos)
/*N*/ {
/*N*/ 	Matrix4D aTransMat;
/*N*/ 
/*N*/ 	long mode=CHART_SHAPE3D_SQUARE;
/*N*/ 	double fHeight=fMaxPos-fMinPos;
/*N*/ 	double a,b;
/*N*/ 
/*N*/ 
/*N*/ 	const SfxPoolItem *pPoolItem = NULL;
/*N*/ 	if (rAttr.GetItemState(SCHATTR_STYLE_SHAPE, TRUE, &pPoolItem) == SFX_ITEM_SET)
/*N*/ 		mode=((const SfxInt32Item*) pPoolItem)->GetValue();
/*N*/ 	if( (mode==CHART_SHAPE3D_ANY) ||
/*N*/ 		(mode==CHART_SHAPE3D_IGNORE)		// BM #66527# shouldn't happen but actually does :-(
/*N*/ 	  )
/*N*/ 		mode=CHART_SHAPE3D_SQUARE;
/*N*/ 
/*N*/ 	CHART_TRACE1( "Create3DBar Mode=%ld", mode );
/*N*/ 
/*N*/ 	PolyPolygon aPolyPoly;
/*N*/ 	Vector3D aDestCenter;
/*N*/ 	E3dCompoundObject* pObj;
/*N*/ 	E3dDefaultAttributes aDefltAttr3D;
/*N*/ 
/*N*/ 	long nSegs=32;
/*N*/ 
/*N*/ 	double fBase =
/*N*/ 		( IsBar()
/*N*/ 		  ? SCH_MIN( aSizeVec.Y(), aSizeVec.Z())
/*N*/ 		  : SCH_MIN( aSizeVec.X(), aSizeVec.Z()) )
/*N*/ 		/ 2.0 - 1.0;
/*N*/ 
/*N*/ 	aDestCenter = aPos + (aSizeVec / 2);
/*N*/ 
/*N*/ 	double fTmp = IsBar()
/*N*/ 		? aDestCenter.X()
/*N*/ 		: aDestCenter.Y();
/*N*/ 	BOOL bNegativ=(fTmp < fOriPos);
/*N*/ 
/*N*/ 	if(mode!=CHART_SHAPE3D_SQUARE && IsBar())
/*N*/ 	{
/*N*/ 		a=aPos.X()-fOriPos;
/*N*/ 		b=aPos.X()+aSizeVec.X()-fOriPos;
/*N*/ 
/*N*/ 		double fTmp=aSizeVec.X();
/*N*/ 		aSizeVec.X()=aSizeVec.Y();
/*N*/ 		aSizeVec.Y()=fTmp;
/*N*/ 		aSizeVec.Z()=-aSizeVec.Z(); //90Grad drehen
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		a=aPos.Y()-fOriPos;
/*N*/ 		b=aPos.Y()+aSizeVec.Y()-fOriPos;
/*N*/ 	}
/*N*/ 
/*N*/ 	const double fRelH  =(double)( (bNegativ) ? fOriPos-fMaxPos : fMinPos-fOriPos);
/*N*/ 
/*N*/ 	a=fRelH-a;
/*N*/ 	b=fRelH-b;
/*N*/ 
/*N*/ 	if(bNegativ)
/*N*/ 	{
/*N*/ 		double tmp=a;
/*N*/ 		a=b;
/*N*/ 		b=tmp;
/*N*/ 	}
/*N*/ 
/*N*/ 
/*N*/ 	if(mode==CHART_SHAPE3D_HANOI)
/*N*/ 	{
/*N*/ 		b=a;
/*N*/ 		mode=CHART_SHAPE3D_CONE;
/*N*/ 	}
/*N*/ 
/*N*/ 	// add extra points to extrusion-rectangle, to get a uniformly light-distribution
/*N*/ 	// the distance of these offset points is dependent of the setting of "edge-rounding"
/*N*/ 	double fRoundedEdge = 0.0;
/*N*/ 
/*N*/ 	fRoundedEdge = ((double)((const Svx3DPercentDiagonalItem&)
/*N*/ 		rAttr.Get(SDRATTR_3DOBJ_PERCENT_DIAGONAL)).GetValue()) / 200.0;
/*N*/ //-/	SfxItemState nState = rAttr.GetItemState( SID_ATTR_3D_PERCENT_DIAGONAL, TRUE, &pPoolItem );
/*N*/ //-/	if( nState == SFX_ITEM_DEFAULT )
/*N*/ //-/		fRoundedEdge = aDefltAttr3D.GetDefaultPercentDiag();
/*N*/ //-/	else if( nState == SFX_ITEM_SET && pPoolItem )
/*N*/ //-/		fRoundedEdge = SAL_STATIC_CAST( double, ( SAL_STATIC_CAST( const SfxUInt16Item*, pPoolItem )->GetValue() ))
/*N*/ //-/			/ 200.0;
/*N*/ 
/*N*/ 	// always use extra points, so set percent diagonal to 0.4 which is 0% in the UI
/*N*/ 	if( fRoundedEdge == 0.0 )
/*N*/ 		fRoundedEdge = 0.4 / 200.0;
/*N*/ 
/*N*/ 	switch( mode )
/*N*/ 	{
/*N*/ 		case CHART_SHAPE3D_CYLINDER:
/*N*/ 			{
/*N*/ 				double fOffset = (fBase * 2.0 * fRoundedEdge) * 1.05;		// increase by 5% for safety
/*N*/ 				short nPolySize;
/*N*/ 
/*N*/ 				aPos = Vector3D( 0.0, 0.0, 0.0 );
/*N*/ 				if( 2.0 * fOffset < fBase &&
/*N*/ 					2.0 * fOffset < aSizeVec.Y() )
/*N*/ 				{
/*N*/ 					nPolySize = 8;
/*N*/ 					Polygon aPoly( nPolySize );
/*N*/ 					aPoly[0] = Point( aPos.X(),						aPos.Y() );
/*N*/ 						  
/*N*/ 					aPoly[1] = Point( aPos.X() + fBase - fOffset,	aPos.Y() );
/*N*/ 					aPoly[2] = Point( aPos.X() + fBase,				aPos.Y() );
/*N*/ 					aPoly[3] = Point( aPos.X() + fBase,				aPos.Y() + fOffset );
/*N*/ 						  
/*N*/ 					aPoly[4] = Point( aPos.X() + fBase,				aPos.Y() + aSizeVec.Y() - fOffset );
/*N*/ 					aPoly[5] = Point( aPos.X() + fBase,				aPos.Y() + aSizeVec.Y());
/*N*/ 					aPoly[6] = Point( aPos.X() + fBase - fOffset,	aPos.Y() + aSizeVec.Y());
/*N*/ 						  
/*N*/ 					aPoly[7] = Point( aPos.X(),						aPos.Y() + aSizeVec.Y());
/*N*/ 
/*N*/ 					aPolyPoly.Insert( aPoly );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					nPolySize = 4;
/*N*/ 					Polygon aPoly( nPolySize );
/*N*/ 
/*N*/ 					aPoly[0]=Point(aPos.X()			, aPos.Y());
/*N*/ 					aPoly[1]=Point(aPos.X()	+ fBase	, aPos.Y());
/*N*/ 					aPoly[2]=Point(aPos.X() + fBase	, aPos.Y() + aSizeVec.Y());
/*N*/ 					aPoly[3]=Point(aPos.X()			, aPos.Y() + aSizeVec.Y());
/*N*/ 					
/*N*/ 					aPolyPoly.Insert(aPoly);
/*N*/ 				}
/*N*/ 			
/*N*/ 				pObj = new SchE3dLatheObj(aDefltAttr3D, aPolyPoly);
/*N*/   				((E3dLatheObj*)pObj)->SetItem( Svx3DHorizontalSegmentsItem( nSegs ));
/*N*/ 
/*N*/ 
/*N*/ 				// #67170# just write the necessary attributes
/*N*/ //-/				SfxItemSet aSegmentAttr(*pItemPool, SID_ATTR_3D_START, SID_ATTR_3D_END,
/*N*/ //-/													SCHATTR_STYLE_START, SCHATTR_STYLE_END,
/*N*/ //-/													0);
/*N*/ 				SfxItemSet aSegmentAttr(*pItemPool, 
/*N*/ 					SDRATTR_3D_FIRST, SDRATTR_3D_LAST,
/*N*/ 					SCHATTR_STYLE_START, SCHATTR_STYLE_END,
/*N*/ 					0, 0);
/*N*/ 
/*N*/   				rAttr.Put( Svx3DHorizontalSegmentsItem( nSegs ));
/*N*/ 
/*N*/ 				aSegmentAttr.Put(rAttr);
/*N*/ 				PutDataPointAttr(nCol, nRow, aSegmentAttr);
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 
/*N*/ 		case CHART_SHAPE3D_PYRAMID:
/*N*/ 			nSegs = 4;
/*N*/ 			aTransMat.RotateY( F_PI / 4.0 );		// rotate edge to front
/*N*/ 			// continue with same code as for cone
/*N*/ 
/*N*/ 		case CHART_SHAPE3D_CONE:
/*N*/ 			{
/*N*/ 				aPos = Vector3D( 0, 0, 0 );
/*N*/ 				double fOffset = (fBase * fRoundedEdge) * 1.05;		// increase by 5% for safety
/*N*/ 
/*N*/ 				BOOL bIsTip = (fRelH == 0.0 || fHeight == 0.0) ||	// nonstacked chart
/*N*/ 					(fBase * b <= fOffset * fRelH);					// tip of stacked chart
/*N*/ 
/*N*/ 				long nPolySize;
/*N*/ 				double r1, r2;
/*N*/ 				if( bIsTip )
/*N*/ 				{
/*N*/ 					r1 = 0.0;
/*N*/ 					r2 = ( fRelH == 0.0 )? fBase : ( fBase * a ) / fRelH;
/*N*/ 					nPolySize = 6;
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					r1 = ( fBase * b ) / fRelH;
/*N*/ 					r2 = ( fBase * a ) / fRelH;
/*N*/ 					nPolySize = 8;
/*N*/ 				}
/*N*/ 				double fTemp	= ((r2-r1) * (r2-r1)) / (aSizeVec.Y() * aSizeVec.Y());
/*N*/ 				double fOffsetX = sqrt( fOffset * fOffset / (1.0 + 1.0/fTemp ));
/*N*/ 				double fOffsetY = sqrt( fOffset * fOffset / (1.0 + fTemp ));
/*N*/ 
/*N*/ 				Polygon aPoly( nPolySize );
/*N*/ 				short i=0;
/*N*/ 
/*N*/ 				if( ! bIsTip )		// skip these points for the tip
/*N*/ 				{
/*N*/ 					aPoly[i++] = Point( aPos.X(),					aPos.Y());
/*N*/ 					aPoly[i++] = Point( aPos.X() + r1 - fOffset,	aPos.Y());
/*N*/ 				}
/*N*/ 				aPoly[i++] = Point( aPos.X() + r1,					aPos.Y());
/*N*/ 				aPoly[i++] = Point( aPos.X() + r1 + fOffsetX,		aPos.Y() + fOffsetY);
/*N*/ 				aPoly[i++] = Point( aPos.X() + r2 - fOffsetX,		aPos.Y() + aSizeVec.Y() - fOffsetY);
/*N*/ 				aPoly[i++] = Point( aPos.X() + r2,					aPos.Y() + aSizeVec.Y());
/*N*/ 				aPoly[i++] = Point( aPos.X() + r2 - fOffset,		aPos.Y() + aSizeVec.Y());
/*N*/ 				aPoly[i++] = Point( aPos.X(),						aPos.Y() + aSizeVec.Y());
/*N*/ 
/*N*/ 				aPolyPoly.Insert( aPoly );
/*N*/ 
/*N*/ 				pObj = new SchE3dLatheObj( aDefltAttr3D, aPolyPoly );
/*N*/   				((E3dLatheObj*)pObj)->SetItem( Svx3DHorizontalSegmentsItem( nSegs ));
/*N*/ 
/*N*/ 				// #67170# just write the necessary attributes
/*N*/ //-/				SfxItemSet aSegmentAttr(*pItemPool, SID_ATTR_3D_START, SID_ATTR_3D_END,
/*N*/ //-/													SCHATTR_STYLE_START, SCHATTR_STYLE_END,
/*N*/ //-/													0);
/*N*/ 				SfxItemSet aSegmentAttr(*pItemPool, 
/*N*/ 					SDRATTR_3D_FIRST,		SDRATTR_3D_LAST,
/*N*/ 					SCHATTR_STYLE_START,	SCHATTR_STYLE_END,
/*N*/ 					0, 0);
/*N*/ 
/*N*/ 				rAttr.Put( Svx3DHorizontalSegmentsItem( nSegs ));
/*N*/ 
/*N*/ 				aSegmentAttr.Put(rAttr);
/*N*/ 				PutDataPointAttr(nCol, nRow, aSegmentAttr);
/*N*/ 			}
/*N*/ 			break;
/*N*/ 		default:
/*N*/ 		case CHART_SHAPE3D_SQUARE:
/*N*/ 			{
/*N*/ 				double fOffset = (aSizeVec.Z() * fRoundedEdge) * 1.05;		// increase by 5% for safety
/*N*/ 
/*N*/ 				if( 2.0 * fOffset < aSizeVec.X() &&
/*N*/ 					2.0 * fOffset < aSizeVec.Y() )
/*N*/ 				{
/*N*/ 					Polygon aPoly( 13 );
/*N*/ 					aPoly[ 0] = Point( aPos.X() + fOffset,					aPos.Y() );
/*N*/ 					aPoly[ 1] = Point( aPos.X(),							aPos.Y() );
/*N*/ 					aPoly[ 2] = Point( aPos.X(),							aPos.Y() + fOffset );
/*N*/ 
/*N*/ 					aPoly[ 3] = Point( aPos.X(),							aPos.Y() + aSizeVec.Y() - fOffset );
/*N*/ 					aPoly[ 4] = Point( aPos.X(),							aPos.Y() + aSizeVec.Y());
/*N*/ 					aPoly[ 5] = Point( aPos.X() + fOffset,					aPos.Y() + aSizeVec.Y());
/*N*/ 
/*N*/ 					aPoly[ 6] = Point( aPos.X() + aSizeVec.X() - fOffset,	aPos.Y() + aSizeVec.Y());
/*N*/ 					aPoly[ 7] = Point( aPos.X() + aSizeVec.X(),				aPos.Y() + aSizeVec.Y());
/*N*/ 					aPoly[ 8] = Point( aPos.X() + aSizeVec.X(),				aPos.Y() + aSizeVec.Y() - fOffset );
/*N*/ 
/*N*/ 					aPoly[ 9] = Point( aPos.X() + aSizeVec.X(),				aPos.Y() + fOffset );
/*N*/ 					aPoly[10] = Point( aPos.X() + aSizeVec.X(),				aPos.Y() );
/*N*/ 					aPoly[11] = Point( aPos.X() + aSizeVec.X() - fOffset,	aPos.Y() );
/*N*/ 
/*N*/ 					aPoly[12] = aPoly[ 0];
/*N*/ 					aPolyPoly.Insert( aPoly );
/*N*/ 				}
/*N*/ 				else
/*N*/ 				{
/*N*/ 					Polygon aPoly(5);
/*N*/ 					aPoly[0]=Point(aPos.X(), aPos.Y());
/*N*/ 					aPoly[1]=Point(aPos.X(), aPos.Y() + aSizeVec.Y());
/*N*/ 					aPoly[2]=Point(aPos.X() + aSizeVec.X(), aPos.Y() + aSizeVec.Y());
/*N*/ 					aPoly[3]=Point(aPos.X() + aSizeVec.X(), aPos.Y());
/*N*/ 					aPoly[4]=aPoly[0];
/*N*/ 
/*N*/ 					aPolyPoly.Insert(aPoly);
/*N*/ 				}
/*N*/ 
/*N*/ 				pObj = new SchE3dExtrudeObj(aDefltAttr3D, aPolyPoly, aSizeVec.Z());
/*N*/ 			}
/*N*/ 			break;
/*N*/ 
/*N*/ 	}
/*N*/ 
/*N*/ 	Vector3D aOldCenter=pObj->GetCenter();
/*N*/ 	if(mode!=CHART_SHAPE3D_SQUARE)
/*N*/ 	{
/*N*/ 		if(IsBar())
/*N*/ 		{
/*N*/ 			aTransMat.Translate(-aOldCenter);//Zentrum in Nullpunkt setzen
/*N*/ 			aTransMat.RotateZ(-3.1415927/2.0);//Saeule umwerfen (90Grad nach rechts)
/*N*/ 			if(bNegativ)
/*N*/ 				aTransMat.RotateZ(3.1415927);//Spitze nach unten oder links(Kegel, etc.)
/*N*/ 			aOldCenter=Vector3D(0,0,0);
/*N*/ 		}
/*N*/ 		else if(bNegativ)
/*N*/ 		{
/*N*/ 			aTransMat.Translate(-aOldCenter);//Zentrum in Nullpunkt setzen
/*N*/ 			if(bNegativ)
/*N*/ 				aTransMat.RotateZ(3.1415927);//Spitze nach unten oder links(Kegel, etc.)
/*N*/ 			aOldCenter=Vector3D(0,0,0);
/*N*/ 		}
/*N*/ 	}
/*N*/ 	aTransMat.Translate(aDestCenter - aOldCenter);//An die gewuenschte Position schieben
/*N*/ 	//aTransMat=pObj->GetTransform()*aTransMat;
/*N*/ 	pObj->NbcSetTransform(aTransMat);
/*N*/ 
/*N*/ 	pObj->SetModel (this);
    // #106658# the ChartModel has a different Pool default for the
    // Svx3DPercentDiagonalItem.  The Geometry is created while using a global
    // pool (Model is not set). Ensure recreating the geometry with new Model
/*N*/     pObj->DestroyGeometry();
/*N*/ 
/*N*/ 	pObj->InsertUserData(new SchObjectId(CHOBJID_DIAGRAM_DATA));
/*N*/ 
/*N*/ 	// the number vertical segments is always fixed
/*N*/ 	rAttr.ClearItem( SDRATTR_3DOBJ_VERT_SEGS );
/*N*/ 	pObj->SetItemSet(rAttr);
/*N*/ 
/*N*/ 	pObj->InsertUserData(new SchDataPoint(nCol, nRow));
/*N*/ 
/*N*/ 	return  pObj;
/*N*/ }
/*************************************************************************
|*
|* Neues 3D-Object erzeugen
|*
\************************************************************************/
/*N*/ E3dObject* ChartModel::Create3DObject (UINT16     ID)
/*N*/ {
/*N*/ 	E3dObject *pMyObject = new SchE3dObject;
/*N*/ 	pMyObject->SetModel (this);
/*N*/ 	pMyObject->InsertUserData(new SchObjectId(ID));
/*N*/ 	return pMyObject;
/*N*/ }

/*N*/ E3dScene* ChartModel::Create3DScene (UINT16     ID)
/*N*/ {
/*N*/ 	E3dScene* pMyObject = new ChartScene( this );
/*N*/ 	pMyObject->InsertUserData(new SchObjectId(ID));
/*N*/ 	return pMyObject;
/*N*/ }

/*N*/ E3dObject* ChartModel::Create3DAxisObj( UINT16 nId )
/*N*/ {
/*N*/ 	E3dObject *pMyObject = new Sch3dAxisObj;
/*N*/ 	pMyObject->SetModel( this );
/*N*/ 	pMyObject->InsertUserData( new SchObjectId( nId ) );
/*N*/ 	return pMyObject;
/*N*/ }

/*************************************************************************
|*
|* 3D-Polygon erzeugen
|*
\************************************************************************/
/*N*/ void  ChartModel::Create3DPolyObject (const SfxItemSet  *pAttr,
/*N*/ 						 E3dPolygonObj     *pMyObject,
/*N*/ 						 UINT16            nID,
/*N*/ 						 E3dObject *pParent)
/*N*/ {
/*N*/ 	pMyObject->InsertUserData(new SchObjectId(nID));
/*N*/ 	pParent->Insert3DObj (pMyObject);
/*N*/ 	pMyObject->SetModel (this);
/*N*/ 
/*N*/ //-/	pMyObject->NbcSetAttributes(*pAttr, FALSE);
/*N*/ 	pMyObject->SetItemSet(*pAttr);
/*N*/ 
/*N*/ }


/*N*/ void  ChartModel::Create3DExtrudePolyObj(const SfxItemSet  *pAttr,
/*N*/ 						 E3dExtrudeObj     *pMyObject,
/*N*/ 						 UINT16            nID,
/*N*/ 						 E3dObject *pParent)
/*N*/ {
/*N*/ 	pMyObject->InsertUserData(new SchObjectId(nID));
/*N*/ 	pParent->Insert3DObj (pMyObject);
/*N*/ 	pMyObject->SetModel (this);
/*N*/ 
/*N*/ //-/	pMyObject->NbcSetAttributes(*pAttr, FALSE);
/*N*/ 	pMyObject->SetItemSet(*pAttr);
/*N*/ 
/*N*/ }

/*************************************************************************
|*
|* 3D-Scene erzeugen
|*
\************************************************************************/

/*N*/ ChartScene* ChartModel::CreateScene (const Rectangle &rRect,
/*N*/ 						 const Vector3D  &aLightVec,
/*N*/ 						 double          fSpotIntensity,
/*N*/ 						 Color&          rSpotColor,
/*N*/ 						 double          fAmbientIntensity,
/*N*/ 						 Color&          rAmbientColor)
/*N*/ {
/*N*/ 	ChartScene *pMyScene = new ChartScene (this);
/*N*/ 
/*N*/ 	pMyScene->InsertUserData(new SchObjectId(CHOBJID_DIAGRAM));
/*N*/ 	pMyScene->NbcSetSnapRect(rRect);
/*N*/ 
/*N*/ 	return pMyScene;
/*N*/ }

/*N*/ void SchRectObj::NbcSetOutlinerParaObject(OutlinerParaObject* pTextObject)
/*N*/ {
/*N*/ 	SdrRectObj::NbcSetOutlinerParaObject(pTextObject);
/*N*/ 	ChartModel* pModel=(ChartModel*)GetModel();
/*N*/ 	if(pModel)
/*N*/ 		pModel->SetTextFromObject(this,pTextObject);
/*N*/ }

//////////////////////////////////////////////////////////////////////////////

/*N*/ void ImpStoreObjcetsAttr(SdrObject *pObj)
/*N*/ {
/*N*/ 	ChartModel* pModel = (ChartModel*)pObj->GetModel();
/*N*/ 	if(pModel)
/*N*/ 		pModel->StoreObjectsAttributes(pObj, pObj->GetItemSet(), FALSE);
/*N*/ }

//////////////////////////////////////////////////////////////////////////////



/*N*/ void SchRectObj::SetItemSet(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	SdrRectObj::SetItemSet(rSet);
/*N*/ 	ImpStoreObjcetsAttr(this);
/*N*/ }

//////////////////////////////////////////////////////////////////////////////

/*N*/ void SchE3dExtrudeObj::SetItem(const SfxPoolItem& rItem)
/*N*/ {
/*N*/ 	E3dExtrudeObj::SetItem(rItem);
/*N*/ 	ImpStoreObjcetsAttr(this);
/*N*/ }


/*N*/ void SchE3dExtrudeObj::SetItemSet(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	E3dExtrudeObj::SetItemSet(rSet);
/*N*/ 	ImpStoreObjcetsAttr(this);
/*N*/ }

//////////////////////////////////////////////////////////////////////////////

/*N*/ void SchE3dPolygonObj::SetItem(const SfxPoolItem& rItem)
/*N*/ {
/*N*/ 	E3dPolygonObj::SetItem(rItem);
/*N*/ 	ImpStoreObjcetsAttr(this);
/*N*/ }


/*N*/ void SchE3dPolygonObj::SetItemSet(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	E3dPolygonObj::SetItemSet(rSet);
/*N*/ 	ImpStoreObjcetsAttr(this);
/*N*/ }

//////////////////////////////////////////////////////////////////////////////

/*N*/ void SchE3dLatheObj::SetItem(const SfxPoolItem& rItem)
/*N*/ {
/*N*/ 	E3dLatheObj::SetItem(rItem);
/*N*/ 	ImpStoreObjcetsAttr(this);
/*N*/ }


/*N*/ void SchE3dLatheObj::SetItemSet(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	E3dLatheObj::SetItemSet(rSet);
/*N*/ 	ImpStoreObjcetsAttr(this);
/*N*/ }

//////////////////////////////////////////////////////////////////////////////



/*N*/ void SchE3dObject::SetItemSet(const SfxItemSet& rSet)
/*N*/ {
/*N*/ 	E3dObject::SetItemSet(rSet);
/*N*/ 	ImpStoreObjcetsAttr(this);
/*N*/ }

//////////////////////////////////////////////////////////////////////////////

//-/void SchRectObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/	SdrRectObj::NbcSetAttributes(rAttr,bReplaceAll);
//-/	ChartModel* pModel=(ChartModel*)GetModel();
//-/	if(pModel)
//-/		pModel->StoreObjectsAttributes(this,rAttr,bReplaceAll);
//-/};
//-/void SchE3dExtrudeObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/	E3dExtrudeObj::NbcSetAttributes(rAttr,bReplaceAll);
//-/	ChartModel* pModel=(ChartModel*)GetModel();
//-/	if(pModel)
//-/		pModel->StoreObjectsAttributes(this,rAttr,bReplaceAll);
//-/};
//-/void SchE3dPolygonObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/	E3dPolygonObj::NbcSetAttributes(rAttr,bReplaceAll);
//-/	ChartModel* pModel=(ChartModel*)GetModel();
//-/	if(pModel)
//-/		pModel->StoreObjectsAttributes(this,rAttr,bReplaceAll);
//-/};
//-/void SchE3dLatheObj::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/	E3dLatheObj::NbcSetAttributes(rAttr,bReplaceAll);
//-/	ChartModel* pModel=(ChartModel*)GetModel();
//-/	if(pModel)
//-/		pModel->StoreObjectsAttributes(this,rAttr,bReplaceAll);
//-/};
//-/void SchE3dObject::NbcSetAttributes(const SfxItemSet& rAttr, FASTBOOL bReplaceAll)
//-/{
//-/	E3dObject::NbcSetAttributes(rAttr,bReplaceAll);
//-/	ChartModel* pModel=(ChartModel*)GetModel();
//-/	if(pModel)
//-/		pModel->StoreObjectsAttributes(this,rAttr,bReplaceAll);
//-/};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
