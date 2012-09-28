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

#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/media/ZoomLevel.hpp>

#include <sax/tools/converter.hxx>

#include <comphelper/storagehelper.hxx>

#include "anim.hxx"

#include <xmloff/shapeexport.hxx>
#include "sdpropls.hxx"
#include <tools/debug.hxx>
#include <tools/helpers.hxx>
#include <rtl/ustrbuf.hxx>
#include <xmloff/xmlexp.hxx>
#include <xmloff/xmluconv.hxx>
#include "XMLImageMapExport.hxx"
#include "xexptran.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/nmspmap.hxx>

#include "xmloff/xmlnmspe.hxx"
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/tuple/b2dtuple.hxx>

using ::rtl::OUString;
using ::rtl::OUStringBuffer;

using namespace ::com::sun::star;
using namespace ::xmloff::token;


//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportNewTrans(const uno::Reference< beans::XPropertySet >& xPropSet,
    sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    // get matrix
    ::basegfx::B2DHomMatrix aMatrix;
    ImpExportNewTrans_GetB2DHomMatrix(aMatrix, xPropSet);

    // decompose and correct abour pRefPoint
    ::basegfx::B2DTuple aTRScale;
    double fTRShear(0.0);
    double fTRRotate(0.0);
    ::basegfx::B2DTuple aTRTranslate;
    ImpExportNewTrans_DecomposeAndRefPoint(aMatrix, aTRScale, fTRShear, fTRRotate, aTRTranslate, pRefPoint);

    // use features and write
    ImpExportNewTrans_FeaturesAndWrite(aTRScale, fTRShear, fTRRotate, aTRTranslate, nFeatures);
}

void XMLShapeExport::ImpExportNewTrans_GetB2DHomMatrix(::basegfx::B2DHomMatrix& rMatrix,
    const uno::Reference< beans::XPropertySet >& xPropSet)
{
    /* Get <TransformationInHoriL2R>, if it exist
       and if the document is exported into the OpenOffice.org file format.
       This property only exists at service com::sun::star::text::Shape - the
       Writer UNO service for shapes.
       This code is needed, because the positioning attributes in the
       OpenOffice.org file format are given in horizontal left-to-right layout
       regardless the layout direction the shape is in. In the OASIS Open Office
       file format the positioning attributes are correctly given in the layout
       direction the shape is in. Thus, this code provides the conversion from
       the OASIS Open Office file format to the OpenOffice.org file format. (#i28749#)
    */
    uno::Any aAny;
    if ( ( GetExport().getExportFlags() & EXPORT_OASIS ) == 0 &&
         xPropSet->getPropertySetInfo()->hasPropertyByName(
            OUString("TransformationInHoriL2R")) )
    {
        aAny = xPropSet->getPropertyValue(OUString("TransformationInHoriL2R"));
    }
    else
    {
        aAny = xPropSet->getPropertyValue(OUString("Transformation"));
    }
    drawing::HomogenMatrix3 aMatrix;
    aAny >>= aMatrix;

    rMatrix.set(0, 0, aMatrix.Line1.Column1);
    rMatrix.set(0, 1, aMatrix.Line1.Column2);
    rMatrix.set(0, 2, aMatrix.Line1.Column3);
    rMatrix.set(1, 0, aMatrix.Line2.Column1);
    rMatrix.set(1, 1, aMatrix.Line2.Column2);
    rMatrix.set(1, 2, aMatrix.Line2.Column3);
    rMatrix.set(2, 0, aMatrix.Line3.Column1);
    rMatrix.set(2, 1, aMatrix.Line3.Column2);
    rMatrix.set(2, 2, aMatrix.Line3.Column3);
}

void XMLShapeExport::ImpExportNewTrans_DecomposeAndRefPoint(const ::basegfx::B2DHomMatrix& rMatrix, ::basegfx::B2DTuple& rTRScale,
    double& fTRShear, double& fTRRotate, ::basegfx::B2DTuple& rTRTranslate, com::sun::star::awt::Point* pRefPoint)
{
    // decompose matrix
    rMatrix.decompose(rTRScale, rTRTranslate, fTRRotate, fTRShear);

    // correct translation about pRefPoint
    if(pRefPoint)
    {
        rTRTranslate -= ::basegfx::B2DTuple(pRefPoint->X, pRefPoint->Y);
    }
}

void XMLShapeExport::ImpExportNewTrans_FeaturesAndWrite(::basegfx::B2DTuple& rTRScale, double fTRShear,
    double fTRRotate, ::basegfx::B2DTuple& rTRTranslate, const sal_Int32 nFeatures)
{
    // allways write Size (rTRScale) since this statement carries the union
    // of the object
    OUString aStr;
    OUStringBuffer sStringBuffer;
    ::basegfx::B2DTuple aTRScale(rTRScale);

    // svg: width
    if(!(nFeatures & SEF_EXPORT_WIDTH))
    {
        aTRScale.setX(1.0);
    }
    else
    {
        if( aTRScale.getX() > 0.0 )
            aTRScale.setX(aTRScale.getX() - 1.0);
        else if( aTRScale.getX() < 0.0 )
            aTRScale.setX(aTRScale.getX() + 1.0);
    }

    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            FRound(aTRScale.getX()));
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_WIDTH, aStr);

    // svg: height
    if(!(nFeatures & SEF_EXPORT_HEIGHT))
    {
        aTRScale.setY(1.0);
    }
    else
    {
        if( aTRScale.getY() > 0.0 )
            aTRScale.setY(aTRScale.getY() - 1.0);
        else if( aTRScale.getY() < 0.0 )
            aTRScale.setY(aTRScale.getY() + 1.0);
    }

    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
            FRound(aTRScale.getY()));
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_HEIGHT, aStr);

    // decide if transformation is neccessary
    sal_Bool bTransformationIsNeccessary(fTRShear != 0.0 || fTRRotate != 0.0);

    if(bTransformationIsNeccessary)
    {
        // write transformation, but WITHOUT scale which is exported as size above
        SdXMLImExTransform2D aTransform;

        aTransform.AddSkewX(atan(fTRShear));

        // #i78696#
        // fTRRotate is mathematically correct, but due to the error
        // we export/import it mirrored. Since the API implementation is fixed and
        // uses the correctly oriented angle, it is necessary for compatibility to
        // mirror the angle here to stay at the old behaviour. There is a follow-up
        // task (#i78698#) to fix this in the next ODF FileFormat version
        aTransform.AddRotate(-fTRRotate);

        aTransform.AddTranslate(rTRTranslate);

        // does transformation need to be exported?
        if(aTransform.NeedsAction())
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_TRANSFORM, aTransform.GetExportString(mrExport.GetMM100UnitConverter()));
    }
    else
    {
        // no shear, no rotate; just add object position to export and we are done
        if(nFeatures & SEF_EXPORT_X)
        {
            // svg: x
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    FRound(rTRTranslate.getX()));
            aStr = sStringBuffer.makeStringAndClear();
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X, aStr);
        }

        if(nFeatures & SEF_EXPORT_Y)
        {
            // svg: y
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    FRound(rTRTranslate.getY()));
            aStr = sStringBuffer.makeStringAndClear();
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y, aStr);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

