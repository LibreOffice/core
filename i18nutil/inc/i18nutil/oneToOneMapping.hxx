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
#ifndef INCLUDED_I18NUTIL_TRANSLITERATION_ONETOONEMAPPING_HXX
#define INCLUDED_I18NUTIL_TRANSLITERATION_ONETOONEMAPPING_HXX

#include <boost/noncopyable.hpp>
#include <rtl/ustring.hxx>
#include "i18nutildllapi.h"

namespace com { namespace sun { namespace star { namespace i18n {

class widthfolding;

struct OneToOneMappingTable_t
{
    sal_Unicode first;
    sal_Unicode second;
};

typedef sal_Int8 UnicodePairFlag;
struct UnicodePairWithFlag
{
    sal_Unicode first;
    sal_Unicode second;
    UnicodePairFlag flag;
};

class I18NUTIL_DLLPUBLIC oneToOneMapping : private boost::noncopyable
{
public:
    oneToOneMapping( OneToOneMappingTable_t *rpTable, const size_t rnSize, const size_t rnUnitSize = sizeof(OneToOneMappingTable_t) );
    virtual ~oneToOneMapping();

    // binary search
    virtual sal_Unicode find( const sal_Unicode nKey ) const;

    // translator
    sal_Unicode operator[] ( const sal_Unicode nKey ) const { return find( nKey ); };

protected:
    OneToOneMappingTable_t *mpTable;
    size_t                  mnSize;
};

class I18NUTIL_DLLPUBLIC oneToOneMappingWithFlag : public oneToOneMapping
{
    friend class widthfolding;

public:
    oneToOneMappingWithFlag( UnicodePairWithFlag *rpTableWF, const size_t rnSize, const UnicodePairFlag rnFlag );
    virtual ~oneToOneMappingWithFlag();

    // make index for fast search
    void makeIndex();

    // index search
    virtual sal_Unicode find( const sal_Unicode nKey ) const;
protected:
    UnicodePairWithFlag  *mpTableWF;
    UnicodePairFlag       mnFlag;
    UnicodePairWithFlag **mpIndex[256];
    bool                  mbHasIndex;
};

} } } }

#endif // _I18N_TRANSLITERATION_ONETOONEMAPPING_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
