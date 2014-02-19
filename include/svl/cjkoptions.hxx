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
#ifndef INCLUDED_SVL_CJKOPTIONS_HXX
#define INCLUDED_SVL_CJKOPTIONS_HXX

#include <svl/svldllapi.h>
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
    SvtCJKOptions(bool bDontLoad = false);
    virtual ~SvtCJKOptions();

    bool IsCJKFontEnabled() const;
    bool IsVerticalTextEnabled() const;
    bool IsAsianTypographyEnabled() const;
    bool IsJapaneseFindEnabled() const;
    bool IsRubyEnabled() const;
    bool IsChangeCaseMapEnabled() const;
    bool IsDoubleLinesEnabled() const;

    void        SetAll(bool bSet);
    bool    IsAnyEnabled() const;
    bool    IsReadOnly(EOption eOption) const;
};

#endif // INCLUDED_SVL_CJKOPTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
