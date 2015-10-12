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
#ifndef INCLUDED_VCL_CONFIGSETTINGS_HXX
#define INCLUDED_VCL_CONFIGSETTINGS_HXX

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <unotools/configitem.hxx>
#include <vcl/dllapi.h>

#include <unordered_map>

namespace vcl
{
    typedef std::unordered_map< OUString, OUString, OUStringHash > OUStrMap;
    class SmallOUStrMap : public OUStrMap { public: SmallOUStrMap() : OUStrMap(1) {} };


    //= SettingsConfigItem

    class VCL_DLLPUBLIC SettingsConfigItem : public ::utl::ConfigItem
    {
    private:
        std::unordered_map< OUString, SmallOUStrMap, OUStringHash > m_aSettings;

        virtual void Notify( const com::sun::star::uno::Sequence< OUString >& rPropertyNames ) override;

        void getValues();
        SettingsConfigItem();

        virtual void ImplCommit() override;

    public:
        virtual ~SettingsConfigItem();

        static SettingsConfigItem* get();

        const OUString& getValue( const OUString& rGroup, const OUString& rKey ) const;
        void setValue( const OUString& rGroup, const OUString& rKey, const OUString& rValue );

    };


} // namespace vcl


#endif // INCLUDED_VCL_CONFIGSETTINGS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
