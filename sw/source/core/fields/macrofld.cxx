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

#include <hintids.hxx>
#include <doc.hxx>
#include <docufld.hxx>
#include <unofldmid.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <comphelper/processfactory.hxx>


using namespace ::com::sun::star;
using ::rtl::OUString;

/*--------------------------------------------------------------------
    Beschreibung: MacroFeldtypen
 --------------------------------------------------------------------*/

SwMacroFieldType::SwMacroFieldType(SwDoc* pDocument)
    : SwFieldType( RES_MACROFLD ),
      pDoc(pDocument)
{
}

SwFieldType* SwMacroFieldType::Copy() const
{
    SwMacroFieldType* pType = new SwMacroFieldType(pDoc);
    return pType;
}

/*--------------------------------------------------------------------
    Beschreibung: Das Macrofeld selbst
 --------------------------------------------------------------------*/

SwMacroField::SwMacroField(SwMacroFieldType* pInitType,
                           const String& rLibAndName, const String& rTxt) :
    SwField(pInitType), aMacro(rLibAndName), aText(rTxt), bIsScriptURL(sal_False)
{
    bIsScriptURL = isScriptURL(aMacro);
}

String SwMacroField::Expand() const
{   // Button malen anstatt von
    return aText ;
}

SwField* SwMacroField::Copy() const
{
    return new SwMacroField((SwMacroFieldType*)GetTyp(), aMacro, aText);
}

String SwMacroField::GetFieldName() const
{
    String aStr(GetTyp()->GetName());
    aStr += ' ';
    aStr += aMacro;
    return aStr;
}

String SwMacroField::GetLibName() const
{
    // if it is a Scripting Framework macro return an empty string
    if (bIsScriptURL)
    {
        return String();
    }

    if (aMacro.Len())
    {
        sal_uInt16 nPos = aMacro.Len();

        for (sal_uInt16 i = 0; i < 3 && nPos > 0; i++)
            while (aMacro.GetChar(--nPos) != '.' && nPos > 0) ;

        return aMacro.Copy(0, nPos );
    }

    DBG_ASSERT(0, "Kein Macroname vorhanden");
    return aEmptyStr;
}

String SwMacroField::GetMacroName() const
{
    if (aMacro.Len())
    {
        if (bIsScriptURL)
        {
            return aMacro.Copy( 0 );
        }
        else
        {
            sal_uInt16 nPos = aMacro.Len();

            for (sal_uInt16 i = 0; i < 3 && nPos > 0; i++)
                while (aMacro.GetChar(--nPos) != '.' && nPos > 0) ;

            return aMacro.Copy( ++nPos );
        }
    }

    DBG_ASSERT(0, "Kein Macroname vorhanden");
    return aEmptyStr;
}

SvxMacro SwMacroField::GetSvxMacro() const
{
  if (bIsScriptURL)
    {
        return SvxMacro(aMacro, String(), EXTENDED_STYPE);
    }
    else
    {
        return SvxMacro(GetMacroName(), GetLibName(), STARBASIC);
    }
}

/*--------------------------------------------------------------------
    Beschreibung: LibName und MacroName
 --------------------------------------------------------------------*/

void SwMacroField::SetPar1(const String& rStr)
{
    aMacro = rStr;
    bIsScriptURL = isScriptURL(aMacro);
}

const String& SwMacroField::GetPar1() const
{
    return aMacro;
}

/*--------------------------------------------------------------------
    Beschreibung: Macrotext
 --------------------------------------------------------------------*/

void SwMacroField::SetPar2(const String& rStr)
{
    aText = rStr;
}

String SwMacroField::GetPar2() const
{
    return aText;
}

/*-----------------05.03.98 13:38-------------------

--------------------------------------------------*/
sal_Bool SwMacroField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= OUString(GetMacroName());
        break;
    case FIELD_PROP_PAR2:
        rAny <<= OUString(aText);
        break;
    case FIELD_PROP_PAR3:
        rAny <<= OUString(GetLibName());
        break;
    case FIELD_PROP_PAR4:
        rAny <<= bIsScriptURL ? OUString(GetMacroName()): OUString();
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return sal_True;
}
/*-----------------05.03.98 13:38-------------------

--------------------------------------------------*/
sal_Bool SwMacroField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    String sTmp;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        CreateMacroString( aMacro, ::GetString(rAny, sTmp), GetLibName());
        break;
    case FIELD_PROP_PAR2:
        ::GetString( rAny, aText );
        break;
    case FIELD_PROP_PAR3:
        CreateMacroString(aMacro, GetMacroName(), ::GetString(rAny, sTmp) );
        break;
    case FIELD_PROP_PAR4:
        ::GetString(rAny, aMacro);
        bIsScriptURL = isScriptURL(aMacro);
        break;
    default:
        DBG_ERROR("illegal property");
    }

    return sal_True;
}

// create an internally used macro name from the library and macro name parts
void SwMacroField::CreateMacroString(
    String& rMacro,
    const String& rMacroName,
    const String& rLibraryName )
{
    // concatenate library and name; use dot only if both strings have content
    rMacro = rLibraryName;
    if ( rLibraryName.Len() > 0 && rMacroName.Len() > 0 )
        rMacro += '.';
    rMacro += rMacroName;
}

sal_Bool SwMacroField::isScriptURL( const String& str )
{
    uno::Reference< lang::XMultiServiceFactory > xSMgr =
        ::comphelper::getProcessServiceFactory();

    uno::Reference< uri::XUriReferenceFactory >
        xFactory( xSMgr->createInstance(
            OUString::createFromAscii(
                "com.sun.star.uri.UriReferenceFactory" ) ), uno::UNO_QUERY );

    if ( xFactory.is() )
    {
        uno::Reference< uri::XVndSunStarScriptUrl >
            xUrl( xFactory->parse( str ), uno::UNO_QUERY );

        if ( xUrl.is() )
        {
            return sal_True;
        }
    }
    return sal_False;
}
