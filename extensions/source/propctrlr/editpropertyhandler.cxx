/*************************************************************************
 *
 *  $RCSfile: editpropertyhandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:04:18 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EDITPROPERTYHANDLER_HXX
#include "editpropertyhandler.hxx"
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
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPBROWSERUI_HXX
#include "propbrowserui.hxx"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#define TEXTTYPE_SINGLELINE     0
#define TEXTTYPE_MULTILINE      1
#define TEXTTYPE_RICHTEXT       2

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;

    //====================================================================
    //= EditPropertyHandler
    //====================================================================
    //--------------------------------------------------------------------
    EditPropertyHandler::EditPropertyHandler( const Reference< XPropertySet >& _rxIntrospectee, const Reference< XTypeConverter >& _rxTypeConverter  )
        :PropertyHandler( _rxIntrospectee, _rxTypeConverter )
    {
    }

    //--------------------------------------------------------------------
    EditPropertyHandler::~EditPropertyHandler( )
    {
    }

    //--------------------------------------------------------------------
    Any SAL_CALL EditPropertyHandler::getPropertyValue( PropertyId _nPropId, bool _bLazy ) const
    {
        Any aReturn;

        try
        {
            switch ( _nPropId )
            {
            case PROPERTY_ID_SHOW_SCROLLBARS:
            {
                sal_Bool bHasVScroll = sal_False;
                m_xIntrospectee->getPropertyValue( PROPERTY_VSCROLL ) >>= bHasVScroll;
                sal_Bool bHasHScroll = sal_False;
                m_xIntrospectee->getPropertyValue( PROPERTY_HSCROLL ) >>= bHasHScroll;

                aReturn <<= (sal_Int32)( ( bHasVScroll ? 2 : 0 ) + ( bHasHScroll ? 1 : 0 ) );
            }
            break;

            case PROPERTY_ID_TEXTTYPE:
            {
                sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
                sal_Bool bRichText = sal_False;
                OSL_VERIFY( m_xIntrospectee->getPropertyValue( PROPERTY_RICHTEXT ) >>= bRichText );
                if ( bRichText )
                    nTextType = TEXTTYPE_RICHTEXT;
                else
                {
                    sal_Bool bMultiLine = sal_False;
                    OSL_VERIFY( m_xIntrospectee->getPropertyValue( PROPERTY_MULTILINE ) >>= bMultiLine );
                    if ( bMultiLine )
                        nTextType = TEXTTYPE_MULTILINE;
                    else
                        nTextType = TEXTTYPE_SINGLELINE;
                }
                aReturn <<= nTextType;
            }
            break;


            default:
                DBG_ERROR( "EditPropertyHandler::getPropertyValue: cannot handle this property!" );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EditPropertyHandler::getPropertyValue: caught an exception!" );
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EditPropertyHandler::setPropertyValue( PropertyId _nPropId, const Any& _rValue )
    {
        try
        {
            switch ( _nPropId )
            {
            case PROPERTY_ID_SHOW_SCROLLBARS:
            {
                sal_Int32 nScrollbars = 0;
                _rValue >>= nScrollbars;

                sal_Bool bHasVScroll = 0 != ( nScrollbars & 2 );
                sal_Bool bHasHScroll = 0 != ( nScrollbars & 1 );

                m_xIntrospectee->setPropertyValue( PROPERTY_VSCROLL, makeAny( (sal_Bool)bHasVScroll ) );
                m_xIntrospectee->setPropertyValue( PROPERTY_HSCROLL, makeAny( (sal_Bool)bHasHScroll ) );
            }
            break;

            case PROPERTY_ID_TEXTTYPE:
            {
                sal_Bool bMultiLine = sal_False;
                sal_Bool bRichText = sal_False;
                sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
                OSL_VERIFY( _rValue >>= nTextType );
                switch ( nTextType )
                {
                case TEXTTYPE_SINGLELINE: bMultiLine = bRichText = sal_False; break;
                case TEXTTYPE_MULTILINE:  bMultiLine = sal_True; bRichText = sal_False; break;
                case TEXTTYPE_RICHTEXT:   bMultiLine = sal_True; bRichText = sal_True; break;
                default:
                    OSL_ENSURE( sal_False, "EditPropertyHandler::setPropertyValue: invalid text type!" );
                }

                m_xIntrospectee->setPropertyValue( PROPERTY_MULTILINE, makeAny( bMultiLine ) );
                m_xIntrospectee->setPropertyValue( PROPERTY_RICHTEXT, makeAny( bRichText ) );
            }
            break;

            default:
                OSL_ENSURE( sal_False, "EditPropertyHandler::setPropertyValue: cannot handle this id!" );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EditPropertyHandler::setPropertyValue: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    bool EditPropertyHandler::implHaveBothScrollBarProperties() const
    {
        // have a "Scrollbars" property if the object supports both "HScroll" and "VScroll"
        Reference< XPropertySetInfo > xPSI;
        if ( m_xIntrospectee.is() )
            xPSI = m_xIntrospectee->getPropertySetInfo();

        return xPSI.is()
            && xPSI->hasPropertyByName( PROPERTY_HSCROLL )
            && xPSI->hasPropertyByName( PROPERTY_VSCROLL );
    }

    //--------------------------------------------------------------------
    bool EditPropertyHandler::implHaveTextTypeProperty() const
    {
        // have a "Scrollbars" property if the object supports both "HScroll" and "VScroll"
        Reference< XPropertySetInfo > xPSI;
        if ( m_xIntrospectee.is() )
            xPSI = m_xIntrospectee->getPropertySetInfo();

        return xPSI.is()
            && xPSI->hasPropertyByName( PROPERTY_RICHTEXT )
            && xPSI->hasPropertyByName( PROPERTY_MULTILINE );
    }

    //--------------------------------------------------------------------
    ::std::vector< Property > SAL_CALL EditPropertyHandler::implDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;

        if ( implHaveBothScrollBarProperties() )
            addInt32PropertyDescription( aProperties, PROPERTY_SHOW_SCROLLBARS );

        if ( implHaveTextTypeProperty() )
            addInt32PropertyDescription( aProperties, PROPERTY_TEXTTYPE );

        return aProperties;
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL EditPropertyHandler::getSupersededProperties( ) const
    {
        ::std::vector< ::rtl::OUString > aSuperseded;
        if ( implHaveBothScrollBarProperties() )
        {
            aSuperseded.push_back( PROPERTY_HSCROLL );
            aSuperseded.push_back( PROPERTY_VSCROLL );
        }
        if ( implHaveTextTypeProperty() )
        {
            aSuperseded.push_back( PROPERTY_RICHTEXT );
            aSuperseded.push_back( PROPERTY_MULTILINE );
        }
        return aSuperseded;
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL EditPropertyHandler::getActuatingProperties( ) const
    {
        ::std::vector< ::rtl::OUString > aInterestingActuatingProps;
        if ( implHaveTextTypeProperty() )
            aInterestingActuatingProps.push_back( PROPERTY_TEXTTYPE );
        aInterestingActuatingProps.push_back( PROPERTY_MULTILINE );
        return aInterestingActuatingProps;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EditPropertyHandler::updateDependentProperties( PropertyId _nActuatingPropId, const Any& _rNewValue, const Any& _rOldValue, IPropertyBrowserUI* _pUpdater )
    {
        switch ( _nActuatingPropId )
        {
        case PROPERTY_ID_TEXTTYPE:
        {
            sal_Int32 nTextType = TEXTTYPE_SINGLELINE;
            getPropertyValue( PROPERTY_ID_TEXTTYPE ) >>= nTextType;

            _pUpdater->enablePropertyUI( PROPERTY_WORDBREAK,       nTextType == TEXTTYPE_RICHTEXT );
            _pUpdater->enablePropertyUI( PROPERTY_MAXTEXTLEN,      nTextType != TEXTTYPE_RICHTEXT );
            _pUpdater->enablePropertyUI( PROPERTY_ECHO_CHAR,       nTextType == TEXTTYPE_SINGLELINE );
            _pUpdater->enablePropertyUI( PROPERTY_FONT_NAME,       nTextType != TEXTTYPE_RICHTEXT );
            _pUpdater->enablePropertyUI( PROPERTY_ALIGN,           nTextType != TEXTTYPE_RICHTEXT );
            _pUpdater->enablePropertyUI( PROPERTY_DEFAULT_TEXT,    nTextType != TEXTTYPE_RICHTEXT );
            _pUpdater->enablePropertyUI( ::rtl::OUString::createFromAscii( "Font" ), nTextType != TEXTTYPE_RICHTEXT );
            _pUpdater->enablePropertyUI( PROPERTY_SHOW_SCROLLBARS, nTextType != TEXTTYPE_SINGLELINE );
            _pUpdater->enablePropertyUI( PROPERTY_LINEEND_FORMAT,  nTextType != TEXTTYPE_SINGLELINE );

            _pUpdater->showCategory( eData, nTextType != TEXTTYPE_RICHTEXT );
        }
        break;

        case PROPERTY_ID_MULTILINE:
        {
            sal_Bool bIsMultiline = sal_False;
            _rNewValue >>= bIsMultiline;

            _pUpdater->enablePropertyUI( PROPERTY_SHOW_SCROLLBARS, bIsMultiline );
            _pUpdater->enablePropertyUI( PROPERTY_ECHO_CHAR, !bIsMultiline );
            _pUpdater->enablePropertyUI( PROPERTY_LINEEND_FORMAT, bIsMultiline );
        }
        break;

        default:
            OSL_ENSURE( sal_False, "EditPropertyHandler::updateDependentProperties: cannot handle this id!" );
        }
    }

//........................................................................
}   // namespace pcr
//........................................................................

