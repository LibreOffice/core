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
#pragma once

#include <sal/config.h>
#include <unotools/unotoolsdllapi.h>
#include <rtl/ustring.hxx>
#include <unordered_map>
#include <string_view>

// Loads the dialog options from config
class UNOTOOLS_DLLPUBLIC SvtOptionsDialogOptions
{
public:

    typedef std::unordered_map< OUString, bool > OptionNodeList;

    SvtOptionsDialogOptions();

    bool        IsGroupHidden   (   std::u16string_view _rGroup ) const;
    bool        IsPageHidden    (   std::u16string_view _rPage,
                                        std::u16string_view _rGroup ) const;
    bool        IsOptionHidden  (   std::u16string_view _rOption,
                                        std::u16string_view _rPage,
                                        std::u16string_view _rGroup ) const;
private:
    bool IsHidden( const OUString& _rPath ) const;

    OptionNodeList m_aOptionNodeList;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