sal_Bool XMLShapeExport::ImpExportPresentationAttributes( const uno::Reference< beans::XPropertySet >& xPropSet, const rtl::OUString& rClass )
{
    sal_Bool bIsEmpty = sal_False;

    OUStringBuffer sStringBuffer;

    // write presentation class entry
    mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_CLASS, rClass);

    if( xPropSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        sal_Bool bTemp = false;

        // is empty pes shape?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(OUString("IsEmptyPresentationObject")))
        {
            xPropSet->getPropertyValue(OUString("IsEmptyPresentationObject")) >>= bIsEmpty;
            if( bIsEmpty )
                mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PLACEHOLDER, XML_TRUE);
        }

        // is user-transformed?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(OUString("IsPlaceholderDependent")))
        {
            xPropSet->getPropertyValue(OUString("IsPlaceholderDependent")) >>= bTemp;
            if(!bTemp)
                mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_USER_TRANSFORMED, XML_TRUE);
        }
    }

    return bIsEmpty;
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportText( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() )
    {
        uno::Reference< container::XEnumerationAccess > xEnumAccess( xShape, uno::UNO_QUERY );
        if( xEnumAccess.is() && xEnumAccess->hasElements() )
            mrExport.GetTextParagraphExport()->exportText( xText );
    }
}

//////////////////////////////////////////////////////////////////////////////
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/presentation/AnimationSpeed.hpp>

namespace {

const sal_Int32 FOUND_CLICKACTION       = 0x00000001;
const sal_Int32 FOUND_BOOKMARK          = 0x00000002;
const sal_Int32 FOUND_EFFECT            = 0x00000004;
const sal_Int32 FOUND_PLAYFULL          = 0x00000008;
const sal_Int32 FOUND_VERB              = 0x00000010;
const sal_Int32 FOUND_SOUNDURL          = 0x00000020;
const sal_Int32 FOUND_SPEED             = 0x00000040;
const sal_Int32 FOUND_CLICKEVENTTYPE    = 0x00000080;
const sal_Int32 FOUND_MACRO             = 0x00000100;
const sal_Int32 FOUND_LIBRARY           = 0x00000200;
const sal_Int32 FOUND_ACTIONEVENTTYPE   = 0x00000400;

} // namespace

void XMLShapeExport::ImpExportEvents( const uno::Reference< drawing::XShape >& xShape )
{
    uno::Reference< document::XEventsSupplier > xEventsSupplier( xShape, uno::UNO_QUERY );
    if( !xEventsSupplier.is() )
        return;

    uno::Reference< container::XNameAccess > xEvents( xEventsSupplier->getEvents(), uno::UNO_QUERY );
    DBG_ASSERT( xEvents.is(), "XEventsSupplier::getEvents() returned NULL" );
    if( !xEvents.is() )
        return;

    sal_Int32 nFound = 0;

    // extract properties from "OnClick" event --------------------------------

    OUString aClickEventType;
    presentation::ClickAction eClickAction = presentation::ClickAction_NONE;
    presentation::AnimationEffect eEffect = presentation::AnimationEffect_NONE;
    presentation::AnimationSpeed eSpeed = presentation::AnimationSpeed_SLOW;
    OUString aStrSoundURL;
    sal_Bool bPlayFull = false;
    sal_Int32 nVerb = 0;
    OUString aStrMacro;
    OUString aStrLibrary;
    OUString aStrBookmark;

    uno::Sequence< beans::PropertyValue > aClickProperties;
    if( xEvents->hasByName( msOnClick ) && (xEvents->getByName( msOnClick ) >>= aClickProperties) )
    {
        const beans::PropertyValue* pProperty = aClickProperties.getConstArray();
        const beans::PropertyValue* pPropertyEnd = pProperty + aClickProperties.getLength();
        for( ; pProperty != pPropertyEnd; ++pProperty )
        {
            if( ( ( nFound & FOUND_CLICKEVENTTYPE ) == 0 ) && pProperty->Name == msEventType )
            {
                if( pProperty->Value >>= aClickEventType )
                    nFound |= FOUND_CLICKEVENTTYPE;
            }
            else if( ( ( nFound & FOUND_CLICKACTION ) == 0 ) && pProperty->Name == msClickAction )
            {
                if( pProperty->Value >>= eClickAction )
                    nFound |= FOUND_CLICKACTION;
            }
            else if( ( ( nFound & FOUND_MACRO ) == 0 ) && ( pProperty->Name == msMacroName || pProperty->Name == msScript ) )
            {
                if( pProperty->Value >>= aStrMacro )
                    nFound |= FOUND_MACRO;
            }
            else if( ( ( nFound & FOUND_LIBRARY ) == 0 ) && pProperty->Name == msLibrary )
            {
                if( pProperty->Value >>= aStrLibrary )
                    nFound |= FOUND_LIBRARY;
            }
            else if( ( ( nFound & FOUND_EFFECT ) == 0 ) && pProperty->Name == msEffect )
            {
                if( pProperty->Value >>= eEffect )
                    nFound |= FOUND_EFFECT;
            }
            else if( ( ( nFound & FOUND_BOOKMARK ) == 0 ) && pProperty->Name == msBookmark )
            {
                if( pProperty->Value >>= aStrBookmark )
                    nFound |= FOUND_BOOKMARK;
            }
            else if( ( ( nFound & FOUND_SPEED ) == 0 ) && pProperty->Name == msSpeed )
            {
                if( pProperty->Value >>= eSpeed )
                    nFound |= FOUND_SPEED;
            }
            else if( ( ( nFound & FOUND_SOUNDURL ) == 0 ) && pProperty->Name == msSoundURL )
            {
                if( pProperty->Value >>= aStrSoundURL )
                    nFound |= FOUND_SOUNDURL;
            }
            else if( ( ( nFound & FOUND_PLAYFULL ) == 0 ) && pProperty->Name == msPlayFull )
            {
                if( pProperty->Value >>= bPlayFull )
                    nFound |= FOUND_PLAYFULL;
            }
            else if( ( ( nFound & FOUND_VERB ) == 0 ) && pProperty->Name == msVerb )
            {
                if( pProperty->Value >>= nVerb )
                    nFound |= FOUND_VERB;
            }
        }
    }

    // create the XML elements

    if( aClickEventType == msPresentation )
    {
        if( ((nFound & FOUND_CLICKACTION) == 0) || (eClickAction == presentation::ClickAction_NONE) )
            return;

        SvXMLElementExport aEventsElemt(mrExport, XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, sal_True, sal_True);

        enum XMLTokenEnum eStrAction;

        switch( eClickAction )
        {
            case presentation::ClickAction_PREVPAGE:        eStrAction = XML_PREVIOUS_PAGE; break;
            case presentation::ClickAction_NEXTPAGE:        eStrAction = XML_NEXT_PAGE; break;
            case presentation::ClickAction_FIRSTPAGE:       eStrAction = XML_FIRST_PAGE; break;
            case presentation::ClickAction_LASTPAGE:        eStrAction = XML_LAST_PAGE; break;
            case presentation::ClickAction_INVISIBLE:       eStrAction = XML_HIDE; break;
            case presentation::ClickAction_STOPPRESENTATION:eStrAction = XML_STOP; break;
            case presentation::ClickAction_PROGRAM:         eStrAction = XML_EXECUTE; break;
            case presentation::ClickAction_BOOKMARK:        eStrAction = XML_SHOW; break;
            case presentation::ClickAction_DOCUMENT:        eStrAction = XML_SHOW; break;
            case presentation::ClickAction_MACRO:           eStrAction = XML_EXECUTE_MACRO; break;
            case presentation::ClickAction_VERB:            eStrAction = XML_VERB; break;
            case presentation::ClickAction_VANISH:          eStrAction = XML_FADE_OUT; break;
            case presentation::ClickAction_SOUND:           eStrAction = XML_SOUND; break;
            default:
                OSL_FAIL( "unknown presentation::ClickAction found!" );
                eStrAction = XML_UNKNOWN;
        }

        OUString aEventQName(
            mrExport.GetNamespaceMap().GetQNameByKey(
                    XML_NAMESPACE_DOM, OUString(  "click"  ) ) );
        mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, aEventQName );
        mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_ACTION, eStrAction );

        if( eClickAction == presentation::ClickAction_VANISH )
        {
            if( nFound & FOUND_EFFECT )
            {
                XMLEffect eKind;
                XMLEffectDirection eDirection;
                sal_Int16 nStartScale;
                sal_Bool bIn;

                SdXMLImplSetEffect( eEffect, eKind, eDirection, nStartScale, bIn );

                if( eKind != EK_none )
                {
                    SvXMLUnitConverter::convertEnum( msBuffer, eKind, aXML_AnimationEffect_EnumMap );
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_EFFECT, msBuffer.makeStringAndClear() );
                }

                if( eDirection != ED_none )
                {
                    SvXMLUnitConverter::convertEnum( msBuffer, eDirection, aXML_AnimationDirection_EnumMap );
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_DIRECTION, msBuffer.makeStringAndClear() );
                }

                if( nStartScale != -1 )
                {
                    ::sax::Converter::convertPercent( msBuffer, nStartScale );
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_START_SCALE, msBuffer.makeStringAndClear() );
                }
            }

            if( nFound & FOUND_SPEED && eEffect != presentation::AnimationEffect_NONE )
            {
                 if( eSpeed != presentation::AnimationSpeed_MEDIUM )
                    {
                    SvXMLUnitConverter::convertEnum( msBuffer, eSpeed, aXML_AnimationSpeed_EnumMap );
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_SPEED, msBuffer.makeStringAndClear() );
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
            mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(msBuffer.makeStringAndClear()) );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
            mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
        }

        if( ( nFound & FOUND_VERB ) && eClickAction == presentation::ClickAction_VERB )
        {
            msBuffer.append( nVerb );
            mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_VERB, msBuffer.makeStringAndClear());
        }

        SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_PRESENTATION, XML_EVENT_LISTENER, sal_True, sal_True);

        if( eClickAction == presentation::ClickAction_VANISH || eClickAction == presentation::ClickAction_SOUND )
        {
            if( ( nFound & FOUND_SOUNDURL ) && !aStrSoundURL.isEmpty() )
            {
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStrSoundURL) );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_NEW );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
                if( nFound & FOUND_PLAYFULL && bPlayFull )
                    mrExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_PLAY_FULL, XML_TRUE );

                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_PRESENTATION, XML_SOUND, sal_True, sal_True );
            }
       }
    }
    else if( aClickEventType == msStarBasic )
    {
        if( nFound & FOUND_MACRO )
        {
            SvXMLElementExport aEventsElemt(mrExport, XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, sal_True, sal_True);

            mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_LANGUAGE,
                        mrExport.GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_OOO,
                            OUString( "starbasic"  ) ) );
            OUString aEventQName(
                mrExport.GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_DOM, OUString(  "click"  ) ) );
            mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, aEventQName );

            if( nFound & FOUND_LIBRARY )
            {
                OUString sLocation( GetXMLToken(
                    (aStrLibrary.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("StarOffice")) ||
                     aStrLibrary.equalsIgnoreAsciiCaseAsciiL(RTL_CONSTASCII_STRINGPARAM("application")) ) ? XML_APPLICATION
                                                                       : XML_DOCUMENT ) );
                OUStringBuffer sTmp( sLocation.getLength() + aStrMacro.getLength() + 1 );
                sTmp = sLocation;
                sTmp.append( sal_Unicode( ':' ) );
                sTmp.append( aStrMacro );
                mrExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_MACRO_NAME,
                                     sTmp.makeStringAndClear());
            }
            else
            {
                mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_MACRO_NAME, aStrMacro );
            }

            SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_SCRIPT, XML_EVENT_LISTENER, sal_True, sal_True);
        }
    }
    else if( aClickEventType == msScript )
    {
        if( nFound & FOUND_MACRO )
        {
            SvXMLElementExport aEventsElemt(mrExport, XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, sal_True, sal_True);
            if ( nFound & FOUND_MACRO )
            {
                mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_LANGUAGE, mrExport.GetNamespaceMap().GetQNameByKey(
                         XML_NAMESPACE_OOO, GetXMLToken(XML_SCRIPT) ) );
                OUString aEventQName(
                    mrExport.GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_DOM, OUString(  "click"  ) ) );
                mrExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, aEventQName );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, aStrMacro );

                SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_SCRIPT, XML_EVENT_LISTENER, sal_True, sal_True);
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

