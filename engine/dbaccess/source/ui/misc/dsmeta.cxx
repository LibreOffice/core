/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#include <dsmeta.hxx>
#include <connectivity/DriversConfig.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

#include <map>
#include <utility>

namespace dbaui
{

    using namespace ::com::sun::star;

    namespace {

    struct FeatureSupport
    {
        // authentication mode of the data source
        AuthenticationMode      eAuthentication;

        FeatureSupport()
            :eAuthentication( AuthUserPwd )
        {
        }

        explicit FeatureSupport(AuthenticationMode Auth)
            :eAuthentication( Auth )
        {
        }
    };

    }

    static AuthenticationMode getAuthenticationMode( const OUString& _sURL )
    {
        static std::map< OUString, FeatureSupport > s_aSupport = []()
        {
            std::map< OUString, FeatureSupport > tmp;
            ::connectivity::DriversConfig aDriverConfig(::comphelper::getProcessComponentContext());
            for (auto& url : aDriverConfig.getURLs())
            {
                FeatureSupport aInit( AuthNone );
                const ::comphelper::NamedValueCollection& aMetaData = aDriverConfig.getMetaData(url);
                if ( aMetaData.has(u"Authentication"_ustr) )
                {
                    OUString sAuth;
                    aMetaData.get(u"Authentication"_ustr) >>= sAuth;
                    if ( sAuth == "UserPassword" )
                        aInit = FeatureSupport(AuthUserPwd);
                    else if ( sAuth == "Password" )
                        aInit = FeatureSupport(AuthPwd);
                }
                tmp.insert(std::make_pair(url, aInit));
            }
            return tmp;
        }();
        OSL_ENSURE(s_aSupport.find(_sURL) != s_aSupport.end(),"Illegal URL!");
        return s_aSupport[ _sURL ].eAuthentication;
    }

    AuthenticationMode getAuthentication( const OUString& _sURL )
    {
        return getAuthenticationMode( _sURL );
    }

} // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
