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


#include "svdoutlinercache.hxx"
#include "svdoutl.hxx"
#include "svdmodel.hxx"
namespace binfilter {

/*N*/ extern SdrOutliner* SdrMakeOutliner( USHORT nOutlinerMode, SdrModel* pModel );

/*N*/ SdrOutlinerCache::SdrOutlinerCache( SdrModel* pModel )
/*N*/ :	mpModel( pModel ),
/*N*/ 	mpModeOutline( NULL ),
/*N*/ 	mpModeText( NULL )
/*N*/ {
/*N*/ }

/*N*/ SdrOutliner* SdrOutlinerCache::createOutliner( sal_uInt16 nOutlinerMode )
/*N*/ {
/*N*/ 	SdrOutliner* pOutliner = NULL;
/*N*/ 
/*N*/ 	if( (OUTLINERMODE_OUTLINEOBJECT == nOutlinerMode) && mpModeOutline )
/*N*/ 	{
/*N*/ 		pOutliner = mpModeOutline;
/*N*/ 		mpModeOutline = NULL;
/*N*/ 	}
/*N*/ 	else if( (OUTLINERMODE_TEXTOBJECT == nOutlinerMode) && mpModeText )
/*N*/ 	{
/*N*/ 		pOutliner = mpModeText;
/*N*/ 		mpModeText = NULL;
/*N*/ 	}
/*N*/ 	else
/*N*/ 	{
/*N*/ 		pOutliner = SdrMakeOutliner( nOutlinerMode, mpModel );
/*N*/ 		Outliner& aDrawOutliner = mpModel->GetDrawOutliner();
/*N*/ 		pOutliner->SetCalcFieldValueHdl( aDrawOutliner.GetCalcFieldValueHdl() );
/*N*/ 	}
/*N*/ 
/*N*/ 	return pOutliner;
/*N*/ }

/*N*/ SdrOutlinerCache::~SdrOutlinerCache()
/*N*/ {
/*N*/ 	if( mpModeOutline )
/*N*/ 	{
/*N*/ 		delete mpModeOutline;
/*N*/ 		mpModeOutline = NULL;
/*N*/ 	}
/*N*/ 
/*N*/ 	if( mpModeText )
/*N*/ 	{
/*N*/ 		delete mpModeText;
/*N*/ 		mpModeText = NULL;
/*N*/ 	}
/*N*/ }

/*N*/ void SdrOutlinerCache::disposeOutliner( SdrOutliner* pOutliner )
/*N*/ {
/*N*/ 	if( pOutliner )
/*N*/ 	{
/*N*/ 		USHORT nOutlMode = pOutliner->GetOutlinerMode();
/*N*/ 
/*N*/ 		if( (OUTLINERMODE_OUTLINEOBJECT == nOutlMode) && (NULL == mpModeOutline) )
/*N*/ 		{
/*N*/ 			mpModeOutline = pOutliner;
/*N*/ 			pOutliner->Clear();
/*N*/ 			pOutliner->SetVertical( false );
/*N*/ 		}
/*N*/ 		else if( (OUTLINERMODE_TEXTOBJECT == nOutlMode) && (NULL == mpModeText) )
/*N*/ 		{
/*N*/ 			mpModeText = pOutliner;
/*N*/ 			pOutliner->Clear();
/*N*/ 			pOutliner->SetVertical( false );
/*N*/ 		}
/*N*/ 		else
/*N*/ 		{
/*N*/ 			delete pOutliner;
/*N*/ 		}
/*N*/ 	}
/*N*/ }


}
