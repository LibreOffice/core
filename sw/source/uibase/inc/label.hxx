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
#include <labelcfg.hxx>
#include <vector>

class SwLabItem;
class SwLabPrtPage;
class SwDBManager;
class Printer;

class SwLabDlg : public SfxTabDialog
{
    SwLabelConfig   aLabelsCfg;
    SwDBManager*     pDBManager;
    VclPtr<SwLabPrtPage>   pPrtPage;

    std::vector<sal_uInt16> aTypeIds;
    std::vector<OUString> aMakes;

    SwLabRecs* m_pRecs;
    OUString   aLstGroup;
    OUString   m_sBusinessCardDlg;
    bool       m_bLabel;
    sal_uInt16 m_nFormatId;
    sal_uInt16 m_nOptionsId;
    sal_uInt16 m_nLabelId;
    sal_uInt16 m_nCardsId;
    sal_uInt16 m_nBusinessId;
    sal_uInt16 m_nPrivateId;
    void          _ReplaceGroup( const OUString &rMake );

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;
public:

     SwLabDlg( vcl::Window* pParent, const SfxItemSet& rSet,
                 SwDBManager* pDBManager, bool bLabel);
    virtual ~SwLabDlg();
    virtual void dispose() override;

    SwLabRec*   GetRecord(const OUString &rRecName, bool bCont);
    void        GetLabItem(SwLabItem &rItem);

          SwLabRecs &Recs()           { return *m_pRecs; }
    const SwLabRecs &Recs()     const { return *m_pRecs; }

          std::vector<sal_uInt16> &TypeIds()       { return aTypeIds; }
    const std::vector<sal_uInt16> &TypeIds() const { return aTypeIds; }

    std::vector<OUString> &Makes() { return aMakes; }
    const std::vector<OUString> &Makes() const { return aMakes; }

    Printer *GetPrt();
    inline void ReplaceGroup( const OUString &rMake );
    void UpdateGroup( const OUString &rMake ) {_ReplaceGroup( rMake );}
    static void UpdateFieldInformation(css::uno::Reference< css::frame::XModel>& xModel,
                                                                                const SwLabItem& rItem);
    const OUString& GetBusinessCardStr() const {return m_sBusinessCardDlg;}

    SwLabelConfig& GetLabelsConfig() {return aLabelsCfg;}

};

inline void SwLabDlg::ReplaceGroup( const OUString &rMake )
{
    if ( rMake != aLstGroup )
        _ReplaceGroup( rMake );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
