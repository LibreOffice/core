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
#ifndef INCLUDED_SVX_NUMINF_HXX
#define INCLUDED_SVX_NUMINF_HXX

#include <svl/poolitem.hxx>
#include <svx/numfmtsh.hxx>
#include <svx/svxdllapi.h>

/** This item is used as a transport medium for a number formatter
 */

class SVX_DLLPUBLIC SvxNumberInfoItem final : public SfxPoolItem
{
public:

    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter,
                       const TypedWhichId<SvxNumberInfoItem> nId  );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, OUString aVal,
                       const TypedWhichId<SvxNumberInfoItem> nId  );
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const double& rVal,
                       const TypedWhichId<SvxNumberInfoItem> nId  );
    // if both double and String are supplied, String is used for text formats
    SvxNumberInfoItem( SvNumberFormatter* pNumFormatter, const double& rVal,
                       OUString aValueStr, const TypedWhichId<SvxNumberInfoItem> nId );
    SvxNumberInfoItem( const SvxNumberInfoItem& );
    virtual ~SvxNumberInfoItem() override;

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual SvxNumberInfoItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;

    SvNumberFormatter*      GetNumberFormatter() const { return pFormatter; }
    const OUString&         GetValueString() const { return aStringVal; }
    double                  GetValueDouble() const  { return nDoubleVal; }

    const std::vector<sal_uInt32> & GetDelFormats() const { return mvDelFormats; }
    void                    SetDelFormats( std::vector<sal_uInt32> && );

    SvxNumberValueType      GetValueType() const { return eValueType; }

private:
    SvNumberFormatter*  pFormatter;
    SvxNumberValueType  eValueType;
    OUString            aStringVal;
    double              nDoubleVal;

    std::vector<sal_uInt32> mvDelFormats;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
