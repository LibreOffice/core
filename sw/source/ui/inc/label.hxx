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
#ifndef _LABEL_HXX
#define _LABEL_HXX

#include <sfx2/tabdlg.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <labelcfg.hxx>
#include <vector>

class SwLabRec;
class SwLabRecs;
class SwLabItem;
class SwLabPrtPage;
class SwNewDBMgr;
class Printer;

class SwLabDlg : public SfxTabDialog
{
    SwLabelConfig   aLabelsCfg;
    SwNewDBMgr*     pNewDBMgr;
    SwLabPrtPage*   pPrtPage;

    std::vector<sal_uInt16> aTypeIds;
    std::vector<OUString> aMakes;

    SwLabRecs* pRecs;
    String     aLstGroup;
    OUString   m_sBusinessCardDlg;
    bool       m_bLabel;
    sal_uInt16 m_nFormatId;
    sal_uInt16 m_nOptionsId;
    sal_uInt16 m_nLabelId;
    sal_uInt16 m_nCardsId;
    sal_uInt16 m_nBusinessId;
    sal_uInt16 m_nPrivateId;
    void          _ReplaceGroup( const String &rMake );

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage );
public:

     SwLabDlg( Window* pParent, const SfxItemSet& rSet,
                 SwNewDBMgr* pNewDBMgr, sal_Bool bLabel);
    ~SwLabDlg();

    SwLabRec*   GetRecord(const String &rRecName, sal_Bool bCont);
    void        GetLabItem(SwLabItem &rItem);

          SwLabRecs &Recs()           { return *pRecs;   }
    const SwLabRecs &Recs()     const { return *pRecs;   }

          std::vector<sal_uInt16> &TypeIds()       { return aTypeIds; }
    const std::vector<sal_uInt16> &TypeIds() const { return aTypeIds; }

    std::vector<OUString> &Makes() { return aMakes; }
    const std::vector<OUString> &Makes() const { return aMakes; }

    Printer *GetPrt();
    inline void ReplaceGroup( const String &rMake );
    void UpdateGroup( const String &rMake ) {_ReplaceGroup( rMake );}
    static void UpdateFieldInformation(::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>& xModel,
                                                                                const SwLabItem& rItem);
    const OUString& GetBusinessCardStr() const {return m_sBusinessCardDlg;}

    SwLabelConfig& GetLabelsConfig() {return aLabelsCfg;}

};

inline void SwLabDlg::ReplaceGroup( const String &rMake )
{
    if ( rMake != aLstGroup )
        _ReplaceGroup( rMake );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
