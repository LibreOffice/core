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
#include "visitors.hxx"

/** Simple visitor for testing SmVisitor */
class MockVisitor : public SmVisitor
{
public:
    virtual ~MockVisitor() {}

    void Visit( SmTableNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmTableNode should have type NTABLE",
                                     NTABLE, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmBraceNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBraceNode should have type NBRACE",
                                     NBRACE, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmBracebodyNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBracebodyNode should have type NBRACEBODY",
                                     NBRACEBODY, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmOperNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmOperNode should have type NOPER",
                                     NOPER, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmAlignNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmAlignNode should have type NALIGN",
                                     NALIGN, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmAttributNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmAttributNode should have type NATTRIBUT",
                                     NATTRIBUT, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmFontNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmFontNode should have type NFONT",
                                     NFONT, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmUnHorNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmUnHorNode should have type NUNHOR",
                                     NUNHOR, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmBinHorNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBinHorNode should have type NBINHOR",
                                     NBINHOR, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmBinVerNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBinVerNode should have type NBINVER",
                                     NBINVER, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmBinDiagonalNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBinDiagonalNode should have type NBINDIAGONAL",
                                     NBINDIAGONAL, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmSubSupNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmSubSupNode should have type NSUBSUP",
                                     NSUBSUP, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmMatrixNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmMatrixNode should have type NMATRIX",
                                     NMATRIX, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmPlaceNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmPlaceNode should have type NPLACE",
                                     NPLACE, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmTextNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmTextNode should have type NTEXT",
                                     NTEXT, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmSpecialNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmSpecialNode should have type NSPECIAL",
                                     NSPECIAL, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmGlyphSpecialNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmGlyphSpecialNode should have type NGLYPH_SPECIAL",
                                     NGLYPH_SPECIAL, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmMathSymbolNode* pNode ) override {
        CPPUNIT_ASSERT_MESSAGE("SmMathSymbolNode should have type NMATH or NMATHIDENT",
                               pNode->GetType() == NMATH || pNode->GetType() == NMATHIDENT);
        VisitChildren( pNode );
    }

    void Visit( SmBlankNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBlankNode should have type NBLANK",
                                     NBLANK, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmErrorNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmErrorNode should have type NERROR",
                                     NERROR, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmLineNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmLineNode should have type NLINE",
                                     NLINE, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmExpressionNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmExpressionNode should have type NEXPRESSION",
                                     NEXPRESSION, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmPolyLineNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmPolyLineNode should have type NPOLYLINE",
                                     NPOLYLINE, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmRootNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmRootNode should have type NROOT",
                                     NROOT, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmRootSymbolNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmRootSymbolNode should have type NROOTSYMBOL",
                                     NROOTSYMBOL, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmDynIntegralNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmDynIntegralNode should have type NDYNINT",
                                     NDYNINT, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmDynIntegralSymbolNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmDynIntegralSymbolNode should have type NDYNINTSYMBOL",
                                     NDYNINTSYMBOL, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmRectangleNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmRectangleNode should have type NRECTANGLE",
                                     NRECTANGLE, pNode->GetType());
        VisitChildren( pNode );
    }

    void Visit( SmVerticalBraceNode* pNode ) override {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmVerticalBraceNode should have type NVERTICAL_BRACE",
                                     NVERTICAL_BRACE, pNode->GetType());
        VisitChildren( pNode );
    }

private:
    /** Auxiliary method for visiting the children of a pNode */
    void VisitChildren( SmNode* pNode ) {
        SmNodeIterator it( pNode );
        while( it.Next() )
            it->Accept( this );
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
