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
#ifndef INCLUDED_SVTOOLS_FONTSUBSTCONFIG_HXX
#define INCLUDED_SVTOOLS_FONTSUBSTCONFIG_HXX

#include <svtools/svtdllapi.h>
#include <unotools/configitem.hxx>
#include <memory>

struct SvtFontSubstConfig_Impl;


struct SubstitutionStruct
{
    OUString    sFont;
    OUString    sReplaceBy;
    bool        bReplaceAlways;
    bool        bReplaceOnScreenOnly;
};

class SVT_DLLPUBLIC SvtFontSubstConfig : public utl::ConfigItem
{
private:
    bool                        bIsEnabled;
    std::unique_ptr<SvtFontSubstConfig_Impl> pImpl;

    virtual void                ImplCommit() override;

public:
    SvtFontSubstConfig();
    virtual ~SvtFontSubstConfig();

    virtual void                Notify( const css::uno::Sequence< OUString >& _rPropertyNames) override;

    bool                        IsEnabled() const {return bIsEnabled;}
    void                        Enable(bool bSet)  {bIsEnabled = bSet; SetModified();}

    sal_Int32                   SubstitutionCount() const;
    void                        ClearSubstitutions();
    const SubstitutionStruct*   GetSubstitution(sal_Int32 nPos);
    void                        AddSubstitution(const SubstitutionStruct&   rToAdd);
    void                        Apply();
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
