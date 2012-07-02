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
#ifndef _SVTOOLS_CJKOPTIONS_HXX
#define _SVTOOLS_CJKOPTIONS_HXX

#include "svl/svldllapi.h"
#include <sal/types.h>
#include <unotools/options.hxx>

class SvtCJKOptions_Impl;

// class SvtCJKOptions --------------------------------------------------

class SVL_DLLPUBLIC SvtCJKOptions: public utl::detail::Options
{
private:
    SvtCJKOptions_Impl*    pImp;

public:

    enum EOption
    {
        E_CJKFONT,
        E_VERTICALTEXT,
        E_ASIANTYPOGRAPHY,
        E_JAPANESEFIND,
        E_RUBY,
        E_CHANGECASEMAP,
        E_DOUBLELINES,
        E_EMPHASISMARKS,
        E_VERTICALCALLOUT,
        E_ALL               // special one for IsAnyEnabled()/SetAll() functionality
    };

    // bDontLoad is for referencing purposes only
    SvtCJKOptions(sal_Bool bDontLoad = sal_False);
    virtual ~SvtCJKOptions();

    sal_Bool IsCJKFontEnabled() const;
    sal_Bool IsVerticalTextEnabled() const;
    sal_Bool IsAsianTypographyEnabled() const;
    sal_Bool IsJapaneseFindEnabled() const;
    sal_Bool IsRubyEnabled() const;
    sal_Bool IsChangeCaseMapEnabled() const;
    sal_Bool IsDoubleLinesEnabled() const;

    void        SetAll(sal_Bool bSet);
    sal_Bool    IsAnyEnabled() const;
    sal_Bool    IsReadOnly(EOption eOption) const;
};

#endif // _SVTOOLS_CJKOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
