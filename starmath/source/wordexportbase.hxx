/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Lubos Lunak <l.lunak@suse.cz> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef SM_WORDEXPORTBASE_HXX
#define SM_WORDEXPORTBASE_HXX

#include "node.hxx"

/**
 Base class implementing writing of formulas to Word.
 */
class SmWordExportBase
{
public:
    SmWordExportBase( const SmNode* pIn );
    virtual ~SmWordExportBase();
protected:
    void HandleNode( const SmNode* pNode, int nLevel );
    void HandleAllSubNodes( const SmNode* pNode, int nLevel );
    void HandleTable( const SmNode* pNode, int nLevel );
    virtual void HandleVerticalStack( const SmNode* pNode, int nLevel ) = 0;
    virtual void HandleText( const SmNode* pNode, int nLevel ) = 0;
    void HandleMath( const SmNode* pNode, int nLevel );
    virtual void HandleFractions( const SmNode* pNode, int nLevel, const char* type = NULL ) = 0;
    void HandleUnaryOperation( const SmUnHorNode* pNode, int nLevel );
    void HandleBinaryOperation( const SmBinHorNode* pNode, int nLevel );
    virtual void HandleRoot( const SmRootNode* pNode, int nLevel ) = 0;
    virtual void HandleAttribute( const SmAttributNode* pNode, int nLevel ) = 0;
    virtual void HandleOperator( const SmOperNode* pNode, int nLevel ) = 0;
    void HandleSubSupScript( const SmSubSupNode* pNode, int nLevel );
    virtual void HandleSubSupScriptInternal( const SmSubSupNode* pNode, int nLevel, int flags ) = 0;
    virtual void HandleMatrix( const SmMatrixNode* pNode, int nLevel ) = 0;
    virtual void HandleBrace( const SmBraceNode* pNode, int nLevel ) = 0;
    virtual void HandleVerticalBrace( const SmVerticalBraceNode* pNode, int nLevel ) = 0;
    virtual void HandleBlank() = 0;
    const SmNode* const m_pTree;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
