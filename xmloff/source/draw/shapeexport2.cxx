/*************************************************************************
 *
 *  $RCSfile: shapeexport2.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 08:50:21 $
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

#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS_HPP_
#include <com/sun/star/container/XEnumerationAccess.hpp>
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

#ifndef _COM_SUN_STAR_MEDIA_ZOOMLEVEL_HPP_
#include <com/sun/star/media/ZoomLevel.hpp>
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

#ifndef _XMLOFF_XMLIMAGEMAPEXPORT_HXX_
#include "XMLImageMapExport.hxx"
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include "nmspmap.hxx"
#endif

#include "xmlnmspe.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::xmloff::token;


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
    // allways write Size (rTRScale) since this statement carries the union
    // of the object
    OUString aStr;
    OUStringBuffer sStringBuffer;
    Vector2D aTRScale = rTRScale;

    // svg: width
    if(!(nFeatures & SEF_EXPORT_WIDTH))
        aTRScale.X() = 1.0;
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, FRound(aTRScale.X()));
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, XML_WIDTH, aStr);

    // svg: height
    if(!(nFeatures & SEF_EXPORT_HEIGHT))
        aTRScale.Y() = 1.0;
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, FRound(aTRScale.Y()));
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, XML_HEIGHT, aStr);

    // decide if transformation is neccessary
    BOOL bTransformationIsNeccessary(fTRShear != 0.0 || fTRRotate != 0.0);

    if(bTransformationIsNeccessary)
    {
        // write transformation, but WITHOUT scale which is exported as size above
        SdXMLImExTransform2D aTransform;

        aTransform.AddSkewX(atan(fTRShear));
        aTransform.AddRotate(fTRRotate);
        aTransform.AddTranslate(rTRTranslate);

        // does transformation need to be exported?
        if(aTransform.NeedsAction())
            rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_TRANSFORM, aTransform.GetExportString(rExport.GetMM100UnitConverter()));
    }
    else
    {
        // no shear, no rotate; just add object position to export and we are done
        if(nFeatures & SEF_EXPORT_X)
        {
            // svg: x
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, FRound(rTRTranslate.X()));
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, XML_X, aStr);
        }

        if(nFeatures & SEF_EXPORT_Y)
        {
            // svg: y
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, FRound(rTRTranslate.Y()));
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y, aStr);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

sal_Bool XMLShapeExport::ImpExportPresentationAttributes( const uno::Reference< beans::XPropertySet >& xPropSet, const rtl::OUString& rClass )
{
    sal_Bool bIsEmpty = sal_False;

    OUStringBuffer sStringBuffer;

    // write presentation class entry
    rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_CLASS, rClass);

    if( xPropSet.is() )
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );

        sal_Bool bTemp;

        // is empty pes shape?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject"))))
        {
            xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject"))) >>= bIsEmpty;
            if( bIsEmpty )
                rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_PLACEHOLDER, XML_TRUE);
        }

        // is user-transformed?
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent"))))
        {
            xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent"))) >>= bTemp;
            if(!bTemp)
                rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_USER_TRANSFORMED, XML_TRUE);
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
            rExport.GetTextParagraphExport()->exportText( xText );
    }
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
            else if( ( ( nFound & FOUND_MACRO ) == 0 ) && ( pProperties->Name == msMacroName || pProperties->Name == msScript ) )
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

            SvXMLElementExport aEventsElemt(rExport, XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, sal_True, sal_True);

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
                DBG_ERROR( "unknown presentation::ClickAction found!" );
            }

            OUString aEventQName(
                rExport.GetNamespaceMap().GetQNameByKey(
                        XML_NAMESPACE_DOM, OUString( RTL_CONSTASCII_USTRINGPARAM( "click" ) ) ) );
            rExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, aEventQName );
            rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_ACTION, eStrAction );

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
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_EFFECT, msBuffer.makeStringAndClear() );
                    }

                    if( eDirection != ED_none )
                    {
                        SvXMLUnitConverter::convertEnum( msBuffer, eDirection, aXML_AnimationDirection_EnumMap );
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_DIRECTION, msBuffer.makeStringAndClear() );
                    }

                    if( nStartScale != -1 )
                    {
                        SvXMLUnitConverter::convertPercent( msBuffer, nStartScale );
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_START_SCALE, msBuffer.makeStringAndClear() );
                    }
                }

                if( nFound & FOUND_SPEED && eEffect != presentation::AnimationEffect_NONE )
                {
                    if( eSpeed != presentation::AnimationSpeed_MEDIUM )
                    {
                        SvXMLUnitConverter::convertEnum( msBuffer, eSpeed, aXML_AnimationSpeed_EnumMap );
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_SPEED, msBuffer.makeStringAndClear() );
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
                rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(msBuffer.makeStringAndClear()) );
                rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_NEW );
                rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
            }

            if( ( nFound & FOUND_VERB ) && eClickAction == presentation::ClickAction_VERB )
            {
                msBuffer.append( nVerb );
                rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_VERB, msBuffer.makeStringAndClear());
            }

            SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_PRESENTATION, XML_EVENT_LISTENER, sal_True, sal_True);

            if( eClickAction == presentation::ClickAction_VANISH || eClickAction == presentation::ClickAction_SOUND )
            {
                if( ( nFound & FOUND_SOUNDURL ) && aStrSoundURL.getLength() != 0 )
                {
                    rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStrSoundURL) );
                    rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                    rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_NEW );
                    rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONREQUEST );
                    if( nFound & FOUND_PLAYFULL && bPlayFull )
                        rExport.AddAttribute( XML_NAMESPACE_PRESENTATION, XML_PLAY_FULL, XML_TRUE );

                    SvXMLElementExport aElem( rExport, XML_NAMESPACE_PRESENTATION, XML_SOUND, sal_True, sal_True );
                }
            }
            break;
        }
        else if( aStrEventType == msStarBasic )
        {
            if( nFound & FOUND_MACRO )
            {
                SvXMLElementExport aEventsElemt(rExport, XML_NAMESPACE_OFFICE, XML_EVENT_LISTENERS, sal_True, sal_True);

                rExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_LANGUAGE, OUString( RTL_CONSTASCII_USTRINGPARAM( "starbasic" ) ) );
                OUString aEventQName(
                    rExport.GetNamespaceMap().GetQNameByKey(
                            XML_NAMESPACE_DOM, OUString( RTL_CONSTASCII_USTRINGPARAM( "click" ) ) ) );
                rExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, aEventQName );

                if( nFound & FOUND_LIBRARY )
                {
                    const OUString& rApp = GetXMLToken( XML_APPLICATION );
                    const OUString& rDoc = GetXMLToken( XML_DOCUMENT );
                    OUString sLocation( GetXMLToken(
                        (aStrLibrary.equalsIgnoreAsciiCaseAscii("StarOffice") ||
                          aStrLibrary.equalsIgnoreAsciiCaseAscii("application") ) ? XML_APPLICATION
                                                                              : XML_DOCUMENT ) );
                    OUStringBuffer sTmp( sLocation.getLength() + aStrMacro.getLength() + 1 );
                    sTmp = sLocation;
                    sTmp.append( sal_Unicode( ':' ) );
                    sTmp.append( aStrMacro );
                    rExport.AddAttribute(XML_NAMESPACE_SCRIPT, XML_MACRO_NAME,
                                         sTmp.makeStringAndClear());
                }
                else
                {
                    rExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_MACRO_NAME, aStrMacro );
                }


                SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SCRIPT, XML_EVENT_LISTENER, sal_True, sal_True);
            }
        }
        else if( aStrEventType == msScript )
        {
            if( nFound & FOUND_MACRO )
            {
                SvXMLElementExport aEventsElemt(rExport, XML_NAMESPACE_OFFICE, XML_EVENTS, sal_True, sal_True);

                rExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_LANGUAGE, XML_SCRIPT );
                rExport.AddAttribute( XML_NAMESPACE_SCRIPT, XML_EVENT_NAME, OUString( RTL_CONSTASCII_USTRINGPARAM( "on-click" ) ) );
                rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_HREF, aStrMacro );

                SvXMLElementExport aEventElemt(rExport, XML_NAMESPACE_SCRIPT, XML_EVENT, sal_True, sal_True);
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
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aPGR(rExport, XML_NAMESPACE_DRAW, XML_G, bCreateNewline, sal_True);

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
        sal_Bool bIsPresShape(FALSE);
        sal_Bool bIsEmptyPresObj(FALSE);
        OUString aStr;

        switch(eShapeType)
        {
            case XmlShapeTypePresSubtitleShape:
            {
                aStr = GetXMLToken(XML_PRESENTATION_SUBTITLE);
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresTitleTextShape:
            {
                aStr = GetXMLToken(XML_PRESENTATION_TITLE);
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresOutlinerShape:
            {
                aStr = GetXMLToken(XML_PRESENTATION_OUTLINE);
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresNotesShape:
            {
                aStr = GetXMLToken(XML_PRESENTATION_NOTES);
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresHeaderShape:
            {
                aStr = GetXMLToken(XML_HEADER);
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresFooterShape:
            {
                aStr = GetXMLToken(XML_FOOTER);
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresSlideNumberShape:
            {
                aStr = GetXMLToken(XML_PAGE_NUMBER);
                bIsPresShape = TRUE;
                break;
            }
            case XmlShapeTypePresDateTimeShape:
            {
                aStr = GetXMLToken(XML_DATE_TIME);
                bIsPresShape = TRUE;
                break;
            }
        }

        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        if(bIsPresShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, aStr );


        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // evtl. corner radius?
        sal_Int32 nCornerRadius(0L);
        xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("CornerRadius"))) >>= nCornerRadius;
        if(nCornerRadius)
        {
            OUStringBuffer sStringBuffer;
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, nCornerRadius);
            rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CORNER_RADIUS, sStringBuffer.makeStringAndClear());
        }

        {
            // write text-box
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_TEXT_BOX, sal_True, sal_True);
            if(!bIsEmptyPresObj)
                ImpExportText( xShape );
        }

        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
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
            rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CORNER_RADIUS, sStringBuffer.makeStringAndClear());
        }

        // write rectangle
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_RECT, bCreateNewline, sal_True);

        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
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

        // #85920# use 'Geometry' to get the points of the line
        // since this slot take anchor pos into account.

        // get matrix
        Matrix3D aMat;
        ImpExportNewTrans_GetMatrix3D(aMat, xPropSet);

        // decompose and correct about pRefPoint
        Vector2D aTRScale;
        double fTRShear(0.0);
        double fTRRotate(0.0);
        Vector2D aTRTranslate;
        ImpExportNewTrans_DecomposeAndRefPoint(aMat, aTRScale, fTRShear, fTRRotate, aTRTranslate, pRefPoint);

        // create base position
        awt::Point aBasePosition(FRound(aTRTranslate.X()), FRound(aTRTranslate.Y()));

        // get the two points
        uno::Any aAny(xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry"))));
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
            rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aStart.X);
            aStr = sStringBuffer.makeStringAndClear();
            rExport.AddAttribute(XML_NAMESPACE_SVG, XML_X1, aStr);
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
            rExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y1, aStr);
        }
        else
        {
            aEnd.Y -= aStart.Y;
        }

        // svg: x2
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.X);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, XML_X2, aStr);

        // svg: y2
        rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.Y);
        aStr = sStringBuffer.makeStringAndClear();
        rExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y2, aStr);

        // write line
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_LINE, bCreateNewline, sal_True);

        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
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
            rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_KIND, sStringBuffer.makeStringAndClear() );

            // export start angle
            SvXMLUnitConverter::convertDouble( sStringBuffer, dStartAngle );
            rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_START_ANGLE, sStringBuffer.makeStringAndClear() );

            // export end angle
            SvXMLUnitConverter::convertDouble( sStringBuffer, dEndAngle );
            rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_END_ANGLE, sStringBuffer.makeStringAndClear() );
        }

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#

        if(bCircle)
        {
            // write circle
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_CIRCLE, bCreateNewline, sal_True);

            ImpExportEvents( xShape );
            ImpExportGluePoints( xShape );
            ImpExportText( xShape );
        }
        else
        {
            // write ellipse
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_ELLIPSE, bCreateNewline, sal_True);

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
        rExport.AddAttribute(XML_NAMESPACE_SVG, XML_VIEWBOX, aViewBox.GetExportString(rExport.GetMM100UnitConverter()));

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#

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
                    rExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aSvgDElement.GetExportString());
                }

                // write object now
                SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_PATH, bCreateNewline, sal_True);

                ImpExportEvents( xShape );
                ImpExportGluePoints( xShape );
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
                        SdXMLImExPointsElement aPoints(pSequence, aViewBox, aPoint, aSize, rExport.GetMM100UnitConverter(),
                            // #96328#
                            bClosed);

                        // write point array
                        rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_POINTS, aPoints.GetExportString());
                    }

                    // write object now
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW,
                        bClosed ? XML_POLYGON : XML_POLYLINE , bCreateNewline, sal_True);

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
                        rExport.AddAttribute(XML_NAMESPACE_SVG, XML_D, aSvgDElement.GetExportString());
                    }

                    // write object now
                    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_PATH, bCreateNewline, sal_True);

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
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        if( !bIsEmptyPresObj )
        {
            OUString aStreamURL;
            OUString aStr;

            xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicURL"))) >>= sImageURL;
            aStr = rExport.AddEmbeddedGraphicObject( sImageURL );
            rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, aStr );

            if( aStr.getLength() )
            {
                if( aStr[ 0 ] == '#' )
                {
                    aStreamURL = OUString::createFromAscii( "vnd.sun.star.Package:" );
                    aStreamURL = aStreamURL.concat( aStr.copy( 1, aStr.getLength() - 1 ) );
                }

                // update stream URL for load on demand
                uno::Any aAny;
                aAny <<= aStreamURL;
                xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicStreamURL")), aAny );

                rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );

                rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );

                rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
            }
        }

        {
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_IMAGE, sal_True, sal_True);

            if( sImageURL.getLength() )
            {
                // optional office:binary-data
                rExport.AddEmbeddedGraphicObjectAsBase64( sImageURL );
            }
            ImpExportText( xShape );
        }

        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );

        // image map
        GetExport().GetImageMapExport().Export( xPropSet );
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
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_CHART, XML_CHART, sal_True, sal_True);
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
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CONTROL, rExport.GetFormExport()->getControlId( xControlModel ) );
        }
    }

    sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_CONTROL, bCreateNewline, sal_True);
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
        rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_TYPE, aStr);
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
        rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_LINE_SKEW, aStr);
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
        rExport.AddAttribute(XML_NAMESPACE_SVG, XML_X1, aStr);
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
        rExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, XML_X2, aStr);

    // svg: y2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y2, aStr);

    uno::Reference< drawing::XShape > xTempShape;

    // export start connection
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartShape") ) );
    if( aAny >>= xTempShape )
    {
        sal_Int32 nShapeId = rExport.GetShapeExport()->getShapeId( xTempShape );
        rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_START_SHAPE, OUString::valueOf( nShapeId ));

        aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartGluePointIndex")) );
        sal_Int32 nGluePointId;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_START_GLUE_POINT, OUString::valueOf( nGluePointId ));
            }
        }
    }

    // export end connection
    aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndShape")) );
    if( aAny >>= xTempShape )
    {

        sal_Int32 nShapeId = rExport.GetShapeExport()->getShapeId( xTempShape );
        rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_END_SHAPE, OUString::valueOf( nShapeId ));

        aAny = xProps->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndGluePointIndex")) );
        sal_Int32 nGluePointId;
        if( aAny >>= nGluePointId )
        {
            if( nGluePointId != -1 )
            {
                rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_END_GLUE_POINT, OUString::valueOf( nGluePointId ));
            }
        }
    }

    // write connector shape. Add Export later.
    sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_CONNECTOR, bCreateNewline, sal_True);

    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );
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
        rExport.AddAttribute(XML_NAMESPACE_SVG, XML_X1, aStr);
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
        rExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y1, aStr);
    }
    else
    {
        aEnd.Y -= aStart.Y;
    }

    // svg: x2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.X);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, XML_X2, aStr);

    // svg: y2
    rExport.GetMM100UnitConverter().convertMeasure(sStringBuffer, aEnd.Y);
    aStr = sStringBuffer.makeStringAndClear();
    rExport.AddAttribute(XML_NAMESPACE_SVG, XML_Y2, aStr);

    // write measure shape
    sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
    SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_MEASURE, bCreateNewline, sal_True);

    ImpExportEvents( xShape );
    ImpExportGluePoints( xShape );

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
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_OBJECT) );
        else if(eShapeType == XmlShapeTypePresChartShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_CHART) );
        else if(eShapeType == XmlShapeTypePresTableShape)
            bIsEmptyPresObj = ImpExportPresentationAttributes( xPropSet, GetXMLToken(XML_PRESENTATION_TABLE) );

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        {
            OUString sClassId;
            OUString sURL;
            sal_Bool bExportEmbedded(0 != (rExport.getExportFlags() & EXPORT_EMBEDDED));
            sal_Bool bInternal;
            xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("IsInternal"))) >>= bInternal;

            if( !bIsEmptyPresObj )
            {
                OUString sPersistName;

                xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM( "PersistName" ) ) ) >>= sPersistName;
                if( sPersistName.getLength() )
                {
                    sURL = OUString( RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.EmbeddedObject:" ) );
                    sURL += sPersistName;
                }

                if( !bInternal )
                    xPropSet->getPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID"))) >>= sClassId;

                if( sClassId.getLength() )
                    rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CLASS_ID, sClassId );
                if(!bExportEmbedded)
                {
                    // xlink:href
                    if( sURL.getLength() )
                    {
                        // #96717# in theorie, if we don't have a url we shouldn't even
                        // export this ole shape. But practical its to risky right now
                        // to change this so we better dispose this on load
                        sURL = rExport.AddEmbeddedObject( sURL );

                        rExport.AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sURL );
                        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
                        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
                        rExport.AddAttribute( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );
                    }
                }
            }
            enum XMLTokenEnum eElem = sClassId.getLength() ? XML_OBJECT_OLE : XML_OBJECT;
            SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, eElem, sal_True, sal_True );

            if(bExportEmbedded && !bIsEmptyPresObj)
            {
                // #100592#
                if(bInternal)
                {
                    // embedded XML
                    uno::Reference< lang::XComponent > xComp;
                    xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ) ) >>= xComp;
                    DBG_ASSERT( xComp.is(), "no xModel for own OLE format" );
                    rExport.ExportEmbeddedOwnObject( xComp );
                }
                else
                {
                    // embed as Base64
                    rExport.AddEmbeddedObjectAsBase64( sURL );
                }
            }
        }

        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
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
        const OUString aPageNumberStr(RTL_CONSTASCII_USTRINGPARAM("PageNumber"));
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(aPageNumberStr))
        {
            sal_Int32 nPageNumber = 0;
            xPropSet->getPropertyValue(aPageNumberStr) >>= nPageNumber;
            rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_PAGE_NUMBER, OUString::valueOf(nPageNumber));
        }

        // a presentation page shape, normally used on notes pages only. If
        // it is used not as presentation shape, it may have been created with
        // copy-paste exchange between draw and impress (this IS possible...)
        if(eShapeType == XmlShapeTypePresPageShape)
        {
            rExport.AddAttribute(XML_NAMESPACE_PRESENTATION, XML_CLASS,
                                 XML_PRESENTATION_PAGE);
        }

        // write Page shape
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_PAGE_THUMBNAIL, bCreateNewline, sal_True);
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportCaptionShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures /* = SEF_DEFAULT */, awt::Point* pRefPoint /* = NULL */)
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
            rExport.AddAttribute(XML_NAMESPACE_DRAW, XML_CORNER_RADIUS, sStringBuffer.makeStringAndClear());
        }

        awt::Point aCaptionPoint;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "CaptionPoint" ) ) ) >>= aCaptionPoint;

        rExport.GetMM100UnitConverter().convertMeasure(msBuffer, aCaptionPoint.X);
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CAPTION_POINT_X, msBuffer.makeStringAndClear() );
        rExport.GetMM100UnitConverter().convertMeasure(msBuffer, aCaptionPoint.Y);
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CAPTION_POINT_Y, msBuffer.makeStringAndClear() );

        // write Caption shape. Add export later.
        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_CAPTION, bCreateNewline, sal_True);

        ImpExportEvents( xShape );
        ImpExportGluePoints( xShape );
        ImpExportText( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportFrameShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // export frame url
        OUString aStr;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FrameURL" ) ) ) >>= aStr;
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStr) );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

        // export name
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FrameName" ) ) ) >>= aStr;
        if( aStr.getLength() )
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_FRAME_NAME, aStr );

        // write floating frame
        {
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_FLOATING_FRAME, sal_True, sal_True);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportAppletShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // export frame url
        OUString aStr;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletCodeBase" ) ) ) >>= aStr;
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStr) );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

        // export draw:applet-name
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletName" ) ) ) >>= aStr;
        if( aStr.getLength() )
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_APPLET_NAME, aStr );

        // export draw:code
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletCode" ) ) ) >>= aStr;
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_CODE, aStr );

        // export draw:may-script
        sal_Bool bIsScript;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletIsScript" ) ) ) >>= bIsScript;
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MAY_SCRIPT, bIsScript ? XML_TRUE : XML_FALSE );

        {
            // write applet
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_APPLET, sal_True, sal_True);

            // export parameters
            uno::Sequence< beans::PropertyValue > aCommands;
            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletCommands" ) ) ) >>= aCommands;
            const sal_Int32 nCount = aCommands.getLength();
            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                aCommands[nIndex].Value >>= aStr;
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aCommands[nIndex].Name );
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportPluginShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // export plugin url
        OUString aStr;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "PluginURL" ) ) ) >>= aStr;
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference(aStr) );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );


        // export mime-type
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "PluginMimeType" ) ) ) >>= aStr;
        if(aStr.getLength())
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIME_TYPE, aStr );

        {
            // write plugin
            SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_PLUGIN, sal_True, sal_True);

            // export parameters
            uno::Sequence< beans::PropertyValue > aCommands;
            xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "PluginCommands" ) ) ) >>= aCommands;
            const sal_Int32 nCount = aCommands.getLength();
            for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
            {
                aCommands[nIndex].Value >>= aStr;
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aCommands[nIndex].Name );
                rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aStr );
                SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True );
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

