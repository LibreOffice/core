/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_STARMATH_QA_CPPUNIT_MOCK_VISITOR_HXX
#define INCLUDED_STARMATH_QA_CPPUNIT_MOCK_VISITOR_HXX

#include <cppunit/TestAssert.h>
#include <visitors.hxx>

/** Simple visitor for testing SmVisitor */
class MockVisitor : public SmVisitor
{
public:
    virtual ~MockVisitor() {}

    void Visit( SmTableNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmTableNode should have type SmNodeType::Table",
                                     SmNodeType::Table, pNode->GetType());
        auto eTT = pNode->GetToken().eType;
        CPPUNIT_ASSERT_MESSAGE("The type of SmTableNode's token should be either TEND, TBINOM, or TSTACK",
                               eTT == TEND || eTT == TBINOM || eTT == TSTACK);
        VisitChildren( pNode );
    }

    void Visit( SmBraceNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBraceNode should have type SmNodeType::Brace",
                                     SmNodeType::Brace, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmBracebodyNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBracebodyNode should have type SmNodeType::Bracebody",
                                     SmNodeType::Bracebody, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmOperNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmOperNode should have type SmNodeType::Oper",
                                     SmNodeType::Oper, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmAlignNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmAlignNode should have type SmNodeType::Align",
                                     SmNodeType::Align, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmAttributeNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmAttributeNode should have type SmNodeType::Attribute",
                                     SmNodeType::Attribute, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmFontNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmFontNode should have type SmNodeType::Font",
                                     SmNodeType::Font, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmUnHorNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmUnHorNode should have type SmNodeType::UnHor",
                                     SmNodeType::UnHor, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmBinHorNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBinHorNode should have type SmNodeType::BinHor",
                                     SmNodeType::BinHor, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmBinVerNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBinVerNode should have type SmNodeType::BinVer",
                                     SmNodeType::BinVer, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmBinDiagonalNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBinDiagonalNode should have type SmNodeType::BinDiagonal",
                                     SmNodeType::BinDiagonal, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmSubSupNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmSubSupNode should have type SmNodeType::SubSup",
                                     SmNodeType::SubSup, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmMatrixNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmMatrixNode should have type SmNodeType::Matrix",
                                     SmNodeType::Matrix, pNode->GetType());
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmMatrixNode's token should be of type TMATRIX",
                                     TMATRIX, pNode->GetToken().eType);
        VisitChildren( pNode );
    }

    void Visit( SmPlaceNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmPlaceNode should have type SmNodeType::Place",
                                     SmNodeType::Place, pNode->GetType());
    }

    void Visit( SmTextNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmTextNode should have type SmNodeType::Text",
                                     SmNodeType::Text, pNode->GetType());
    }

    void Visit( SmSpecialNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmSpecialNode should have type SmNodeType::Special",
                                     SmNodeType::Special, pNode->GetType());
    }

    void Visit( SmGlyphSpecialNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmGlyphSpecialNode should have type SmNodeType::GlyphSpecial",
                                     SmNodeType::GlyphSpecial, pNode->GetType());
    }

    void Visit( SmMathSymbolNode* pNode ) override {
        CPPUNIT_ASSERT_MESSAGE("SmMathSymbolNode should have type SmNodeType::Math or SmNodeType::MathIdent",
                               pNode->GetType() == SmNodeType::Math || pNode->GetType() == SmNodeType::MathIdent);
    }

    void Visit( SmBlankNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBlankNode should have type SmNodeType::Blank",
                                     SmNodeType::Blank, pNode->GetType());
    }

    void Visit( SmErrorNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmErrorNode should have type SmNodeType::Error",
                                     SmNodeType::Error, pNode->GetType());
    }

    void Visit( SmLineNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmLineNode should have type SmNodeType::Line",
                                     SmNodeType::Line, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmExpressionNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmExpressionNode should have type SmNodeType::Expression",
                                     SmNodeType::Expression, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmPolyLineNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmPolyLineNode should have type SmNodeType::PolyLine",
                                     SmNodeType::PolyLine, pNode->GetType());
    }

    void Visit( SmRootNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmRootNode should have type SmNodeType::Root",
                                     SmNodeType::Root, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmRootSymbolNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmRootSymbolNode should have type SmNodeType::RootSymbol",
                                     SmNodeType::RootSymbol, pNode->GetType());
    }

    void Visit( SmRectangleNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmRectangleNode should have type SmNodeType::Rectangle",
                                     SmNodeType::Rectangle, pNode->GetType());
    }

    void Visit( SmVerticalBraceNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmVerticalBraceNode should have type SmNodeType::VerticalBrace",
                                     SmNodeType::VerticalBrace, pNode->GetType());
        VisitChildren( pNode );
    }

private:
    /** Auxiliary method for visiting the children of a pNode */
    void VisitChildren( SmStructureNode* pNode ) {
        for (auto pChild : *pNode)
        {
            if (pChild)
                pChild->Accept(this);
        }
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
