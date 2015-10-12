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
#include "swdllapi.h"

class SwLabCfgItem;

class SW_DLLPUBLIC SwLabItem : public SfxPoolItem
{

public:

    SwLabItem();
    SwLabItem(const SwLabItem& rItem);

    SwLabItem& operator =(const SwLabItem& rItem);

    virtual bool operator ==(const SfxPoolItem& rItem) const override;

    virtual SfxPoolItem*     Clone(SfxItemPool* = 0) const override;

    OUString   aLstMake; // remember last selection
    OUString   aLstType;
    OUString   sDBName;  // used database

    OUString   aWriting; // label
    OUString   aMake;   // label mark
    OUString   aType;   // label type
    OUString   aBin;    // printer shaft
    sal_Int32       lHDist;  // horizontal distance (user)
    sal_Int32       lVDist;  // vertical distance (user)
    sal_Int32       lWidth;  // width (user)
    sal_Int32       lHeight;  // height (user)
    sal_Int32       lLeft;   // left border (user)
    sal_Int32       lUpper;  // upper border (user)
    sal_Int32       nCols;   // number of columns (user)
    sal_Int32       nRows;   // number of rows (user)
    sal_Int32       nCol;    // column for single print
    sal_Int32       nRow;    // row for single print
    sal_Int32       lPHeight; // paper height
    sal_Int32       lPWidth;  // paper width
    bool        bAddr;// address as label?
    bool        bCont;// continuous paper?
    bool        bPage;// whole page or single labels?
    bool        bSynchron;// synchronise all labels

    //parts of the business card
    OUString   aPrivFirstName;
    OUString   aPrivName;
    OUString   aPrivShortCut;
    OUString   aPrivFirstName2;
    OUString   aPrivName2;
    OUString   aPrivShortCut2;
    OUString   aPrivStreet;
    OUString   aPrivZip;
    OUString   aPrivCity;
    OUString   aPrivCountry;
    OUString   aPrivState;
    OUString   aPrivTitle;
    OUString   aPrivProfession;
    OUString   aPrivPhone;
    OUString   aPrivMobile;
    OUString   aPrivFax;
    OUString   aPrivWWW;
    OUString   aPrivMail;
    OUString   aCompCompany;
    OUString   aCompCompanyExt;
    OUString   aCompSlogan;
    OUString   aCompStreet;
    OUString   aCompZip;
    OUString   aCompCity;
    OUString   aCompCountry;
    OUString   aCompState;
    OUString   aCompPosition;
    OUString   aCompPhone;
    OUString   aCompMobile;
    OUString   aCompFax;
    OUString   aCompWWW;
    OUString   aCompMail;

    OUString   sGlossaryGroup;
    OUString   sGlossaryBlockName;
};

class SwLabCfgItem : public utl::ConfigItem
{
private:
    SwLabItem   aItem;
    bool    bIsLabel;

    com::sun::star::uno::Sequence<OUString> GetPropertyNames();

    virtual void ImplCommit() override;

public:
    SwLabCfgItem(bool bLabel);

    SwLabItem&              GetItem() {return aItem;}

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
