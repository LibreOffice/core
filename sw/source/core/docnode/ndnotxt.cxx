/*************************************************************************
 *
 *  $RCSfile: ndnotxt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:17 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "hintids.hxx"

#ifndef _IPOBJ_HXX //autogen
#include <so3/ipobj.hxx>
#endif
#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif
#ifndef _CONTDLG_HXX_ //autogen
#include <svx/contdlg.hxx>
#endif

#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#include "doc.hxx"
#include "fmtcol.hxx"
#include "ndnotxt.hxx"
#include "ndgrf.hxx"
#include "ndole.hxx"
#include "ndindex.hxx"


SwNoTxtNode::SwNoTxtNode( const SwNodeIndex & rWhere,
                  const BYTE nNdType,
                  SwGrfFmtColl *pGrfColl,
                  SwAttrSet* pAutoAttr ) :
    SwCntntNode( rWhere, nNdType, pGrfColl ),
    pContour( 0 )
{
    // soll eine Harte-Attributierung gesetzt werden?
    if( pAutoAttr )
        SetAttr( *pAutoAttr );
}


SwNoTxtNode::~SwNoTxtNode()
{
    delete pContour;
}


// erzeugt fuer alle Ableitungen einen AttrSet mit Bereichen
// fuer Frame- und Grafik-Attributen
void SwNoTxtNode::NewAttrSet( SwAttrPool& rPool )
{
    ASSERT( !pAttrSet, "AttrSet ist doch gesetzt" );
    pAttrSet = new SwAttrSet( rPool, aNoTxtNodeSetRange );
    pAttrSet->SetParent( &GetFmtColl()->GetAttrSet() );
}

// Dummies fuer das Laden/Speichern von persistenten Daten
// bei Grafiken und OLE-Objekten


BOOL SwNoTxtNode::RestorePersistentData()
{
    return TRUE;
}


BOOL SwNoTxtNode::SavePersistentData()
{
    return TRUE;
}


void SwNoTxtNode::SetContour( const PolyPolygon *pPoly )
{
    delete pContour;
    if ( pPoly )
        pContour = new PolyPolygon( *pPoly );
    else
        pContour = 0;
}


void SwNoTxtNode::CreateContour()
{
    ASSERT( !pContour, "Contour available." );
    pContour = new PolyPolygon( SvxContourDlg::CreateAutoContour( GetGraphic() ) );
}


void SwNoTxtNode::GetContour( PolyPolygon &rPoly ) const
{
    ASSERT( pContour, "Contour not available." );
    rPoly = *pContour;
}


Graphic SwNoTxtNode::GetGraphic() const
{
    Graphic aRet;
    if ( GetGrfNode() )
    {
        ((SwGrfNode*)this)->SwapIn( TRUE );
        aRet = ((SwGrfNode*)this)->GetGrf();
    }
    else
    {
        ASSERT( GetOLENode(), "new type of Node?" );
        SvInPlaceObjectRef xObj( ((SwOLENode*)this)->GetOLEObj().GetOleRef() );

        SvData aData( FORMAT_GDIMETAFILE );
        if ( xObj->GetData( &aData ) )
        {
            GDIMetaFile *pPtr;
            if ( aData.GetData( &pPtr, TRANSFER_REFERENCE ) )
                aRet = *pPtr;
        }
    }
    return aRet;
}



