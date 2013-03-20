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

#pragma once
#if 1

#include "wordexportbase.hxx"

#include <sax/fshelper.hxx>
#include <oox/core/filterbase.hxx>

/**
 Class implementing writing of formulas to OOXML.
 */
class SmOoxmlExport : public SmWordExportBase
{
public:
    SmOoxmlExport( const SmNode* pIn, oox::core::OoxmlVersion version );
    bool ConvertFromStarMath( ::sax_fastparser::FSHelperPtr m_pSerializer );
private:
    virtual void HandleVerticalStack( const SmNode* pNode, int nLevel );
    virtual void HandleText( const SmNode* pNode, int nLevel );
    virtual void HandleFractions( const SmNode* pNode, int nLevel, const char* type = NULL );
    virtual void HandleRoot( const SmRootNode* pNode, int nLevel );
    virtual void HandleAttribute( const SmAttributNode* pNode, int nLevel );
    virtual void HandleOperator( const SmOperNode* pNode, int nLevel );
    virtual void HandleSubSupScriptInternal( const SmSubSupNode* pNode, int nLevel, int flags );
    virtual void HandleMatrix( const SmMatrixNode* pNode, int nLevel );
    virtual void HandleBrace( const SmBraceNode* pNode, int nLevel );
    virtual void HandleVerticalBrace( const SmVerticalBraceNode* pNode, int nLevel );
    virtual void HandleBlank();
    ::sax_fastparser::FSHelperPtr m_pSerializer;
    oox::core::OoxmlVersion version;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
