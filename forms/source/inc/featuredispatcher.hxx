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

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Any.hxx>


namespace frm
{

    class IFeatureDispatcher
    {
    public:
        /** dispatches a feature

            @param _nFeatureId
                the id of the feature to dispatch
        */
        virtual void    dispatch( sal_Int16 _nFeatureId ) const = 0;

        /** dispatches a feature, with an additional named parameter

            @param _nFeatureId
                the id of the feature to dispatch

            @param _pParamAsciiName
                the Ascii name of the parameter of the dispatch call

            @param _rParamValue
                the value of the parameter of the dispatch call
        */
        virtual void dispatchWithArgument(
                sal_Int16 _nFeatureId,
                const OUString& _pParamName,
                const css::uno::Any& _rParamValue
             ) const = 0;

        /** checks whether a given feature is enabled
        */
        virtual bool    isEnabled( sal_Int16 _nFeatureId ) const = 0;

        /** returns the boolean state of a feature

            Certain features may support more status information than only the enabled/disabled
            state. The type of such additional information is fixed relative to a given feature, but
            may differ between different features.

            This method allows retrieving status information about features which have an additional
            boolean information associated with it.
        */
        virtual bool    getBooleanState( sal_Int16 _nFeatureId ) const = 0;

        /** returns the string state of a feature

            Certain features may support more status information than only the enabled/disabled
            state. The type of such additional information is fixed relative to a given feature, but
            may differ between different features.

            This method allows retrieving status information about features which have an additional
            string information associated with it.
        */
        virtual OUString getStringState( sal_Int16 _nFeatureId ) const = 0;

        /** returns the integer state of a feature

            Certain features may support more status information than only the enabled/disabled
            state. The type of such additional information is fixed relative to a given feature, but
            may differ between different features.

            This method allows retrieving status information about features which have an additional
            integer information associated with it.
        */
        virtual sal_Int32       getIntegerState( sal_Int16 _nFeatureId ) const = 0;

    protected:
        ~IFeatureDispatcher() {}
    };


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
