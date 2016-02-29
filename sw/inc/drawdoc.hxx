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
#ifndef INCLUDED_SW_INC_DRAWDOC_HXX
#define INCLUDED_SW_INC_DRAWDOC_HXX

#include <svx/fmmodel.hxx>

class SwDoc;
class SwDocShell;

class SwDrawModel : public FmFormModel
{
private:
    SwDoc* m_pDoc;

public:
    SwDrawModel( SwDoc* pDoc );
    virtual ~SwDrawModel();

    const SwDoc& GetDoc() const { return *m_pDoc; }
          SwDoc& GetDoc()       { return *m_pDoc; }
    /// Put needed items for XPropertyList entries from the DrawModel.
    void PutAreaListItems(SfxItemSet& rSet) const;

    virtual SdrPage* AllocPage(bool bMasterPage) override;

    virtual css::uno::Reference<css::embed::XStorage> GetDocumentStorage() const override;

protected:
    /// override of <SdrModel::createUnoModel()> is needed to provide corresponding uno model.
    virtual css::uno::Reference< css::uno::XInterface > createUnoModel() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
