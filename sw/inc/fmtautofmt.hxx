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
#ifndef INCLUDED_SW_INC_FMTAUTOFMT_HXX
#define INCLUDED_SW_INC_FMTAUTOFMT_HXX

#include <svl/poolitem.hxx>
#include <format.hxx>
#include <boost/shared_ptr.hpp>

class SwFmtAutoFmt: public SfxPoolItem
{
    boost::shared_ptr<SfxItemSet> mpHandle;

public:
    SwFmtAutoFmt( sal_uInt16 nWhich = RES_TXTATR_AUTOFMT );

    /// single argument ctors shall be explicit.
    virtual ~SwFmtAutoFmt();

    /// @@@ public copy ctor, but no copy assignment?
    SwFmtAutoFmt( const SwFmtAutoFmt& rAttr );
private:
    /// @@@ public copy ctor, but no copy assignment?
    SwFmtAutoFmt & operator= (const SwFmtAutoFmt &);
public:

    TYPEINFO_OVERRIDE();

    /// "pure virtual methods" of SfxPoolItem
    virtual bool            operator==( const SfxPoolItem& ) const SAL_OVERRIDE;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const SAL_OVERRIDE;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText,
                                    const IntlWrapper*    pIntl = 0 ) const SAL_OVERRIDE;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const SAL_OVERRIDE;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) SAL_OVERRIDE;

    void SetStyleHandle( boost::shared_ptr<SfxItemSet> pHandle ) { mpHandle = pHandle; }
    const boost::shared_ptr<SfxItemSet> GetStyleHandle() const { return mpHandle; }
          boost::shared_ptr<SfxItemSet> GetStyleHandle() { return mpHandle; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
