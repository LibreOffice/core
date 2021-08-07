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
#ifndef INCLUDED_SW_INC_SWTBLFMT_HXX
#define INCLUDED_SW_INC_SWTBLFMT_HXX

#include "frmfmt.hxx"

class SAL_DLLPUBLIC_RTTI SwTableFormat final : public SwFrameFormat
{
    friend class SwDoc;

    SwTableFormat( SwAttrPool& rPool, const OUString &rFormatNm,
                    SwFrameFormat *pDrvdFrame )
        : SwFrameFormat( rPool, rFormatNm, pDrvdFrame, RES_FRMFMT, aTableSetRange )
    {}

public:
    virtual bool supportsFullDrawingLayerFillAttributeSet() const override;
};

class SwTableLineFormat final : public SwFrameFormat
{
    friend class SwDoc;

    SwTableLineFormat( SwAttrPool& rPool, SwFrameFormat *pDrvdFrame )
        : SwFrameFormat( rPool, OUString(), pDrvdFrame, RES_FRMFMT, aTableLineSetRange )
    {}

public:
    virtual bool supportsFullDrawingLayerFillAttributeSet() const override;
};

class SAL_DLLPUBLIC_RTTI SwTableBoxFormat final: public SwFrameFormat
{
    friend class SwDoc;

    SwTableBoxFormat(SwAttrPool& rPool, SwFrameFormat* pDrvdFrame)
        : SwFrameFormat(rPool, OUString(), pDrvdFrame, RES_FRMFMT, aTableBoxSetRange)
    {}

    // For recognition of changes (especially TableBoxAttribute).
    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;
    void BoxAttributeChanged(SwTableBox& rBox, const SwTableBoxNumFormat* pNewFormat, const SwTableBoxFormula* pNewFormula, const SwTableBoxValue* pNewValue, const sal_uLong nOldFormat);

public:
    virtual bool supportsFullDrawingLayerFillAttributeSet() const override;
    SwTableBox* GetTableBox();
    const SwTableBox* GetTableBox() const
            { return const_cast<SwTableBoxFormat*>(this)->GetTableBox(); };
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