/** #i68101# export shape Title and Description */
void XMLShapeExport::ImpExportDescription( const uno::Reference< drawing::XShape >& xShape )
{
    try
    {
        OUString aTitle;
        OUString aDescription;

        uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY_THROW );
        xProps->getPropertyValue( OUString( "Title" ) ) >>= aTitle;
        xProps->getPropertyValue( OUString( "Description" ) ) >>= aDescription;

        if(!aTitle.isEmpty())
        {
            SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_SVG, XML_TITLE, sal_True, sal_False);
            mrExport.Characters( aTitle );
        }

        if(!aDescription.isEmpty())
        {
            SvXMLElementExport aEventElemt(mrExport, XML_NAMESPACE_SVG, XML_DESC, sal_True, sal_False );
            mrExport.Characters( aDescription );
        }
    }
    catch( uno::Exception& )
    {
        OSL_FAIL( "could not export Title and/or Description for shape!" );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportGroupShape( const uno::Reference< drawing::XShape >& xShape, XmlShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    uno::Reference< drawing::XShapes > xShapes(xShape, uno::UNO_QUERY);
    if(xShapes.is() && xShapes->getCount())
    {
        // write group shape
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aPGR(mrExport, XML_NAMESPACE_DRAW, XML_G, bCreateNewline, sal_True);

        ImpExportDescription( xShape ); // #i68101#
        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );

        // #89764# if export of position is supressed for group shape,
        // positions of contained objects should be written relative to
        // the upper left edge of the group.
        awt::Point aUpperLeft;

        if(!(nFeatures & SEF_EXPORT_POSITION))
        {
            nFeatures |= SEF_EXPORT_POSITION;
            aUpperLeft = xShape->getPosition();
            pRefPoint = &aUpperLeft;
        }

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
        sal_Bool bIsPresShape(sal_False);
        sal_Bool bIsEmptyPresObj(sal_False);
        OUString aStr;

        switch(eShapeType)
        {
            case XmlShapeTypePresSubtitleShape:
            {
                aStr = GetXMLToken(XML_PRESENTATION_SUBTITLE);
                bIsPresShape = sal_True;
                break;
            }
            case XmlShapeTypePresTitleTextShape:
            {
                aStr = GetXMLToken(XML_PRESENTATION_TITLE);
                bIsPresShape = sal_True;
                break;
            }
            case XmlShapeTypePresOutlinerShape:
            {
                aStr = GetXMLToken(XML_PRESENTATION_OUTLINE);
                bIsPresShape = sal_True;
                break;
            }
            case XmlShapeTypePresNotesShape:
            {
                aStr = GetXMLToken(XML_PRESENTATION_NOTES);
                bIsPresShape = sal_True;
                break;
            }
            case XmlShapeTypePresHeaderShape:
            {
                aStr = GetXMLToken(XML_HEADER);
                bIsPresShape = sal_True;
                break;
            }
            case XmlShapeTypePresFooterShape:
            {
                aStr = GetXMLToken(XML_FOOTER);
                bIsPresShape = sal_True;
                break;
            }
            case XmlShapeTypePresSlideNumberShape:
            {
                aStr = GetXMLToken(XML_PAGE_NUMBER);
                bIsPresShape = sal_True;
                break;
            }
            case XmlShapeTypePresDateTimeShape:
            {
                aStr = GetXMLToken(XML_DATE_TIME);
                bIsPresShape = sal_True;
                break;
            }
            default:
                break;
        }

        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        if(bIsPresShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, aStr );


        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // evtl. corner radius?
        sal_Int32 nCornerRadius(0L);
        xPropSet->getPropertyValue(OUString("CornerRadius")) >>= nCornerRadius;
        if(nCornerRadius)
        {
            OUStringBuffer sStringBuffer;
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    nCornerRadius);
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CORNER_RADIUS, sStringBuffer.makeStringAndClear());
        }

        {
            // write text-box
            SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_TEXT_BOX, sal_True, sal_True);
            if(!bIsEmptyPresObj)
                ImpExportText( xShape );
        }

        ImpExportDescription( xShape ); // #i68101#
        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportRectangleShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        // evtl. corner radius?
        sal_Int32 nCornerRadius(0L);
        xPropSet->getPropertyValue(OUString("CornerRadius")) >>= nCornerRadius;
        if(nCornerRadius)
        {
            OUStringBuffer sStringBuffer;
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    nCornerRadius);
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CORNER_RADIUS, sStringBuffer.makeStringAndClear());
        }

        // write rectangle
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_RECT, bCreateNewline, sal_True);

        ImpExportDescription( xShape ); // #i68101#
        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
        ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportLineShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        OUString aStr;
        OUStringBuffer sStringBuffer;
        awt::Point aStart(0,0);
        awt::Point aEnd(1,1);

        // #85920# use 'Geometry' to get the points of the line
        // since this slot take anchor pos into account.

        // get matrix
        ::basegfx::B2DHomMatrix aMatrix;
        ImpExportNewTrans_GetB2DHomMatrix(aMatrix, xPropSet);

        // decompose and correct about pRefPoint
        ::basegfx::B2DTuple aTRScale;
        double fTRShear(0.0);
        double fTRRotate(0.0);
        ::basegfx::B2DTuple aTRTranslate;
        ImpExportNewTrans_DecomposeAndRefPoint(aMatrix, aTRScale, fTRShear, fTRRotate, aTRTranslate, pRefPoint);

        // create base position
        awt::Point aBasePosition(FRound(aTRTranslate.getX()), FRound(aTRTranslate.getY()));

        // get the two points
        uno::Any aAny(xPropSet->getPropertyValue(OUString("Geometry")));
        drawing::PointSequenceSequence* pSourcePolyPolygon = (drawing::PointSequenceSequence*)aAny.getValue();

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
                            aStart = awt::Point(
                                pArray->X + aBasePosition.X,
                                pArray->Y + aBasePosition.Y);
                            pArray++;
                        }

                        if(pInnerSequence->getLength() > 1)
                        {
                            aEnd = awt::Point(
                                pArray->X + aBasePosition.X,
                                pArray->Y + aBasePosition.Y);
                        }
                    }
                }
            }
        }

        if( nFeatures & SEF_EXPORT_X )
        {
            // svg: x1
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    aStart.X);
            aStr = sStringBuffer.makeStringAndClear();
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X1, aStr);
        }
        else
        {
            aEnd.X -= aStart.X;
        }

        if( nFeatures & SEF_EXPORT_Y )
        {
            // svg: y1
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    aStart.Y);
            aStr = sStringBuffer.makeStringAndClear();
            mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y1, aStr);
        }
        else
        {
            aEnd.Y -= aStart.Y;
        }

        // svg: x2
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aEnd.X);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X2, aStr);

        // svg: y2
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aEnd.Y);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y2, aStr);

        // write line
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_LINE, bCreateNewline, sal_True);

        ImpExportDescription( xShape ); // #i68101#
        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
        ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportEllipseShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // get size to decide between Circle and Ellipse
        awt::Size aSize = xShape->getSize();
        sal_Int32 nRx((aSize.Width + 1) / 2);
        sal_Int32 nRy((aSize.Height + 1) / 2);
        sal_Bool bCircle(nRx == nRy);

        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        drawing::CircleKind eKind = drawing::CircleKind_FULL;
        xPropSet->getPropertyValue( OUString("CircleKind") ) >>= eKind;
        if( eKind != drawing::CircleKind_FULL )
        {
            OUStringBuffer sStringBuffer;
            sal_Int32 nStartAngle = 0;
            sal_Int32 nEndAngle = 0;
            xPropSet->getPropertyValue( OUString("CircleStartAngle") ) >>= nStartAngle;
            xPropSet->getPropertyValue( OUString("CircleEndAngle") ) >>= nEndAngle;

            const double dStartAngle = nStartAngle / 100.0;
            const double dEndAngle = nEndAngle / 100.0;

            // export circle kind
            SvXMLUnitConverter::convertEnum( sStringBuffer, (sal_uInt16)eKind, aXML_CircleKind_EnumMap );
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_KIND, sStringBuffer.makeStringAndClear() );

            // export start angle
            ::sax::Converter::convertDouble( sStringBuffer, dStartAngle );
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_START_ANGLE, sStringBuffer.makeStringAndClear() );

            // export end angle
            ::sax::Converter::convertDouble( sStringBuffer, dEndAngle );
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_END_ANGLE, sStringBuffer.makeStringAndClear() );
        }

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#

        if(bCircle)
        {
            // write circle
            SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_CIRCLE, bCreateNewline, sal_True);

            ImpExportDescription( xShape ); // #i68101#
            ImpExportEvents( xShape );
            ImpExportGluePoints( xShape );
            ImpExportText( xShape );
        }
        else
        {
            // write ellipse
            SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_ELLIPSE, bCreateNewline, sal_True);

            ImpExportDescription( xShape ); // #i68101#
            ImpExportEvents( xShape );
            ImpExportGluePoints( xShape );
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
        sal_Bool bClosed(eShapeType == XmlShapeTypeDrawPolyPolygonShape
            || eShapeType == XmlShapeTypeDrawClosedBezierShape);
        sal_Bool bBezier(eShapeType == XmlShapeTypeDrawClosedBezierShape
            || eShapeType == XmlShapeTypeDrawOpenBezierShape);

        // get matrix
        ::basegfx::B2DHomMatrix aMatrix;
        ImpExportNewTrans_GetB2DHomMatrix(aMatrix, xPropSet);

        // decompose and correct abour pRefPoint
        ::basegfx::B2DTuple aTRScale;
        double fTRShear(0.0);
        double fTRRotate(0.0);
        ::basegfx::B2DTuple aTRTranslate;
        ImpExportNewTrans_DecomposeAndRefPoint(aMatrix, aTRScale, fTRShear, fTRRotate, aTRTranslate, pRefPoint);

        // use features and write
        ImpExportNewTrans_FeaturesAndWrite(aTRScale, fTRShear, fTRRotate, aTRTranslate, nFeatures);

        // create and export ViewBox
        awt::Point aPoint(0, 0);
        awt::Size aSize(FRound(aTRScale.getX()), FRound(aTRScale.getY()));
        SdXMLImExViewBox aViewBox(0, 0, aSize.Width, aSize.Height);
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString());

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#

        if(bBezier)
        {
            // get PolygonBezier
            uno::Any aAny( xPropSet->getPropertyValue(OUString("Geometry")) );
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
                    SdXMLImExSvgDElement aSvgDElement(aViewBox, GetExport());

                    for(sal_Int32 a(0L); a < nOuterCnt; a++)
                    {
                        drawing::PointSequence* pSequence = pOuterSequence++;
                        drawing::FlagSequence* pFlags = pOuterFlags++;

                        if(pSequence && pFlags)
                        {
                            aSvgDElement.AddPolygon(pSequence, pFlags,
                                aPoint, aSize, bClosed);
                        }
                    }

                    // write point array
                    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aSvgDElement.GetExportString());
                }

                // write object now
                SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_PATH, bCreateNewline, sal_True);

                ImpExportDescription( xShape ); // #i68101#
                ImpExportEvents( xShape );
                ImpExportGluePoints( xShape );
                ImpExportText( xShape );
            }
        }
        else
        {
            // get non-bezier polygon
            uno::Any aAny( xPropSet->getPropertyValue(OUString("Geometry")) );
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
                        SdXMLImExPointsElement aPoints(pSequence, aViewBox, aPoint, aSize,
                            // #96328#
                            bClosed);

                        // write point array
                        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_POINTS, aPoints.GetExportString());
                    }

                    // write object now
                    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW,
                        bClosed ? XML_POLYGON : XML_POLYLINE , bCreateNewline, sal_True);

                    ImpExportDescription( xShape ); // #i68101#
                    ImpExportEvents( xShape );
                    ImpExportGluePoints( xShape );
                    ImpExportText( xShape );
                }
                else
                {
                    // polypolygon or bezier, needs to be written as a svg:path sequence
                    drawing::PointSequence* pOuterSequence = pSourcePolyPolygon->getArray();
                    if(pOuterSequence)
                    {
                        // prepare svx:d element export
                        SdXMLImExSvgDElement aSvgDElement(aViewBox, GetExport());

                        for(sal_Int32 a(0L); a < nOuterCnt; a++)
                        {
                            drawing::PointSequence* pSequence = pOuterSequence++;
                            if(pSequence)
                            {
                                aSvgDElement.AddPolygon(pSequence, 0L, aPoint,
                                    aSize, bClosed);
                            }
                        }

                        // write point array
                        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aSvgDElement.GetExportString());
                    }

                    // write object now
                    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_PATH, bCreateNewline, sal_True);

                    ImpExportDescription( xShape ); // #i68101#
                    ImpExportEvents( xShape );
                    ImpExportGluePoints( xShape );
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

        OUString sImageURL;

        if(eShapeType == XmlShapeTypePresGraphicObjectShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_GRAPHIC) );

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        const bool bSaveBackwardsCompatible = ( mrExport.getExportFlags() & EXPORT_SAVEBACKWARDCOMPATIBLE );

        if( !bIsEmptyPresObj || bSaveBackwardsCompatible )
        {
            if( !bIsEmptyPresObj )
            {
                OUString aStreamURL;
                OUString aStr;

                xPropSet->getPropertyValue( OUString("GraphicStreamURL")) >>= aStreamURL;
                xPropSet->getPropertyValue( OUString("GraphicURL")) >>= sImageURL;

                OUString aResolveURL( sImageURL );
                const rtl::OUString sPackageURL( "vnd.sun.star.Package:" );

                    // sj: trying to preserve the filename
                if ( aStreamURL.match( sPackageURL, 0 ) )
                {
                    rtl::OUString sRequestedName( aStreamURL.copy( sPackageURL.getLength(), aStreamURL.getLength() - sPackageURL.getLength() ) );
                    sal_Int32 nLastIndex = sRequestedName.lastIndexOf( '/' ) + 1;
                    if ( ( nLastIndex > 0 ) && ( nLastIndex < sRequestedName.getLength() ) )
                        sRequestedName = sRequestedName.copy( nLastIndex, sRequestedName.getLength() - nLastIndex );
                    nLastIndex = sRequestedName.lastIndexOf( '.' );
                    if ( nLastIndex >= 0 )
                        sRequestedName = sRequestedName.copy( 0, nLastIndex );
                    if ( !sRequestedName.isEmpty() )
                    {
                        aResolveURL = aResolveURL.concat( OUString("?requestedName="));
                        aResolveURL = aResolveURL.concat( sRequestedName );
                    }
                }

                aStr = mrExport.AddEmbeddedGraphicObject( aResolveURL );
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, aStr );

                if( !aStr.isEmpty() )
                {
                    if( aStr[ 0 ] == '#' )
                    {
                        aStreamURL = OUString( "vnd.sun.star.Package:" );
                        aStreamURL = aStreamURL.concat( aStr.copy( 1, aStr.getLength() - 1 ) );
                    }

                    // update stream URL for load on demand
                    uno::Any aAny;
                    aAny <<= aStreamURL;
                    xPropSet->setPropertyValue( OUString("GraphicStreamURL"), aAny );

                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                    mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
                }
            }
            else
            {
                OUString aStr;
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, aStr );
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
            }

            {
                SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_IMAGE, sal_True, sal_True);

                if( !sImageURL.isEmpty() )
                {
                    // optional office:binary-data
                    mrExport.AddEmbeddedGraphicObjectAsBase64( sImageURL );
                }
                if( !bIsEmptyPresObj )
                    ImpExportText( xShape );
            }
        }

        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );

        // image map
        GetExport().GetImageMapExport().Export( xPropSet );
        ImpExportDescription( xShape ); // #i68101#
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportChartShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint,
    SvXMLAttributeList* pAttrList )
{
    ImpExportOLE2Shape( xShape, eShapeType, nFeatures, pRefPoint, pAttrList );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportControlShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures, awt::Point* pRefPoint)
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
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CONTROL, mrExport.GetFormExport()->getControlId( xControlModel ) );
        }
    }

    sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_CONTROL, bCreateNewline, sal_True);

    ImpExportDescription( xShape ); // #i68101#
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportConnectorShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );

    OUString aStr;
    OUStringBuffer sStringBuffer;

    // export connection kind
    drawing::ConnectorType eType = drawing::ConnectorType_STANDARD;
    uno::Any aAny = xProps->getPropertyValue(OUString("EdgeKind"));
    aAny >>= eType;

    if( eType != drawing::ConnectorType_STANDARD )
    {
        SvXMLUnitConverter::convertEnum( sStringBuffer, (sal_uInt16)eType, aXML_ConnectionKind_EnumMap );
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_TYPE, aStr);
    }

    // export line skew
    sal_Int32 nDelta1 = 0, nDelta2 = 0, nDelta3 = 0;

    aAny = xProps->getPropertyValue(OUString("EdgeLine1Delta"));
    aAny >>= nDelta1;
    aAny = xProps->getPropertyValue(OUString("EdgeLine2Delta"));
    aAny >>= nDelta2;
    aAny = xProps->getPropertyValue(OUString("EdgeLine3Delta"));
    aAny >>= nDelta3;

    if( nDelta1 != 0 || nDelta2 != 0 || nDelta3 != 0 )
    {
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                nDelta1);
        if( nDelta2 != 0 || nDelta3 != 0 )
        {
            const char aSpace = ' ';
            sStringBuffer.appendAscii( &aSpace, 1 );
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    nDelta2);
            if( nDelta3 != 0 )
            {
                sStringBuffer.appendAscii( &aSpace, 1 );
                mrExport.GetMM100UnitConverter().convertMeasureToXML(
                        sStringBuffer, nDelta3);
            }
        }

        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_LINE_SKEW, aStr);
    }

    // export start and end point
    awt::Point aStart(0,0);
    awt::Point aEnd(1,1);

    /* Get <StartPositionInHoriL2R> and
       <EndPositionInHoriL2R>, if they exist and if the document is exported
       into the OpenOffice.org file format.
       These properties only exist at service com::sun::star::text::Shape - the
       Writer UNO service for shapes.
       This code is needed, because the positioning attributes in the
       OpenOffice.org file format are given in horizontal left-to-right layout
       regardless the layout direction the shape is in. In the OASIS Open Office
       file format the positioning attributes are correctly given in the layout
       direction the shape is in. Thus, this code provides the conversion from
       the OASIS Open Office file format to the OpenOffice.org file format. (#i36248#)
    */
    if ( ( GetExport().getExportFlags() & EXPORT_OASIS ) == 0 &&
         xProps->getPropertySetInfo()->hasPropertyByName(
            OUString("StartPositionInHoriL2R")) &&
         xProps->getPropertySetInfo()->hasPropertyByName(
            OUString("EndPositionInHoriL2R")) )
    {
        xProps->getPropertyValue(OUString("StartPositionInHoriL2R")) >>= aStart;
        xProps->getPropertyValue(OUString("EndPositionInHoriL2R")) >>= aEnd;
    }
    else
    {
        xProps->getPropertyValue(OUString("StartPosition")) >>= aStart;
        xProps->getPropertyValue(OUString("EndPosition")) >>= aEnd;
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
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.X);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X1, aStr);
    }
    else
    {
        aEnd.X -= aStart.X;
    }

    if( nFeatures & SEF_EXPORT_Y )
    {
        // svg: y1
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.Y);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X2, aStr);

    // svg: y2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y2, aStr);

    // #i39320#
    uno::Reference< uno::XInterface > xRefS;
    uno::Reference< uno::XInterface > xRefE;

    // export start connection
    xProps->getPropertyValue(OUString("StartShape" ) ) >>= xRefS;
    if( xRefS.is() )
    {
        const OUString& rShapeId = mrExport.getInterfaceToIdentifierMapper().getIdentifier( xRefS );
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_START_SHAPE, rShapeId);

        aAny = xProps->getPropertyValue(OUString("StartGluePointIndex") );
        sal_Int32 nGluePointId = 0;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_START_GLUE_POINT, OUString::valueOf( nGluePointId ));
            }
        }
    }

    // export end connection
    xProps->getPropertyValue(OUString("EndShape") ) >>= xRefE;
    if( xRefE.is() )
    {
        const OUString& rShapeId = mrExport.getInterfaceToIdentifierMapper().getIdentifier( xRefE );
        mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_END_SHAPE, rShapeId);

        aAny = xProps->getPropertyValue(OUString("EndGluePointIndex") );
        sal_Int32 nGluePointId = 0;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_END_GLUE_POINT, OUString::valueOf( nGluePointId ));
            }
        }
    }

    if( xProps->getPropertyValue( OUString( "PolyPolygonBezier" ) ) >>= aAny )
    {
        // get PolygonBezier
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
                awt::Point aPoint( 0, 0 );
                awt::Size aSize( 1, 1 );
                SdXMLImExViewBox aViewBox( 0, 0, 1, 1 );
                SdXMLImExSvgDElement aSvgDElement(aViewBox, GetExport());

                for(sal_Int32 a(0L); a < nOuterCnt; a++)
                {
                    drawing::PointSequence* pSequence = pOuterSequence++;
                    drawing::FlagSequence* pFlags = pOuterFlags++;

                    if(pSequence && pFlags)
                    {
                        aSvgDElement.AddPolygon(pSequence, pFlags,
                            aPoint, aSize, sal_False );
                    }
                }

                // write point array
                mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aSvgDElement.GetExportString());
            }
        }
    }

    // get matrix
    ::basegfx::B2DHomMatrix aMatrix;
    ImpExportNewTrans_GetB2DHomMatrix(aMatrix, xProps);

    // decompose and correct about pRefPoint
    ::basegfx::B2DTuple aTRScale;
    double fTRShear(0.0);
    double fTRRotate(0.0);
    ::basegfx::B2DTuple aTRTranslate;
    ImpExportNewTrans_DecomposeAndRefPoint(aMatrix, aTRScale, fTRShear,
            fTRRotate, aTRTranslate, pRefPoint);

    // fdo#49678: create and export ViewBox
    awt::Point aPoint(0, 0);
    awt::Size aSize(FRound(aTRScale.getX()), FRound(aTRScale.getY()));
    SdXMLImExViewBox aViewBox(0, 0, aSize.Width, aSize.Height);
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString());

    // write connector shape. Add Export later.
    sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_CONNECTOR, bCreateNewline, sal_True);

    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );
    ImpExportText( xShape );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportMeasureShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    uno::Reference< beans::XPropertySet > xProps( xShape, uno::UNO_QUERY );

    OUString aStr;
    OUStringBuffer sStringBuffer;

    // export start and end point
    awt::Point aStart(0,0);
    awt::Point aEnd(1,1);

    /* Get <StartPositionInHoriL2R> and
       <EndPositionInHoriL2R>, if they exist and if the document is exported
       into the OpenOffice.org file format.
       These properties only exist at service com::sun::star::text::Shape - the
       Writer UNO service for shapes.
       This code is needed, because the positioning attributes in the
       OpenOffice.org file format are given in horizontal left-to-right layout
       regardless the layout direction the shape is in. In the OASIS Open Office
       file format the positioning attributes are correctly given in the layout
       direction the shape is in. Thus, this code provides the conversion from
       the OASIS Open Office file format to the OpenOffice.org file format. (#i36248#)
    */
    if ( ( GetExport().getExportFlags() & EXPORT_OASIS ) == 0 &&
         xProps->getPropertySetInfo()->hasPropertyByName(
            OUString("StartPositionInHoriL2R")) &&
         xProps->getPropertySetInfo()->hasPropertyByName(
            OUString("EndPositionInHoriL2R")) )
    {
        xProps->getPropertyValue(OUString("StartPositionInHoriL2R")) >>= aStart;
        xProps->getPropertyValue(OUString("EndPositionInHoriL2R")) >>= aEnd;
    }
    else
    {
        xProps->getPropertyValue(OUString("StartPosition")) >>= aStart;
        xProps->getPropertyValue(OUString("EndPosition")) >>= aEnd;
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
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.X);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X1, aStr);
    }
    else
    {
        aEnd.X -= aStart.X;
    }

    if( nFeatures & SEF_EXPORT_Y )
    {
        // svg: y1
        mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                aStart.Y);
        aStr = sStringBuffer.makeStringAndClear();
        mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_X2, aStr);

    // svg: y2
    mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    mrExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y2, aStr);

    // write measure shape
    sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
    SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_MEASURE, bCreateNewline, sal_True);

    ImpExportDescription( xShape ); // #i68101#
    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );

    uno::Reference< text::XText > xText( xShape, uno::UNO_QUERY );
    if( xText.is() )
        mrExport.GetTextParagraphExport()->exportText( xText );
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportOLE2Shape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */,
    SvXMLAttributeList* pAttrList /* = NULL */ )
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
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_OBJECT) );
        else if(eShapeType == XmlShapeTypePresChartShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_CHART) );
        else if(eShapeType == XmlShapeTypePresSheetShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_TABLE) );

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        sal_Bool bExportEmbedded(0 != (mrExport.getExportFlags() & EXPORT_EMBEDDED));
        OUString sPersistName;
        SvXMLElementExport aElement( mrExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        const bool bSaveBackwardsCompatible = ( mrExport.getExportFlags() & EXPORT_SAVEBACKWARDCOMPATIBLE );

        if( !bIsEmptyPresObj || bSaveBackwardsCompatible )
        {
            if (pAttrList)
            {
                mrExport.AddAttributeList(pAttrList);
            }

            OUString sClassId;
            OUString sURL;
            sal_Bool bInternal = false;
            xPropSet->getPropertyValue(OUString("IsInternal")) >>= bInternal;

            if( !bIsEmptyPresObj )
            {

                if ( bInternal )
                {
                    // OOo internal links have no storage persistance, URL is stored in the XML file
                    // the result LinkURL is empty in case the object is not a link
                    xPropSet->getPropertyValue( OUString( "LinkURL" ) ) >>= sURL;
                }

                xPropSet->getPropertyValue( OUString( "PersistName"  ) ) >>= sPersistName;
                if ( sURL.isEmpty() )
                {
                    if( !sPersistName.isEmpty() )
                    {
                        sURL = OUString(  "vnd.sun.star.EmbeddedObject:"  );
                        sURL += sPersistName;
                    }
                }

                if( !bInternal )
                    xPropSet->getPropertyValue(OUString("CLSID")) >>= sClassId;

                if( !sClassId.isEmpty() )
                    mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CLASS_ID, sClassId );
                if(!bExportEmbedded)
                {
                    // xlink:href
                    if( !sURL.isEmpty() )
                    {
                        // #96717# in theorie, if we don't have a url we shouldn't even
                        // export this ole shape. But practical its to risky right now
                        // to change this so we better dispose this on load
                        sURL = mrExport.AddEmbeddedObject( sURL );

                        mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sURL );
                        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                        mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
                    }
                }
            }
            else
            {
                // export empty href for empty placeholders to be valid odf
                OUString sEmptyURL;

                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sEmptyURL );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
            }

            enum XMLTokenEnum eElem = sClassId.isEmpty() ? XML_OBJECT : XML_OBJECT_OLE ;
            SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW, eElem, sal_True, sal_True );

            if(bExportEmbedded && !bIsEmptyPresObj)
            {
                // #100592#
                if(bInternal)
                {
                    // embedded XML
                    uno::Reference< lang::XComponent > xComp;
                    xPropSet->getPropertyValue( OUString("Model" ) ) >>= xComp;
                    DBG_ASSERT( xComp.is(), "no xModel for own OLE format" );
                    mrExport.ExportEmbeddedOwnObject( xComp );
                }
                else
                {
                    // embed as Base64
                    // this is an alien object ( currently MSOLE is the only supported type of such objects )
                    // in case it is not an OASIS format the object should be asked to store replacement image if possible

                    ::rtl::OUString sURLRequest( sURL );
                    if ( ( mrExport.getExportFlags() & EXPORT_OASIS ) == 0 )
                        sURLRequest += ::rtl::OUString(  "?oasis=false"  );
                    mrExport.AddEmbeddedObjectAsBase64( sURLRequest );
                }
            }
        }
        if( !bIsEmptyPresObj )
        {
            OUString sURL(  "vnd.sun.star.GraphicObject:"  );
            sURL += sPersistName;
            if( !bExportEmbedded )
            {
                sURL = GetExport().AddEmbeddedObject( sURL );
                mrExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sURL );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                mrExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
            }

            SvXMLElementExport aElem( GetExport(), XML_NAMESPACE_DRAW,
                                      XML_IMAGE, sal_False, sal_True );

            if( bExportEmbedded )
                GetExport().AddEmbeddedObjectAsBase64( sURL );
        }

        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
        ImpExportDescription( xShape ); // #i68101#
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportPageShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // #86163# Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        // export page number used for this page
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
        const OUString aPageNumberStr("PageNumber");
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(aPageNumberStr))
        {
            sal_Int32 nPageNumber = 0;
            xPropSet->getPropertyValue(aPageNumberStr) >>= nPageNumber;
            if( nPageNumber )
                mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_PAGE_NUMBER, OUString::valueOf(nPageNumber));
        }

        // a presentation page shape, normally used on notes pages only. If
        // it is used not as presentation shape, it may have been created with
        // copy-paste exchange between draw and impress (this IS possible...)
        if(eShapeType == XmlShapeTypePresPageShape)
        {
            mrExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_CLASS,
                                 XML_PRESENTATION_PAGE);
        }

        // write Page shape
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_PAGE_THUMBNAIL, bCreateNewline, sal_True);
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportCaptionShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        // evtl. corner radius?
        sal_Int32 nCornerRadius(0L);
        xPropSet->getPropertyValue(OUString("CornerRadius")) >>= nCornerRadius;
        if(nCornerRadius)
        {
            OUStringBuffer sStringBuffer;
            mrExport.GetMM100UnitConverter().convertMeasureToXML(sStringBuffer,
                    nCornerRadius);
            mrExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CORNER_RADIUS, sStringBuffer.makeStringAndClear());
        }

        awt::Point aCaptionPoint;
        xPropSet->getPropertyValue( OUString(  "CaptionPoint"  ) ) >>= aCaptionPoint;

        mrExport.GetMM100UnitConverter().convertMeasureToXML(msBuffer,
                aCaptionPoint.X);
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CAPTION_POINT_X, msBuffer.makeStringAndClear() );
        mrExport.GetMM100UnitConverter().convertMeasureToXML(msBuffer,
                aCaptionPoint.Y);
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CAPTION_POINT_Y, msBuffer.makeStringAndClear() );

        // write Caption shape. Add export later.
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        sal_Bool bAnnotation( (nFeatures & SEF_EXPORT_ANNOTATION) == SEF_EXPORT_ANNOTATION );

        SvXMLElementExport aObj( mrExport,
                                 (bAnnotation ? XML_NAMESPACE_OFFICE
                                               : XML_NAMESPACE_DRAW),
                                 (bAnnotation ? XML_ANNOTATION : XML_CAPTION),
                                 bCreateNewline, sal_True );

        ImpExportDescription( xShape ); // #i68101#
        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
        if( bAnnotation )
            mrExport.exportAnnotationMeta( xShape );
        ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportFrameShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // export frame url
        OUString aStr;
        xPropSet->getPropertyValue( OUString(  "FrameURL"  ) ) >>= aStr;
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStr) );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

        // export name
        xPropSet->getPropertyValue( OUString(  "FrameName"  ) ) >>= aStr;
        if( !aStr.isEmpty() )
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_FRAME_NAME, aStr );

        // write floating frame
        {
            SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_FLOATING_FRAME, sal_True, sal_True);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportAppletShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElement( mrExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // export frame url
        OUString aStr;
        xPropSet->getPropertyValue( OUString(  "AppletCodeBase"  ) ) >>= aStr;
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStr) );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

        // export draw:applet-name
        xPropSet->getPropertyValue( OUString(  "AppletName"  ) ) >>= aStr;
        if( !aStr.isEmpty() )
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_APPLET_NAME, aStr );

        // export draw:code
        xPropSet->getPropertyValue( OUString(  "AppletCode"  ) ) >>= aStr;
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CODE, aStr );

        // export draw:may-script
        sal_Bool bIsScript = false;
        xPropSet->getPropertyValue( OUString(  "AppletIsScript"  ) ) >>= bIsScript;
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MAY_SCRIPT, bIsScript ? XML_TRUE : XML_FALSE );

        {
            // write applet
            SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_APPLET, sal_True, sal_True);

            // export parameters
            uno::Sequence< beans::PropertyValue > aCommands;
            xPropSet->getPropertyValue( OUString(  "AppletCommands"  ) ) >>= aCommands;
            const sal_Int32 nCount = aCommands.getLength();
            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                aCommands[nIndex].Value >>= aStr;
                mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aCommands[nIndex].Name );
                mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportPluginShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElement( mrExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // export plugin url
        OUString aStr;
        xPropSet->getPropertyValue( OUString(  "PluginURL"  ) ) >>= aStr;
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStr) );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );


        // export mime-type
        xPropSet->getPropertyValue( OUString(  "PluginMimeType"  ) ) >>= aStr;
        if(!aStr.isEmpty())
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIME_TYPE, aStr );

        {
            // write plugin
            SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_PLUGIN, sal_True, sal_True);

            // export parameters
            uno::Sequence< beans::PropertyValue > aCommands;
            xPropSet->getPropertyValue( OUString(  "PluginCommands"  ) ) >>= aCommands;
            const sal_Int32 nCount = aCommands.getLength();
            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                aCommands[nIndex].Value >>= aStr;
                mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aCommands[nIndex].Name );
                mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

