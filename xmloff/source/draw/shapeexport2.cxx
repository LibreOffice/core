/*************************************************************************
 *
 *  $RCSfile: shapeexport2.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: cl $ $Date: 2001-02-21 18:04:45 $
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

#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_CHART_XCHARTDOCUMENT_HPP_
#include <com/sun/star/chart/XChartDocument.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CONNECTORTYPE_HPP_
#include <com/sun/star/drawing/ConnectorType.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX3_HPP_
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#endif

#ifndef _XMLOFF_ANIM_HXX
#include "anim.hxx"
#endif

#ifndef _XMLOFF_SHAPEEXPORT_HXX
#include "shapeexport.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

#include "xmlkywd.hxx"
#include "xmlnmspe.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;


//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportNewTrans(const uno::Reference< beans::XPropertySet >& xPropSet,
    sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    // get matrix
    Matrix3D aMat;
    ImpExportNewTrans_GetMatrix3D(aMat, xPropSet);

    // decompose and correct abour pRefPoint
    Vector2D aTRScale;
    double fTRShear(0.0);
    double fTRRotate(0.0);
    Vector2D aTRTranslate;
    ImpExportNewTrans_DecomposeAndRefPoint(aMat, aTRScale, fTRShear, fTRRotate, aTRTranslate, pRefPoint);

    // use features and write
    ImpExportNewTrans_FeaturesAndWrite(aTRScale, fTRShear, fTRRotate, aTRTranslate, nFeatures);
}

void XMLShapeExport::ImpExportNewTrans_GetMatrix3D(Matrix3D& rMat,
    const uno::Reference< beans::XPropertySet >& xPropSet)
{
    uno::Any aAny = xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Transformation")));
    drawing::HomogenMatrix3 aMatrix;
    aAny >>= aMatrix;
    rMat[0] = Point3D( aMatrix.Line1.Column1, aMatrix.Line1.Column2, aMatrix.Line1.Column3 );
    rMat[1] = Point3D( aMatrix.Line2.Column1, aMatrix.Line2.Column2, aMatrix.Line2.Column3 );
    rMat[2] = Point3D( aMatrix.Line3.Column1, aMatrix.Line3.Column2, aMatrix.Line3.Column3 );
}

void XMLShapeExport::ImpExportNewTrans_DecomposeAndRefPoint(const Matrix3D& rMat,
    Vector2D& rTRScale, double& fTRShear, double& fTRRotate, Vector2D& rTRTranslate,
    awt::Point* pRefPoint)
{
    // decompose matrix
    rMat.DecomposeAndCorrect(rTRScale, fTRShear, fTRRotate, rTRTranslate);

    // correct translation about pRefPoint
    if(pRefPoint)
    {
        rTRTranslate.X() -= pRefPoint->X;
        rTRTranslate.Y() -= pRefPoint->Y;
    }
}

void XMLShapeExport::ImpExportNewTrans_FeaturesAndWrite(Vector2D& rTRScale, double fTRShear,
    double fTRRotate, Vector2D& rTRTranslate, const sal_Int32 nFeatures)
{
    SdXMLImExTransform2D aTransform;

    if(!(nFeatures & SEF_EXPORT_WIDTH))
        rTRScale.X() = 0.0;

    if(!(nFeatures & SEF_EXPORT_HEIGHT))
        rTRScale.Y() = 0.0;

    if(!(nFeatures & SEF_EXPORT_X))
        rTRTranslate.X() = 0.0;

    if(!(nFeatures & SEF_EXPORT_Y))
        rTRTranslate.Y() = 0.0;

    aTransform.AddScale(rTRScale);
    aTransform.AddSkewX(fTRShear);
    aTransform.AddRotate(fTRRotate);
    aTransform.AddTranslate(rTRTranslate);

    // does transformation need to be exported?
    if(aTransform.NeedsAction())
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_transform, aTransform.GetExportString(rExport.GetMM100UnitConverter()));
}

//////////////////////////////////////////////////////////////////////////////

sal_Bool XMLShapeExport::ImpExportPresentationAttributes( const uno::Reference< beans::XPropertySet >& xPropSet, const rtl::OUString& rClass )
{
    sal_Bool bIsEmpty = sal_False;

    OUStringBuffer sStringBuffer;

    // write presentation class entry
    rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_class, rClass);

    if( xPropSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        sal_Bool bTemp;

        // is empty pes shape?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject"))))
        {
            xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject"))) >>= bIsEmpty;
            if( bIsEmpty )
                rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_placeholder, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)));
        }

        // is user-transformed?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent"))))
        {
            xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent"))) >>= bTemp;
            if(!bTemp)
                rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_user_transformed, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_true)));
        }
    }

    return bIsEmpty;
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportText( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() && xText->getString().getLength() )
        rExport.GetTextParagraphExport()->exportText( xText );
}

//////////////////////////////////////////////////////////////////////////////

#ifndef _COM_SUN_STAR_PRESENTATION_CLICKACTION_HPP_
#include <com/sun/star/presentation/ClickAction.hpp>
#endif
#ifndef _COM_SUN_STAR_PRESENTATION_ANIMATIONSPEED_HPP_
#include <com/sun/star/presentation/AnimationSpeed.hpp>
#endif

#define FOUND_CLICKACTION   0x0001
#define FOUND_BOOKMARK      0x0002
#define FOUND_EFFECT        0x0004
#define FOUND_PLAYFULL      0x0008
#define FOUND_VERB          0x0010
#define FOUND_SOUNDURL      0x0020
#define FOUND_SPEED         0x0040
#define FOUND_EVENTTYPE     0x0080
#define FOUND_MACRO         0x0100
#define FOUND_LIBRARY       0x0200

void XMLShapeExport::ImpExportEvents( const uno::Reference< drawing::XShape >& xShape )
{
    do
    {
        uno::Reference< document::XEventsSupplier > xEventsSupplier( xShape, uno::UNO_QUERY );
        if( !xEventsSupplier.is() )
            break;

        uno::Reference< container::XNameReplace > xEvents( xEventsSupplier->getEvents() );
        DBG_ASSERT( xEvents.is(), "XEventsSupplier::getEvents() returned NULL" );
        if( !xEvents.is() )
            break;

        uno::Sequence< beans::PropertyValue > aProperties;
        if( !xEvents->hasByName( msOnClick ) )
            break;

        if( !(xEvents->getByName( msOnClick ) >>= aProperties) )
            break;

        sal_Int32 nFound = 0;
        const beans::PropertyValue* pProperties = aProperties.getConstArray();

        OUString aStrEventType;
        presentation::ClickAction eClickAction;
        presentation::AnimationEffect eEffect;
        presentation::AnimationSpeed eSpeed;
        OUString aStrSoundURL;
        sal_Bool bPlayFull;
        sal_Int32 nVerb;
        OUString aStrMacro;
        OUString aStrLibrary;
        OUString aStrBookmark;

        const sal_Int32 nCount = aProperties.getLength();
        sal_Int32 nIndex;
        for( nIndex = 0; nIndex < nCount; nIndex++, pProperties++ )
        {
            if( ( ( nFound & FOUND_EVENTTYPE ) == 0 ) && pProperties->Name == msEventType )
            {
                if( pProperties->Value >>= aStrEventType )
                    nFound |= FOUND_EVENTTYPE;
            }
            else if( ( ( nFound & FOUND_CLICKACTION ) == 0 ) && pProperties->Name == msClickAction )
            {
                if( pProperties->Value >>= eClickAction )
                    nFound |= FOUND_CLICKACTION;
            }
            else if( ( ( nFound & FOUND_MACRO ) == 0 ) && pProperties->Name == msMacroName )
            {
                if( pProperties->Value >>= aStrMacro )
                    nFound |= FOUND_MACRO;
            }
            else if( ( ( nFound & FOUND_LIBRARY ) == 0 ) && pProperties->Name == msLibrary )
            {
                if( pProperties->Value >>= aStrLibrary )
                    nFound |= FOUND_LIBRARY;
            }
            else if( ( ( nFound & FOUND_EFFECT ) == 0 ) && pProperties->Name == msEffect )
            {
                if( pProperties->Value >>= eEffect )
                    nFound |= FOUND_EFFECT;
            }
            else if( ( ( nFound & FOUND_BOOKMARK ) == 0 ) && pProperties->Name == msBookmark )
            {
                if( pProperties->Value >>= aStrBookmark )
                    nFound |= FOUND_BOOKMARK;
            }
            else if( ( ( nFound & FOUND_SPEED ) == 0 ) && pProperties->Name == msSpeed )
            {
                if( pProperties->Value >>= eSpeed )
                    nFound |= FOUND_SPEED;
            }
            else if( ( ( nFound & FOUND_SOUNDURL ) == 0 ) && pProperties->Name == msSoundURL )
            {
                if( pProperties->Value >>= aStrSoundURL )
                    nFound |= FOUND_SOUNDURL;
            }
            else if( ( ( nFound & FOUND_PLAYFULL ) == 0 ) && pProperties->Name == msPlayFull )
            {
                if( pProperties->Value >>= bPlayFull )
                    nFound |= FOUND_PLAYFULL;
            }
            else if( ( ( nFound & FOUND_VERB ) == 0 ) && pProperties->Name == msVerb )
            {
                if( pProperties->Value >>= nVerb )
                    nFound |= FOUND_VERB;
            }
        }

        if( ( nFound & FOUND_EVENTTYPE ) == 0 )
            break;

        if( aStrEventType == msPresentation )
        {
            if( ( nFound & FOUND_CLICKACTION ) == 0 )
                break;

            if( eClickAction == presentation::ClickAction_NONE )
                break;

            SvXMLElementExport aEventsElemt(rExport, XML_NAMESPACE_OFFICE, sXML_events, sal_True, sal_True);

            OUString aStrAction;

            switch( eClickAction )
            {
            case presentation::ClickAction_PREVPAGE:        aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_previous_page ) ); break;
            case presentation::ClickAction_NEXTPAGE:        aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_next_page ) ); break;
            case presentation::ClickAction_FIRSTPAGE:       aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_first_page ) ); break;
            case presentation::ClickAction_LASTPAGE:        aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_last_page ) ); break;
            case presentation::ClickAction_INVISIBLE:       aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_hide ) ); break;
            case presentation::ClickAction_STOPPRESENTATION:aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_stop ) ); break;
            case presentation::ClickAction_PROGRAM:         aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_execute ) ); break;
            case presentation::ClickAction_BOOKMARK:        aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_show ) ); break;
            case presentation::ClickAction_DOCUMENT:        aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_show ) ); break;
            case presentation::ClickAction_MACRO:           aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_execute_macro ) ); break;
            case presentation::ClickAction_VERB:            aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_verb ) ); break;
            case presentation::ClickAction_VANISH:          aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_fade_out ) ); break;
            case presentation::ClickAction_SOUND:           aStrAction = OUString( RTL_CONSTASCII_USTRINGPARAM( sXML_sound ) ); break;
            default:
                DBG_ERROR( "unknown presentation::ClickAction found!" );
            }

            rExport.AddAttribute( XML_NAMESPACE_SCRIPT, sXML_event_name, OUString( RTL_CONSTASCII_USTRINGPARAM( "on-click" ) ) );
            rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_action, aStrAction );

            if( eClickAction == presentation::ClickAction_VANISH )
            {
                if( nFound & FOUND_EFFECT )
                {
                    XMLEffect eKind;
                    XMLEffectDirection eDirection;
                    sal_Int16 nStartScale;
                    sal_Bool bIn;

                    SdXMLImplSetEffect( eEffect, eKind, eDirection, nStartScale, bIn );

                    if( eEffect != EK_none )
                    {
                        SvXMLUnitConverter::convertEnum( msBuffer, eKind, aXML_AnimationEffect_EnumMap );
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_effect, msBuffer.makeStringAndClear() );
                    }

                    if( eDirection != ED_none )
                    {
                        SvXMLUnitConverter::convertEnum( msBuffer, eDirection, aXML_AnimationDirection_EnumMap );
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_direction, msBuffer.makeStringAndClear() );
                    }

                    if( nStartScale != -1 )
                    {
                        SvXMLUnitConverter::convertPercent( msBuffer, nStartScale );
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_start_scale, msBuffer.makeStringAndClear() );
                    }
                }

                if( nFound & FOUND_SPEED && eEffect != presentation::AnimationEffect_NONE )
                {
                    if( eSpeed != presentation::AnimationSpeed_MEDIUM )
                    {
                        SvXMLUnitConverter::convertEnum( msBuffer, eSpeed, aXML_AnimationSpeed_EnumMap );
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, sXML_speed, msBuffer.makeStringAndClear() );
                    }
                }
            }

            if( eClickAction == presentation::ClickAction_PROGRAM ||
                eClickAction == presentation::ClickAction_BOOKMARK ||
                eClickAction == presentation::ClickAction_DOCUMENT )
            {
                if( eClickAction == presentation::ClickAction_BOOKMARK )
                    msBuffer.append( sal_Unicode('#') );

                msBuffer.append( aStrBookmark );
                rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_href, msBuffer.makeStringAndClear() );
                rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_type, sXML_simple );
                rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_show, sXML_new );
                rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_actuate, sXML_onRequest );
            }

            if( ( nFound & FOUND_VERB ) && eClickAction == presentation::ClickAction_VERB )
            {
                msBuffer.append( nVerb );
                rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_verb, msBuffer.makeStringAndClear());
            }

            SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_PRESENTATION, sXML_event, sal_True, sal_True);

            if( eClickAction == presentation::ClickAction_VANISH || eClickAction == presentation::ClickAction_SOUND )
            {
                if( ( nFound & FOUND_SOUNDURL ) && aStrSoundURL.getLength() != 0 )
                {
                    rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_href, aStrSoundURL );
                    rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_type, sXML_simple );
                    rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_show, sXML_new );
                    rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_actuate, sXML_onRequest );
                    if( nFound & FOUND_PLAYFULL && bPlayFull )
                        rExport.AddAttributeASCII( XML_NAMESPACE_PRESENTATION, sXML_play_full, sXML_true );

                    SvXMLElementExport aElem( rExport, XML_NAMESPACE_PRESENTATION, sXML_sound, sal_True, sal_True );
                }
            }
            break;
        }
        else if( aStrEventType == msStarBasic )
        {
            if( nFound & FOUND_MACRO )
            {
                SvXMLElementExport aEventsElemt(rExport, XML_NAMESPACE_OFFICE, sXML_events, sal_True, sal_True);

                rExport.AddAttribute( XML_NAMESPACE_SCRIPT, sXML_language, OUString( RTL_CONSTASCII_USTRINGPARAM( "starbasic" ) ) );
                rExport.AddAttribute( XML_NAMESPACE_SCRIPT, sXML_event_name, OUString( RTL_CONSTASCII_USTRINGPARAM( "on-click" ) ) );
                rExport.AddAttribute( XML_NAMESPACE_SCRIPT, sXML_macro_name, aStrMacro );

                if( nFound & FOUND_LIBRARY )
                    rExport.AddAttribute( XML_NAMESPACE_SCRIPT, sXML_library, aStrLibrary );

                SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SCRIPT, sXML_event, sal_True, sal_True);
            }
        }
    }
    while(0);
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportGroupShape( const uno::Reference< drawing::XShape >& xShape, XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY);
    if(xShapes.is() && xShapes->getCount())
    {
        // write group shape
        SvXMLElementExport aPGR(rExport, XML_NAMESPACE_DRAW, sXML_g, sal_True, sal_True);

        ImpExportEvents( xShape );

        // write members
        exportShapes( xShapes, nFeatures, pRefPoint );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportTextBoxShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        // presentation attribute (if presentation)
        sal_Bool bIsPresShape(FALSE);
        sal_Bool bIsEmptyPresObj(FALSE);
        OUString aStr;

        switch(eShapeType)
        {
            case XmlShapeTypePresSubtitleShape:
            {
                aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_subtitle));
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresTitleTextShape:
            {
                aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_title));
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresOutlinerShape:
            {
                aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_outline));
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresNotesShape:
            {
                aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_notes));
                bIsPresShape = TRUE;
                break;
            }
        }

        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        if(bIsPresShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, aStr );

        // write text-box
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_text_box, sal_True, sal_True);

        ImpExportEvents( xShape );
        if(!bIsEmptyPresObj)
            ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportRectangleShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        // evtl. corner radius?
        sal_Int32 nCornerRadius(0L);
        xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("CornerRadius"))) >>= nCornerRadius;
        if(nCornerRadius)
        {
            OUStringBuffer sStringBuffer;
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nCornerRadius);
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_corner_radius, sStringBuffer.makeStringAndClear());
        }

        // write rectangle
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_rect, sal_True, sal_True);

        ImpExportEvents( xShape );
        ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportLineShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        OUString aStr;
        OUStringBuffer sStringBuffer;
        awt::Point aStart(0,0);
        awt::Point aEnd(1,1);

        drawing::PointSequenceSequence* pSourcePolyPolygon = 0L;
        uno::Any aAny = xPropSet->getPropertyValue(
            OUString(RTL_CONSTASCII_USTRINGPARAM("PolyPolygon")));
        pSourcePolyPolygon = (drawing::PointSequenceSequence*)aAny.getValue();

        if(pSourcePolyPolygon)
        {
            drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();
            if(pOuterSequence)
            {
                drawing::PointSequence* pInnerSequence = pOuterSequence++;
                if(pInnerSequence)
                {
                    awt::Point* pArray = pInnerSequence->getArray();
                    if(pArray)
                    {
                        if(pInnerSequence->getLength() > 0)
                        {
                            aStart = awt::Point(pArray->X, pArray->Y);
                            pArray++;
                        }

                        if(pInnerSequence->getLength() > 1)
                        {
                            aEnd = awt::Point(pArray->X, pArray->Y);
                        }
                    }
                }
            }
        }

        if( pRefPoint )
        {
            aStart.X -= pRefPoint->X;
            aStart.Y -= pRefPoint->Y;
            aEnd.X -= pRefPoint->X;
            aEnd.Y -= pRefPoint->Y;
        }

        if( nFeatures & SEF_EXPORT_X )
        {
            // svg: x1
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.X);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x1, aStr);
        }
        else
        {
            aEnd.X -= aStart.X;
        }

        if( nFeatures & SEF_EXPORT_Y )
        {
            // svg: y1
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.Y);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y1, aStr);
        }
        else
        {
            aEnd.Y -= aStart.Y;
        }

        // svg: x2
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.X);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x2, aStr);

        // svg: y2
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.Y);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y2, aStr);

        // write line
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_line, sal_True, sal_True);

        ImpExportEvents( xShape );
        ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportEllipseShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // get size to decide between Circle and Ellipse
        awt::Size aSize = xShape->getSize();
        sal_Int32 nRx((aSize.Width + 1) / 2);
        sal_Int32 nRy((aSize.Height + 1) / 2);
        BOOL bCircle(nRx == nRy);

        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        drawing::CircleKind eKind = drawing::CircleKind_FULL;
        xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleKind")) ) >>= eKind;
        if( eKind != drawing::CircleKind_FULL )
        {
            OUStringBuffer sStringBuffer;
            sal_Int32 nStartAngle;
            sal_Int32 nEndAngle;
            xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleStartAngle")) ) >>= nStartAngle;
            xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleEndAngle")) ) >>= nEndAngle;

            const double dStartAngle = nStartAngle / 100.0;
            const double dEndAngle = nEndAngle / 100.0;

            // export circle kind
            SvXMLUnitConverter::convertEnum( sStringBuffer, (USHORT)eKind, aXML_CircleKind_EnumMap );
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_kind, sStringBuffer.makeStringAndClear() );

            // export start angle
            SvXMLUnitConverter::convertNumber( sStringBuffer, dStartAngle );
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_start_angle, sStringBuffer.makeStringAndClear() );

            // export end angle
            SvXMLUnitConverter::convertNumber( sStringBuffer, dEndAngle );
            rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_end_angle, sStringBuffer.makeStringAndClear() );
        }

        if(bCircle)
        {
            // write circle
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_circle, sal_True, sal_True);

            ImpExportEvents( xShape );
            ImpExportText( xShape );
        }
        else
        {
            // write ellipse
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_ellipse, sal_True, sal_True);

            ImpExportEvents( xShape );
            ImpExportText( xShape );
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportPolygonShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        BOOL bClosed(eShapeType == XmlShapeTypeDrawPolyPolygonShape
            || eShapeType == XmlShapeTypeDrawClosedBezierShape);
        BOOL bBezier(eShapeType == XmlShapeTypeDrawClosedBezierShape
            || eShapeType == XmlShapeTypeDrawOpenBezierShape);

        // get matrix
        Matrix3D aMat;
        ImpExportNewTrans_GetMatrix3D(aMat, xPropSet);

        // decompose and correct abour pRefPoint
        Vector2D aTRScale;
        double fTRShear(0.0);
        double fTRRotate(0.0);
        Vector2D aTRTranslate;
        ImpExportNewTrans_DecomposeAndRefPoint(aMat, aTRScale, fTRShear, fTRRotate, aTRTranslate, pRefPoint);

        // use features and write
        ImpExportNewTrans_FeaturesAndWrite(aTRScale, fTRShear, fTRRotate, aTRTranslate, nFeatures);

        // create and export ViewBox
        awt::Point aPoint(0, 0);
        awt::Size aSize(FRound(aTRScale.X()), FRound(aTRScale.Y()));
        SdXMLImExViewBox aViewBox(0, 0, aSize.Width, aSize.Height);
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_viewBox, aViewBox.GetExportString(rExport.GetMM100UnitConverter()));

        if(bBezier)
        {
            // get PolygonBezier
            uno::Any aAny( xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry"))) );
            drawing::PolyPolygonBezierCoords* pSourcePolyPolygon =
                (drawing::PolyPolygonBezierCoords*)aAny.getValue();

            if(pSourcePolyPolygon && pSourcePolyPolygon->Coordinates.getLength())
            {
                sal_Int32 nOuterCnt(pSourcePolyPolygon->Coordinates.getLength());
                drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->Coordinates.getArray();
                drawing::FlagSequence*  pOuterFlags = pSourcePolyPolygon->Flags.getArray();

                if(pOuterSequence && pOuterFlags)
                {
                    // prepare svx:d element export
                    SdXMLImExSvgDElement aSvgDElement(aViewBox);

                    for(sal_Int32 a(0L); a < nOuterCnt; a++)
                    {
                        drawing::PointSequence* pSequence = pOuterSequence++;
                        drawing::FlagSequence* pFlags = pOuterFlags++;

                        if(pSequence && pFlags)
                        {
                            aSvgDElement.AddPolygon(pSequence, pFlags,
                                aPoint, aSize, rExport.GetMM100UnitConverter(), bClosed);
                        }
                    }

                    // write point array
                    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_d, aSvgDElement.GetExportString());
                }

                // write object now
                SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_path, sal_True, sal_True);

                ImpExportEvents( xShape );
                ImpExportText( xShape );
            }
        }
        else
        {
            // get non-bezier polygon
            uno::Any aAny( xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry"))) );
            drawing::PointSequenceSequence* pSourcePolyPolygon = (drawing::PointSequenceSequence*)aAny.getValue();

            if(pSourcePolyPolygon && pSourcePolyPolygon->getLength())
            {
                sal_Int32 nOuterCnt(pSourcePolyPolygon->getLength());

                if(1L == nOuterCnt && !bBezier)
                {
                    // simple polygon shape, can be written as svg:points sequence
                    drawing::PointSequence* pSequence = pSourcePolyPolygon->getArray();
                    if(pSequence)
                    {
                        SdXMLImExPointsElement aPoints(pSequence, aViewBox, aPoint, aSize, rExport.GetMM100UnitConverter());

                        // write point array
                        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_points, aPoints.GetExportString());
                    }

                    // write object now
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW,
                        bClosed ? sXML_polygon : sXML_polyline , sal_True, sal_True);

                    ImpExportEvents( xShape );
                    ImpExportText( xShape );
                }
                else
                {
                    // polypolygon or bezier, needs to be written as a svg:path sequence
                    drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();
                    if(pOuterSequence)
                    {
                        // prepare svx:d element export
                        SdXMLImExSvgDElement aSvgDElement(aViewBox);

                        for(sal_Int32 a(0L); a < nOuterCnt; a++)
                        {
                            drawing::PointSequence* pSequence = pOuterSequence++;
                            if(pSequence)
                            {
                                aSvgDElement.AddPolygon(pSequence, 0L, aPoint,
                                    aSize, rExport.GetMM100UnitConverter(), bClosed);
                            }
                        }

                        // write point array
                        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_d, aSvgDElement.GetExportString());
                    }

                    // write object now
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_path, sal_True, sal_True);

                    ImpExportEvents( xShape );
                    ImpExportText( xShape );
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportGraphicObjectShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        sal_Bool bIsEmptyPresObj = sal_False;
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        if(eShapeType == XmlShapeTypePresGraphicObjectShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_graphic)) );

        if( !bIsEmptyPresObj )
        {
            OUString aStreamURL;
            OUString aStr;

            xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicURL"))) >>= aStr;
            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_href, aStr = rExport.AddEmbeddedGraphicObject( aStr ) );

            if( aStr.getLength() && aStr[ 0 ] == '#' )
            {
                aStreamURL = OUString::createFromAscii( "vnd.sun.star.Package:" );
                aStreamURL = aStreamURL.concat( aStr.copy( 1, aStr.getLength() - 1 ) );
            }

            // update stream URL for load on demand
            uno::Any aAny;
            aAny <<= aStreamURL;
            xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicStreamURL")), aAny );

            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_simple));
            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_type, aStr );

            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_embed));
            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_show, aStr );

            aStr = OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_onLoad));
            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_actuate, aStr );
        }
        // write graphic object
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_image, sal_True, sal_True);
        ImpExportEvents( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportChartShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    ImpExportOLE2Shape( xShape, eShapeType, nFeatures, pRefPoint );
/*
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        uno::Reference< chart::XChartDocument > xChartDoc;
        if( !bIsEmptyPresObj )
            xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ) ) >>= xChartDoc;

        if( xChartDoc.is() )
        {
            // export chart data if the flag is not set (default)
            sal_Bool bExportOwnData = ( nFeatures & SEF_EXPORT_NO_CHART_DATA ) == 0;
            rExport.GetChartExport()->exportChart( xChartDoc, bExportOwnData );
        }
        else
        {
            // write chart object (fake for now, replace later)
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_CHART, sXML_chart, sal_True, sal_True);
        }
    }
*/
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportSpreadsheetShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    ImpExportOLE2Shape( xShape, eShapeType, nFeatures, pRefPoint );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportControlShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);
    }

    uno::Reference< drawing::XControlShape > xControl( xShape, uno::UNO_QUERY );
    DBG_ASSERT( xControl.is(), "Control shape is not supporting XControlShape" );
    if( xControl.is() )
    {
        uno::Reference< beans::XPropertySet > xControlModel( xControl->getControl(), uno::UNO_QUERY );
        DBG_ASSERT( xControlModel.is(), "Control shape has not XControlModel" );
        if( xControlModel.is() )
        {
            rExport.AddAttribute( XML_NAMESPACE_FORM, sXML_id, rExport.GetFormExport()->getControlId( xControlModel ) );
        }
    }

    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_control, sal_True, sal_True);
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportConnectorShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );

    OUString aStr;
    OUStringBuffer sStringBuffer;

    // export connection kind
    drawing::ConnectorType eType = drawing::ConnectorType_STANDARD;
    uno::Any aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeKind")));
    aAny >>= eType;

    if( eType != drawing::ConnectorType_STANDARD )
    {
        SvXMLUnitConverter::convertEnum( sStringBuffer, (sal_uInt16)eType, aXML_ConnectionKind_EnumMap );
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_type, aStr);
    }

    // export line skew
    sal_Int32 nDelta1 = 0, nDelta2 = 0, nDelta3 = 0;

    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine1Delta")));
    aAny >>= nDelta1;
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine2Delta")));
    aAny >>= nDelta2;
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine3Delta")));
    aAny >>= nDelta3;

    if( nDelta1 != 0 || nDelta2 != 0 || nDelta3 != 0 )
    {
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nDelta1);
        if( nDelta2 != 0 || nDelta3 != 0 )
        {
            const char aSpace = ' ';
            sStringBuffer.appendAscii( &aSpace, 1 );
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nDelta2);
            if( nDelta3 != 0 )
            {
                sStringBuffer.appendAscii( &aSpace, 1 );
                rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nDelta3);
            }
        }

        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_line_skew, aStr);
    }

    // export start and end point
    awt::Point aStart(0,0);
    awt::Point aEnd(1,1);

    xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition"))) >>= aStart;
    xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition"))) >>= aEnd;

    if( pRefPoint )
    {
        aStart.X -= pRefPoint->X;
        aStart.Y -= pRefPoint->Y;
        aEnd.X -= pRefPoint->X;
        aEnd.Y -= pRefPoint->Y;
    }

    if( nFeatures & SEF_EXPORT_X )
    {
        // svg: x1
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.X);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x1, aStr);
    }
    else
    {
        aEnd.X -= aStart.X;
    }

    if( nFeatures & SEF_EXPORT_Y )
    {
        // svg: y1
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.Y);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x2, aStr);

    // svg: y2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y2, aStr);

    uno::Reference< drawing::XShape > xTempShape;

    // export start connection
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartShape") ) );
    if( aAny >>= xTempShape )
    {
        sal_Int32 nShapeId = rExport.GetShapeExport()->getShapeId( xTempShape );
        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_start_shape, OUString::valueOf( nShapeId ));

        aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartGluePointIndex")) );
        sal_Int32 nGluePointId;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_start_glue_point, OUString::valueOf( nGluePointId ));
            }
        }
    }

    // export end connection
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndShape")) );
    if( aAny >>= xTempShape )
    {
        sal_Int32 nShapeId = rExport.GetShapeExport()->getShapeId( xTempShape );
        rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_end_shape, OUString::valueOf( nShapeId ));

        aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndGluePointIndex")) );
        sal_Int32 nGluePointId;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_end_glue_point, OUString::valueOf( nGluePointId ));
            }
        }
    }

    // write connector shape. Add Export later.
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_connector, sal_True, sal_True);

    ImpExportEvents( xShape );
    ImpExportText( xShape );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportMeasureShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );

    OUString aStr;
    OUStringBuffer sStringBuffer;

    // export start and end point
    awt::Point aStart(0,0);
    awt::Point aEnd(1,1);

    uno::Any aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition")));
    aAny >>= aStart;

    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition")));
    aAny >>= aEnd;

    if( pRefPoint )
    {
        aStart.X -= pRefPoint->X;
        aStart.Y -= pRefPoint->Y;
        aEnd.X -= pRefPoint->X;
        aEnd.Y -= pRefPoint->Y;
    }

    if( nFeatures & SEF_EXPORT_X )
    {
        // svg: x1
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.X);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x1, aStr);
    }
    else
    {
        aEnd.X -= aStart.X;
    }

    if( nFeatures & SEF_EXPORT_Y )
    {
        // svg: y1
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.Y);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_x2, aStr);

    // svg: y2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, sXML_y2, aStr);

    // write measure shape
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_measure, sal_True, sal_True);

    ImpExportEvents( xShape );

    uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() )
        rExport.GetTextParagraphExport()->exportText( xText );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportOLE2Shape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    uno::Reference< container::XNamed > xNamed(xShape, uno::UNO_QUERY);

    DBG_ASSERT( xPropSet.is() && xNamed.is(), "ole shape is not implementing needed interfaces");
    if(xPropSet.is() && xNamed.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        sal_Bool bIsEmptyPresObj = sal_False;

        // presentation settings
        if(eShapeType == XmlShapeTypePresOLE2Shape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_object)));
        else if(eShapeType == XmlShapeTypePresChartShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_chart)) );
        else if(eShapeType == XmlShapeTypePresTableShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_table)) );

        OUString sClassId;

        if( !bIsEmptyPresObj )
        {
            // xlink:href
            OUString sURL(RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.EmbeddedObject:" ));
            sURL += xNamed->getName();

            sal_Bool bInternal;
            xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("IsInternal"))) >>= bInternal;

            if( !bInternal )
                xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID"))) >>= sClassId;

            if( sClassId.getLength() )
                rExport.AddAttribute(XML_NAMESPACE_DRAW, sXML_class_id, sClassId );

            sURL = rExport.AddEmbeddedObject( sURL );

            rExport.AddAttribute(XML_NAMESPACE_XLINK, sXML_href, sURL );
            rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_type, sXML_simple );
            rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_show, sXML_embed );
            rExport.AddAttributeASCII( XML_NAMESPACE_XLINK, sXML_actuate, sXML_onLoad );
        }

        const sal_Char *pElem = sClassId.getLength() ? sXML_object_ole : sXML_object;
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, pElem, sal_False, sal_True );

        ImpExportEvents( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportPageShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    OUString aStr;

    // a presentation page shape, normally used on notes pages only. If
    // it is used not as presentation shape, it may have been created with
    // copy-paste exchange between draw and impress (this IS possible...)
    if(eShapeType == XmlShapeTypePresPageShape)
    {
        rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, sXML_class,
            OUString(RTL_CONSTASCII_USTRINGPARAM(sXML_presentation_page)));
    }

    // write Page shape
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_page_thumbnail, sal_True, sal_True);
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportCaptionShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    // write Caption shape. Add export later.
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, sXML_caption, sal_True, sal_True);

    ImpExportEvents( xShape );
    ImpExportText( xShape );
}

