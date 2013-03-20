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
#if 1

#include <unotools/configitem.hxx>
#include <tools/link.hxx>
#include "scdllapi.h"


class ScOptionsUtil
{
public:
    // values must correspond with integer values stored in the configuration
    enum KeyBindingType { KEY_DEFAULT = 0, KEY_OOO_LEGACY = 1 };

    static sal_Bool     IsMetricSystem();
};


//  ConfigItem for classes that use items from several sub trees

class SC_DLLPUBLIC ScLinkConfigItem : public utl::ConfigItem
{
    Link    aCommitLink;

public:
            ScLinkConfigItem( const rtl::OUString& rSubTree );
            ScLinkConfigItem( const rtl::OUString& rSubTree, sal_Int16 nMode );
    void    SetCommitLink( const Link& rLink );

    virtual void    Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames );
    virtual void    Commit();

    void    SetModified()   { ConfigItem::SetModified(); }
    com::sun::star::uno::Sequence< com::sun::star::uno::Any>
            GetProperties(const com::sun::star::uno::Sequence< rtl::OUString >& rNames)
                            { return ConfigItem::GetProperties( rNames ); }
    sal_Bool PutProperties( const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
                            const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues)
                            { return ConfigItem::PutProperties( rNames, rValues ); }

    using ConfigItem::EnableNotification;
    using ConfigItem::GetNodeNames;

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