static void lcl_CopyStream(
        uno::Reference<io::XInputStream> const& xInStream,
        uno::Reference<embed::XStorage> const& xTarget,
        ::rtl::OUString const& rPath)
{
    ::comphelper::LifecycleProxy proxy;
    uno::Reference<io::XStream> const xStream(
        ::comphelper::OStorageHelper::GetStreamAtPackageURL(xTarget, rPath,
            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE, proxy));
    uno::Reference<io::XOutputStream> const xOutStream(
            (xStream.is()) ? xStream->getOutputStream() : 0);
    if (!xOutStream.is())
    {
        SAL_WARN("xmloff", "no output stream");
        throw uno::Exception(
            ::rtl::OUString("no output stream"),0);
    }
    uno::Reference< beans::XPropertySet > const xStreamProps(xStream,
        uno::UNO_QUERY);
    if (xStreamProps.is()) { // this is NOT supported in FileSystemStorage
        xStreamProps->setPropertyValue(
            ::rtl::OUString("MediaType"),
            uno::makeAny(::rtl::OUString(
            //FIXME how to detect real media type?
            //but currently xmloff has this one hardcoded anyway...
                    "application/vnd.sun.star.media")));
        xStreamProps->setPropertyValue( // turn off compression
            ::rtl::OUString("Compressed"),
            uno::makeAny(sal_False));
    }
    ::comphelper::OStorageHelper::CopyInputToOutput(xInStream, xOutStream);
    xOutStream->closeOutput();
    proxy.commitStorages();
}

