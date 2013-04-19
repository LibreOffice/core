/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef SM_OOXMLEXPORT_HXX
#define SM_OOXMLEXPORT_HXX

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