void XMLShapeExport::ImpExportMediaShape(
    const uno::Reference< drawing::XShape >& xShape,
    XmlShapeType eShapeType, sal_Int32 nFeatures, com::sun::star::awt::Point* pRefPoint)
{
    const uno::Reference< beans::XPropertySet > xPropSet(xShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // Transformation
        ImpExportNewTrans(xPropSet, nFeatures, pRefPoint);

        sal_Bool bCreateNewline( (nFeatures & SEF_EXPORT_NO_WS) == 0 ); // #86116#/#92210#
        SvXMLElementExport aElem( rExport, XML_NAMESPACE_DRAW,
                                  XML_FRAME, bCreateNewline, sal_True );

        // export media url
        OUString aMediaURL;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaURL" ) ) ) >>= aMediaURL;
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_HREF, GetExport().GetRelativeReference( aMediaURL ) );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_SHOW, XML_EMBED );
        rExport.AddAttribute ( XML_NAMESPACE_XLINK, XML_ACTUATE, XML_ONLOAD );

        // export mime-type
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_MIME_TYPE, OUString( RTL_CONSTASCII_USTRINGPARAM( "application/vnd.sun.star.media" ) ) );

        // write plugin
        SvXMLElementExport aOBJ(rExport, XML_NAMESPACE_DRAW, XML_PLUGIN, !( nFeatures & SEF_EXPORT_NO_WS ), sal_True);

        // export parameters
        const OUString aFalseStr( RTL_CONSTASCII_USTRINGPARAM( "false" ) ), aTrueStr( RTL_CONSTASCII_USTRINGPARAM( "true" ) );

        sal_Bool bLoop;
        const OUString aLoopStr( RTL_CONSTASCII_USTRINGPARAM( "Loop" ) );
        xPropSet->getPropertyValue( aLoopStr ) >>= bLoop;
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aLoopStr );
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, bLoop ? aTrueStr : aFalseStr );
        delete( new SvXMLElementExport( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True ) );

        sal_Bool bMute;
        const OUString aMuteStr( RTL_CONSTASCII_USTRINGPARAM( "Mute" ) );
        xPropSet->getPropertyValue( aMuteStr ) >>= bMute;
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aMuteStr );
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, bMute ? aTrueStr : aFalseStr );
        delete( new SvXMLElementExport( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True ) );

        sal_Int16 nVolumeDB;
        const OUString aVolumeDBStr( RTL_CONSTASCII_USTRINGPARAM( "VolumeDB" ) );
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "VolumeDB" ) ) ) >>= nVolumeDB;
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aVolumeDBStr );
        rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, OUString::valueOf( static_cast< sal_Int32 >( nVolumeDB ) ) );
        delete( new SvXMLElementExport( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True ) );

        media::ZoomLevel eZoom;
        const OUString aZoomStr( RTL_CONSTASCII_USTRINGPARAM( "Zoom" ) );
        OUString aZoomValue;
        xPropSet->getPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Zoom" ) ) ) >>= eZoom;
        switch( eZoom )
        {
            case( media::ZoomLevel_ZOOM_1_TO_4 ): aZoomValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "25%" ) ); break;
            case( media::ZoomLevel_ZOOM_1_TO_2 ): aZoomValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "50%" ) ); break;
            case( media::ZoomLevel_ORIGINAL ): aZoomValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "100%" ) ); break;
            case( media::ZoomLevel_ZOOM_2_TO_1 ): aZoomValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "200%" ) ); break;
            case( media::ZoomLevel_ZOOM_4_TO_1 ): aZoomValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "400%" ) ); break;
            case( media::ZoomLevel_FIT_TO_WINDOW ): aZoomValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "fit" ) ); break;
            case( media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT ): aZoomValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "fixedfit" ) ); break;
            case( media::ZoomLevel_FULLSCREEN ): aZoomValue = OUString( RTL_CONSTASCII_USTRINGPARAM( "fullscreen" ) ); break;

            default:
            break;
        }

        if( aZoomValue.getLength() )
        {
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_NAME, aZoomStr );
            rExport.AddAttribute( XML_NAMESPACE_DRAW, XML_VALUE, aZoomValue );
            delete( new SvXMLElementExport( rExport, XML_NAMESPACE_DRAW, XML_PARAM, sal_False, sal_True ) );
        }
    }
}
