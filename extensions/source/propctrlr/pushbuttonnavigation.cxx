/*************************************************************************
 *
 *  $RCSfile: pushbuttonnavigation.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-06 13:46:23 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX
#include "pushbuttonnavigation.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif

#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <cppuhelper/extract.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;

    //------------------------------------------------------------------------
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
            NULL
        };

        static sal_Int32 lcl_getNavigationURLIndex( const ::rtl::OUString& _rNavURL )
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

    //========================================================================
    //= PushButtonNavigation
    //========================================================================
    //------------------------------------------------------------------------
    PushButtonNavigation::PushButtonNavigation( const Reference< XPropertySet >& _rxControlModel )
        :m_xControlModel( _rxControlModel )
        ,m_bIsPushButton( sal_False )
    {
        OSL_ENSURE( m_xControlModel.is(), "PushButtonNavigation::PushButtonNavigation: invalid control model!" );

        try
        {
            m_bIsPushButton = ::comphelper::hasProperty( PROPERTY_BUTTONTYPE, m_xControlModel );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "PushButtonNavigation::PushButtonNavigation: caught an exception!" );
        }
    }

    //------------------------------------------------------------------------
    sal_Int32 PushButtonNavigation::implGetCurrentButtonType() const SAL_THROW((Exception))
    {
        sal_Int32 nButtonType = FormButtonType_PUSH;
        if ( !m_xControlModel.is() )
            return nButtonType;
        OSL_VERIFY( ::cppu::enum2int( nButtonType, m_xControlModel->getPropertyValue( PROPERTY_BUTTONTYPE ) ) );

        if ( nButtonType == FormButtonType_URL )
        {
            // there's a chance that this is a "virtual" button type
            // (which are realized by special URLs)
            ::rtl::OUString sTargetURL;
            m_xControlModel->getPropertyValue( PROPERTY_TARGET_URL ) >>= sTargetURL;

            sal_Int32 nNavigationURLIndex = lcl_getNavigationURLIndex( sTargetURL );
            if ( nNavigationURLIndex >= 0)
                // it actually *is* a virtual button type
                nButtonType = s_nFirstVirtualButtonType + nNavigationURLIndex;
        }
        return nButtonType;
    }

    //------------------------------------------------------------------------
    Any PushButtonNavigation::getCurrentButtonType() const SAL_THROW(())
    {
        OSL_ENSURE( m_bIsPushButton, "PushButtonNavigation::getCurrentButtonType: not expected to be called for forms!" );
        Any aReturn;

        try
        {
            aReturn <<= implGetCurrentButtonType();
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "PushButtonNavigation::getCurrentButtonType: caught an exception!" );
        }
        return aReturn;
    }

    //------------------------------------------------------------------------
    void PushButtonNavigation::setCurrentButtonType( const Any& _rValue ) const SAL_THROW(())
    {
        OSL_ENSURE( m_bIsPushButton, "PushButtonNavigation::setCurrentButtonType: not expected to be called for forms!" );
        if ( !m_xControlModel.is() )
            return;

        try
        {
            sal_Int32 nButtonType = FormButtonType_PUSH;
            OSL_VERIFY( ::cppu::enum2int( nButtonType, _rValue ) );
            ::rtl::OUString sTargetURL;

            bool bIsVirtualButtonType = nButtonType >= s_nFirstVirtualButtonType;
            if ( bIsVirtualButtonType )
            {
                const sal_Char* pURL = lcl_getNavigationURL( nButtonType - s_nFirstVirtualButtonType );
                sTargetURL = ::rtl::OUString::createFromAscii( pURL );

                nButtonType = FormButtonType_URL;
            }

            m_xControlModel->setPropertyValue( PROPERTY_BUTTONTYPE, makeAny( static_cast< FormButtonType >( nButtonType ) ) );
            m_xControlModel->setPropertyValue( PROPERTY_TARGET_URL, makeAny( sTargetURL ) );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "PushButtonNavigation::setCurrentButtonType: caught an exception!" );
        }
    }

    //------------------------------------------------------------------------
    PropertyState PushButtonNavigation::getCurrentButtonTypeState( ) const SAL_THROW(())
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
            OSL_ENSURE( sal_False, "PushButtonNavigation::getCurrentButtonTypeState: caught an exception!" );
        }

        return eState;
    }

    //------------------------------------------------------------------------
    Any PushButtonNavigation::getCurrentTargetURL() const SAL_THROW(())
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
                    aReturn <<= ::rtl::OUString();
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "PushButtonNavigation::getCurrentTargetURL: caught an exception!" );
        }
        return aReturn;
    }

    //------------------------------------------------------------------------
    void PushButtonNavigation::setCurrentTargetURL( const Any& _rValue ) const SAL_THROW(())
    {
        if ( !m_xControlModel.is() )
            return;

        try
        {
            m_xControlModel->setPropertyValue( PROPERTY_TARGET_URL, _rValue );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "PushButtonNavigation::setCurrentTargetURL: caught an exception!" );
        }
    }

    //------------------------------------------------------------------------
    PropertyState PushButtonNavigation::getCurrentTargetURLState( ) const SAL_THROW(())
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
            OSL_ENSURE( sal_False, "PushButtonNavigation::setCurrentTargetURL: caught an exception!" );
        }

        return eState;
    }

//............................................................................
}   // namespace pcr
//............................................................................
