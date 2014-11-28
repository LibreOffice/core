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

    void Visit( SmTableNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmTableNode should have type NTABLE",
                                     pNode->GetType(), NTABLE);
        VisitChildren( pNode );
    }

    void Visit( SmBraceNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBraceNode should have type NBRACE",
                                     pNode->GetType(), NBRACE);
        VisitChildren( pNode );
    }

    void Visit( SmBracebodyNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBracebodyNode should have type NBRACEBODY",
                                     pNode->GetType(), NBRACEBODY);
        VisitChildren( pNode );
    }

    void Visit( SmOperNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmOperNode should have type NOPER",
                                     pNode->GetType(), NOPER);
        VisitChildren( pNode );
    }

    void Visit( SmAlignNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmAlignNode should have type NALIGN",
                                     pNode->GetType(), NALIGN);
        VisitChildren( pNode );
    }

    void Visit( SmAttributNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmAttributNode should have type NATTRIBUT",
                                     pNode->GetType(), NATTRIBUT);
        VisitChildren( pNode );
    }

    void Visit( SmFontNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmFontNode should have type NFONT",
                                     pNode->GetType(), NFONT);
        VisitChildren( pNode );
    }

    void Visit( SmUnHorNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmUnHorNode should have type NUNHOR",
                                     pNode->GetType(), NUNHOR);
        VisitChildren( pNode );
    }

    void Visit( SmBinHorNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBinHorNode should have type NBINHOR",
                                     pNode->GetType(), NBINHOR);
        VisitChildren( pNode );
    }

    void Visit( SmBinVerNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBinVerNode should have type NBINVER",
                                     pNode->GetType(), NBINVER);
        VisitChildren( pNode );
    }

    void Visit( SmBinDiagonalNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBinDiagonalNode should have type NBINDIAGONAL",
                                     pNode->GetType(), NBINDIAGONAL);
        VisitChildren( pNode );
    }

    void Visit( SmSubSupNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmSubSupNode should have type NSUBSUP",
                                     pNode->GetType(), NSUBSUP);
        VisitChildren( pNode );
    }

    void Visit( SmMatrixNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmMatrixNode should have type NMATRIX",
                                     pNode->GetType(), NMATRIX);
        VisitChildren( pNode );
    }

    void Visit( SmPlaceNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmPlaceNode should have type NPLACE",
                                     pNode->GetType(), NPLACE);
        VisitChildren( pNode );
    }

    void Visit( SmTextNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmTextNode should have type NTEXT",
                                     pNode->GetType(), NTEXT);
        VisitChildren( pNode );
    }

    void Visit( SmSpecialNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmSpecialNode should have type NSPECIAL",
                                     pNode->GetType(), NSPECIAL);
        VisitChildren( pNode );
    }

    void Visit( SmGlyphSpecialNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmGlyphSpecialNode should have type NGLYPH_SPECIAL",
                                     pNode->GetType(), NGLYPH_SPECIAL);
        VisitChildren( pNode );
    }

    void Visit( SmMathSymbolNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_MESSAGE("SmMathSymbolNode should have type NMATH or NMATHIDENT",
                               pNode->GetType() == NMATH || pNode->GetType() == NMATHIDENT);
        VisitChildren( pNode );
    }

    void Visit( SmBlankNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmBlankNode should have type NBLANK",
                                     pNode->GetType(), NBLANK);
        VisitChildren( pNode );
    }

    void Visit( SmErrorNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmErrorNode should have type NERROR",
                                     pNode->GetType(), NERROR);
        VisitChildren( pNode );
    }

    void Visit( SmLineNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmLineNode should have type NLINE",
                                     pNode->GetType(), NLINE);
        VisitChildren( pNode );
    }

    void Visit( SmExpressionNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmExpressionNode should have type NEXPRESSION",
                                     pNode->GetType(), NEXPRESSION);
        VisitChildren( pNode );
    }

    void Visit( SmPolyLineNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmPolyLineNode should have type NPOLYLINE",
                                     pNode->GetType(), NPOLYLINE);
        VisitChildren( pNode );
    }

    void Visit( SmRootNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmRootNode should have type NROOT",
                                     pNode->GetType(), NROOT);
        VisitChildren( pNode );
    }

    void Visit( SmRootSymbolNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmRootSymbolNode should have type NROOTSYMBOL",
                                     pNode->GetType(), NROOTSYMBOL);
        VisitChildren( pNode );
    }

    void Visit( SmDynIntegralNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmDynIntegralNode should have type NDYNINT",
                                     pNode->GetType(), NDYNINT);
        VisitChildren( pNode );
    }

    void Visit( SmDynIntegralSymbolNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmDynIntegralSymbolNode should have type NDYNINTSYMBOL",
                                     pNode->GetType(), NDYNINTSYMBOL);
        VisitChildren( pNode );
    }

    void Visit( SmRectangleNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmRectangleNode should have type NRECTANGLE",
                                     pNode->GetType(), NRECTANGLE);
        VisitChildren( pNode );
    }

    void Visit( SmVerticalBraceNode* pNode ) SAL_OVERRIDE {
        CPPUNIT_ASSERT_EQUAL_MESSAGE("SmVerticalBraceNode should have type NVERTICAL_BRACE",
                                     pNode->GetType(), NVERTICAL_BRACE);
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
