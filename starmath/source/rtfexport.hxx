/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_STARMATH_SOURCE_RTFEXPORT_HXX
#define INCLUDED_STARMATH_SOURCE_RTFEXPORT_HXX

#include "wordexportbase.hxx"

#include <rtl/strbuf.hxx>

/**
 Class implementing writing of formulas to RTF.
 */
class SmRtfExport : public SmWordExportBase
{
public:
    explicit SmRtfExport(const SmNode* pIn);
    void ConvertFromStarMath(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding);
private:
    virtual void HandleVerticalStack(const SmNode* pNode, int nLevel) override;
    virtual void HandleText(const SmNode* pNode, int nLevel) override;
    virtual void HandleFractions(const SmNode* pNode, int nLevel, const char* type) override;
    virtual void HandleRoot(const SmRootNode* pNode, int nLevel) override;
    virtual void HandleAttribute(const SmAttributNode* pNode, int nLevel) override;
    virtual void HandleOperator(const SmOperNode* pNode, int nLevel) override;
    virtual void HandleSubSupScriptInternal(const SmSubSupNode* pNode, int nLevel, int flags) override;
    virtual void HandleMatrix(const SmMatrixNode* pNode, int nLevel) override;
    virtual void HandleBrace(const SmBraceNode* pNode, int nLevel) override;
    virtual void HandleVerticalBrace(const SmVerticalBraceNode* pNode, int nLevel) override;
    virtual void HandleBlank() override;

    OStringBuffer* m_pBuffer;
    rtl_TextEncoding m_nEncoding;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
