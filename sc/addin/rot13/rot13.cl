/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rot13.cl,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 11:51:31 $
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

/* static char rot13_Id[]="@(#) StarCalc Rot13 AddIn (c) 1998-2000 Sun Microsystems, Inc."; */

#include <string.h>
#include <stdio.h>

#include <xlang.h>
#include <addin.h>
#include <rot13.hrc>

/**
 * the current language the Addin is using
 */
static USHORT _nLanguage=LANGUAGE_ENGLISH;

/**
 * StarCalc calls this function to set a new current Language for the Addin
 *
 * @param *nLanguage
 *
 */
void CALLTYPE SetLanguage( USHORT* nLanguage )
{
	_nLanguage = GetNeutralLanguage( *nLanguage );
}


/**
 * Tell StarCalc how many new functions this Addin provides.
 *
 * @param *nCount - returns the number of functions which are exported to StarCalc
 *
 */
void CALLTYPE GetFunctionCount( USHORT *nCount )
{
    *nCount = 1;
}

/**
 * Provides neccessary data for each new function to StarCalc
 *
 * @param *nNo Input: Function number between 0 and nCount - 1
 * @param *pFuncName Output: Functionname which should be called in the AddIn-DLL
 * @param *nParamCount Output: Number of Parameter. Must be greater than 0, because there's always a return-Value. Maximum is 16.
 * @param *peType Output: Pointer to arrray with exactly 16 variables of typ Paramtype. nParamCount Entries are set to the type of the corresponding Parameters.
 * @param *pInternalName Output: Functionname as seen by the Spreadsheet user
 *
 * @see #GetFunctionCount, #GetParameterDescription
 *
 */
void CALLTYPE GetFunctionData( USHORT *    nNo,
			       char *      pFuncName,
			       USHORT *    nParamCount,
			       ParamType * peType,
			       char *      pInternalName )
{

    switch( *nNo ) {
	case 0:
	    /* the function name is the same in all languages */
	    SO_StringCopy( pInternalName, "Rot13" );
	    SO_StringCopy( pFuncName,     "Rot13" );
	    peType[0] = PTR_STRING;
	    peType[1] = PTR_STRING;
	    *nParamCount=2;
	    break;
	 default:
	    *nParamCount    = 0;
	    *pFuncName     = 0;
	    *pInternalName = 0;
	    break;
    }
}

/**
 * Provides descriptions for each new function to StarCalc
 * which are shown is the autopilot
 *
 * @param *nNo Input Parameter, Function number between 0 and nCount - 1
 * @param *nParam Parameter Number
 * @param *pName Output: Name of the parameter
 * @param *pDesc Output: Description of the parameter
 *
 * @see #GetFunctionCount, #GetParameterDescription
 */
void CALLTYPE GetParameterDescription( USHORT* nNo, USHORT* nParam,
char* pName, char* pDesc )
{
	*pName = 0;
	*pDesc = 0;


	switch ( *nNo ) {
	case 0:
		switch ( *nParam ) {
		case 0:
			SO_StringCopy(pDesc,getText(ROT13_DESC));
			break;
		case 1:
			SO_StringCopy(pName,getText(ROT13_PAR1_NAME));
			SO_StringCopy(pDesc,getText(ROT13_PAR1_DESC));
		}
	}
}

/**
 * ROT13 Algorithm, each alphabetical character of the text is rotated by 13 in the alphabet
 *
 * @param *ret
 * @param *src
 *
 * ER: well, my personal favorite algorithm is
 * main(_){while(_=~getchar())putchar(~_-1/(~(_|32)/13*2-11)*13);}
 * but for clarification we do it somehow different here ;-)
 */
void CALLTYPE Rot13(char *ret, char *src)
{
    if ( ! ret ) return;
    if ( ! src ) *ret='\0';

    for(;src && *src; src++ , ret++) {
	*ret=*src;
	if (*ret >= 'A' && *ret <= 'Z') {
	    if ( (*ret +=13) > 'Z' ) *ret-=26;
	} else if (*ret >= 'a' && *ret < 'n') {
	    *ret +=13;
	} else if (*ret >= 'n' && *ret <= 'z') {
	    *ret -=13;
	}
    }
    *ret=*src;
}
