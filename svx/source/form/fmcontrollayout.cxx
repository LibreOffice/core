/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fmcontrollayout.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:05:11 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifndef SVX_SOURCE_INC_FMCONTROLLAYOUT_HXX
#include "fmcontrollayout.hxx"
#endif
#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_VISUALEFFECT_HPP_
#include <com/sun/star/awt/VisualEffect.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

//........................................................................
namespace svxform
{
//........................................................................

    using namespace ::utl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::form;

    //--------------------------------------------------------------------
    namespace
    {
        const sal_Char* lcl_getApplikationNodeName( DocumentType _eDocType )
        {
            switch( _eDocType )
            {
            case eTextDocument:
                return "TextDocument";
            case eWebDocument:
                return "WebDocument";
            case eSpreadsheetDocument:
                return "SpreadsheetDocument";
            case eDrawingDocument:
                return "DrawingDocument";
            case ePresentationDocument:
                return "PresentationDocument";
            case eEnhancedForm:
                return "XMLFormDocument";
            case eDatabaseForm:
                return "DatabaseFormDocument";
            default:
                break;
            }
            DBG_ERROR( "lcl_getApplikationNodeName: unknown doc type!" );
            return "TextDocument";
        }
    }

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

            // if the control lives in an eForm or database, give it some nicer layout
            // Notice that in such documents, and FmXFormController will additionally enable
            // dynamic control border colors (i.e. the color changes when the mouse hovers over
            // the control, or the control has the focus)
/*
            if ( xPSI.is() && xPSI->hasPropertyByName( FM_PROP_BORDER ) && xPSI->hasPropertyByName( FM_PROP_BORDERCOLOR ) )
            {
                sal_Int16 nCurrentBorder = VisualEffect::NONE;
                OSL_VERIFY( _rxControlModel->getPropertyValue( FM_PROP_BORDER ) >>= nCurrentBorder );
                if ( nCurrentBorder != VisualEffect::NONE )
                {
                    if ( ( eDocType == eElectronicForm ) || ( eDocType == eDatabaseForm ) )
                    {
                        _rxControlModel->setPropertyValue( FM_PROP_BORDER, makeAny( VisualEffect::FLAT ) );
                        if ( nClassId != FormComponentType::GRIDCONTROL )
                        {
                        }
                    }
                }
            }
*/
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
        sConfigName += ::rtl::OUString::createFromAscii( lcl_getApplikationNodeName( _eDocType ) );
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

