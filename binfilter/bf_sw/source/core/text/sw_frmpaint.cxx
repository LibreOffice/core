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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif



#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx> // SwPageDesc
#endif

#ifndef _ITRPAINT_HXX
#include <itrpaint.hxx>     // SwTxtPainter
#endif

#ifndef _HORIORNT_HXX
#include <horiornt.hxx>
#endif

namespace binfilter {


// steht im number.cxx
extern const sal_Char __FAR_DATA sBulletFntName[];

extern FASTBOOL bOneBeepOnly;

sal_Bool bInitFont = sal_True;

#define REDLINE_DISTANCE 567/4
#define REDLINE_MINDIST  567/10


/*************************************************************************
 * SwExtraPainter::PaintExtra()
 **************************************************************************/




/*************************************************************************
 *                      SwTxtFrm::Paint()
 *************************************************************************/

/*N*/ SwRect SwTxtFrm::Paint()
/*N*/ {
/*N*/ #if OSL_DEBUG_LEVEL > 1
/*N*/ 	const SwTwips nDbgY = Frm().Top();
/*N*/ #endif
/*N*/ 
/*N*/ 	// finger layout
/*N*/ 	ASSERT( GetValidPosFlag(), "+SwTxtFrm::Paint: no Calc()" );
/*N*/ 
/*N*/ 	SwRect aRet( Prt() );
/*N*/ 	if ( IsEmpty() || !HasPara() )
/*N*/ 		aRet += Frm().Pos();
/*N*/ 	else
/*N*/ 	{
/*N*/ 		// AMA: Wir liefern jetzt mal das richtige Repaintrechteck zurueck,
/*N*/ 		// 		d.h. als linken Rand den berechneten PaintOfst!
/*N*/ 		SwRepaint *pRepaint = GetPara()->GetRepaint();
/*N*/ 		long l;
/*N*/ 		if( pRepaint->GetOfst() )
/*N*/ 			pRepaint->Left( pRepaint->GetOfst() );
/*N*/ 
/*N*/ 		l = pRepaint->GetRightOfst();
/*N*/ 		if( l && ( pRepaint->GetOfst() || l > pRepaint->Right() ) )
/*N*/ 			 pRepaint->Right( l );
/*N*/ 		pRepaint->SetOfst( 0 );
/*N*/ 		aRet = *pRepaint;
/*N*/ 
/*N*/ #ifdef BIDI
/*N*/         if ( IsRightToLeft() )
                {DBG_BF_ASSERT(0, "STRIP");} //STRIP001 /*?*/             SwitchLTRtoRTL( aRet );
/*N*/ #endif
/*N*/         if ( IsVertical() )
/*N*/             SwitchHorizontalToVertical( aRet );
/*N*/ 	}
/*N*/ 	ResetRepaint();
/*N*/ 
/*N*/     return aRet;
/*N*/ }

/*************************************************************************
 *                      SwTxtFrm::Paint()
 *************************************************************************/


/*************************************************************************
 *                      SwTxtFrm::Paint()
 *************************************************************************/


}
