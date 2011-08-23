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

#include <docufld.hxx>
#include <unofldmid.h>
namespace binfilter {

extern String& GetString( const ::com::sun::star::uno::Any& rAny, String& rStr ); //STRIP008

using namespace ::com::sun::star;
using namespace ::rtl;
/*--------------------------------------------------------------------
    Beschreibung: ScriptField
 --------------------------------------------------------------------*/

/*N*/ SwScriptFieldType::SwScriptFieldType( SwDoc* pD )
/*N*/ 	: SwFieldType( RES_SCRIPTFLD ), pDoc( pD )
/*N*/ {}

/*N*/ SwFieldType* SwScriptFieldType::Copy() const
/*N*/ {
        return new SwScriptFieldType( pDoc );
/*N*/ }


/*--------------------------------------------------------------------
    Beschreibung: SwScriptField
 --------------------------------------------------------------------*/

/*N*/ SwScriptField::SwScriptField( SwScriptFieldType* pType,
/*N*/ 								const String& rType, const String& rCode,
/*N*/ 								BOOL bURL )
/*N*/ 	: SwField( pType ), sType( rType ), sCode( rCode ), bCodeURL( bURL )
/*N*/ {
/*N*/ }

/*N*/ String SwScriptField::Expand() const
/*N*/ {
/*N*/ 	return aEmptyStr;
/*N*/ }

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
BOOL SwScriptField::QueryValue( uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= OUString( sType );
        break;
    case FIELD_PROP_PAR2:
        rAny <<= OUString( sCode );
        break;
    case FIELD_PROP_BOOL1:
        rAny.setValue(&bCodeURL, ::getBooleanCppuType());
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}
/*-----------------05.03.98 15:00-------------------

--------------------------------------------------*/
BOOL SwScriptField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        ::binfilter::GetString( rAny, sType );
        break;
    case FIELD_PROP_PAR2:
        ::binfilter::GetString( rAny, sCode );
        break;
    case FIELD_PROP_BOOL1:
        bCodeURL = *(sal_Bool*)rAny.getValue();
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
