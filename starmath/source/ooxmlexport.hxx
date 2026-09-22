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
                  oox::drawingml::DocumentType documentType, sal_Int32 nFontSizeInHalfPoints);
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
    /// True when the run properties are written out, which needs a DOCX.
    bool WritesRunProperties() const;
    /// True when the node has a color of its own rather than the default one.
    static bool HasOwnColor(const SmNode* pNode);
    /// Writes the w:sz and w:szCs carrying the size the formula is drawn at.
    void WriteFontSize();
    /// Writes the m:ctrlPr carrying the size, color, bold and italic of the parts a
    /// construct draws itself.
    void WriteCtrlPr(const SmNode* pNode);
    /// Writes a property element that a construct carries only to hold its control properties.
    void WritePropertiesElement(sal_Int32 nElement, const SmNode* pNode);
    ::sax_fastparser::FSHelperPtr m_pSerializer;
    oox::core::OoxmlVersion version;
    /// needed to determine markup for nested run properties
    oox::drawingml::DocumentType const m_DocumentType;
    /// The size the formula is drawn at, in half points.
    sal_Int32 const m_nFontSizeInHalfPoints;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
