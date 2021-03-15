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
    Line, // a line of math until newline
    Expression, // { content in here }
    PolyLine, // ^
    Root, // root node
    RootSymbol, // 3 of cubic root
    Rectangle, //just structural
    VerticalBrace, // vertical {}
    MathIdent // identities and variables
};

namespace starmathdatabase
{
inline bool isStructuralNode(SmNodeType aNodeTypeName)
{
    return aNodeTypeName == SmNodeType::Table || aNodeTypeName == SmNodeType::Line
           || aNodeTypeName == SmNodeType::UnHor || aNodeTypeName == SmNodeType::BinHor
           || aNodeTypeName == SmNodeType::BinVer || aNodeTypeName == SmNodeType::BinDiagonal
           || aNodeTypeName == SmNodeType::SubSup || aNodeTypeName == SmNodeType::Matrix
           || aNodeTypeName == SmNodeType::Root || aNodeTypeName == SmNodeType::Expression
           || aNodeTypeName == SmNodeType::Brace || aNodeTypeName == SmNodeType::Bracebody
           || aNodeTypeName == SmNodeType::Oper || aNodeTypeName == SmNodeType::Align
           || aNodeTypeName == SmNodeType::Attribute || aNodeTypeName == SmNodeType::Font;
}

inline bool isBinOperatorNode(SmNodeType aNodeTypeName)
{
    return aNodeTypeName == SmNodeType::BinHor || aNodeTypeName == SmNodeType::BinVer
           || aNodeTypeName == SmNodeType::BinDiagonal || aNodeTypeName == SmNodeType::SubSup;
}

inline bool isUnOperatorNode(SmNodeType aNodeTypeName)
{
    return aNodeTypeName == SmNodeType::UnHor;
}

inline bool isOperatorNode(SmNodeType aNodeTypeName)
{
    return aNodeTypeName == SmNodeType::BinHor || aNodeTypeName == SmNodeType::BinVer
           || aNodeTypeName == SmNodeType::BinDiagonal || aNodeTypeName == SmNodeType::SubSup
           || aNodeTypeName == SmNodeType::UnHor || aNodeTypeName == SmNodeType::Oper;
}

inline bool isStandaloneNode(SmNodeType aNodeTypeName)
{
    return aNodeTypeName == SmNodeType::Text || aNodeTypeName == SmNodeType::Special
           || aNodeTypeName == SmNodeType::GlyphSpecial || aNodeTypeName == SmNodeType::Math
           || aNodeTypeName == SmNodeType::Blank || aNodeTypeName == SmNodeType::MathIdent;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
