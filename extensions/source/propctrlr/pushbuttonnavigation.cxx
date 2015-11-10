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

#include "pushbuttonnavigation.hxx"
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include "formstrings.hxx"
#include <comphelper/extract.hxx>
#include <comphelper/property.hxx>
#include <osl/diagnose.h>
#include <tools/diagnose_ex.h>


namespace pcr
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;


    namespace
    {
        static const sal_Int32 s_nFirstVirtualButtonType = 1 + (sal_Int32)FormButtonType_URL;

        static const sal_Char* pNavigationURLs[] =
        {
            ".uno:FormController/moveToFirst",
            ".uno:FormController/moveToPrev",
            ".uno:FormController/moveToNext",
            ".uno:FormController/moveToLast",
            ".uno:FormController/saveRecord",
            ".uno:FormController/undoRecord",
            ".uno:FormController/moveToNew",
            ".uno:FormController/deleteRecord",
            ".uno:FormController/refreshForm",
            nullptr
        };

        static sal_Int32 lcl_getNavigationURLIndex( const OUString& _rNavURL )
        {
            const sal_Char** pLookup = pNavigationURLs;
            while ( *pLookup )
            {
                if ( _rNavURL.equalsAscii( *pLookup ) )
                    return pLookup - pNavigationURLs;
                ++pLookup;
            }
            return -1;
        }

        static const sal_Char* lcl_getNavigationURL( sal_Int32 _nButtonTypeIndex )
        {
            const sal_Char** pLookup = pNavigationURLs;
            while ( _nButtonTypeIndex-- && *pLookup++ )
                ;
            OSL_ENSURE( *pLookup, "lcl_getNavigationURL: invalid index!" );
            return *pLookup;
        }
    }


    //= PushButtonNavigation


    PushButtonNavigation::PushButtonNavigation( const Reference< XPropertySet >& _rxControlModel )
        :m_xControlModel( _rxControlModel )
        ,m_bIsPushButton( false )
    {
        OSL_ENSURE( m_xControlModel.is(), "PushButtonNavigation::PushButtonNavigation: invalid control model!" );

        try
        {
            m_bIsPushButton = ::comphelper::hasProperty( PROPERTY_BUTTONTYPE, m_xControlModel );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "PushButtonNavigation::PushButtonNavigation: caught an exception!" );
        }
    }


    sal_Int32 PushButtonNavigation::implGetCurrentButtonType() const
    {
        sal_Int32 nButtonType = FormButtonType_PUSH;
        if ( !m_xControlModel.is() )
            return nButtonType;
        OSL_VERIFY( ::cppu::enum2int( nButtonType, m_xControlModel->getPropertyValue( PROPERTY_BUTTONTYPE ) ) );

        if ( nButtonType == FormButtonType_URL )
        {
            // there's a chance that this is a "virtual" button type
            // (which are realized by special URLs)
            OUString sTargetURL;
            m_xControlModel->getPropertyValue( PROPERTY_TARGET_URL ) >>= sTargetURL;

            sal_Int32 nNavigationURLIndex = lcl_getNavigationURLIndex( sTargetURL );
            if ( nNavigationURLIndex >= 0)
                // it actually *is* a virtual button type
                nButtonType = s_nFirstVirtualButtonType + nNavigationURLIndex;
        }
        return nButtonType;
    }


    Any PushButtonNavigation::getCurrentButtonType() const
    {
        OSL_ENSURE( m_bIsPushButton, "PushButtonNavigation::getCurrentButtonType: not expected to be called for forms!" );
        Any aReturn;

        try
        {
            aReturn <<= implGetCurrentButtonType();
        }
        catch( const Exception& )
        {
            OSL_FAIL( "PushButtonNavigation::getCurrentButtonType: caught an exception!" );
        }
        return aReturn;
    }


    void PushButtonNavigation::setCurrentButtonType( const Any& _rValue ) const
    {
        OSL_ENSURE( m_bIsPushButton, "PushButtonNavigation::setCurrentButtonType: not expected to be called for forms!" );
        if ( !m_xControlModel.is() )
            return;

        try
        {
            sal_Int32 nButtonType = FormButtonType_PUSH;
            OSL_VERIFY( ::cppu::enum2int( nButtonType, _rValue ) );
            OUString sTargetURL;

            bool bIsVirtualButtonType = nButtonType >= s_nFirstVirtualButtonType;
            if ( bIsVirtualButtonType )
            {
                const sal_Char* pURL = lcl_getNavigationURL( nButtonType - s_nFirstVirtualButtonType );
                sTargetURL = OUString::createFromAscii( pURL );

                nButtonType = FormButtonType_URL;
            }

            m_xControlModel->setPropertyValue( PROPERTY_BUTTONTYPE, makeAny( static_cast< FormButtonType >( nButtonType ) ) );
            m_xControlModel->setPropertyValue( PROPERTY_TARGET_URL, makeAny( sTargetURL ) );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "PushButtonNavigation::setCurrentButtonType: caught an exception!" );
        }
    }


    PropertyState PushButtonNavigation::getCurrentButtonTypeState( ) const
    {
        OSL_ENSURE( m_bIsPushButton, "PushButtonNavigation::getCurrentButtonTypeState: not expected to be called for forms!" );
        PropertyState eState = PropertyState_DIRECT_VALUE;

        try
        {
            Reference< XPropertyState > xStateAccess( m_xControlModel, UNO_QUERY );
            if ( xStateAccess.is() )
            {
                // let's see what the model says about the ButtonType property
                eState = xStateAccess->getPropertyState( PROPERTY_BUTTONTYPE );
                if ( eState == PropertyState_DIRECT_VALUE )
                {
                    sal_Int32 nRealButtonType = FormButtonType_PUSH;
                    OSL_VERIFY( ::cppu::enum2int( nRealButtonType, m_xControlModel->getPropertyValue( PROPERTY_BUTTONTYPE ) ) );
                    // perhaps it's one of the virtual button types?
                    if ( FormButtonType_URL == nRealButtonType )
                    {
                        // yes, it is -> rely on the state of the URL property
                        eState = xStateAccess->getPropertyState( PROPERTY_TARGET_URL );
                    }
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "PushButtonNavigation::getCurrentButtonTypeState: caught an exception!" );
        }

        return eState;
    }


    Any PushButtonNavigation::getCurrentTargetURL() const
    {
        Any aReturn;
        if ( !m_xControlModel.is() )
            return aReturn;

        try
        {
            aReturn = m_xControlModel->getPropertyValue( PROPERTY_TARGET_URL );
            if ( m_bIsPushButton )
            {
                sal_Int32 nCurrentButtonType = implGetCurrentButtonType();
                bool bIsVirtualButtonType = nCurrentButtonType >= s_nFirstVirtualButtonType;
                if ( bIsVirtualButtonType )
                {
                    // pretend (to the user) that there's no URL set - since
                    // virtual button types imply a special (technical) URL which
                    // the user should not see
                    aReturn <<= OUString();
                }
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "PushButtonNavigation::getCurrentTargetURL: caught an exception!" );
        }
        return aReturn;
    }


    void PushButtonNavigation::setCurrentTargetURL( const Any& _rValue ) const
    {
        if ( !m_xControlModel.is() )
            return;

        try
        {
            m_xControlModel->setPropertyValue( PROPERTY_TARGET_URL, _rValue );
        }
        catch( const Exception& )
        {
            OSL_FAIL( "PushButtonNavigation::setCurrentTargetURL: caught an exception!" );
        }
    }


    PropertyState PushButtonNavigation::getCurrentTargetURLState( ) const
    {
        PropertyState eState = PropertyState_DIRECT_VALUE;

        try
        {
            Reference< XPropertyState > xStateAccess( m_xControlModel, UNO_QUERY );
            if ( xStateAccess.is() )
            {
                eState = xStateAccess->getPropertyState( PROPERTY_TARGET_URL );
            }
        }
        catch( const Exception& )
        {
            OSL_FAIL( "PushButtonNavigation::setCurrentTargetURL: caught an exception!" );
        }

        return eState;
    }


    bool PushButtonNavigation::currentButtonTypeIsOpenURL() const
    {
        sal_Int32 nButtonType( FormButtonType_PUSH );
        try
        {
            nButtonType = implGetCurrentButtonType();
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return nButtonType == FormButtonType_URL;
    }


    bool PushButtonNavigation::hasNonEmptyCurrentTargetURL() const
    {
        OUString sTargetURL;
        OSL_VERIFY( getCurrentTargetURL() >>= sTargetURL );
        return !sTargetURL.isEmpty();
    }


}   // namespace pcr


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
