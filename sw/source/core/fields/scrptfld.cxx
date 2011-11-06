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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#include <docufld.hxx>
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#include <tools/resid.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;
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

SwScriptField::SwScriptField( SwScriptFieldType* pInitType,
                                const String& rType, const String& rCode,
                                sal_Bool bURL )
    : SwField( pInitType ), sType( rType ), sCode( rCode ), bCodeURL( bURL )
{
}

String SwScriptField::GetDescription() const
{
    return SW_RES(STR_SCRIPT);
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
sal_Bool SwScriptField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
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
    return sal_True;
}
/*-----------------05.03.98 15:00-------------------

--------------------------------------------------*/
sal_Bool SwScriptField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        ::GetString( rAny, sType );
        break;
    case FIELD_PROP_PAR2:
        ::GetString( rAny, sCode );
        break;
    case FIELD_PROP_BOOL1:
        bCodeURL = *(sal_Bool*)rAny.getValue();
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}

