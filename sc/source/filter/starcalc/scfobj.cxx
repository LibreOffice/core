/*************************************************************************
 *
 *  $RCSfile: scfobj.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 11:05:55 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

// INCLUDE ---------------------------------------------------------------

#include <svtools/moduleoptions.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/objsh.hxx>
#include <so3/ipobj.hxx>
#include <so3/svstor.hxx>
#include <sch/schdll.hxx>
#include <sch/memchrt.hxx>
#include <sfx2/app.hxx>
#include <sot/clsids.hxx>
#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#include "scfobj.hxx"
#include "document.hxx"
#include "drwlayer.hxx"
#include "chartarr.hxx"


// STATIC DATA -----------------------------------------------------------


//==================================================================

void Sc10InsertObject::InsertChart( ScDocument* pDoc, SCTAB nDestTab, const Rectangle& rRect,
                                SCTAB nSrcTab, USHORT nX1, USHORT nY1, USHORT nX2, USHORT nY2 )
{
    //  wenn Chart nicht installiert ist, darf nicht auf SCH_MOD zugegriffen werden!
    if ( !SvtModuleOptions().IsChart() )
        return;

    SvInPlaceObjectRef aNewIPObj = SvInPlaceObject::CreateObject( SvGlobalName( SO3_SCH_CLASSID ) );
    if ( aNewIPObj.Is() )
    {
        String aName = pDoc->GetDocumentShell()->InsertObject( aNewIPObj, String() )->GetObjName();
        SdrOle2Obj* pSdrOle2Obj = new SdrOle2Obj( aNewIPObj, aName, rRect );

        ScDrawLayer* pModel = pDoc->GetDrawLayer();
        if (!pModel)
        {
            pDoc->InitDrawLayer();
            pModel = pDoc->GetDrawLayer();
            DBG_ASSERT(pModel,"DrawLayer ?");
        }

        SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nDestTab));
        DBG_ASSERT(pPage,"Page ?");
        pPage->InsertObject(pSdrOle2Obj);

        pSdrOle2Obj->SetLogicRect(rRect);               // erst nach InsertObject !!!
        aNewIPObj->SetVisAreaSize(rRect.GetSize());

            // hier kann das Chart noch nicht mit Daten gefuettert werden,
            // weil die Formeln noch nicht berechnet sind.
            // Deshalb in die ChartCollection, die Daten werden dann im
            // Sc10Import dtor geholt.

        ScChartCollection* pColl = pDoc->GetChartCollection();
        pColl->Insert( new ScChartArray( pDoc, nSrcTab, static_cast<SCCOL>(nX1), static_cast<SCROW>(nY1), static_cast<SCCOL>(nX2), static_cast<SCROW>(nY2), aName ) );
    }
}