static char const s_PkgScheme[] = "vnd.sun.star.Package:";

static ::rtl::OUString
lcl_StoreMediaAndGetURL(SvXMLExport & rExport,
    uno::Reference<beans::XPropertySet> const& xPropSet,
    ::rtl::OUString const& rURL)
{
    if (0 == rtl_ustr_ascii_shortenedCompareIgnoreAsciiCase_WithLength(
                rURL.getStr(), rURL.getLength(),
                s_PkgScheme, SAL_N_ELEMENTS(s_PkgScheme) - 1))
    {
        try // video is embedded
        {
            uno::Reference<embed::XStorage> const xTarget(
                    rExport.GetTargetStorage(), uno::UNO_QUERY_THROW);
            uno::Reference<io::XInputStream> xInStream;
            xPropSet->getPropertyValue(
                ::rtl::OUString("PrivateStream"))
                    >>= xInStream;

            if (!xInStream.is())
            {
                SAL_WARN("xmloff", "no input stream");
                return ::rtl::OUString();
            }

            ::rtl::OUString const urlPath(
                    rURL.copy(SAL_N_ELEMENTS(s_PkgScheme)-1));

            lcl_CopyStream(xInStream, xTarget, rURL);

            return urlPath;
        }
        catch (uno::Exception const& e)
        {
            SAL_INFO("xmloff", "exception while storing embedded media: '"
                        << e.Message << "'");
        }
        return ::rtl::OUString();
    }
    else
    {
        return rExport.GetRelativeReference(rURL); // linked
    }
}

