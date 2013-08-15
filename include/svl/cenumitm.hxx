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

#ifndef SVTOOLS_CENUMITM_HXX
#define SVTOOLS_CENUMITM_HXX

#include "svl/svldllapi.h"
#include <tools/debug.hxx>
#include <svl/poolitem.hxx>

//============================================================================
DBG_NAMEEX(SfxEnumItemInterface)

class SVL_DLLPUBLIC SfxEnumItemInterface: public SfxPoolItem
{
protected:
    explicit SfxEnumItemInterface(sal_uInt16 which): SfxPoolItem(which) {}

    SfxEnumItemInterface(const SfxEnumItemInterface & rItem):
        SfxPoolItem(rItem) {}

public:
    TYPEINFO();

    virtual int operator ==(const SfxPoolItem & rItem) const;

    virtual SfxItemPresentation GetPresentation(SfxItemPresentation,
                                                SfxMapUnit, SfxMapUnit,
                                                OUString & rText,
                                                const IntlWrapper * = 0)
        const;

    virtual bool QueryValue(com::sun::star::uno::Any & rVal, sal_uInt8 = 0) const;

    virtual bool PutValue(const com::sun::star::uno::Any & rVal, sal_uInt8 = 0);

    virtual sal_uInt16 GetValueCount() const = 0;

    virtual OUString GetValueTextByPos(sal_uInt16 nPos) const;

    virtual sal_uInt16 GetValueByPos(sal_uInt16 nPos) const;

    /// Return the position of some value within this enumeration.
    ///
    /// @descr  This method is implemented using GetValueCount() and
    /// GetValueByPos().  Derived classes may replace this with a more
    /// efficient implementation.
    ///
    /// @param nValue  Some value.
    ///
    /// @return  The position of nValue within this enumeration, or USHRT_MAX
    /// if not included.
    virtual sal_uInt16 GetPosByValue(sal_uInt16 nValue) const;

    virtual sal_Bool IsEnabled(sal_uInt16 nValue) const;

    virtual sal_uInt16 GetEnumValue() const = 0;

    virtual void SetEnumValue(sal_uInt16 nValue) = 0;

    virtual int HasBoolValue() const;

    virtual sal_Bool GetBoolValue() const;

    virtual void SetBoolValue(sal_Bool bValue);
};

#endif // SVTOOLS_CENUMITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
