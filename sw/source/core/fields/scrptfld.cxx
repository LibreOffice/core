/*************************************************************************
 *
 *  $RCSfile: scrptfld.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:19 $
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

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#include "docufld.hxx"

#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;
/*--------------------------------------------------------------------
    Beschreibung: ScriptField
 --------------------------------------------------------------------*/

SwScriptFieldType::SwScriptFieldType( SwDoc* pD )
    : SwFieldType( RES_SCRIPTFLD ), pDoc( pD )
{}

SwFieldType* SwScriptFieldType::Copy() const
{
    return new SwScriptFieldType( pDoc );
}


/*--------------------------------------------------------------------
    Beschreibung: SwScriptField
 --------------------------------------------------------------------*/

SwScriptField::SwScriptField( SwScriptFieldType* pType,
                                const String& rType, const String& rCode,
                                BOOL bURL )
    : SwField( pType ), sType( rType ), sCode( rCode ), bCodeURL( bURL )
{
}

String SwScriptField::Expand() const
{
    return aEmptyStr;
}

SwField* SwScriptField::Copy() const
{
    return new SwScriptField( (SwScriptFieldType*)GetTyp(), sType, sCode, bCodeURL );
}

/*--------------------------------------------------------------------
    Beschreibung: Type setzen
 --------------------------------------------------------------------*/

void SwScriptField::SetPar1( const String& rStr )
{
    sType = rStr;
}

const String& SwScriptField::GetPar1() const
{
    return sType;
}

/*--------------------------------------------------------------------
    Beschreibung: Code setzen
 --------------------------------------------------------------------*/

void SwScriptField::SetPar2( const String& rStr )
{
    sCode = rStr;
}


String SwScriptField::GetPar2() const
{
    return sCode;
}
/*-----------------05.03.98 15:00-------------------

--------------------------------------------------*/
BOOL SwScriptField::QueryValue( uno::Any& rAny, const String& rProperty ) const
{
    if(rProperty.EqualsAscii(UNO_NAME_URL_CONTENT))
        rAny.setValue(&bCodeURL, ::getBooleanCppuType());
    else if(rProperty.EqualsAscii(UNO_NAME_SCRIPT_TYPE))
        rAny <<= OUString(sType);
    else if( rProperty.EqualsAscii(UNO_NAME_CONTENT))
         rAny <<= OUString(sCode);
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;

}
/*-----------------05.03.98 15:00-------------------

--------------------------------------------------*/
BOOL SwScriptField::PutValue( const uno::Any& rAny, const String& rProperty )
{
    if(rProperty.EqualsAscii(UNO_NAME_URL_CONTENT))
        bCodeURL = *(sal_Bool*)rAny.getValue();
    else if(rProperty.EqualsAscii(UNO_NAME_SCRIPT_TYPE))
    {
        OUString uTmp;
        rAny >>= uTmp;
        sType = String(uTmp);
    }
    else if( rProperty.EqualsAscii(UNO_NAME_CONTENT))
    {
        OUString uTmp;
        rAny >>= uTmp;
          sCode = String(uTmp);
    }
#ifdef DBG_UTIL
    else
        DBG_ERROR("Welches Property?")
#endif
    return TRUE;
}

