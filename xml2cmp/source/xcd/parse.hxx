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
    /// @return false in case of empty tag with no attributes.
    bool                CheckAndPassBeginTag(
                            const char *        i_sElementName );
    void                CheckAndPassEndTag(
                            const char *        i_sElementName );
    /// @precond IsBeginTag() == true.
    bool                IsAbsoluteEmpty() const;


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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