void XMLShapeExport::ImpExportMediaShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        if(eShapeType == XmlShapeTypePresMediaShape)
            ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_OBJECT) );

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( mrExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // export media url
        OUString aMediaURL;
        xPropSet->getPropertyValue( OUString(  "MediaURL"  ) ) >>= aMediaURL;
        OUString const persistentURL =
            lcl_StoreMediaAndGetURL(GetExport(), xPropSet, aMediaURL);
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, persistentURL );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        mrExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

        // export mime-type
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIME_TYPE, OUString(  "application/vnd.sun.star.media"  ) );

        // write plugin
        SvXMLElementExport aOBJ(mrExport, XML_NAMESPACE_DRAW, XML_PLUGIN, !( nFeatures & SEF_EXPORT_NO_WS ), sal_True);

        // export parameters
        const OUString aFalseStr(  "false"  ), aTrueStr(  "true"  );

        sal_Bool bLoop = false;
        const OUString aLoopStr(  "Loop"  );
        xPropSet->getPropertyValue( aLoopStr ) >>= bLoop;
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aLoopStr );
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, bLoop ? aTrueStr : aFalseStr );
        delete( new SvXMLElementExport( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True ) );

        sal_Bool bMute = false;
        const OUString aMuteStr(  "Mute"  );
        xPropSet->getPropertyValue( aMuteStr ) >>= bMute;
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aMuteStr );
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, bMute ? aTrueStr : aFalseStr );
        delete( new SvXMLElementExport( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True ) );

        sal_Int16 nVolumeDB = 0;
        const OUString aVolumeDBStr(  "VolumeDB"  );
        xPropSet->getPropertyValue( OUString(  "VolumeDB"  ) ) >>= nVolumeDB;
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aVolumeDBStr );
        mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, OUString::valueOf( static_cast< sal_Int32 >( nVolumeDB ) ) );
        delete( new SvXMLElementExport( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True ) );

        media::ZoomLevel eZoom;
        const OUString aZoomStr(  "Zoom"  );
        OUString aZoomValue;
        xPropSet->getPropertyValue( OUString(  "Zoom"  ) ) >>= eZoom;
        switch( eZoom )
        {
            case( media::ZoomLevel_ZOOM_1_TO_4 ): aZoomValue = OUString(  "25%"  ); break;
            case( media::ZoomLevel_ZOOM_1_TO_2 ): aZoomValue = OUString(  "50%"  ); break;
            case( media::ZoomLevel_ORIGINAL ): aZoomValue = OUString(  "100%"  ); break;
            case( media::ZoomLevel_ZOOM_2_TO_1 ): aZoomValue = OUString(  "200%"  ); break;
            case( media::ZoomLevel_ZOOM_4_TO_1 ): aZoomValue = OUString(  "400%"  ); break;
            case( media::ZoomLevel_FIT_TO_WINDOW ): aZoomValue = OUString(  "fit"  ); break;
            case( media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT ): aZoomValue = OUString(  "fixedfit"  ); break;
            case( media::ZoomLevel_FULLSCREEN ): aZoomValue = OUString(  "fullscreen"  ); break;

            default:
            break;
        }

        if( !aZoomValue.isEmpty() )
        {
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aZoomStr );
            mrExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aZoomValue );
            delete( new SvXMLElementExport( mrExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True ) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
