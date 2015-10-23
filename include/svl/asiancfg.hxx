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

#ifndef INCLUDED_SVL_ASIANCFG_HXX
#define INCLUDED_SVL_ASIANCFG_HXX

#include <sal/config.h>

#include <boost/noncopyable.hpp>
#include <memory>
#include <com/sun/star/uno/Sequence.hxx>
#include <sal/types.h>
#include <svl/svldllapi.h>

namespace com { namespace sun { namespace star { namespace lang {
    struct Locale;
} } } }

class SVL_DLLPUBLIC SvxAsianConfig: private boost::noncopyable {
public:
    SvxAsianConfig();

    ~SvxAsianConfig();

    void Commit();

    bool IsKerningWesternTextOnly() const;

    void SetKerningWesternTextOnly(bool value);

    sal_Int16 GetCharDistanceCompression() const;

    void SetCharDistanceCompression(sal_Int16 value);

    css::uno::Sequence< css::lang::Locale > GetStartEndCharLocales() const;

    bool GetStartEndChars(
        css::lang::Locale const & locale, OUString & startChars,
        OUString & endChars) const;

    void SetStartEndChars(
        css::lang::Locale const & locale,
        OUString const * startChars, OUString const * endChars);

private:
    struct Impl;

    std::unique_ptr< Impl > impl_;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
