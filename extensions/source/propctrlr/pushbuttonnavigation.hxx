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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyState.hpp>


namespace pcr
{



    //= PushButtonNavigation

    class PushButtonNavigation
    {
    protected:
        css::uno::Reference< css::beans::XPropertySet >
                    m_xControlModel;
        bool        m_bIsPushButton;

    public:
        /** ctor
            @param _rxControlModel
                the control model which is or will be bound
        */
        PushButtonNavigation(
            const css::uno::Reference< css::beans::XPropertySet >& _rxControlModel
        );

        /** returns the current value of the "ButtonType" property, taking into account
            the "virtual" button types such as "move-to-next-record button".
        */
        css::uno::Any
                getCurrentButtonType() const;

        /** sets the current value of the "ButtonType" property, taking into account
            the "virtual" button types such as "move-to-next-record button".
        */
        void    setCurrentButtonType( const css::uno::Any& _rValue ) const;

        /** retrieves the current state of the "ButtonType" property, taking into account
            the "virtual" button types such as "move-to-next-record button".
        */
        css::beans::PropertyState
                getCurrentButtonTypeState( ) const;

        /** returns the current value of the "TargetURL" property, taking into account
            that some URLs are special values caused by "virtual" ButtonTypes
        */
        css::uno::Any
                getCurrentTargetURL() const;

        /** sets the current value of the "TargetURL" property, taking into account
            that some URLs are special values caused by "virtual" ButtonTypes
        */
        void    setCurrentTargetURL( const css::uno::Any& _rValue ) const;

        /** retrieves the current state of the "TargetURL" property, taking into account
            that some URLs are special values caused by "virtual" ButtonTypes
        */
        css::beans::PropertyState
                getCurrentTargetURLState( ) const;

        /** determines whether the current button tpye is FormButtonType_URL
        */
        bool    currentButtonTypeIsOpenURL() const;

        /** determines whether the TargetURL property does currently denote a non-empty string
        */
        bool    hasNonEmptyCurrentTargetURL() const;

    private:
        sal_Int32 implGetCurrentButtonType() const;
    };


}   // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
