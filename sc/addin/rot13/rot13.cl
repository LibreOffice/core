/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/

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
