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

    virtual SdrPage* AllocPage(bool bMasterPage) SAL_OVERRIDE;

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::embed::XStorage> GetDocumentStorage() const SAL_OVERRIDE;
    /// Get the callback and callback data, previously given to registerLibreOfficeKitCallback().
    void getLibreOfficeKitCallback(LibreOfficeKitCallback& rCallback, void*& rLibreOfficeKitData);

    /// For saving of rectangles as control-replacement for versions < 5.0.
    virtual SdrLayerID GetControlExportLayerId( const SdrObject & ) const SAL_OVERRIDE;

protected:
    /// override of <SdrModel::createUnoModel()> is needed to provide corresponding uno model.
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > createUnoModel() SAL_OVERRIDE;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
