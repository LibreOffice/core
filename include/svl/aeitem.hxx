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
#ifndef INCLUDED_SVL_AEITEM_HXX
#define INCLUDED_SVL_AEITEM_HXX

#include <svl/svldllapi.h>
#include <svl/poolitem.hxx>
#include <svl/eitem.hxx>
#include <vector>

class SfxAllEnumValueArr;

class SVL_DLLPUBLIC SfxAllEnumItem: public SfxEnumItem
{
    SfxAllEnumValueArr*      pValues;
    std::vector<sal_uInt16>* pDisabledValues;

protected:
    sal_uInt16                  _GetPosByValue( sal_uInt16 nValue ) const;

public:
    TYPEINFO_OVERRIDE();
                            SfxAllEnumItem();
    explicit                SfxAllEnumItem( sal_uInt16 nWhich);
                            SfxAllEnumItem( sal_uInt16 nWhich, sal_uInt16 nVal );
                            SfxAllEnumItem( sal_uInt16 nWhich, SvStream &rStream );
                            SfxAllEnumItem( const SfxAllEnumItem & );
                            virtual ~SfxAllEnumItem();

    void                    InsertValue( sal_uInt16 nValue );
    void                    InsertValue( sal_uInt16 nValue, const OUString &rText );
    void                    RemoveValue( sal_uInt16 nValue );

    sal_uInt16              GetPosByValue( sal_uInt16 nValue ) const override;

    virtual sal_uInt16      GetValueCount() const override;
    virtual sal_uInt16      GetValueByPos( sal_uInt16 nPos ) const override;
    virtual OUString   GetValueTextByPos( sal_uInt16 nPos ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create(SvStream &, sal_uInt16 nVersion) const override;
    virtual bool        IsEnabled( sal_uInt16 ) const override;
    void                    DisableValue( sal_uInt16 );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
