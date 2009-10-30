/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include "fmcontrollayout.hxx"
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

/** === begin UNO includes === **/
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
/** === end UNO includes === **/
#include <tools/debug.hxx>
#include <comphelper/processfactory.hxx>

//........................................................................
namespace svxform
{
//........................................................................

    using namespace ::utl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;

    //====================================================================
    //= ControlLayouter
    //====================================================================
    //--------------------------------------------------------------------
    void ControlLayouter::initializeControlLayout( const Reference< XPropertySet >& _rxControlModel, DocumentType _eDocType )
    {
        DBG_ASSERT( _rxControlModel.is(), "ControlLayouter::initializeControlLayout: invalid model!" );
        if ( !_rxControlModel.is() )
            return;

        try
        {
            Reference< XPropertySetInfo > xPSI( _rxControlModel->getPropertySetInfo() );
            if ( !xPSI.is() )
                // can't do anything
                return;

            // the control type
            sal_Int16 nClassId = FormComponentType::CONTROL;
            _rxControlModel->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId;

            // the document type
            if ( _eDocType == eUnknownDocumentType )
                _eDocType = DocumentClassification::classifyHostDocument( _rxControlModel.get() );

            // let's see what the configuration says about the visual effect
            OConfigurationNode  aConfig = getLayoutSettings( _eDocType );
            Any aVisualEffect = aConfig.getNodeValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "VisualEffect" ) ) );
            if ( aVisualEffect.hasValue() )
            {
                ::rtl::OUString sVisualEffect;
                OSL_VERIFY( aVisualEffect >>= sVisualEffect );

                sal_Int16 nVisualEffect = VisualEffect::NONE;
                if ( sVisualEffect.equalsAscii( "flat" ) )
                    nVisualEffect = VisualEffect::FLAT;
                else if ( sVisualEffect.equalsAscii( "3D" ) )
                    nVisualEffect = VisualEffect::LOOK3D;

                if ( xPSI->hasPropertyByName( FM_PROP_BORDER ) )
                {
                    if  (  ( nClassId != FormComponentType::COMMANDBUTTON )
                        && ( nClassId != FormComponentType::RADIOBUTTON )
                        && ( nClassId != FormComponentType::CHECKBOX    )
                        && ( nClassId != FormComponentType::GROUPBOX )
                        && ( nClassId != FormComponentType::FIXEDTEXT )
                        && ( nClassId != FormComponentType::SCROLLBAR )
                        && ( nClassId != FormComponentType::SPINBUTTON )
                        )
                    {
                        _rxControlModel->setPropertyValue( FM_PROP_BORDER, makeAny( nVisualEffect ) );
                        if  (   ( nVisualEffect == VisualEffect::FLAT )
                            &&  ( xPSI->hasPropertyByName( FM_PROP_BORDERCOLOR ) )
                            )
                            // light gray flat border
                            _rxControlModel->setPropertyValue( FM_PROP_BORDERCOLOR, makeAny( (sal_Int32)0x00C0C0C0 ) );
                    }
                }
                if ( xPSI->hasPropertyByName( FM_PROP_VISUALEFFECT ) )
                    _rxControlModel->setPropertyValue( FM_PROP_VISUALEFFECT, makeAny( nVisualEffect ) );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "ControlLayouter::initializeControlLayout: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    ::utl::OConfigurationNode ControlLayouter::getLayoutSettings( DocumentType _eDocType )
    {
        ::rtl::OUString sConfigName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/org.openoffice.Office.Common/Forms/ControlLayout/" ) );
        sConfigName += DocumentClassification::getModuleIdentifierForDocumentType( _eDocType );
        return OConfigurationTreeRoot::createWithServiceFactory(
            ::comphelper::getProcessServiceFactory(),    // TODO
            sConfigName );
    }

    //--------------------------------------------------------------------
    bool ControlLayouter::useDynamicBorderColor( DocumentType _eDocType )
    {
        OConfigurationNode aConfig = getLayoutSettings( _eDocType );
        Any aDynamicBorderColor = aConfig.getNodeValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DynamicBorderColors" ) ) );
        bool bDynamicBorderColor = false;
        OSL_VERIFY( aDynamicBorderColor >>= bDynamicBorderColor );
        return bDynamicBorderColor;
    }

//........................................................................
} // namespace svxform
//........................................................................

