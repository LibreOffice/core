/*************************************************************************
 *
 *  $RCSfile: parse.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: np $Date:  $
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


#include <parse.hxx>

#include <string.h>
#include <ostream.h>
#include <xmltree.hxx>


// NOT FULLY DEFINED SERVICES

char  C_sMODULEDESCRIPTION[] = "module-description";
char  C_sCOMPONENTDESCRIPTION[] = "component-description";
char  C_sAuthor[] = "author";
char  C_sName[] = "name";
char  C_sDescription[] = "description";
char  C_sReferenceDocu[] = "reference-docu";
char  C_sModuleName[] = "module-name";
char  C_sLoaderName[] = "loader-name";
char  C_sSupportedService[] = "supported-service";
char  C_sServiceDependency[] = "service-dependency";
char  C_sProjectBuildDependency[] = "project-build-dependency";
char  C_sRuntimeModuleDependency[] = "runtime-module-dependency";
char  C_sLanguage[] = "language";
char  C_sStatus[] = "status";
char  C_sType[] = "type";



#define AssertionOf(x)  \
    {if (!(x)) {cerr << "Assertion failed: " << #x << __FILE__ << __LINE__ << endl; exit(3); }}
#define SyntaxAssertion(x,file) \
    {if (!(x)) {cerr << "Syntax error in file: " << #file << endl; exit(3); }}



X2CParser::X2CParser( ModuleDescription & o_rData )
    :   pData(&o_rData)
{
}

X2CParser::~X2CParser()
{
}

bool
X2CParser::Parse( const char * i_sFilename )
{
    // Load file:
    if ( ! LoadXmlFile( aFile, i_sFilename ) )
        return false;

    // Test correct end:
    const char * pLastTag = strrchr(aFile.operator const char *(),'<');
    if (pLastTag == 0)
        return false;
    if ( strnicmp(pLastTag+2, C_sMODULEDESCRIPTION, strlen(C_sMODULEDESCRIPTION)) != 0
         || strnicmp(pLastTag, "</", 2) != 0 )
        return false;
    if (strchr(pLastTag,'>') == 0)
        return false;

    // Parse:
    text = aFile.operator const char *();

    Parse_XmlDeclaration();
    Parse_Doctype();
    Parse_ModuleDescription();

    return true;
}

void
X2CParser::Parse_XmlDeclaration()
{
    Goto('<');
    if ( IsText("<?xml") )
    {
        Goto_And_Pass('>');
    }
}

void
X2CParser::Parse_Doctype()
{
    Goto('<');
    if ( IsText("<!DOCTYPE") )
        Goto_And_Pass('>');
}

void
X2CParser::Parse_ModuleDescription()
{
    Goto('<');
    AssertionOf( IsBeginTag(C_sMODULEDESCRIPTION) );
    Goto_And_Pass('>');

    Parse_TextElement(C_sModuleName,*pData, true, false);

    for ( Goto('<');
          IsBeginTag(C_sCOMPONENTDESCRIPTION);
          Goto('<') )
    {
        Parse_ComponentDescription(*pData);
    }

    for ( ; ! IsEndTag(C_sMODULEDESCRIPTION); Goto('<') )
    {
        if ( IsBeginTag(C_sProjectBuildDependency) )
            Parse_TextElement(C_sProjectBuildDependency,*pData,false,false);
        else if ( IsBeginTag(C_sRuntimeModuleDependency) )
            Parse_TextElement(C_sRuntimeModuleDependency,*pData,false,false);
    }
    Goto_And_Pass('>');
}

void
X2CParser::Parse_ComponentDescription(ModuleDescription & o_rParent)
{
    Goto('<');
    AssertionOf( IsBeginTag(C_sCOMPONENTDESCRIPTION) );
    Goto_And_Pass('>');

    ComponentDescription * dpCD = new ComponentDescription;
    o_rParent.AddComponentDescription(*dpCD);

    Parse_TextElement(C_sAuthor,*dpCD,true,false);
    Parse_TextElement(C_sName,*dpCD,true,true);
    Parse_TextElement(C_sDescription,*dpCD,true,false);
    Parse_TextElement(C_sLoaderName,*dpCD,true,true);
    Parse_TextElement(C_sLanguage,*dpCD,true,false);
    Parse_Status(*dpCD);
    for ( Goto('<');
          IsBeginTag(C_sSupportedService);
          Goto('<') )
    {
        Parse_TextElement(C_sSupportedService,*dpCD,false,true);
    }

    for ( Goto('<');
          ! IsEndTag(C_sCOMPONENTDESCRIPTION);
          Goto('<') )
    {
        if ( IsBeginTag(C_sReferenceDocu) )
        {
            ReferenceDocuElement * pNewElem = new ReferenceDocuElement;
            Parse_ReferenceDocu(*pNewElem);
            dpCD->DocuRefs().push_back(pNewElem);
        }
        else if ( IsBeginTag(C_sServiceDependency) )
            Parse_TextElement(C_sServiceDependency,*dpCD,false,true);
        else if ( IsBeginTag(C_sType) )
            Parse_TextElement(C_sType,*dpCD,false,true);
    }
    Goto_And_Pass('>');

    // Take Name to front:
    ParentElement::ChildList & rDescrElements = dpCD->Children();
    TextElement * pTE = rDescrElements[0];
    rDescrElements[0] = rDescrElements[1];
    rDescrElements[1] = pTE;
}

void
X2CParser::Parse_ReferenceDocu(ReferenceDocuElement & o_rElement)
{
    Move( strlen(C_sReferenceDocu) + 1 );

    for ( Pass_White(); *text != '/'; Pass_White() )
    {
        Get_ReferenceDocuAttribute(o_rElement);
    }
    Move(1);
}

void
X2CParser::Parse_TextElement( const char *    i_sElementName,
                              ParentElement & o_rParent,
                              bool            i_bSingle,
                              bool            i_bReverseName )
{
    Goto('<');
    text++;
    AssertionOf( IsText(i_sElementName) );
    Goto_And_Pass('>');
    Pass_White();

    TextElement * pNew = 0;

    if ( !i_bSingle && o_rParent.Children().size() > 0 )
    {
        if ( strcmp( o_rParent.Children().back()->Name(),
                     i_sElementName ) == 0 )
        {
            pNew = o_rParent.Children().back();
        }
    }
    if (pNew == 0)
    {
        pNew = i_bSingle
                    ?   (TextElement*) new SglTextElement(i_sElementName)
                    :   (TextElement*) new MultipleTextElement(i_sElementName);
        o_rParent.AddChild(*pNew);
    }

    // Add new Element
    Simstr sText;
    GetTextTill( sText, '<', i_bReverseName );
    sText.remove_trailing_blanks();
    pNew->SetText(sText);

    AssertionOf( IsText("</") );
    Move(2);
    AssertionOf( IsText(i_sElementName) );
    Goto_And_Pass('>');
}

void
X2CParser::Parse_Status( ComponentDescription & o_rParent )
{
    Goto('<');
    AssertionOf( IsBeginTag(C_sStatus) );
    Move( strlen(C_sStatus) + 1 );

    Pass_White();
    AssertionOf(*text != '>');
    Simstr sAttrName;
    Simstr sAttrValue = Get_Attribute(sAttrName);
    o_rParent.SetStatus(sAttrValue);

    Move(2);
}

void
X2CParser::Get_ReferenceDocuAttribute( ReferenceDocuElement & o_rElement )
{
    Simstr sAttrName;
    Simstr sAttrValue = Get_Attribute(sAttrName);
    if ( strcmp( sAttrName.str(), "xlink:href") == 0 )
        o_rElement.sAttr_href = sAttrValue;
    else if ( strcmp( sAttrName.str(), "xlink:role") == 0 )
        o_rElement.sAttr_role = sAttrValue;
    else if ( strcmp( sAttrName.str(), "xlink:title") == 0 )
        o_rElement.sAttr_title = sAttrValue;
}

Simstr
X2CParser::Get_Attribute( Simstr & o_rAttrName )
{
    GetTextTill( o_rAttrName, '=');

    while (*(++text) != '"')
    {
        AssertionOf(*text != '\0');
    }

    ++text;
    Simstr ret;
    GetTextTill( ret, '"');
    ++text;
    return ret;
}

bool
X2CParser::IsText( const char * i_sComparedText )
{
    return strnicmp( text, i_sComparedText, strlen(i_sComparedText) ) == 0;
}

bool
X2CParser::IsBeginTag( const char * i_sTagName )
{
    return strnicmp( text+1, i_sTagName, strlen(i_sTagName) ) == 0
           && *text == '<';
}

bool
X2CParser::IsEndTag( const char * i_sTagName )
{
    return strnicmp( text+2, i_sTagName, strlen(i_sTagName) ) == 0
           && strnicmp( text, "</", 2 ) == 0;
}

void
X2CParser::Goto( char i_cNext )
{
    while (*text != i_cNext)
    {
        ++text;
        AssertionOf(*text != '\0');
    }
}

void
X2CParser::Goto_And_Pass( char i_cNext )
{
    Goto(i_cNext);
    ++text;
}

void
X2CParser::Move( int i_nForward )
{
    text += i_nForward;
}

void
X2CParser::Pass_White()
{
    while (*text <= 32)
    {
        ++text;
        AssertionOf(*text != '\0');
    }
}

void
X2CParser::GetTextTill( Simstr & o_rText,
                        char     i_cEnd,
                        bool     i_bReverseName )
{
    char * pResult = &sWord[0];

    for ( char * pSet = pResult;
          *text != i_cEnd;
          ++text )
    {
        AssertionOf(*text !='\0');
        *pSet++ = *text;
    }

    while ( *pResult < 33 && *pResult > 0 )
        ++pResult;
    while ( pSet > pResult ? *(pSet-1) < 33 : false )
        pSet--;
    *pSet = '\0';


    if (i_bReverseName)
    {
        char * sBreak = strrchr(pResult,'.');
        if (sBreak != 0)
        {
            static char sScope[1000];
            static char sName[1000];

            unsigned nScopeLen = sBreak - pResult;
            strncpy ( sScope, pResult, nScopeLen );
            sScope[nScopeLen] = '\0';
            strcpy( sName, sBreak + 1 );
            strcat( sName, " in " );
            strcat( sName, sScope );

            o_rText = sName;
            return;
        }
    }

    o_rText = &sWord[0];
}


