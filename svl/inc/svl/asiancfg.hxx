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

#ifndef INCLUDED_SVL_ASIANCFG_HXX
#define INCLUDED_SVL_ASIANCFG_HXX

#include "sal/config.h"

#include "boost/noncopyable.hpp"
#include "boost/scoped_ptr.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "sal/types.h"
#include "svl/svldllapi.h"

namespace com { namespace sun { namespace star { namespace lang {
    struct Locale;
} } } }
namespace rtl { class OUString; }

class SVL_DLLPUBLIC SvxAsianConfig: private boost::noncopyable {
public:
    SvxAsianConfig();

    ~SvxAsianConfig();

    void Commit();

    bool IsKerningWesternTextOnly() const;

    void SetKerningWesternTextOnly(bool value);

    sal_Int16 GetCharDistanceCompression() const;

    void SetCharDistanceCompression(sal_Int16 value);

    com::sun::star::uno::Sequence< com::sun::star::lang::Locale >
    GetStartEndCharLocales() const;

    bool GetStartEndChars(
        com::sun::star::lang::Locale const & locale, rtl::OUString & startChars,
        rtl::OUString & endChars) const;

    void SetStartEndChars(
        com::sun::star::lang::Locale const & locale,
        rtl::OUString const * startChars, rtl::OUString const * endChars);

private:
    struct Impl;

    boost::scoped_ptr< Impl > impl_;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
