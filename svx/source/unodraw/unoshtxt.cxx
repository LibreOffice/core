/*************************************************************************
 *
 *  $RCSfile: unoshtxt.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-13 15:13:37 $
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
#include "svdotext.hxx"
#include "svdpage.hxx"
#include "editeng.hxx"

#include "unotext.hxx"

//------------------------------------------------------------------------

SvxTextEditSource::SvxTextEditSource( SdrObject* pObject ) :
    pObj            ( pObject ),
    pOutliner       ( NULL ),
    pTextForwarder  ( NULL ),
    bDataValid      ( FALSE ),
    bDestroyed      ( FALSE )
{
    DBG_ASSERT( pObj, "invalid pObject!" );

    if( pObj )
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
    if( bDestroyed || pObj == NULL )
        return NULL;

    if (!pTextForwarder)
    {
        if( pOutliner == NULL )
        {
            SdrTextObj* pTextObj = PTR_CAST( SdrTextObj, pObj );
            USHORT nOutlMode = OUTLINERMODE_TEXTOBJECT;
            if( pTextObj && pTextObj->IsTextFrame() && pTextObj->GetTextKind() == OBJ_OUTLINETEXT )
                nOutlMode = OUTLINERMODE_OUTLINEOBJECT;
            SdrModel* pModel = pObj->GetModel();
            pOutliner = SdrMakeOutliner( nOutlMode, pModel );
            Outliner& aDrawOutliner = pModel->GetDrawOutliner();
            pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
        }

        pTextForwarder = new SvxOutlinerForwarder( *pOutliner );
    }

    if( pObj && !bDataValid )
    {
        OutlinerParaObject* pOutlinerParaObject = pObj->GetOutlinerParaObject();
        if( pOutlinerParaObject && (!pObj->IsEmptyPresObj() || pObj->GetPage()->IsMasterPage()) )
        {
            pOutliner->SetText( *pOutlinerParaObject );
        }
        else
        {
            // set objects style sheet on empty outliner
            SfxStyleSheetPool* pPool = (SfxStyleSheetPool*)pObj->GetModel()->GetStyleSheetPool();
            if( pPool )
                pOutliner->SetStyleSheetPool( pPool );

            SfxStyleSheet* pStyleSheet = pObj->GetPage()->GetTextStyleSheetForObject( pObj );
            if( pStyleSheet )
                pOutliner->SetStyleSheet( 0, pStyleSheet );
        }

        bDataValid = TRUE;
    }

    return pTextForwarder;
}

//------------------------------------------------------------------------
void SvxTextEditSource::UpdateData()
{
    if( pOutliner && pObj && !bDestroyed )
    {
        if( pOutliner->GetParagraphCount() != 1 || pOutliner->GetEditEngine().GetTextLen( 0 ) )
            pObj->SetOutlinerParaObject( pOutliner->CreateParaObject() );
        else
            pObj->SetOutlinerParaObject( NULL );

        if( pObj->IsEmptyPresObj() )
            pObj->SetEmptyPresObj(sal_False);
    }
}

//------------------------------------------------------------------------
void SvxTextEditSource::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SdrHint* pSdrHint = PTR_CAST( SdrHint, &rHint );

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

        pTextForwarder = NULL;
    }
}




