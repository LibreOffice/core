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
#ifndef INCLUDED_SW_INC_CHARFMT_HXX
#define INCLUDED_SW_INC_CHARFMT_HXX

#include "format.hxx"
#include "hintids.hxx"

class SW_DLLPUBLIC SwCharFormat : public SwFormat
{
    friend class SwDoc;
    friend class SwTextFormatColl;

    SwCharFormat( SwAttrPool& rPool, const sal_Char* pFormatName,
                SwCharFormat *pDerivedFrom )
          : SwFormat( rPool, pFormatName, aCharFormatSetRange, pDerivedFrom, RES_CHRFMT )
    {}
    SwCharFormat( SwAttrPool& rPool, const OUString &rFormatName,
                SwCharFormat *pDerivedFrom )
          : SwFormat( rPool, rFormatName, aCharFormatSetRange, pDerivedFrom, RES_CHRFMT )
    {}

public:

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

namespace CharFormat
{
    SW_DLLPUBLIC extern const SfxItemSet* GetItemSet( const SfxPoolItem& rAttr );
    extern const SfxPoolItem* GetItem( const SwTextAttr& rAttr, sal_uInt16 nWhich );
    template<class T> const T* GetItem( const SwTextAttr& rAttr, TypedWhichId<T> nWhich )
    {
        return static_cast<const T*>(GetItem(rAttr, sal_uInt16(nWhich)));
    }
    extern bool IsItemIncluded( const sal_uInt16 nWhich, const SwTextAttr *pAttr );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
