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
#ifndef _SVX_NUMINF_HXX
#define _SVX_NUMINF_HXX

#include <svl/itempool.hxx>
#include <svx/numfmtsh.hxx>
#include "svx/svxdllapi.h"

// class SvxNumberInfoItem -----------------------------------------------



/*
This item is used as a transport medium for a number formatter
*/

class SVX_DLLPUBLIC SvxNumberInfoItem : public SfxPoolItem
{
public:
    TYPEINFO();

    SvxNumberInfoItem( const sal_uInt16 nId  );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                       const sal_uInt16 nId  );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const OUString& rVal,
                       const sal_uInt16 nId  );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const double& rVal,
                       const sal_uInt16 nId  );
    // if both double and String are supplied, String is used for text formats
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const double& rVal,
                       const OUString& rValueStr, const sal_uInt16 nId );
    SvxNumberInfoItem( const SvxNumberInfoItem& );
    ~SvxNumberInfoItem();

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual SfxPoolItem*     Create( SvStream& rStream, sal_uInt16 nVer ) const;
    virtual SvStream&        Store( SvStream& , sal_uInt16 nItemVersion ) const;

    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    SvNumberFormatter*      GetNumberFormatter() const { return pFormatter; }
    const OUString&         GetValueString() const { return aStringVal; }
    double                  GetValueDouble() const  { return nDoubleVal; }

    const sal_uInt32*       GetDelArray() const { return pDelFormatArr; }
    void                    SetDelFormatArray( const sal_uInt32* pData,
                                               const sal_uInt32  nCount );

    SvxNumberValueType      GetValueType() const { return eValueType; }
    sal_uInt32              GetDelCount() const  { return nDelCount; }

private:
    SvNumberFormatter*  pFormatter;
    SvxNumberValueType  eValueType;
    OUString            aStringVal;
    double              nDoubleVal;

    sal_uInt32*         pDelFormatArr;
    sal_uInt32          nDelCount;
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
