/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "wordexportbase.hxx"

#include <sax/fshelper.hxx>
#include <oox/core/filterbase.hxx>
#include <oox/export/utils.hxx>

/**
 Class implementing writing of formulas to OOXML.
 */
class SmOoxmlExport : public SmWordExportBase
{
public:
    SmOoxmlExport(const SmNode* pIn, oox::core::OoxmlVersion version,
                  oox::drawingml::DocumentType documentType);
    void ConvertFromStarMath(const ::sax_fastparser::FSHelperPtr& m_pSerializer, const sal_Int8);

private:
    void HandleVerticalStack(const SmNode* pNode, int nLevel) override;
    void HandleText(const SmNode* pNode, int nLevel) override;
    void HandleFractions(const SmNode* pNode, int nLevel, const char* type) override;
    void HandleRoot(const SmRootNode* pNode, int nLevel) override;
    void HandleAttribute(const SmAttributeNode* pNode, int nLevel) override;
    void HandleOperator(const SmOperNode* pNode, int nLevel) override;
    void HandleSubSupScriptInternal(const SmSubSupNode* pNode, int nLevel, int flags) override;
    void HandleMatrix(const SmMatrixNode* pNode, int nLevel) override;
    void HandleBrace(const SmBraceNode* pNode, int nLevel) override;
    void HandleVerticalBrace(const SmVerticalBraceNode* pNode, int nLevel) override;
    void HandleBlank() override;
    ::sax_fastparser::FSHelperPtr m_pSerializer;
    oox::core::OoxmlVersion version;
    /// needed to determine markup for nested run properties
    oox::drawingml::DocumentType const m_DocumentType;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
