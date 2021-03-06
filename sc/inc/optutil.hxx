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

#include <unotools/configitem.hxx>
#include <tools/link.hxx>
#include "scdllapi.h"

class ScOptionsUtil
{
public:
    // values must correspond with integer values stored in the configuration
    enum KeyBindingType { KEY_DEFAULT = 0, KEY_OOO_LEGACY = 1 };

    static bool     IsMetricSystem();
};

//  ConfigItem for classes that use items from several sub trees

class SC_DLLPUBLIC ScLinkConfigItem final : public utl::ConfigItem
{
    Link<ScLinkConfigItem&,void>  aCommitLink;

public:
            ScLinkConfigItem( const OUString& rSubTree );
            ScLinkConfigItem( const OUString& rSubTree, ConfigItemMode nMode );
    void    SetCommitLink( const Link<ScLinkConfigItem&,void>& rLink );

    virtual void    Notify( const css::uno::Sequence<OUString>& aPropertyNames ) override;
    virtual void    ImplCommit() override;

    using ConfigItem::SetModified;
    css::uno::Sequence< css::uno::Any>
            GetProperties(const css::uno::Sequence< OUString >& rNames)
                            { return ConfigItem::GetProperties( rNames ); }
    void    PutProperties( const css::uno::Sequence< OUString >& rNames,
                            const css::uno::Sequence< css::uno::Any>& rValues)
                            { ConfigItem::PutProperties( rNames, rValues ); }

    using ConfigItem::EnableNotification;
    using ConfigItem::GetNodeNames;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
