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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_LABEL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_LABEL_HXX

#include <sfx2/tabdlg.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include "labelcfg.hxx"
#include <vector>

class SwLabItem;
class SwLabPrtPage;
class SwDBManager;
class Printer;

class SwLabDlg : public SfxTabDialogController
{
    SwLabelConfig   aLabelsCfg;
    SwDBManager*    pDBManager;
    SwLabPrtPage* m_pPrtPage;

    std::vector<sal_uInt16> aTypeIds;
    std::vector<OUString> aMakes;

    std::unique_ptr<SwLabRecs> m_pRecs;
    OUString   aLstGroup;
    OUString   m_sBusinessCardDlg;
    bool       m_bLabel;
    void          ReplaceGroup_( const OUString &rMake );

    virtual void PageCreated(const OString& rId, SfxTabPage &rPage) override;
public:

    SwLabDlg(weld::Window* pParent, const SfxItemSet& rSet,
             SwDBManager* pDBManager, bool bLabel);
    virtual ~SwLabDlg() override;

    SwLabRec*   GetRecord(std::u16string_view rRecName, bool bCont);
    void        GetLabItem(SwLabItem &rItem);

          SwLabRecs &Recs()           { return *m_pRecs; }
    const SwLabRecs &Recs()     const { return *m_pRecs; }

          std::vector<sal_uInt16> &TypeIds()       { return aTypeIds; }
    const std::vector<sal_uInt16> &TypeIds() const { return aTypeIds; }

    std::vector<OUString> &Makes() { return aMakes; }
    const std::vector<OUString> &Makes() const { return aMakes; }

    Printer *GetPrt();
    void ReplaceGroup( const OUString &rMake )
    {
        if ( rMake != aLstGroup )
            ReplaceGroup_( rMake );
    }

    void UpdateGroup( const OUString &rMake ) {ReplaceGroup_( rMake );}
    static void UpdateFieldInformation(css::uno::Reference< css::frame::XModel> const & xModel,
                                       const SwLabItem& rItem);
    const OUString& GetBusinessCardStr() const {return m_sBusinessCardDlg;}

    SwLabelConfig& GetLabelsConfig() {return aLabelsCfg;}

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
