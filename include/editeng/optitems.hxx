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
#ifndef INCLUDED_EDITENG_OPTITEMS_HXX
#define INCLUDED_EDITENG_OPTITEMS_HXX

#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <editeng/editengdllapi.h>

// forward ---------------------------------------------------------------
namespace com { namespace sun { namespace star {
namespace linguistic2{
    class XSpellChecker1;
}}}}


// class SfxSpellCheckItem -----------------------------------------------

class EDITENG_DLLPUBLIC SfxSpellCheckItem: public SfxPoolItem
{
public:

    SfxSpellCheckItem( css::uno::Reference<
                            css::linguistic2::XSpellChecker1 >  &xChecker,
                       sal_uInt16 nWhich  );
    SfxSpellCheckItem( const SfxSpellCheckItem& rItem );

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;

    css::uno::Reference< css::linguistic2::XSpellChecker1 >
            GetXSpellChecker() const { return xSpellCheck; }

private:
    css::uno::Reference< css::linguistic2::XSpellChecker1 >   xSpellCheck;
};


// class SfxHyphenRegionItem ---------------------------------------------

class EDITENG_DLLPUBLIC SfxHyphenRegionItem: public SfxPoolItem
{
    sal_uInt8 nMinLead;
    sal_uInt8 nMinTrail;

public:

    SfxHyphenRegionItem( const sal_uInt16 nId  );
    SfxHyphenRegionItem( const SfxHyphenRegionItem& rItem );

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = nullptr ) const override;

    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*     Create( SvStream& rStrm, sal_uInt16 nVer ) const override;
    virtual SvStream&        Store( SvStream& rStrm, sal_uInt16 ) const override;

    inline sal_uInt8 &GetMinLead() { return nMinLead; }
    inline sal_uInt8 GetMinLead() const { return nMinLead; }

    inline sal_uInt8 &GetMinTrail() { return nMinTrail; }
    inline sal_uInt8 GetMinTrail() const { return nMinTrail; }

    inline SfxHyphenRegionItem& operator=( const SfxHyphenRegionItem& rNew )
    {
        nMinLead = rNew.GetMinLead();
        nMinTrail = rNew.GetMinTrail();
        return *this;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
