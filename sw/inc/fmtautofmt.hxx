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
#ifndef _FMTAUTOFMT_HXX
#define _FMTAUTOFMT_HXX


#include <svl/poolitem.hxx>
#include <format.hxx>
#include <boost/shared_ptr.hpp>

// ATT_AUTOFMT *********************************************


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

    TYPEINFO();

    /// "pure virtual methods" of SfxPoolItem
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    String &rText,
                                    const IntlWrapper*    pIntl = 0 ) const;

    virtual bool QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    void SetStyleHandle( boost::shared_ptr<SfxItemSet> pHandle ) { mpHandle = pHandle; }
    const boost::shared_ptr<SfxItemSet> GetStyleHandle() const { return mpHandle; }
          boost::shared_ptr<SfxItemSet> GetStyleHandle() { return mpHandle; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
