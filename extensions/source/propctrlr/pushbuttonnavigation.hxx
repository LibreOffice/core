/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX
#define EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyState.hpp>

//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= PushButtonNavigation
    //========================================================================
    class PushButtonNavigation
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >
                    m_xControlModel;
        sal_Bool    m_bIsPushButton;

    public:
        /** ctor
            @param _rxControlModel
                the control model which is or will be bound
        */
        PushButtonNavigation(
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxControlModel
        );

        /** returns the current value of the "ButtonType" property, taking into account
            the "virtual" button types such as "move-to-next-record button".
        */
        ::com::sun::star::uno::Any
                getCurrentButtonType() const SAL_THROW(());

        /** sets the current value of the "ButtonType" property, taking into account
            the "virtual" button types such as "move-to-next-record button".
        */
        void    setCurrentButtonType( const ::com::sun::star::uno::Any& _rValue ) const SAL_THROW(());

        /** retrieves the current state of the "ButtonType" property, taking into account
            the "virtual" button types such as "move-to-next-record button".
        */
        ::com::sun::star::beans::PropertyState
                getCurrentButtonTypeState( ) const SAL_THROW(());

        /** returns the current value of the "TargetURL" property, taking into account
            that some URLs are special values caused by "virtual" ButtonTypes
        */
        ::com::sun::star::uno::Any
                getCurrentTargetURL() const SAL_THROW(());

        /** sets the current value of the "TargetURL" property, taking into account
            that some URLs are special values caused by "virtual" ButtonTypes
        */
        void    setCurrentTargetURL( const ::com::sun::star::uno::Any& _rValue ) const SAL_THROW(());

        /** retrieves the current state of the "TargetURL" property, taking into account
            that some URLs are special values caused by "virtual" ButtonTypes
        */
        ::com::sun::star::beans::PropertyState
                getCurrentTargetURLState( ) const SAL_THROW(());

        /** determines whether the current button tpye is FormButtonType_URL
        */
        bool    currentButtonTypeIsOpenURL() const;

        /** determines whether the TargetURL property does currently denote a non-empty string
        */
        bool    hasNonEmptyCurrentTargetURL() const;

    private:
        sal_Int32 implGetCurrentButtonType() const SAL_THROW(( ::com::sun::star::uno::Exception ));
    };

//............................................................................
}   // namespace pcr
//............................................................................

#endif // EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
