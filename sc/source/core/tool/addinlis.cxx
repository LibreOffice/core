/*************************************************************************
 *
 *  $RCSfile: addinlis.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:17 $
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
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include <tools/debug.hxx>
#include <sfx2/objsh.hxx>


#include "addinlis.hxx"
#include "miscuno.hxx"      // SC_IMPL_SERVICE_INFO
#include "document.hxx"
#include "brdcst.hxx"
#include "unoguard.hxx"
#include "sc.hrc"

using namespace com::sun::star;

//------------------------------------------------------------------------

//SMART_UNO_IMPLEMENTATION( ScAddInListener, UsrObject );

SC_SIMPLE_SERVICE_INFO( ScAddInListener, "ScAddInListener", "stardiv.one.sheet.AddInListener" )

//------------------------------------------------------------------------

List ScAddInListener::aAllListeners;

//------------------------------------------------------------------------

//  static
ScAddInListener* ScAddInListener::CreateListener(
                        uno::Reference<sheet::XVolatileResult> xVR, ScDocument* pDoc )
{
    ScAddInListener* pNew = new ScAddInListener( xVR, pDoc );

    pNew->acquire();                                // for aAllListeners
    aAllListeners.Insert( pNew, LIST_APPEND );

    if ( xVR.is() )
        xVR->addResultListener( pNew );             // after at least 1 ref exists!

    return pNew;
}

ScAddInListener::ScAddInListener( uno::Reference<sheet::XVolatileResult> xVR, ScDocument* pDoc ) :
    xVolRes( xVR )
{
    pDocs = new ScAddInDocs( 1, 1 );
    pDocs->Insert( pDoc );
}

ScAddInListener::~ScAddInListener()
{
    delete pDocs;
}

// static
ScAddInListener* ScAddInListener::Get( uno::Reference<sheet::XVolatileResult> xVR )
{
    sheet::XVolatileResult* pComp = xVR.get();

    ULONG nCount = aAllListeners.Count();
    for (ULONG nPos=0; nPos<nCount; nPos++)
    {
        ScAddInListener* pLst = (ScAddInListener*)aAllListeners.GetObject(nPos);
        if ( pComp == (sheet::XVolatileResult*)pLst->xVolRes.get() )
            return pLst;
    }
    return NULL;        // not found
}

//! move to some container object?
// static
void ScAddInListener::RemoveDocument( ScDocument* pDocumentP )
{
    ULONG nPos = aAllListeners.Count();
    while (nPos)
    {
        //  loop backwards because elements are removed
        --nPos;
        ScAddInListener* pLst = (ScAddInListener*)aAllListeners.GetObject(nPos);
        ScAddInDocs* p = pLst->pDocs;
        USHORT nFoundPos;
        if ( p->Seek_Entry( pDocumentP, &nFoundPos ) )
        {
            p->Remove( nFoundPos );
            if ( p->Count() == 0 )
            {
                // this AddIn is no longer used
                //  dont delete, just remove the ref for the list

                aAllListeners.Remove( nPos );

                if ( pLst->xVolRes.is() )
                    pLst->xVolRes->removeResultListener( pLst );

                pLst->release();    // Ref for aAllListeners - pLst may be deleted here
            }
        }
    }
}

//------------------------------------------------------------------------

// XResultListener

void SAL_CALL ScAddInListener::modified( const ::com::sun::star::sheet::ResultEvent& aEvent )
                                throw(::com::sun::star::uno::RuntimeException)
{
    ScUnoGuard aGuard;          //! or generate a UserEvent

    aResult = aEvent.Value;     // store result

    if ( !HasListeners() )
    {
        //! remove from list and removeListener, as in RemoveDocument ???

#if 0
        //! this will crash if called before first StartListening !!!
        aAllListeners.Remove( this );
        if ( xVolRes.is() )
            xVolRes->removeResultListener( this );
        release();  // Ref for aAllListeners - this may be deleted here
        return;
#endif
    }

    //  notify document of changes

    Broadcast( ScHint( SC_HINT_DATACHANGED, ScAddress( 0 ), NULL ) );

    const ScDocument** ppDoc = (const ScDocument**) pDocs->GetData();
    USHORT nCount = pDocs->Count();
    for ( USHORT j=0; j<nCount; j++, ppDoc++ )
    {
        ScDocument* pDoc = (ScDocument*)*ppDoc;
        pDoc->TrackFormulas();
        pDoc->GetDocumentShell()->Broadcast( SfxSimpleHint( FID_DATACHANGED ) );
        pDoc->ResetChanged( ScRange(0,0,0,MAXCOL,MAXROW,MAXTAB) );
    }
}

// XEventListener

void SAL_CALL ScAddInListener::disposing( const ::com::sun::star::lang::EventObject& Source )
                                throw(::com::sun::star::uno::RuntimeException)
{
    // hold a ref so this is not deleted at removeResultListener
    uno::Reference<sheet::XResultListener> xRef( this );

    if ( xVolRes.is() )
    {
        xVolRes->removeResultListener( this );
        xVolRes = NULL;
    }
}


//------------------------------------------------------------------------



