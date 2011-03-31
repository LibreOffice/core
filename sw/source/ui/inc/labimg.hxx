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

#ifndef _LABIMG_HXX
#define _LABIMG_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include "swdllapi.h"

// class SwLabItem ----------------------------------------------------------
class SwLabCfgItem;

class SW_DLLPUBLIC SwLabItem : public SfxPoolItem
{

public:

    SwLabItem();
    SwLabItem(const SwLabItem& rItem);

    SwLabItem& operator =(const SwLabItem& rItem);

    virtual int operator ==(const SfxPoolItem& rItem) const;

    virtual SfxPoolItem*     Clone(SfxItemPool* = 0) const;

    rtl::OUString   aLstMake; // remember last selection
    rtl::OUString   aLstType;
    rtl::OUString   sDBName;  // used database

    rtl::OUString   aWriting; // label
    rtl::OUString   aMake;   // label mark
    rtl::OUString   aType;   // label type
    rtl::OUString   aBin;    // printer shaft
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
    sal_Bool        bAddr;// address as label?
    sal_Bool        bCont;// continuous paper?
    sal_Bool        bPage;// whole page or single labels?
    sal_Bool        bSynchron;// synchronise all labels

    //parts of the business card
    rtl::OUString   aPrivFirstName;
    rtl::OUString   aPrivName;
    rtl::OUString   aPrivShortCut;
    rtl::OUString   aPrivFirstName2;
    rtl::OUString   aPrivName2;
    rtl::OUString   aPrivShortCut2;
    rtl::OUString   aPrivStreet;
    rtl::OUString   aPrivZip;
    rtl::OUString   aPrivCity;
    rtl::OUString   aPrivCountry;
    rtl::OUString   aPrivState;
    rtl::OUString   aPrivTitle;
    rtl::OUString   aPrivProfession;
    rtl::OUString   aPrivPhone;
    rtl::OUString   aPrivMobile;
    rtl::OUString   aPrivFax;
    rtl::OUString   aPrivWWW;
    rtl::OUString   aPrivMail;
    rtl::OUString   aCompCompany;
    rtl::OUString   aCompCompanyExt;
    rtl::OUString   aCompSlogan;
    rtl::OUString   aCompStreet;
    rtl::OUString   aCompZip;
    rtl::OUString   aCompCity;
    rtl::OUString   aCompCountry;
    rtl::OUString   aCompState;
    rtl::OUString   aCompPosition;
    rtl::OUString   aCompPhone;
    rtl::OUString   aCompMobile;
    rtl::OUString   aCompFax;
    rtl::OUString   aCompWWW;
    rtl::OUString   aCompMail;

    rtl::OUString   sGlossaryGroup;
    rtl::OUString   sGlossaryBlockName;
};

// class SwLabCfgItem -------------------------------------------------------
class SwLabCfgItem : public utl::ConfigItem
{
    SwLabItem   aItem;
    sal_Bool    bIsLabel;

    com::sun::star::uno::Sequence<rtl::OUString> GetPropertyNames();
public:
    SwLabCfgItem(sal_Bool bLabel);

    SwLabItem&              GetItem() {return aItem;}

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& aPropertyNames );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
