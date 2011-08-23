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


#ifdef _MSC_VER
#pragma hdrstop
#endif


#include "wrong.hxx"
namespace binfilter {

/*************************************************************************
 * sal_Bool SwWrongList::InWrongWord() gibt den Anfang und die Laenge des Wortes
 * zurueck, wenn es als falsch markiert ist.
 *************************************************************************/

/*************************************************************************
 * sal_Bool SwWrongList::Check() liefert den ersten falschen Bereich
 *************************************************************************/

/*************************************************************************
 * xub_StrLen SwWrongList::NextWrong() liefert die naechste Fehlerposition
 *************************************************************************/


/*************************************************************************
 * xub_StrLen SwWrongList::LastWrong() liefert die letzte Fehlerposition
 *************************************************************************/


/*************************************************************************
 *				   MSHORT SwWrongList::GetPos( xub_StrLen nValue )
 *  sucht die erste Position im Array, die groessergleich nValue ist,
 * dies kann natuerlich auch hinter dem letzten Element sein!
 *************************************************************************/


/*************************************************************************
 *				   void SwWrongList::Invalidate()
 *************************************************************************/

/*N*/ void SwWrongList::_Invalidate( xub_StrLen nBegin, xub_StrLen nEnd )
/*N*/ {
/*N*/ 	if ( nBegin < GetBeginInv() )
/*N*/ 		nBeginInvalid = nBegin;
/*N*/ 	if ( nEnd > GetEndInv() )
/*N*/ 		nEndInvalid = nEnd;
/*N*/ }

/*************************************************************************
 *				   		SwWrongList::Move( xub_StrLen nPos, long nDiff )
 *  veraendert alle Positionen ab nPos um den angegebenen Wert,
 *  wird nach Einfuegen oder Loeschen von Buchstaben benoetigt.
 *************************************************************************/


/*************************************************************************
 *				   		SwWrongList::Clear()/( xub_StrLen nBegin, xub_StrLen nEnd )
 *  loescht das Array im angegebenen Bereich
 *************************************************************************/



/*************************************************************************
 *                      SwWrongList::Fresh
 *
 * In this method the wrong list is updated, new wrong words are inserted,
 * and by evaluating the postiztion of wrong words, we also know, which
 * words are not wrong any longer and have to be removed.
 * Note: Fresh has to be called at the end of the check of the invalid region,
 * in order to find words, which are behind the last wrong word but not wrong
 * any longer
 *************************************************************************/

/*N*/ sal_Bool SwWrongList::InvalidateWrong( )
/*N*/ {
/*N*/ 	if( Count() )
/*N*/ 	{
/*N*/ 		xub_StrLen nFirst = WRPOS( 0 );
/*N*/ 		xub_StrLen nLast = WRPOS( Count() - 1 ) + WRLEN( Count() - 1 );
/*N*/ 		Invalidate( nFirst, nLast );
/*N*/ 		return sal_True;
/*N*/ 	}
/*N*/ 	else
/*N*/ 		return sal_False;
/*N*/ }



}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
