/*************************************************************************
 *
 *  $RCSfile: buttonnavigationhandler.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 17:24:52 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_BUTTONNAVIGATIONHANDLER_HXX
#include "buttonnavigationhandler.hxx"
#endif

#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
#endif
#ifndef EXTENSIONS_PROPCTRLR_PUSHBUTTONNAVIGATION_HXX
#include "pushbuttonnavigation.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::form;

    //====================================================================
    //= ButtonNavigationHandler
    //====================================================================
    //--------------------------------------------------------------------
    ButtonNavigationHandler::ButtonNavigationHandler( const Reference< XPropertySet >& _rxIntrospectee, const Reference< XTypeConverter >& _rxTypeConverter  )
        :PropertyHandler( _rxIntrospectee, _rxTypeConverter )
    {
    }

    //--------------------------------------------------------------------
    ButtonNavigationHandler::~ButtonNavigationHandler( )
    {
    }

    //--------------------------------------------------------------------
    PropertyState  SAL_CALL ButtonNavigationHandler::getPropertyState( PropertyId _nPropId ) const
    {
        PropertyState eState = PropertyState_DIRECT_VALUE;

        OSL_ENSURE( isNavigationCapableButton( m_xIntrospectee ), "ButtonNavigationHandler::getPropertyState: we don't have any SupportedProperties!" );

        switch ( _nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xIntrospectee );
            eState = aHelper.getCurrentButtonTypeState();
        }
        break;
        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xIntrospectee );
            eState = aHelper.getCurrentTargetURLState();
        }
        break;

        default:
            DBG_ERROR( "ButtonNavigationHandler::getPropertyState: cannot handle this property!" );
        }

        return eState;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL ButtonNavigationHandler::getPropertyValue( PropertyId _nPropId, bool _bLazy ) const
    {
        OSL_ENSURE( isNavigationCapableButton( m_xIntrospectee ), "ButtonNavigationHandler::getPropertyValue: we don't have any SupportedProperties!" );

        Any aReturn;

        switch ( _nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xIntrospectee );
            aReturn = aHelper.getCurrentButtonType();
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xIntrospectee );
            aReturn = aHelper.getCurrentTargetURL();
        }
        break;

        default:
            DBG_ERROR( "ButtonNavigationHandler::getPropertyValue: cannot handle this property!" );
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL ButtonNavigationHandler::setPropertyValue( PropertyId _nPropId, const Any& _rValue )
    {
        OSL_ENSURE( isNavigationCapableButton( m_xIntrospectee ), "ButtonNavigationHandler::setPropertyValue: we don't have any SupportedProperties!" );

        switch ( _nPropId )
        {
        case PROPERTY_ID_BUTTONTYPE:
        {
            PushButtonNavigation aHelper( m_xIntrospectee );
            aHelper.setCurrentButtonType( _rValue );
        }
        break;

        case PROPERTY_ID_TARGET_URL:
        {
            PushButtonNavigation aHelper( m_xIntrospectee );
            aHelper.setCurrentTargetURL( _rValue );
        }
        break;

        default:
            OSL_ENSURE( sal_False, "ButtonNavigationHandler::setPropertyValue: cannot handle this id!" );
        }
    }

    //--------------------------------------------------------------------
    bool ButtonNavigationHandler::isNavigationCapableButton( const Reference< XPropertySet >& _rxComponent )
    {
        Reference< XPropertySetInfo > xPSI;
        if ( _rxComponent.is() )
            xPSI = _rxComponent->getPropertySetInfo();

        return xPSI.is()
            && xPSI->hasPropertyByName( PROPERTY_TARGET_URL )
            && xPSI->hasPropertyByName( PROPERTY_BUTTONTYPE );
    }

    //--------------------------------------------------------------------
    ::std::vector< Property > SAL_CALL ButtonNavigationHandler::implDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;

        if ( isNavigationCapableButton( m_xIntrospectee ) )
        {
            addStringPropertyDescription( aProperties, PROPERTY_TARGET_URL );
            implAddPropertyDescription( aProperties, PROPERTY_BUTTONTYPE, ::getCppuType( static_cast< FormButtonType* >( NULL ) ) );
        }

        return aProperties;
    }

//........................................................................
}   // namespace pcr
//........................................................................

