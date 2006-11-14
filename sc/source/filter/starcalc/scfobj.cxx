/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: scfobj.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 15:48:43 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"


#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_XVISUALOBJECT_HPP_
#include <com/sun/star/embed/XVisualObject.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif


using namespace com::sun::star;

// INCLUDE ---------------------------------------------------------------

#include <svtools/moduleoptions.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/objsh.hxx>
#include <sot/storage.hxx>
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

    ::rtl::OUString aName;
    uno::Reference < embed::XEmbeddedObject > xObj = pDoc->GetDocumentShell()->
            GetEmbeddedObjectContainer().CreateEmbeddedObject( SvGlobalName( SO3_SCH_CLASSID ).GetByteSequence(), aName );
    if ( xObj.is() )
    {
        SdrOle2Obj* pSdrOle2Obj = new SdrOle2Obj( ::svt::EmbeddedObjectRef( xObj, embed::Aspects::MSOLE_CONTENT ), aName, rRect );

        ScDrawLayer* pModel = pDoc->GetDrawLayer();
        if (!pModel)
        {
            pDoc->InitDrawLayer();
            pModel = pDoc->GetDrawLayer();
            DBG_ASSERT(pModel,"Draw Layer ?");
        }

        SdrPage* pPage = pModel->GetPage(static_cast<sal_uInt16>(nDestTab));
        DBG_ASSERT(pPage,"Page ?");
        pPage->InsertObject(pSdrOle2Obj);

        pSdrOle2Obj->SetLogicRect(rRect);               // erst nach InsertObject !!!
        awt::Size aSz;
        aSz.Width = rRect.GetSize().Width();
        aSz.Height = rRect.GetSize().Height();
        xObj->setVisualAreaSize( embed::Aspects::MSOLE_CONTENT, aSz );

            // hier kann das Chart noch nicht mit Daten gefuettert werden,
            // weil die Formeln noch nicht berechnet sind.
            // Deshalb in die ChartCollection, die Daten werden dann im
            // Sc10Import dtor geholt.

        ScChartCollection* pColl = pDoc->GetChartCollection();
        pColl->Insert( new ScChartArray( pDoc, nSrcTab, static_cast<SCCOL>(nX1), static_cast<SCROW>(nY1), static_cast<SCCOL>(nX2), static_cast<SCROW>(nY2), aName ) );
    }
}



