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

/**
   * This file provides definition for the nodetypes.
   * Also provides analysis of the node functions
   */

enum class SmNodeType : int_fast16_t
{
    Table, // tree header
    Brace, // () [] {}
    Bracebody, // content of () [] {}
    Oper, // largeop
    Align, // alignment
    Attribute, // attributes
    Font, // fonts
    UnHor, // unmo
    BinHor, // binmo
    BinVer, // over frac
    BinDiagonal, // slash
    SubSup, // lsub lsup rsub rsup csub csup
    Matrix, // binom matrix
    Place, // <?>
    Text, // "text"
    Special, // %glyph
    GlyphSpecial, // %glyph
    Math, // operator value
    Blank, // ~
    Error, // Syntax error
    Line, // a line of math untill newline
    Expression, // { content in here }
    PolyLine, // ^
    Root, // root node
    RootSymbol, // 3 of cubic root
    Rectangle, //just structual
    VerticalBrace, // vertical {}
    MathIdent // identities and variables
};

namespace starmathdatabase
{
bool isStructuralNode()
{
    return m_aNodeTypeName == SmNodeType::Table || m_aNodeTypeName == SmNodeType::Line
           || m_aNodeTypeName == SmNodeType::UnHor || m_aNodeTypeName == SmNodeType::BinHor
           || m_aNodeTypeName == SmNodeType::BinVer || m_aNodeTypeName == SmNodeType::BinDiagonal
           || m_aNodeTypeName == SmNodeType::SubSup || m_aNodeTypeName == SmNodeType::Matrix
           || m_aNodeTypeName == SmNodeType::Root || m_aNodeTypeName == SmNodeType::Expression
           || m_aNodeTypeName == SmNodeType::Brace || m_aNodeTypeName == SmNodeType::Bracebody
           || m_aNodeTypeName == SmNodeType::Oper || m_aNodeTypeName == SmNodeType::Align
           || m_aNodeTypeName == SmNodeType::Attribute || m_aNodeTypeName == SmNodeType::Font;
}

bool isBinOperatorNode()
{
    return m_aNodeTypeName == SmNodeType::BinHor || m_aNodeTypeName == SmNodeType::BinVer
           || m_aNodeTypeName == SmNodeType::BinDiagonal || m_aNodeTypeName == SmNodeType::SubSup;
}

bool isUnOperatorNode() { return m_aNodeTypeName == SmNodeType::UnHor; }

bool isOperatorNode()
{
    return m_aNodeTypeName == SmNodeType::BinHor || m_aNodeTypeName == SmNodeType::BinVer
           || m_aNodeTypeName == SmNodeType::BinDiagonal || m_aNodeTypeName == SmNodeType::SubSup
           || m_aNodeTypeName == SmNodeType::UnHor || m_aNodeTypeName == SmNodeType::Oper;
}

bool isStandaloneNode()
{
    return m_aNodeTypeName == SmNodeType::Text || m_aNodeTypeName == SmNodeType::Special
           || m_aNodeTypeName == SmNodeType::GlyphSpecial || m_aNodeTypeName == SmNodeType::Math
           || m_aNodeTypeName == SmNodeType::Blank || m_aNodeTypeName == SmNodeType::MathIdent;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
