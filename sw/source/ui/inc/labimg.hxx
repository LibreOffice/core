/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    sal_Bool        bAddr;// adress as label?
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
