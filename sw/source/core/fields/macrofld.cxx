/*************************************************************************
 *
 *  $RCSfile: macrofld.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 14:06:31 $
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


#pragma hdrstop

#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif

using namespace ::com::sun::star;
using namespace ::rtl;
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

SwMacroField::SwMacroField(SwMacroFieldType* pType,
                           const String& rLibAndName, const String& rTxt) :
    SwField(pType), aMacro(rLibAndName), aText(rTxt)
{
}

String SwMacroField::Expand() const
{   // Button malen anstatt von
    return aText ;
}

SwField* SwMacroField::Copy() const
{
    return new SwMacroField((SwMacroFieldType*)GetTyp(), aMacro, aText);
}

String SwMacroField::GetCntnt(BOOL bName) const
{
    if(bName)
    {
        String aStr(GetTyp()->GetName());
        aStr += ' ';
        aStr += aMacro;
        return aStr;
    }
    return Expand();
}

String SwMacroField::GetLibName() const
{
    if (aMacro.Len())
    {
        USHORT nPos = aMacro.Len();

        for (USHORT i = 0; i < 3 && nPos > 0; i++)
            while (aMacro.GetChar(--nPos) != '.' && nPos > 0);

        return aMacro.Copy(0, nPos );
    }

    DBG_ASSERT(0, "Kein Macroname vorhanden")
    return aEmptyStr;
}

String SwMacroField::GetMacroName() const
{
    if (aMacro.Len())
    {
        USHORT nPos = aMacro.Len();

        for (USHORT i = 0; i < 3 && nPos > 0; i++)
            while (aMacro.GetChar(--nPos) != '.' && nPos > 0);

        return aMacro.Copy( ++nPos );
    }

    DBG_ASSERT(0, "Kein Macroname vorhanden")
    return aEmptyStr;
}

/*--------------------------------------------------------------------
    Beschreibung: LibName und MacroName
 --------------------------------------------------------------------*/

void SwMacroField::SetPar1(const String& rStr)
{
    aMacro = rStr;
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
BOOL SwMacroField::QueryValue( uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
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
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}
/*-----------------05.03.98 13:38-------------------

--------------------------------------------------*/
BOOL SwMacroField::PutValue( const uno::Any& rAny, BYTE nMId )
{
    String sTmp;
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
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
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
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

