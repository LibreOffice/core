/*************************************************************************
 *
 *  $RCSfile: unoshtxt.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:27 $
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

#pragma hdrstop

//#include <tools/debug.hxx>
//#include <svx/editeng.hxx>

#include <unoshtxt.hxx>

#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif
#ifndef _SVDMODEL_HXX //autogen
#include <svdmodel.hxx>
#endif
#ifndef _SVDOUTL_HXX //autogen
#include <svdoutl.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svdobj.hxx>
#endif
#ifndef _SVDETC_HXX
#include <svdetc.hxx>
#endif
#ifndef _OUTLINER_HXX
#include <outliner.hxx>
#endif
#ifndef _SVX_UNOFOROU_HXX
#include <unoforou.hxx>
#endif

#include "unotext.hxx"

//------------------------------------------------------------------------

SvxTextEditSource::SvxTextEditSource( SdrObject* pObject ) :
    pObj            ( pObject ),
    pOutliner       ( NULL ),
    pTextForwarder  ( NULL ),
    bDataValid      ( FALSE )
{
    DBG_ASSERT( pObj, "pObject muss gueltig sein" );

    StartListening( *pObj->GetModel() );
}

//------------------------------------------------------------------------
SvxTextEditSource::~SvxTextEditSource()
{
    if( pObj )
        EndListening( *pObj->GetModel() );

    delete pTextForwarder;
    delete pOutliner;
}

//------------------------------------------------------------------------
SvxEditSource* SvxTextEditSource::Clone() const
{
    return new SvxTextEditSource( pObj );
}

//------------------------------------------------------------------------
SvxTextForwarder* SvxTextEditSource::GetTextForwarder()
{
    if (!pTextForwarder)
    {
        if( pOutliner == NULL )
        {
            SdrModel* pModel = pObj->GetModel();
            pOutliner = SdrMakeOutliner( OUTLINERMODE_OUTLINEOBJECT, pModel );
            Outliner& aDrawOutliner = pModel->GetDrawOutliner();
            pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
        }

        pTextForwarder = new SvxOutlinerForwarder( *pOutliner );
    }

    if( pObj && !bDataValid )
    {
        OutlinerParaObject* pOutlinerParaObject = pObj->GetOutlinerParaObject();
        if( pOutlinerParaObject )
        {
            pOutliner->SetText( *pOutlinerParaObject );
        }

        bDataValid = TRUE;
    }

    return pTextForwarder;
}

//------------------------------------------------------------------------
void SvxTextEditSource::UpdateData()
{
    if( pOutliner )
    {
        OutlinerParaObject* pOutlinerParaObject = pOutliner->CreateParaObject();
        pObj->SetOutlinerParaObject( pOutlinerParaObject );
    }
}

//------------------------------------------------------------------------
void SvxTextEditSource::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

    BOOL bDestroyed = FALSE;

    if( pSdrHint )
    {
        if( pSdrHint->GetKind() == HINT_OBJCHG )
            bDataValid = FALSE;                     // Text muss neu geholt werden
        if( pSdrHint->GetKind() == HINT_OBJREMOVED )
        {
            if( pObj == pSdrHint->GetObject() )
            {
                pObj = NULL;
                bDestroyed = TRUE;
            }
        }
        else if( pSdrHint->GetKind() == HINT_MODELCLEARED ||
                 pSdrHint->GetKind() == HINT_OBJLISTCLEARED )
        {
            if( pObj )
                EndListening( *pObj->GetModel() );
            pObj = NULL;
            bDestroyed = TRUE;
        }
    }

    if( bDestroyed )
    {
        delete pTextForwarder;
        delete pOutliner;
        pOutliner = NULL;

        pTextForwarder = new SvxDummyTextSource();
    }

}




