/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
