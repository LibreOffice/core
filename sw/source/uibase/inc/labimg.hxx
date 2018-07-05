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

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_LABIMG_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_LABIMG_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include <swdllapi.h>

class SwLabCfgItem;

class SW_DLLPUBLIC SwLabItem : public SfxPoolItem
{

public:

    SwLabItem();

    SwLabItem& operator =(const SwLabItem& rItem);
    /*TODO: SfxPoolItem copy function dichotomy*/SwLabItem(SwLabItem const &) = default;

    virtual bool operator ==(const SfxPoolItem& rItem) const override;

    virtual SfxPoolItem*     Clone(SfxItemPool* = nullptr) const override;

    OUString   m_aLstMake; // remember last selection
    OUString   m_aLstType;
    OUString   m_sDBName;  // used database

    OUString   m_aWriting; // label
    OUString   m_aMake;   // label mark
    OUString   m_aType;   // label type
    OUString   m_aBin;    // printer shaft
    sal_Int32       m_lHDist;  // horizontal distance (user)
    sal_Int32       m_lVDist;  // vertical distance (user)
    sal_Int32       m_lWidth;  // width (user)
    sal_Int32       m_lHeight;  // height (user)
    sal_Int32       m_lLeft;   // left border (user)
    sal_Int32       m_lUpper;  // upper border (user)
    sal_Int32       m_nCols;   // number of columns (user)
    sal_Int32       m_nRows;   // number of rows (user)
    sal_Int32       m_nCol;    // column for single print
    sal_Int32       m_nRow;    // row for single print
    sal_Int32       m_lPHeight; // paper height
    sal_Int32       m_lPWidth;  // paper width
    bool        m_bAddr;// address as label?
    bool        m_bCont;// continuous paper?
    bool        m_bPage;// whole page or single labels?
    bool        m_bSynchron;// synchronise all labels

    //parts of the business card
    OUString   m_aPrivFirstName;
    OUString   m_aPrivName;
    OUString   m_aPrivShortCut;
    OUString   m_aPrivFirstName2;
    OUString   m_aPrivName2;
    OUString   m_aPrivShortCut2;
    OUString   m_aPrivStreet;
    OUString   m_aPrivZip;
    OUString   m_aPrivCity;
    OUString   m_aPrivCountry;
    OUString   m_aPrivState;
    OUString   m_aPrivTitle;
    OUString   m_aPrivProfession;
    OUString   m_aPrivPhone;
    OUString   m_aPrivMobile;
    OUString   m_aPrivFax;
    OUString   m_aPrivWWW;
    OUString   m_aPrivMail;
    OUString   m_aCompCompany;
    OUString   m_aCompCompanyExt;
    OUString   m_aCompSlogan;
    OUString   m_aCompStreet;
    OUString   m_aCompZip;
    OUString   m_aCompCity;
    OUString   m_aCompCountry;
    OUString   m_aCompState;
    OUString   m_aCompPosition;
    OUString   m_aCompPhone;
    OUString   m_aCompMobile;
    OUString   m_aCompFax;
    OUString   m_aCompWWW;
    OUString   m_aCompMail;

    OUString   m_sGlossaryGroup;
    OUString   m_sGlossaryBlockName;
};

class SwLabCfgItem : public utl::ConfigItem
{
private:
    SwLabItem   aItem;
    bool    bIsLabel;

    css::uno::Sequence<OUString> GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwLabCfgItem(bool bLabel);

    SwLabItem&              GetItem() {return aItem;}

    virtual void Notify( const css::uno::Sequence< OUString >& aPropertyNames ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
