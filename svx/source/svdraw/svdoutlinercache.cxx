/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdoutlinercache.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:37:56 $
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


#include "svdoutlinercache.hxx"
#include "svdoutl.hxx"
#include "svdmodel.hxx"

extern SdrOutliner* SdrMakeOutliner( USHORT nOutlinerMode, SdrModel* pModel );

SdrOutlinerCache::SdrOutlinerCache( SdrModel* pModel )
:   mpModel( pModel ),
    mpModeOutline( NULL ),
    mpModeText( NULL )
{
}

SdrOutliner* SdrOutlinerCache::createOutliner( sal_uInt16 nOutlinerMode )
{
    SdrOutliner* pOutliner = NULL;

    if( (OUTLINERMODE_OUTLINEOBJECT == nOutlinerMode) && mpModeOutline )
    {
        pOutliner = mpModeOutline;
        mpModeOutline = NULL;
    }
    else if( (OUTLINERMODE_TEXTOBJECT == nOutlinerMode) && mpModeText )
    {
        pOutliner = mpModeText;
        mpModeText = NULL;
    }
    else
    {
        pOutliner = SdrMakeOutliner( nOutlinerMode, mpModel );
        Outliner& aDrawOutliner = mpModel->GetDrawOutliner();
        pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
    }

    return pOutliner;
}

SdrOutlinerCache::~SdrOutlinerCache()
{
    if( mpModeOutline )
    {
        delete mpModeOutline;
        mpModeOutline = NULL;
    }

    if( mpModeText )
    {
        delete mpModeText;
        mpModeText = NULL;
    }
}

void SdrOutlinerCache::disposeOutliner( SdrOutliner* pOutliner )
{
    if( pOutliner )
    {
        USHORT nOutlMode = pOutliner->GetOutlinerMode();

        if( (OUTLINERMODE_OUTLINEOBJECT == nOutlMode) && (NULL == mpModeOutline) )
        {
            mpModeOutline = pOutliner;
            pOutliner->Clear();
            pOutliner->SetVertical( false );
        }
        else if( (OUTLINERMODE_TEXTOBJECT == nOutlMode) && (NULL == mpModeText) )
        {
            mpModeText = pOutliner;
            pOutliner->Clear();
            pOutliner->SetVertical( false );
        }
        else
        {
            delete pOutliner;
        }
    }
}


