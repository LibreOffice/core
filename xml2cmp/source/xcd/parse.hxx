/*************************************************************************
 *
 *  $RCSfile: parse.hxx,v $
 *
 *  $Revision: 1.4 $
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

#ifndef X2C_PARSE_HXX
#define X2C_PARSE_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include "filebuff.hxx"
#include "../support/sistr.hxx"
#include "../support/list.hxx"
    // PARAMETERS


class XmlElement;
class ListElement;

class X2CParser
{
  public:
    typedef XmlElement * (*F_CREATE)(const Simstr &);

                        X2CParser(
                            XmlElement &        o_rDocumentData );
                        ~X2CParser();

    bool                LoadFile(
                            const char *        i_sFilename );
    void                Parse();
    bool                Parse(
                            const char *        i_sFilename );


    const char *        PureText() const        { return aFile.operator const char*(); }

    void                Parse_Sequence(
                           DynamicList<XmlElement> &
                                                o_rElements,
                           const Simstr &       i_sElementName );
    void                Parse_FreeChoice(
                            DynamicList<XmlElement> &
                                                o_rElements );
    void                Parse_List(
                            ListElement &       o_rListElem );
    void                Parse_Text(
                            Simstr &            o_sText,
                            const Simstr &      i_sElementName,
                            bool                i_bReverseName );
    void                Parse_MultipleText(
                            List<Simstr> &      o_rTexts,
                            const Simstr &      i_sElementName,
                            bool                i_bReverseName );
    void                Parse_SglAttr(
                            Simstr &            o_sAttrValue,
                            const Simstr &      i_sElementName,
                            const Simstr &      i_sAttrName );
    void                Parse_MultipleAttr(
                            List<Simstr> &      o_rAttrValues,
                            const Simstr &      i_sElementName,
                            const List<Simstr> &
                                                i_rAttrNames );

  private:
    void                Parse_XmlDeclaration();
    void                Parse_Doctype();

    void                Get_Attribute(
                            Simstr &            o_rAttrValue,
                            Simstr &            o_rAttrName );
    bool                IsText(
                            const char *        i_sComparedText );
    bool                IsBeginTag(
                            const char *        i_sTagName );
    bool                IsEndTag(
                            const char *        i_sTagName );
    void                Goto(
                            char                i_cNext );
    void                Goto_And_Pass(
                            char                i_cNext );
    void                Move(
                            int                 i_nForward );
    void                Pass_White();
    void                GetTextTill(
                            Simstr &            o_rText,
                            char                i_cEnd,
                            bool                i_bReverseName = false );
    void                CheckAndPassBeginTag(
                            const char *        i_sElementName );
    void                CheckAndPassEndTag(
                            const char *        i_sElementName );


    void                SyntaxError(
                            const char *        i_sText );
    void                TestCurChar();

    // DATA
    Simstr              sFileName;
    unsigned            nFileLine;

    Buffer              aFile;
    XmlElement *        pDocumentData;

    char                sWord[8192];
    const char *        text;
};



// IMPLEMENTATION

#endif

