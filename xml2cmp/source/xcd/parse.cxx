/*************************************************************************
 *
 *  $RCSfile: parse.cxx,v $
 *
 *  $Revision: 1.3 $
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
#include <iostream>
#include <xmlelem.hxx>

#ifdef UNX
#define strnicmp strncasecmp
#endif


using namespace std;


// NOT FULLY DEFINED SERVICES



#define AssertionOf(x)  \
    {if (!(x)) {cerr << "Assertion failed: " << #x << __FILE__ << __LINE__ << std::endl; exit(3); }}



X2CParser::X2CParser( XmlElement & o_rDocumentData )
    :   // sFileName,
        nFileLine(0),
        pDocumentData(&o_rDocumentData),
        // sWord,
        text(0)
{
}

X2CParser::~X2CParser()
{
}


bool
X2CParser::LoadFile( const char * i_sFilename )
{
    sFileName = i_sFilename;
    nFileLine = 1;

    // Load file:
    if ( ! LoadXmlFile( aFile, i_sFilename ) )
        return false;

    // Test correct end:
    const char * pLastTag = strrchr(aFile.operator const char *(),'<');
    if (pLastTag == 0)
        return false;
    if ( strnicmp(pLastTag+2, pDocumentData->Name().str(), pDocumentData->Name().l()) != 0
         || strnicmp(pLastTag, "</", 2) != 0 )
        return false;
    if (strchr(pLastTag,'>') == 0)
        return false;
    return true;
}

void
X2CParser::Parse()
{
    // Parse:
    text = aFile.operator const char *();

    Parse_XmlDeclaration();
    Parse_Doctype();

    pDocumentData->Parse(*this);
}

bool
X2CParser::Parse( const char * i_sFilename )
{
    bool ret = LoadFile(i_sFilename);
    if (ret)
        Parse();
    return ret;
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
X2CParser::Parse_Sequence( DynamicList<XmlElement> & o_rElements,
                           const Simstr &            i_sElementName  )
{
    CheckAndPassBeginTag(i_sElementName.str());

    unsigned int i_max = o_rElements.size();
    for (unsigned i = 0; i < i_max; ++i)
    {
        o_rElements[i]->Parse(*this);
    }  // end for

    CheckAndPassEndTag(i_sElementName.str());
}

void
X2CParser::Parse_FreeChoice( DynamicList<XmlElement> & o_rElements )
{
    unsigned        nSize = o_rElements.size();

    for ( bool bBreak = false;  !bBreak; )
    {
        bBreak = true;
        for ( unsigned i = 0; i < nSize; ++i )
        {
            Goto('<');
            if ( IsBeginTag(o_rElements[i]->Name().str()) )
            {
                o_rElements[i]->Parse(*this);
                bBreak = false;
                break;
            }
        }   // end for i
    }   // end for !bBreak
}

void
X2CParser::Parse_List( ListElement &  o_rListElem )
{

    for ( Goto('<'); IsBeginTag(o_rListElem.Name().str()); Goto('<') )
    {
        XmlElement * pNew = o_rListElem.Create_and_Add_NewElement();
        pNew->Parse(*this);
    }
}

void
X2CParser::Parse_Text( Simstr &         o_sText,
                       const Simstr &   i_sElementName,
                       bool             i_bReverseName )
{
    CheckAndPassBeginTag(i_sElementName.str());

    // Add new Element
    GetTextTill( o_sText, '<', i_bReverseName );
    o_sText.remove_trailing_blanks();

    CheckAndPassEndTag(i_sElementName.str());
}

void
X2CParser::Parse_MultipleText( List<Simstr> &   o_rTexts,
                               const Simstr &   i_sElementName,
                               bool             i_bReverseName )
{
    for ( Goto('<'); IsBeginTag(i_sElementName.str()); Goto('<') )
    {
        o_rTexts.push_back(Simstr());
        Parse_Text(o_rTexts.back(), i_sElementName, i_bReverseName);
    }
}

void
X2CParser::Parse_SglAttr( Simstr &          o_sAttrValue,
                          const Simstr &    i_sElementName,
                          const Simstr &    i_sAttrName )
{
    Goto('<');
    if ( !IsBeginTag(i_sElementName.str()) )
        SyntaxError("unexpected element");
    Move( i_sElementName.l() + 1 );

    Pass_White();
    if (*text == '>')
        SyntaxError("no attribute found, where one was expected");
    Simstr sAttrName;
    Get_Attribute(o_sAttrValue, sAttrName);
    if (sAttrName != i_sAttrName)
        SyntaxError("unknown attribute found");
    Pass_White();
    if (strncmp(text,"/>",2) != 0)
        SyntaxError("missing \"/>\" at end of empty element");
    Move(2);
}

void
X2CParser::Parse_MultipleAttr( List<Simstr> &       o_rAttrValues,
                               const Simstr &       i_sElementName,
                               const List<Simstr> & i_rAttrNames )
{
    Goto('<');
    if ( !IsBeginTag(i_sElementName.str()) )
        SyntaxError("unexpected element");
    Move( i_sElementName.l() + 1 );
    Simstr sAttrName;
    Simstr sAttrValue;
    unsigned nSize = i_rAttrNames.size();

    for ( Pass_White(); *text != '/'; Pass_White() )
    {

        Get_Attribute(sAttrValue, sAttrName);

        for ( unsigned i = 0; i < nSize; ++i )
        {
            if ( i_rAttrNames[i] == sAttrName )
            {
                o_rAttrValues[i] = sAttrValue;
                break;
            }
        }
        if (i == nSize)
            SyntaxError("unknown attribute found");
    }
    Move(2);
}


void
X2CParser::Get_Attribute( Simstr & o_rAttrValue,
                          Simstr & o_rAttrName )
{
    GetTextTill( o_rAttrName, '=');

    while (*(++text) != '"')
    {
        if (*text == '\0')
            SyntaxError("unexpected end of file");
    }

    ++text;
    GetTextTill( o_rAttrValue, '"');
    ++text;
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
        TestCurChar();
        ++text;
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
        TestCurChar();
        ++text;
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
        TestCurChar();
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

void
X2CParser::CheckAndPassBeginTag( const char * i_sElementName )
{
    Goto('<');
    if ( ! IsBeginTag(i_sElementName) )
        SyntaxError( "unexpected element");
    Goto_And_Pass('>');
    Pass_White();

}

void
X2CParser::CheckAndPassEndTag( const char * i_sElementName )
{
    Pass_White();
    if ( !IsEndTag(i_sElementName) )
        SyntaxError("missing or not matching end tag");
    Goto_And_Pass('>');
}

void
X2CParser::SyntaxError( const char * i_sText )
{
    cerr << "Syntax error "
         << i_sText
         << " in file: "
         << sFileName.str()
         << " in line "
         << nFileLine
         << "."
         << endl;

     exit(3);
}

void
X2CParser::TestCurChar()
{
//  if (*text == '\0')
//      SyntaxError("unexpected end of file");
//  else

    if (*text == '\n')
        nFileLine++;
}


