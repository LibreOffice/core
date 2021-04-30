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

#include <config_wasm_strip.h>

#include <cassert>

#include <sal/log.hxx>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/drawing/GluePoint2.hpp>
#include <com/sun/star/drawing/Alignment.hpp>
#include <com/sun/star/drawing/EscapeDirection.hpp>
#include <com/sun/star/media/ZoomLevel.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "ximpshap.hxx"
#include <xmloff/XMLBase64ImportContext.hxx>
#include <xmloff/XMLShapeStyleContext.hxx>
#include <xmloff/xmluconv.hxx>
#include <com/sun/star/container/XNamed.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <xexptran.hxx>
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/style/XStyle.hpp>

#include <sax/tools/converter.hxx>
#include <comphelper/sequence.hxx>
#include <tools/diagnose_ex.h>

#include <xmloff/families.hxx>
#include<xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>
#include <EnhancedCustomShapeToken.hxx>
#include <XMLReplacementImageContext.hxx>
#include <XMLImageMapContext.hxx>
#include "sdpropls.hxx"
#include "eventimp.hxx"
#include "descriptionimp.hxx"
#include "SignatureLineContext.hxx"
#include "QRCodeContext.hxx"
#include "ximpcustomshape.hxx"
#include <XMLEmbeddedObjectImportContext.hxx>
#include <xmloff/xmlerror.hxx>
#include <xmloff/table/XMLTableImport.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/attrlist.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/vector/b2dvector.hxx>
#include <o3tl/any.hxx>
#include <o3tl/safeint.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::xmloff::token;
using namespace ::xmloff::EnhancedCustomShapeToken;

SvXMLEnumMapEntry<drawing::Alignment> const aXML_GlueAlignment_EnumMap[] =
{
    { XML_TOP_LEFT,     drawing::Alignment_TOP_LEFT },
    { XML_TOP,          drawing::Alignment_TOP },
    { XML_TOP_RIGHT,    drawing::Alignment_TOP_RIGHT },
    { XML_LEFT,         drawing::Alignment_LEFT },
    { XML_CENTER,       drawing::Alignment_CENTER },
    { XML_RIGHT,        drawing::Alignment_RIGHT },
    { XML_BOTTOM_LEFT,  drawing::Alignment_BOTTOM_LEFT },
    { XML_BOTTOM,       drawing::Alignment_BOTTOM },
    { XML_BOTTOM_RIGHT, drawing::Alignment_BOTTOM_RIGHT },
    { XML_TOKEN_INVALID, drawing::Alignment(0) }
};

SvXMLEnumMapEntry<drawing::EscapeDirection> const aXML_GlueEscapeDirection_EnumMap[] =
{
    { XML_AUTO,         drawing::EscapeDirection_SMART },
    { XML_LEFT,         drawing::EscapeDirection_LEFT },
    { XML_RIGHT,        drawing::EscapeDirection_RIGHT },
    { XML_UP,           drawing::EscapeDirection_UP },
    { XML_DOWN,         drawing::EscapeDirection_DOWN },
    { XML_HORIZONTAL,   drawing::EscapeDirection_HORIZONTAL },
    { XML_VERTICAL,     drawing::EscapeDirection_VERTICAL },
    { XML_TOKEN_INVALID, drawing::EscapeDirection(0) }
};

static bool ImpIsEmptyURL( std::u16string_view rURL )
{
    if( rURL.empty() )
        return true;

    // #i13140# Also compare against 'toplevel' URLs. which also
    // result in empty filename strings.
    if( rURL == u"#./" )
        return true;

    return false;
}


SdXMLShapeContext::SdXMLShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
    : SvXMLShapeContext( rImport, bTemporaryShape )
    , mxShapes( rShapes )
    , mxAttrList(xAttrList)
    , mbListContextPushed( false )
    , mnStyleFamily(XmlStyleFamily::SD_GRAPHICS_ID)
    , mbIsPlaceholder(false)
    , mbClearDefaultAttributes( true )
    , mbIsUserTransformed(false)
    , mnZOrder(-1)
    , maSize(1, 1)
    , mnRelWidth(0)
    , mnRelHeight(0)
    , maPosition(0, 0)
    , mbVisible(true)
    , mbPrintable(true)
    , mbHaveXmlId(false)
    , mbTextBox(false)
{
}

SdXMLShapeContext::~SdXMLShapeContext()
{
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContextRef xContext;
    // #i68101#
    if( nElement == XML_ELEMENT(SVG, XML_TITLE) || nElement == XML_ELEMENT(SVG, XML_DESC)
        || nElement == XML_ELEMENT(SVG_COMPAT, XML_TITLE) || nElement == XML_ELEMENT(SVG_COMPAT, XML_DESC) )
    {
        xContext = new SdXMLDescriptionContext( GetImport(), nElement, mxShape );
    }
    else if( nElement == XML_ELEMENT(LO_EXT, XML_SIGNATURELINE) )
    {
        xContext = new SignatureLineContext( GetImport(), nElement, xAttrList, mxShape );
    }
    else if( nElement == XML_ELEMENT(LO_EXT, XML_QRCODE) )
    {
        xContext = new QRCodeContext( GetImport(), nElement, xAttrList, mxShape );
    }
    else if( nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) )
    {
        xContext = new SdXMLEventsContext( GetImport(), mxShape );
    }
    else if( nElement == XML_ELEMENT(DRAW, XML_GLUE_POINT) )
    {
        addGluePoint( xAttrList );
    }
    else if( nElement == XML_ELEMENT(DRAW, XML_THUMBNAIL) )
    {
        // search attributes for xlink:href
        maThumbnailURL = xAttrList->getOptionalValue(XML_ELEMENT(XLINK, XML_HREF));
    }
    else
    {
        // create text cursor on demand
        if( !mxCursor.is() )
        {
            uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
            if( xText.is() )
            {
                rtl::Reference < XMLTextImportHelper > xTxtImport =
                    GetImport().GetTextImport();
                mxOldCursor = xTxtImport->GetCursor();
                mxCursor = xText->createTextCursor();
                if( mxCursor.is() )
                {
                    xTxtImport->SetCursor( mxCursor );
                }

                // remember old list item and block (#91964#) and reset them
                // for the text frame
                xTxtImport->PushListContext();
                mbListContextPushed = true;
            }
        }

        // if we have a text cursor, lets  try to import some text
        if( mxCursor.is() )
        {
            xContext = GetImport().GetTextImport()->CreateTextChildContext(
                GetImport(), nElement, xAttrList,
                ( mbTextBox ? XMLTextType::TextBox : XMLTextType::Shape ) );
        }
    }

    if (!xContext)
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return xContext;
}

void SdXMLShapeContext::addGluePoint( const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    // get the gluepoints container for this shape if it's not already there
    if( !mxGluePoints.is() )
    {
        uno::Reference< drawing::XGluePointsSupplier > xSupplier( mxShape, uno::UNO_QUERY );
        if( !xSupplier.is() )
            return;

        mxGluePoints.set( xSupplier->getGluePoints(), UNO_QUERY );

        if( !mxGluePoints.is() )
            return;
    }

    drawing::GluePoint2 aGluePoint;
    aGluePoint.IsUserDefined = true;
    aGluePoint.Position.X = 0;
    aGluePoint.Position.Y = 0;
    aGluePoint.Escape = drawing::EscapeDirection_SMART;
    aGluePoint.PositionAlignment = drawing::Alignment_CENTER;
    aGluePoint.IsRelative = true;

    sal_Int32 nId = -1;

    // read attributes for the 3DScene
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        switch(aIter.getToken())
        {
            case XML_ELEMENT(SVG, XML_X):
            case XML_ELEMENT(SVG_COMPAT, XML_X):
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aGluePoint.Position.X, aIter.toString());
                break;
            case XML_ELEMENT(SVG, XML_Y):
            case XML_ELEMENT(SVG_COMPAT, XML_Y):
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aGluePoint.Position.Y, aIter.toString());
                break;
            case XML_ELEMENT(DRAW, XML_ID):
                nId = aIter.toInt32();
                break;
            case XML_ELEMENT(DRAW, XML_ALIGN):
            {
                drawing::Alignment eKind;
                if( SvXMLUnitConverter::convertEnum( eKind, aIter.toView(), aXML_GlueAlignment_EnumMap ) )
                {
                    aGluePoint.PositionAlignment = eKind;
                    aGluePoint.IsRelative = false;
                }
                break;
            }
            case XML_ELEMENT(DRAW, XML_ESCAPE_DIRECTION):
            {
                SvXMLUnitConverter::convertEnum( aGluePoint.Escape, aIter.toView(), aXML_GlueEscapeDirection_EnumMap );
                break;
            }
            default:
                XMLOFF_WARN_UNKNOWN("xmloff", aIter);
        }
    }

    if( nId != -1 )
    {
        try
        {
            sal_Int32 nInternalId = mxGluePoints->insert( uno::makeAny( aGluePoint ) );
            GetImport().GetShapeImport()->addGluePointMapping( mxShape, nId, nInternalId );
        }
        catch(const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION( "xmloff", "during setting of gluepoints");
        }
    }
}

void SdXMLShapeContext::startFastElement (sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/)
{
    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

void SdXMLShapeContext::endFastElement(sal_Int32 )
{
    if(mxCursor.is())
    {
        // delete addition newline
        mxCursor->gotoEnd( false );
        mxCursor->goLeft( 1, true );
        mxCursor->setString( "" );

        // reset cursor
        GetImport().GetTextImport()->ResetCursor();
    }

    if(mxOldCursor.is())
        GetImport().GetTextImport()->SetCursor( mxOldCursor );

    // reinstall old list item (if necessary) #91964#
    if (mbListContextPushed) {
        GetImport().GetTextImport()->PopListContext();
    }

    if( !msHyperlink.isEmpty() ) try
    {
        uno::Reference< beans::XPropertySet > xProp( mxShape, uno::UNO_QUERY );

        if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName( "Hyperlink" ) )
            xProp->setPropertyValue( "Hyperlink", uno::Any( msHyperlink ) );
        Reference< XEventsSupplier > xEventsSupplier( mxShape, UNO_QUERY );

        if( xEventsSupplier.is() )
        {
            Reference< XNameReplace > xEvents( xEventsSupplier->getEvents(), UNO_SET_THROW );

            uno::Sequence< beans::PropertyValue > aProperties{
                { /* Name   */ "EventType",
                  /* Handle */ -1,
                  /* Value  */ uno::Any(OUString( "Presentation" )),
                  /* State  */ beans::PropertyState_DIRECT_VALUE },

                { /* Name   */ "ClickAction",
                  /* Handle */ -1,
                  /* Value  */ uno::Any(css::presentation::ClickAction_DOCUMENT),
                  /* State  */ beans::PropertyState_DIRECT_VALUE },

                { /* Name   */ "Bookmark",
                  /* Handle */ -1,
                  /* Value  */ uno::Any(msHyperlink),
                  /* State  */ beans::PropertyState_DIRECT_VALUE }
            };

            xEvents->replaceByName( "OnClick", Any( aProperties ) );
        }
        else
        {
            // in draw use the Bookmark property
            Reference< beans::XPropertySet > xSet( mxShape, UNO_QUERY_THROW );
            xSet->setPropertyValue( "Bookmark", Any( msHyperlink ) );
            xSet->setPropertyValue("OnClick", Any( css::presentation::ClickAction_DOCUMENT ) );
        }
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("xmloff", "while setting hyperlink");
    }

    if( mxLockable.is() )
        mxLockable->removeActionLock();
}

void SdXMLShapeContext::AddShape(uno::Reference< drawing::XShape >& xShape)
{
    if(xShape.is())
    {
        // set shape local
        mxShape = xShape;

        if(!maShapeName.isEmpty())
        {
            uno::Reference< container::XNamed > xNamed( mxShape, uno::UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( maShapeName );
        }

        rtl::Reference< XMLShapeImportHelper > xImp( GetImport().GetShapeImport() );
        xImp->addShape( xShape, mxAttrList, mxShapes );

        if( mbClearDefaultAttributes )
        {
            uno::Reference<beans::XMultiPropertyStates> xMultiPropertyStates(xShape, uno::UNO_QUERY );
            if (xMultiPropertyStates.is())
                xMultiPropertyStates->setAllPropertiesToDefault();
        }

        if( !mbVisible || !mbPrintable ) try
        {
            uno::Reference< beans::XPropertySet > xSet( xShape, uno::UNO_QUERY_THROW );
            if( !mbVisible )
                xSet->setPropertyValue("Visible", uno::Any( false ) );

            if( !mbPrintable )
                xSet->setPropertyValue("Printable", uno::Any( false ) );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION( "xmloff", "while setting visible or printable" );
        }

        if(!mbTemporaryShape && (!GetImport().HasTextImport()
            || !GetImport().GetTextImport()->IsInsideDeleteContext()))
        {
            xImp->shapeWithZIndexAdded( xShape, mnZOrder );
        }

        if (mnRelWidth || mnRelHeight)
        {
            uno::Reference<beans::XPropertySet> xPropertySet(xShape, uno::UNO_QUERY);
            uno::Reference<beans::XPropertySetInfo> xPropertySetInfo = xPropertySet->getPropertySetInfo();
            if (mnRelWidth && xPropertySetInfo->hasPropertyByName("RelativeWidth"))
                xPropertySet->setPropertyValue("RelativeWidth", uno::makeAny(mnRelWidth));
            if (mnRelHeight && xPropertySetInfo->hasPropertyByName("RelativeHeight"))
                xPropertySet->setPropertyValue("RelativeHeight", uno::makeAny(mnRelHeight));
        }

        if( !maShapeId.isEmpty() )
        {
            uno::Reference< uno::XInterface > xRef( static_cast<uno::XInterface *>(xShape.get()) );
            GetImport().getInterfaceToIdentifierMapper().registerReference( maShapeId, xRef );
        }

        // #91065# count only if counting for shape import is enabled
        if(GetImport().GetShapeImport()->IsHandleProgressBarEnabled())
        {
            // #80365# increment progress bar at load once for each draw object
            GetImport().GetProgressBarHelper()->Increment();
        }
    }

    mxLockable.set( xShape, UNO_QUERY );

    if( mxLockable.is() )
        mxLockable->addActionLock();

}

void SdXMLShapeContext::AddShape(OUString const & serviceName)
{
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(!xServiceFact.is())
        return;

    try
    {
        /* Since fix for issue i33294 the Writer model doesn't support
           com.sun.star.drawing.OLE2Shape anymore.
           To handle Draw OLE objects it's decided to import these
           objects as com.sun.star.drawing.OLE2Shape and convert these
           objects after the import into com.sun.star.drawing.GraphicObjectShape.
        */
        uno::Reference< drawing::XShape > xShape;
        if ( serviceName == "com.sun.star.drawing.OLE2Shape" &&
             uno::Reference< text::XTextDocument >(GetImport().GetModel(), uno::UNO_QUERY).is() )
        {
            xShape.set(xServiceFact->createInstance("com.sun.star.drawing.temporaryForXMLImportOLE2Shape"), uno::UNO_QUERY);
        }
        else if (serviceName == "com.sun.star.drawing.GraphicObjectShape"
                 || serviceName == "com.sun.star.drawing.MediaShape"
                 || serviceName == "com.sun.star.presentation.MediaShape")
        {
            xShape.set( xServiceFact->createInstanceWithArguments(serviceName, { css::uno::Any(GetImport().GetDocumentBase()) }),
                        css::uno::UNO_QUERY);
        }
        else
        {
            xShape.set(xServiceFact->createInstance(serviceName), uno::UNO_QUERY);
        }
        if( xShape.is() )
            AddShape( xShape );
    }
    catch(const uno::Exception& e)
    {
        uno::Sequence<OUString> aSeq { serviceName };
        GetImport().SetError( XMLERROR_FLAG_ERROR | XMLERROR_API,
                              aSeq, e.Message, nullptr );
    }
}

void SdXMLShapeContext::SetTransformation()
{
    if(!mxShape.is())
        return;

    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
    if(!xPropSet.is())
        return;

    maUsedTransformation.identity();

    if(maSize.Width != 1 || maSize.Height != 1)
    {
        // take care there are no zeros used by error
        if(0 == maSize.Width)
            maSize.Width = 1;
        if(0 == maSize.Height)
            maSize.Height = 1;

        // set global size. This should always be used.
        maUsedTransformation.scale(maSize.Width, maSize.Height);
    }

    if(maPosition.X != 0 || maPosition.Y != 0)
    {
        // if global position is used, add it to transformation
        maUsedTransformation.translate(maPosition.X, maPosition.Y);
    }

    if(mnTransform.NeedsAction())
    {
        // transformation is used, apply to object.
        // NOTICE: The transformation is applied AFTER evtl. used
        // global positioning and scaling is used, so any shear or
        // rotate used herein is applied around the (0,0) position
        // of the PAGE object !!!
        ::basegfx::B2DHomMatrix aMat;
        mnTransform.GetFullTransform(aMat);

        // now add to transformation
        maUsedTransformation *= aMat;
    }

    // now set transformation for this object

    // maUsedTransformtion contains the mathematical correct matrix, which if
    // applied to a unit square would generate the transformed shape. But the property
    // "Transformation" contains a matrix, which can be used in TRSetBaseGeometry
    // and would be created by TRGetBaseGeometry. And those use a mathematically wrong
    // sign for the shearing angle. So we need to adapt the matrix here.
    basegfx::B2DTuple aScale;
    basegfx::B2DTuple aTranslate;
    double fRotate;
    double fShearX;
    maUsedTransformation.decompose(aScale, aTranslate, fRotate, fShearX);
    basegfx::B2DHomMatrix aB2DHomMatrix;
    aB2DHomMatrix = basegfx::utils::createScaleShearXRotateTranslateB2DHomMatrix(
                                aScale,
                                basegfx::fTools::equalZero(fShearX) ? 0.0 : -fShearX,
                                basegfx::fTools::equalZero(fRotate) ? 0.0 : fRotate,
                            aTranslate);
    drawing::HomogenMatrix3 aUnoMatrix;

    aUnoMatrix.Line1.Column1 = aB2DHomMatrix.get(0, 0);
    aUnoMatrix.Line1.Column2 = aB2DHomMatrix.get(0, 1);
    aUnoMatrix.Line1.Column3 = aB2DHomMatrix.get(0, 2);

    aUnoMatrix.Line2.Column1 = aB2DHomMatrix.get(1, 0);
    aUnoMatrix.Line2.Column2 = aB2DHomMatrix.get(1, 1);
    aUnoMatrix.Line2.Column3 = aB2DHomMatrix.get(1, 2);

    aUnoMatrix.Line3.Column1 = aB2DHomMatrix.get(2, 0);
    aUnoMatrix.Line3.Column2 = aB2DHomMatrix.get(2, 1);
    aUnoMatrix.Line3.Column3 = aB2DHomMatrix.get(2, 2);

    xPropSet->setPropertyValue("Transformation", Any(aUnoMatrix));
}

void SdXMLShapeContext::SetStyle( bool bSupportsStyle /* = true */)
{
    try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if( !xPropSet.is() )
            return;

        do
        {
            // set style on shape
            if(maDrawStyleName.isEmpty())
                break;

            const SvXMLStyleContext* pStyle = nullptr;
            bool bAutoStyle(false);

            if(GetImport().GetShapeImport()->GetAutoStylesContext())
                pStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(mnStyleFamily, maDrawStyleName);

            if(pStyle)
                bAutoStyle = true;

            if(!pStyle && GetImport().GetShapeImport()->GetStylesContext())
                pStyle = GetImport().GetShapeImport()->GetStylesContext()->FindStyleChildContext(mnStyleFamily, maDrawStyleName);

            OUString aStyleName = maDrawStyleName;
            uno::Reference< style::XStyle > xStyle;

            XMLPropStyleContext* pDocStyle
                = dynamic_cast<XMLShapeStyleContext*>(const_cast<SvXMLStyleContext*>(pStyle));
            if (pDocStyle)
            {
                if( pDocStyle->GetStyle().is() )
                {
                    xStyle = pDocStyle->GetStyle();
                }
                else
                {
                    aStyleName = pDocStyle->GetParentName();
                }
            }

            if( !xStyle.is() && !aStyleName.isEmpty() )
            {
                try
                {

                    uno::Reference< style::XStyleFamiliesSupplier > xFamiliesSupplier( GetImport().GetModel(), uno::UNO_QUERY );

                    if( xFamiliesSupplier.is() )
                    {
                        uno::Reference< container::XNameAccess > xFamilies( xFamiliesSupplier->getStyleFamilies() );
                        if( xFamilies.is() )
                        {

                            uno::Reference< container::XNameAccess > xFamily;

                            if( XmlStyleFamily::SD_PRESENTATION_ID == mnStyleFamily )
                            {
                                aStyleName = GetImport().GetStyleDisplayName(
                                    XmlStyleFamily::SD_PRESENTATION_ID,
                                    aStyleName );
                                sal_Int32 nPos = aStyleName.lastIndexOf( '-' );
                                if( -1 != nPos )
                                {
                                    OUString aFamily( aStyleName.copy( 0, nPos ) );

                                    xFamilies->getByName( aFamily ) >>= xFamily;
                                    aStyleName = aStyleName.copy( nPos + 1 );
                                }
                            }
                            else
                            {
                                // get graphics family
                                xFamilies->getByName("graphics") >>= xFamily;
                                aStyleName = GetImport().GetStyleDisplayName(
                                    XmlStyleFamily::SD_GRAPHICS_ID,
                                    aStyleName );
                            }

                            if( xFamily.is() )
                                xFamily->getByName( aStyleName ) >>= xStyle;
                        }
                    }
                }
                catch(const uno::Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION( "xmloff", "finding style for shape" );
                }
            }

            if( bSupportsStyle && xStyle.is() )
            {
                try
                {
                    // set style on object
                    xPropSet->setPropertyValue("Style", Any(xStyle));
                }
                catch(const uno::Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION( "xmloff", "setting style for shape" );
                }
            }

            // Writer shapes: if this one has a TextBox, set it here. We need to do it before
            // pDocStyle->FillPropertySet, because setting some properties depend on the format
            // having RES_CNTNT attribute (e.g., UNO_NAME_TEXT_(LEFT|RIGHT|UPPER|LOWER)DIST; see
            // SwTextBoxHelper::syncProperty, which indirectly calls SwTextBoxHelper::isTextBox)
            uno::Reference<beans::XPropertySetInfo> xPropertySetInfo
                = xPropSet->getPropertySetInfo();
            if (xPropertySetInfo->hasPropertyByName("TextBox"))
                xPropSet->setPropertyValue("TextBox", uno::makeAny(mbTextBox));

            // if this is an auto style, set its properties
            if(bAutoStyle && pDocStyle)
            {
                // set PropertySet on object
                pDocStyle->FillPropertySet(xPropSet);
            }

        } while(false);

        // try to set text auto style
        do
        {
            // set style on shape
            if( maTextStyleName.isEmpty() )
                break;

            if( nullptr == GetImport().GetShapeImport()->GetAutoStylesContext())
                break;

            const SvXMLStyleContext* pTempStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(XmlStyleFamily::TEXT_PARAGRAPH, maTextStyleName);
            XMLPropStyleContext* pStyle = const_cast<XMLPropStyleContext*>(dynamic_cast<const XMLPropStyleContext*>( pTempStyle ) ); // use temp var, PTR_CAST is a bad macro, FindStyleChildContext will be called twice
            if( pStyle == nullptr )
                break;

            // set PropertySet on object
            pStyle->FillPropertySet(xPropSet);

        } while(false);
    }
    catch(const uno::Exception&)
    {
    }
}

void SdXMLShapeContext::SetLayer()
{
    if( maLayerName.isEmpty() )
        return;

    try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is() )
        {
            xPropSet->setPropertyValue("LayerName", Any(maLayerName));
            return;
        }
    }
    catch(const uno::Exception&)
    {
    }
}

void SdXMLShapeContext::SetThumbnail()
{
    if( maThumbnailURL.isEmpty() )
        return;

    try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if( !xPropSet.is() )
            return;

        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName( "ThumbnailGraphic" ) )
        {
            // load the thumbnail graphic and export it to a wmf stream so we can set
            // it at the api

            uno::Reference<graphic::XGraphic> xGraphic = GetImport().loadGraphicByURL(maThumbnailURL);
            xPropSet->setPropertyValue("ThumbnailGraphic", uno::makeAny(xGraphic));
        }
    }
    catch(const uno::Exception&)
    {
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    sal_Int32 nTmp;
    switch (aIter.getToken())
    {
        case XML_ELEMENT(DRAW, XML_ZINDEX):
        case XML_ELEMENT(DRAW_EXT, XML_ZINDEX):
            mnZOrder = aIter.toInt32();
            break;
        case XML_ELEMENT(DRAW, XML_ID):
        case XML_ELEMENT(DRAW_EXT, XML_ID):
            if (!mbHaveXmlId) { maShapeId = aIter.toString(); }
            break;
        case XML_ELEMENT(DRAW, XML_NAME):
        case XML_ELEMENT(DRAW_EXT, XML_NAME):
            maShapeName = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_STYLE_NAME):
        case XML_ELEMENT(DRAW_EXT, XML_STYLE_NAME):
            maDrawStyleName = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_TEXT_STYLE_NAME):
        case XML_ELEMENT(DRAW_EXT, XML_TEXT_STYLE_NAME):
            maTextStyleName = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_LAYER):
        case XML_ELEMENT(DRAW_EXT, XML_LAYER):
            maLayerName = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_TRANSFORM):
        case XML_ELEMENT(DRAW_EXT, XML_TRANSFORM):
            mnTransform.SetString(aIter.toString(), GetImport().GetMM100UnitConverter());
            break;
        case XML_ELEMENT(DRAW, XML_DISPLAY):
        case XML_ELEMENT(DRAW_EXT, XML_DISPLAY):
            mbVisible = IsXMLToken( aIter, XML_ALWAYS ) || IsXMLToken( aIter, XML_SCREEN );
            mbPrintable = IsXMLToken( aIter, XML_ALWAYS ) || IsXMLToken( aIter, XML_PRINTER );
            break;
        case XML_ELEMENT(PRESENTATION, XML_USER_TRANSFORMED):
            mbIsUserTransformed = IsXMLToken( aIter, XML_TRUE );
            break;
        case XML_ELEMENT(PRESENTATION, XML_PLACEHOLDER):
            mbIsPlaceholder = IsXMLToken( aIter, XML_TRUE );
            if( mbIsPlaceholder )
                mbClearDefaultAttributes = false;
            break;
        case XML_ELEMENT(PRESENTATION, XML_CLASS):
            maPresentationClass = aIter.toString();
            break;
        case XML_ELEMENT(PRESENTATION, XML_STYLE_NAME):
            maDrawStyleName = aIter.toString();
            mnStyleFamily = XmlStyleFamily::SD_PRESENTATION_ID;
            break;
        case XML_ELEMENT(SVG, XML_X):
        case XML_ELEMENT(SVG_COMPAT, XML_X):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maPosition.X, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_Y):
        case XML_ELEMENT(SVG_COMPAT, XML_Y):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maPosition.Y, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_WIDTH):
        case XML_ELEMENT(SVG_COMPAT, XML_WIDTH):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maSize.Width, aIter.toView());
            if (maSize.Width > 0)
                maSize.Width = o3tl::saturating_add<sal_Int32>(maSize.Width, 1);
            else if (maSize.Width < 0)
                maSize.Width = o3tl::saturating_add<sal_Int32>(maSize.Width, -1);
            break;
        case XML_ELEMENT(SVG, XML_HEIGHT):
        case XML_ELEMENT(SVG_COMPAT, XML_HEIGHT):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maSize.Height, aIter.toView());
            if (maSize.Height > 0)
                maSize.Height = o3tl::saturating_add<sal_Int32>(maSize.Height, 1);
            else if (maSize.Height < 0)
                maSize.Height = o3tl::saturating_add<sal_Int32>(maSize.Height, -1);
            break;
        case XML_ELEMENT(SVG, XML_TRANSFORM):
        case XML_ELEMENT(SVG_COMPAT, XML_TRANSFORM):
            // because of #85127# take svg:transform into account and handle like
            // draw:transform for compatibility
            mnTransform.SetString(aIter.toString(), GetImport().GetMM100UnitConverter());
            break;
        case XML_ELEMENT(STYLE, XML_REL_WIDTH):
            if (sax::Converter::convertPercent(nTmp, aIter.toView()))
                mnRelWidth = static_cast<sal_Int16>(nTmp);
            break;
        case XML_ELEMENT(STYLE, XML_REL_HEIGHT):
            if (sax::Converter::convertPercent(nTmp, aIter.toView()))
                mnRelHeight = static_cast<sal_Int16>(nTmp);
            break;
        case XML_ELEMENT(NONE, XML_ID):
        case XML_ELEMENT(XML, XML_ID):
            maShapeId = aIter.toString();
            mbHaveXmlId = true;
            break;
        default:
            return false;
    }
    return true;
}

bool SdXMLShapeContext::isPresentationShape() const
{
    if( !maPresentationClass.isEmpty() && const_cast<SdXMLShapeContext*>(this)->GetImport().GetShapeImport()->IsPresentationShapesSupported() )
    {
        if(XmlStyleFamily::SD_PRESENTATION_ID == mnStyleFamily)
        {
            return true;
        }

        if( IsXMLToken( maPresentationClass, XML_HEADER ) || IsXMLToken( maPresentationClass, XML_FOOTER ) ||
            IsXMLToken( maPresentationClass, XML_PAGE_NUMBER ) || IsXMLToken( maPresentationClass, XML_DATE_TIME ) )
        {
            return true;
        }
    }

    return false;
}

SdXMLRectShapeContext::SdXMLRectShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape ),
    mnRadius( 0 )
{
}

SdXMLRectShapeContext::~SdXMLRectShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLRectShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(DRAW, XML_CORNER_RADIUS):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRadius, aIter.toView());
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLRectShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.RectangleShape");
    if(!mxShape.is())
        return;

    // Add, set Style and properties from base shape
    SetStyle();
    SetLayer();

    // set pos, size, shear and rotate
    SetTransformation();

    if(mnRadius)
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            try
            {
                xPropSet->setPropertyValue("CornerRadius", uno::makeAny( mnRadius ) );
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION( "xmloff", "setting corner radius");
            }
        }
    }
    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}


SdXMLLineShapeContext::SdXMLLineShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape ),
    mnX1( 0 ),
    mnY1( 0 ),
    mnX2( 1 ),
    mnY2( 1 )
{
}

SdXMLLineShapeContext::~SdXMLLineShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLLineShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(SVG, XML_X1):
        case XML_ELEMENT(SVG_COMPAT, XML_X1):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnX1, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_Y1):
        case XML_ELEMENT(SVG_COMPAT, XML_Y1):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnY1, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_X2):
        case XML_ELEMENT(SVG_COMPAT, XML_X2):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnX2, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_Y2):
        case XML_ELEMENT(SVG_COMPAT, XML_Y2):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnY2, aIter.toView());
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLLineShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // #85920# use SetTransformation() to handle import of simple lines.
    // This is necessary to take into account all anchor positions and
    // other things. All shape imports use the same import schemata now.
    // create necessary shape (Line Shape)
    AddShape("com.sun.star.drawing.PolyLineShape");

    if(!mxShape.is())
        return;

    // Add, set Style and properties from base shape
    SetStyle();
    SetLayer();

    // get sizes and offsets
    awt::Point aTopLeft(mnX1, mnY1);
    awt::Point aBottomRight(mnX2, mnY2);

    if(mnX1 > mnX2)
    {
        aTopLeft.X = mnX2;
        aBottomRight.X = mnX1;
    }

    if(mnY1 > mnY2)
    {
        aTopLeft.Y = mnY2;
        aBottomRight.Y = mnY1;
    }

    // set local parameters on shape
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        drawing::PointSequenceSequence aPolyPoly(1);
        drawing::PointSequence* pOuterSequence = aPolyPoly.getArray();
        pOuterSequence->realloc(2);
        awt::Point* pInnerSequence = pOuterSequence->getArray();

        *pInnerSequence = awt::Point(o3tl::saturating_sub(mnX1, aTopLeft.X), o3tl::saturating_sub(mnY1, aTopLeft.Y));
        pInnerSequence++;
        *pInnerSequence = awt::Point(o3tl::saturating_sub(mnX2, aTopLeft.X), o3tl::saturating_sub(mnY2, aTopLeft.Y));

        xPropSet->setPropertyValue("Geometry", Any(aPolyPoly));
    }

    // Size is included in point coordinates
    maSize.Width = 1;
    maSize.Height = 1;
    maPosition.X = aTopLeft.X;
    maPosition.Y = aTopLeft.Y;

    // set pos, size, shear and rotate and get copy of matrix
    SetTransformation();

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}


SdXMLEllipseShapeContext::SdXMLEllipseShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape ),
    mnCX( 0 ),
    mnCY( 0 ),
    mnRX( 1 ),
    mnRY( 1 ),
    meKind( drawing::CircleKind_FULL ),
    mnStartAngle( 0 ),
    mnEndAngle( 0 )
{
}

SdXMLEllipseShapeContext::~SdXMLEllipseShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLEllipseShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(SVG, XML_RX):
        case XML_ELEMENT(SVG_COMPAT, XML_RX):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRX, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_RY):
        case XML_ELEMENT(SVG_COMPAT, XML_RY):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRY, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_CX):
        case XML_ELEMENT(SVG_COMPAT, XML_CX):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnCX, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_CY):
        case XML_ELEMENT(SVG_COMPAT, XML_CY):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnCY, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_R):
        case XML_ELEMENT(SVG_COMPAT, XML_R):
            // single radius, it's a circle and both radii are the same
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRX, aIter.toView());
            mnRY = mnRX;
            break;
        case XML_ELEMENT(DRAW, XML_KIND):
            SvXMLUnitConverter::convertEnum( meKind, aIter.toView(), aXML_CircleKind_EnumMap );
            break;
        case XML_ELEMENT(DRAW, XML_START_ANGLE):
        {
            double dStartAngle;
            if (::sax::Converter::convertDouble( dStartAngle, aIter.toView() ))
                mnStartAngle = static_cast<sal_Int32>(dStartAngle * 100.0);
            break;
        }
        case XML_ELEMENT(DRAW, XML_END_ANGLE):
        {
            double dEndAngle;
            if (::sax::Converter::convertDouble( dEndAngle, aIter.toView() ))
                mnEndAngle = static_cast<sal_Int32>(dEndAngle * 100.0);
            break;
        }
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLEllipseShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.EllipseShape");
    if(!mxShape.is())
        return;

    // Add, set Style and properties from base shape
    SetStyle();
    SetLayer();

    if(mnCX != 0 || mnCY != 0 || mnRX != 1 || mnRY != 1)
    {
        // #i121972# center/radius is used, put to pos and size
        maSize.Width = 2 * mnRX;
        maSize.Height = 2 * mnRY;
        maPosition.X = mnCX - mnRX;
        maPosition.Y = mnCY - mnRY;
    }

    // set pos, size, shear and rotate
    SetTransformation();

    if( meKind != drawing::CircleKind_FULL )
    {
        uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            xPropSet->setPropertyValue("CircleKind", Any( meKind) );
            xPropSet->setPropertyValue("CircleStartAngle", Any(mnStartAngle) );
            xPropSet->setPropertyValue("CircleEndAngle", Any(mnEndAngle) );
        }
    }

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}


SdXMLPolygonShapeContext::SdXMLPolygonShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes, bool bClosed, bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape ),
    mbClosed( bClosed )
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLPolygonShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(SVG, XML_VIEWBOX):
        case XML_ELEMENT(SVG_COMPAT, XML_VIEWBOX):
            maViewBox = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_POINTS):
            maPoints = aIter.toString();
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter);
    }
    return true;
}

SdXMLPolygonShapeContext::~SdXMLPolygonShapeContext()
{
}

void SdXMLPolygonShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // Add, set Style and properties from base shape
    if(mbClosed)
        AddShape("com.sun.star.drawing.PolyPolygonShape");
    else
        AddShape("com.sun.star.drawing.PolyLineShape");

    if( !mxShape.is() )
        return;

    SetStyle();
    SetLayer();

    // set local parameters on shape
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        // set polygon
        if(!maPoints.isEmpty() && !maViewBox.isEmpty())
        {
            const SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
            basegfx::B2DVector aSize(aViewBox.GetWidth(), aViewBox.GetHeight());

            // Is this correct? It overrides ViewBox stuff; OTOH it makes no
            // sense to have the geometry content size different from object size
            if(maSize.Width != 0 && maSize.Height != 0)
            {
                aSize = basegfx::B2DVector(maSize.Width, maSize.Height);
            }

            basegfx::B2DPolygon aPolygon;

            if(basegfx::utils::importFromSvgPoints(aPolygon, maPoints))
            {
                if(aPolygon.count())
                {
                    const basegfx::B2DRange aSourceRange(
                        aViewBox.GetX(), aViewBox.GetY(),
                        aViewBox.GetX() + aViewBox.GetWidth(), aViewBox.GetY() + aViewBox.GetHeight());
                    const basegfx::B2DRange aTargetRange(
                        aViewBox.GetX(), aViewBox.GetY(),
                        aViewBox.GetX() + aSize.getX(), aViewBox.GetY() + aSize.getY());

                    if(!aSourceRange.equal(aTargetRange))
                    {
                        aPolygon.transform(
                            basegfx::utils::createSourceRangeTargetRangeTransform(
                                aSourceRange,
                                aTargetRange));
                    }

                    css::drawing::PointSequenceSequence aPointSequenceSequence;
                    basegfx::utils::B2DPolyPolygonToUnoPointSequenceSequence(basegfx::B2DPolyPolygon(aPolygon), aPointSequenceSequence);
                    xPropSet->setPropertyValue("Geometry", Any(aPointSequenceSequence));
                    // Size is now contained in the point coordinates, adapt maSize for
                    // to use the correct transformation matrix in SetTransformation()
                    maSize.Width = 1;
                    maSize.Height = 1;
                }
            }
        }
    }

    // set pos, size, shear and rotate and get copy of matrix
    SetTransformation();

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}


SdXMLPathShapeContext::SdXMLPathShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLPathShapeContext::~SdXMLPathShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLPathShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(SVG, XML_VIEWBOX):
        case XML_ELEMENT(SVG_COMPAT, XML_VIEWBOX):
            maViewBox = aIter.toString();
            break;
        case XML_ELEMENT(SVG, XML_D):
        case XML_ELEMENT(SVG_COMPAT, XML_D):
            maD = aIter.toString();
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLPathShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create polygon shape
    if(maD.isEmpty())
        return;

    const SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
    basegfx::B2DVector aSize(aViewBox.GetWidth(), aViewBox.GetHeight());

    // Is this correct? It overrides ViewBox stuff; OTOH it makes no
    // sense to have the geometry content size different from object size
    if(maSize.Width != 0 && maSize.Height != 0)
    {
        aSize = basegfx::B2DVector(maSize.Width, maSize.Height);
    }

    basegfx::B2DPolyPolygon aPolyPolygon;

    if(!basegfx::utils::importFromSvgD(aPolyPolygon, maD, GetImport().needFixPositionAfterZ(), nullptr))
        return;

    if(!aPolyPolygon.count())
        return;

    const basegfx::B2DRange aSourceRange(
        aViewBox.GetX(), aViewBox.GetY(),
        aViewBox.GetX() + aViewBox.GetWidth(), aViewBox.GetY() + aViewBox.GetHeight());
    const basegfx::B2DRange aTargetRange(
        aViewBox.GetX(), aViewBox.GetY(),
        aViewBox.GetX() + aSize.getX(), aViewBox.GetY() + aSize.getY());

    if(!aSourceRange.equal(aTargetRange))
    {
        aPolyPolygon.transform(
            basegfx::utils::createSourceRangeTargetRangeTransform(
                aSourceRange,
                aTargetRange));
    }

    // create shape
    OUString service;

    if(aPolyPolygon.areControlPointsUsed())
    {
        if(aPolyPolygon.isClosed())
        {
            service = "com.sun.star.drawing.ClosedBezierShape";
        }
        else
        {
            service = "com.sun.star.drawing.OpenBezierShape";
        }
    }
    else
    {
        if(aPolyPolygon.isClosed())
        {
            service = "com.sun.star.drawing.PolyPolygonShape";
        }
        else
        {
            service = "com.sun.star.drawing.PolyLineShape";
        }
    }

    // Add, set Style and properties from base shape
    AddShape(service);

    // #89344# test for mxShape.is() and not for mxShapes.is() to support
    // shape import helper classes WITHOUT XShapes (member mxShapes). This
    // is used by the writer.
    if( !mxShape.is() )
        return;

    SetStyle();
    SetLayer();

    // set local parameters on shape
    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);

    if(xPropSet.is())
    {
        uno::Any aAny;

        // set polygon data
        if(aPolyPolygon.areControlPointsUsed())
        {
            drawing::PolyPolygonBezierCoords aSourcePolyPolygon;

            basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords(
                aPolyPolygon,
                aSourcePolyPolygon);
            aAny <<= aSourcePolyPolygon;
        }
        else
        {
            drawing::PointSequenceSequence aSourcePolyPolygon;

            basegfx::utils::B2DPolyPolygonToUnoPointSequenceSequence(
                aPolyPolygon,
                aSourcePolyPolygon);
            aAny <<= aSourcePolyPolygon;
        }

        xPropSet->setPropertyValue("Geometry", aAny);
        // Size is now contained in the point coordinates, adapt maSize for
        // to use the correct transformation matrix in SetTransformation()
        maSize.Width = 1;
        maSize.Height = 1;
    }

    // set pos, size, shear and rotate
    SetTransformation();

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}


SdXMLTextBoxShapeContext::SdXMLTextBoxShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, false/*bTemporaryShape*/ ),
    mnRadius(0),
    maChainNextName("")
{
}

SdXMLTextBoxShapeContext::~SdXMLTextBoxShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLTextBoxShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(DRAW, XML_CORNER_RADIUS):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRadius, aIter.toView());
            break;
        case XML_ELEMENT(DRAW, XML_CHAIN_NEXT_NAME):
            maChainNextName = aIter.toString();
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLTextBoxShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create textbox shape
    bool bIsPresShape = false;
    bool bClearText = false;

    OUString service;

    if( isPresentationShape() )
    {
        // check if the current document supports presentation shapes
        if( GetImport().GetShapeImport()->IsPresentationShapesSupported() )
        {
            if( IsXMLToken( maPresentationClass, XML_SUBTITLE ))
            {
                // XmlShapeTypePresSubtitleShape
                service = "com.sun.star.presentation.SubtitleShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_OUTLINE ) )
            {
                // XmlShapeTypePresOutlinerShape
                service = "com.sun.star.presentation.OutlinerShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_NOTES ) )
            {
                // XmlShapeTypePresNotesShape
                service = "com.sun.star.presentation.NotesShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_HEADER ) )
            {
                // XmlShapeTypePresHeaderShape
                service = "com.sun.star.presentation.HeaderShape";
                bClearText = true;
            }
            else if( IsXMLToken( maPresentationClass, XML_FOOTER ) )
            {
                // XmlShapeTypePresFooterShape
                service = "com.sun.star.presentation.FooterShape";
                bClearText = true;
            }
            else if( IsXMLToken( maPresentationClass, XML_PAGE_NUMBER ) )
            {
                // XmlShapeTypePresSlideNumberShape
                service = "com.sun.star.presentation.SlideNumberShape";
                bClearText = true;
            }
            else if( IsXMLToken( maPresentationClass, XML_DATE_TIME ) )
            {
                // XmlShapeTypePresDateTimeShape
                service = "com.sun.star.presentation.DateTimeShape";
                bClearText = true;
            }
            else //  IsXMLToken( maPresentationClass, XML_TITLE ) )
            {
                // XmlShapeTypePresTitleTextShape
                service = "com.sun.star.presentation.TitleTextShape";
            }
            bIsPresShape = true;
        }
    }

    if( service.isEmpty() )
    {
        // normal text shape
        service = "com.sun.star.drawing.TextShape";
    }

    // Add, set Style and properties from base shape
    AddShape(service);

    if( !mxShape.is() )
        return;

    SetStyle();
    SetLayer();

    if(bIsPresShape)
    {
        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
        if(xProps.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
            if( xPropsInfo.is() )
            {
                if( !mbIsPlaceholder && xPropsInfo->hasPropertyByName("IsEmptyPresentationObject"))
                    xProps->setPropertyValue("IsEmptyPresentationObject", css::uno::Any(false) );

                if( mbIsUserTransformed && xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                    xProps->setPropertyValue("IsPlaceholderDependent", css::uno::Any(false) );
            }
        }
    }

    if( bClearText )
    {
        uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
        xText->setString( "" );
    }

    // set parameters on shape
//A AW->CL: Eventually You need to strip scale and translate from the transformation
//A to reach the same goal again.
//A     if(!bIsPresShape || mbIsUserTransformed)
//A     {
//A         // set pos and size on shape, this should remove binding
//A         // to pres object on masterpage
//A         SetSizeAndPosition();
//A     }

    // set pos, size, shear and rotate
    SetTransformation();

    if(mnRadius)
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            try
            {
                xPropSet->setPropertyValue("CornerRadius", uno::makeAny( mnRadius ) );
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION( "xmloff", "setting corner radius");
            }
        }
    }

    if(!maChainNextName.isEmpty())
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            try
            {
                xPropSet->setPropertyValue("TextChainNextName",
                                           uno::makeAny( maChainNextName ) );
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION( "xmloff", "setting name of next chain link");
            }
        }
    }

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}


SdXMLControlShapeContext::SdXMLControlShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLControlShapeContext::~SdXMLControlShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLControlShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(DRAW, XML_CONTROL):
            maFormId = aIter.toString();
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLControlShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create Control shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.ControlShape");
    if( !mxShape.is() )
        return;

    SAL_WARN_IF( !!maFormId.isEmpty(), "xmloff", "draw:control without a form:id attribute!" );
    if( !maFormId.isEmpty() )
    {
        if( GetImport().IsFormsSupported() )
        {
            uno::Reference< awt::XControlModel > xControlModel( GetImport().GetFormImport()->lookupControl( maFormId ), uno::UNO_QUERY );
            if( xControlModel.is() )
            {
                uno::Reference< drawing::XControlShape > xControl( mxShape, uno::UNO_QUERY );
                if( xControl.is() )
                    xControl->setControl(  xControlModel );

            }
        }
    }

    SetStyle();
    SetLayer();

    // set pos, size, shear and rotate
    SetTransformation();

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}


SdXMLConnectorShapeContext::SdXMLConnectorShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape ),
    maStart(0,0),
    maEnd(1,1),
    mnType( drawing::ConnectorType_STANDARD ),
    mnStartGlueId(-1),
    mnEndGlueId(-1),
    mnDelta1(0),
    mnDelta2(0),
    mnDelta3(0)
{
}

SdXMLConnectorShapeContext::~SdXMLConnectorShapeContext()
{
}

bool SvXMLImport::needFixPositionAfterZ() const
{
    bool bWrongPositionAfterZ( false );
    sal_Int32 nUPD( 0 );
    sal_Int32 nBuildId( 0 );
    if ( getBuildIds( nUPD, nBuildId ) && // test OOo and old versions of LibO and AOO
       ( ( ( nUPD == 641 ) || ( nUPD == 645 ) || ( nUPD == 680 ) || ( nUPD == 300 ) ||
           ( nUPD == 310 ) || ( nUPD == 320 ) || ( nUPD == 330 ) || ( nUPD == 340 ) ||
           ( nUPD == 350 && nBuildId < 202 ) )
       || (getGeneratorVersion() == SvXMLImport::AOO_40x))) // test if AOO 4.0.x
           // apparently bug was fixed in AOO by i#123433 f15874d8f976f3874bdbcb53429eeefa65c28841
    {
        bWrongPositionAfterZ = true;
    }
    return bWrongPositionAfterZ;
}


// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLConnectorShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() )
    {
        case XML_ELEMENT(DRAW, XML_START_SHAPE):
            maStartShapeId = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_START_GLUE_POINT):
            mnStartGlueId = aIter.toInt32();
            break;
        case XML_ELEMENT(DRAW, XML_END_SHAPE):
            maEndShapeId = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_END_GLUE_POINT):
            mnEndGlueId = aIter.toInt32();
            break;
        case XML_ELEMENT(DRAW, XML_LINE_SKEW):
        {
            OUString sValue = aIter.toString();
            SvXMLTokenEnumerator aTokenEnum( sValue );
            std::u16string_view aToken;
            if( aTokenEnum.getNextToken( aToken ) )
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        mnDelta1, aToken);
                if( aTokenEnum.getNextToken( aToken ) )
                {
                    GetImport().GetMM100UnitConverter().convertMeasureToCore(
                            mnDelta2, aToken);
                    if( aTokenEnum.getNextToken( aToken ) )
                    {
                        GetImport().GetMM100UnitConverter().convertMeasureToCore(
                                mnDelta3, aToken);
                    }
                }
            }
            break;
        }
        case XML_ELEMENT(DRAW, XML_TYPE):
        {
            (void)SvXMLUnitConverter::convertEnum( mnType, aIter.toView(), aXML_ConnectionKind_EnumMap );
            break;
        }
        // #121965# draw:transform may be used in ODF1.2, e.g. exports from MS seem to use these
        case XML_ELEMENT(DRAW, XML_TRANSFORM):
            mnTransform.SetString(aIter.toString(), GetImport().GetMM100UnitConverter());
            break;

        case XML_ELEMENT(SVG, XML_X1):
        case XML_ELEMENT(SVG_COMPAT, XML_X1):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maStart.X, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_Y1):
        case XML_ELEMENT(SVG_COMPAT, XML_Y1):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maStart.Y, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_X2):
        case XML_ELEMENT(SVG_COMPAT, XML_X2):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maEnd.X, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_Y2):
        case XML_ELEMENT(SVG_COMPAT, XML_Y2):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maEnd.Y, aIter.toView());
            break;
        case XML_ELEMENT(SVG, XML_D):
        case XML_ELEMENT(SVG_COMPAT, XML_D):
        {
            basegfx::B2DPolyPolygon aPolyPolygon;

            if(basegfx::utils::importFromSvgD(aPolyPolygon, aIter.toString(), GetImport().needFixPositionAfterZ(), nullptr))
            {
                if(aPolyPolygon.count())
                {
                    drawing::PolyPolygonBezierCoords aSourcePolyPolygon;

                    basegfx::utils::B2DPolyPolygonToUnoPolyPolygonBezierCoords(
                        aPolyPolygon,
                        aSourcePolyPolygon);
                    maPath <<= aSourcePolyPolygon;
                }
            }
            break;
        }
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLConnectorShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // For security reasons, do not add empty connectors. There may have been an error in EA2
    // that created empty, far set off connectors (e.g. 63 meters below top of document). This
    // is not guaranteed, but it's definitely safe to not add empty connectors.
    bool bDoAdd(true);

    if(    maStartShapeId.isEmpty()
        && maEndShapeId.isEmpty()
        && maStart.X == maEnd.X
        && maStart.Y == maEnd.Y
        && 0 == mnDelta1
        && 0 == mnDelta2
        && 0 == mnDelta3
        )
    {
        bDoAdd = false;
    }

    if(!bDoAdd)
        return;

    // create Connector shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.ConnectorShape");
    if(!mxShape.is())
        return;

    // #121965# if draw:transform is used, apply directly to the start
    // and end positions before using these
    if(mnTransform.NeedsAction())
    {
        // transformation is used, apply to object.
        ::basegfx::B2DHomMatrix aMat;
        mnTransform.GetFullTransform(aMat);

        if(!aMat.isIdentity())
        {
            basegfx::B2DPoint aStart(maStart.X, maStart.Y);
            basegfx::B2DPoint aEnd(maEnd.X, maEnd.Y);

            aStart = aMat * aStart;
            aEnd = aMat * aEnd;

            maStart.X = basegfx::fround(aStart.getX());
            maStart.Y = basegfx::fround(aStart.getY());
            maEnd.X = basegfx::fround(aEnd.getX());
            maEnd.Y = basegfx::fround(aEnd.getY());
        }
    }

    // add connection ids
    if( !maStartShapeId.isEmpty() )
        GetImport().GetShapeImport()->addShapeConnection( mxShape, true, maStartShapeId, mnStartGlueId );
    if( !maEndShapeId.isEmpty() )
        GetImport().GetShapeImport()->addShapeConnection( mxShape, false, maEndShapeId, mnEndGlueId );

    uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
    if( xProps.is() )
    {
        xProps->setPropertyValue("StartPosition", Any(maStart));
        xProps->setPropertyValue("EndPosition", Any(maEnd) );
        xProps->setPropertyValue("EdgeKind", Any(mnType) );
        xProps->setPropertyValue("EdgeLine1Delta", Any(mnDelta1) );
        xProps->setPropertyValue("EdgeLine2Delta", Any(mnDelta2) );
        xProps->setPropertyValue("EdgeLine3Delta", Any(mnDelta3) );
    }
    SetStyle();
    SetLayer();

    if ( maPath.hasValue() )
    {
        // #i115492#
        // Ignore svg:d attribute for text documents created by OpenOffice.org
        // versions before OOo 3.3, because these OOo versions are storing
        // svg:d values not using the correct unit.
        bool bApplySVGD( true );
        if ( uno::Reference< text::XTextDocument >(GetImport().GetModel(), uno::UNO_QUERY).is() )
        {
            sal_Int32 nUPD( 0 );
            sal_Int32 nBuild( 0 );
            const bool bBuildIdFound = GetImport().getBuildIds( nUPD, nBuild );
            if ( GetImport().IsTextDocInOOoFileFormat() ||
                 ( bBuildIdFound &&
                   ( ( nUPD == 641 ) || ( nUPD == 645 ) ||  // prior OOo 2.0
                     ( nUPD == 680 ) ||                     // OOo 2.x
                     ( nUPD == 300 ) ||                     // OOo 3.0 - OOo 3.0.1
                     ( nUPD == 310 ) ||                     // OOo 3.1 - OOo 3.1.1
                     ( nUPD == 320 ) ) ) )                  // OOo 3.2 - OOo 3.2.1
            {
                bApplySVGD = false;
            }
        }

        if ( bApplySVGD )
        {
            // tdf#83360 use path data only when redundant data of start and end point coordinates of
            // path start/end and connector start/end is equal. This is to avoid using erraneous
            // or inconsistent path data at import of foreign formats. Office itself always
            // writes out a consistent data set. Not using it when there is inconsistency
            // is okay since the path data is redundant, buffered data just to avoid recalculation
            // of the connector's layout at load time, no real information would be lost.
            // A 'connected' end has prio to direct coordinate data in Start/EndPosition
            // to the path data (which should have the start/end redundant in the path)
            const drawing::PolyPolygonBezierCoords* pSource = static_cast< const drawing::PolyPolygonBezierCoords* >(maPath.getValue());
            const sal_uInt32 nSequenceCount(pSource->Coordinates.getLength());
            bool bStartEqual(false);
            bool bEndEqual(false);

            if(nSequenceCount)
            {
                const drawing::PointSequence& rStartSeq = pSource->Coordinates[0];
                const sal_uInt32 nStartCount = rStartSeq.getLength();

                if(nStartCount)
                {
                    const awt::Point& rStartPoint = rStartSeq.getConstArray()[0];

                    if(rStartPoint.X == maStart.X && rStartPoint.Y == maStart.Y)
                    {
                        bStartEqual = true;
                    }
                }

                const drawing::PointSequence& rEndSeq = pSource->Coordinates[nSequenceCount - 1];
                const sal_uInt32 nEndCount = rEndSeq.getLength();

                if(nEndCount)
                {
                    const awt::Point& rEndPoint = rEndSeq.getConstArray()[nEndCount - 1];

                    if(rEndPoint.X == maEnd.X && rEndPoint.Y == maEnd.Y)
                    {
                        bEndEqual = true;
                    }
                }
            }

            if(!bStartEqual || !bEndEqual)
            {
                bApplySVGD = false;
            }
        }

        if ( bApplySVGD )
        {
            assert(maPath.getValueType() == cppu::UnoType<drawing::PolyPolygonBezierCoords>::get());
            xProps->setPropertyValue("PolyPolygonBezier", maPath);
        }
    }

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}


SdXMLMeasureShapeContext::SdXMLMeasureShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape ),
    maStart(0,0),
    maEnd(1,1)
{
}

SdXMLMeasureShapeContext::~SdXMLMeasureShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLMeasureShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() )
    {
        case XML_ELEMENT(SVG, XML_X1):
        case XML_ELEMENT(SVG_COMPAT, XML_X1):
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maStart.X, aIter.toView());
            break;
        }
        case XML_ELEMENT(SVG, XML_Y1):
        case XML_ELEMENT(SVG_COMPAT, XML_Y1):
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maStart.Y, aIter.toView());
            break;
        }
        case XML_ELEMENT(SVG, XML_X2):
        case XML_ELEMENT(SVG_COMPAT, XML_X2):
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maEnd.X, aIter.toView());
            break;
        }
        case XML_ELEMENT(SVG, XML_Y2):
        case XML_ELEMENT(SVG_COMPAT, XML_Y2):
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maEnd.Y, aIter.toView());
            break;
        }
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLMeasureShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create Measure shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.MeasureShape");
    if(!mxShape.is())
        return;

    SetStyle();
    SetLayer();

    uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
    if( xProps.is() )
    {
        xProps->setPropertyValue("StartPosition", Any(maStart));
        xProps->setPropertyValue("EndPosition", Any(maEnd) );
    }

    // delete pre created fields
    uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
    if( xText.is() )
    {
        xText->setString( " " );
    }

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}

void SdXMLMeasureShapeContext::endFastElement(sal_Int32 nElement)
{
    do
    {
        // delete pre created fields
        uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
        if( !xText.is() )
            break;

        uno::Reference< text::XTextCursor > xCursor( xText->createTextCursor() );
        if( !xCursor.is() )
            break;

        xCursor->collapseToStart();
        xCursor->goRight( 1, true );
        xCursor->setString( "" );
    }
    while(false);

    SdXMLShapeContext::endFastElement(nElement);
}


SdXMLPageShapeContext::SdXMLPageShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape ), mnPageNumber(0)
{
    mbClearDefaultAttributes = false;
}

SdXMLPageShapeContext::~SdXMLPageShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLPageShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    if( aIter.getToken() == XML_ELEMENT(DRAW, XML_PAGE_NUMBER) )
        mnPageNumber = aIter.toInt32();
    else
        return SdXMLShapeContext::processAttribute( aIter );
    return true;
}

void SdXMLPageShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create Page shape
    // add, set style and properties from base shape

    // #86163# take into account which type of PageShape needs to
    // be constructed. It's a pres shape if presentation:XML_CLASS == XML_PAGE.
    bool bIsPresentation = !maPresentationClass.isEmpty() &&
           GetImport().GetShapeImport()->IsPresentationShapesSupported();

    uno::Reference< lang::XServiceInfo > xInfo( mxShapes, uno::UNO_QUERY );
    const bool bIsOnHandoutPage = xInfo.is() && xInfo->supportsService("com.sun.star.presentation.HandoutMasterPage");

    if( bIsOnHandoutPage )
    {
        AddShape("com.sun.star.presentation.HandoutShape");
    }
    else
    {
        if(bIsPresentation && !IsXMLToken( maPresentationClass, XML_PAGE ) )
        {
            bIsPresentation = false;
        }

        if(bIsPresentation)
        {
            AddShape("com.sun.star.presentation.PageShape");
        }
        else
        {
            AddShape("com.sun.star.drawing.PageShape");
        }
    }

    if(!mxShape.is())
        return;

    SetStyle();
    SetLayer();

    // set pos, size, shear and rotate
    SetTransformation();

    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
    if(xPropSet.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
        static const OUStringLiteral aPageNumberStr(u"PageNumber");
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(aPageNumberStr))
            xPropSet->setPropertyValue(aPageNumberStr, uno::makeAny( mnPageNumber ));
    }

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}


SdXMLCaptionShapeContext::SdXMLCaptionShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape ),
    // #86616# for correct edge rounding import mnRadius needs to be initialized
    mnRadius( 0 )
{
}

SdXMLCaptionShapeContext::~SdXMLCaptionShapeContext()
{
}

void SdXMLCaptionShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create Caption shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.CaptionShape");
    if( !mxShape.is() )
        return;

    SetStyle();
    SetLayer();

    uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );

    // SJ: If AutoGrowWidthItem is set, SetTransformation will lead to the wrong SnapRect
    // because NbcAdjustTextFrameWidthAndHeight() is called (text is set later and center alignment
    // is the default setting, so the top left reference point that is used by the caption point is
    // no longer correct) There are two ways to solve this problem, temporarily disabling the
    // autogrowwidth as we are doing here or to apply the CaptionPoint after setting text
    bool bIsAutoGrowWidth = false;
    if ( xProps.is() )
    {
        uno::Any aAny( xProps->getPropertyValue("TextAutoGrowWidth") );
        aAny >>= bIsAutoGrowWidth;

        if ( bIsAutoGrowWidth )
            xProps->setPropertyValue("TextAutoGrowWidth", uno::makeAny( false ) );
    }

    // set pos, size, shear and rotate
    SetTransformation();
    if( xProps.is() )
        xProps->setPropertyValue("CaptionPoint", uno::makeAny( maCaptionPoint ) );

    if ( bIsAutoGrowWidth )
        xProps->setPropertyValue("TextAutoGrowWidth", uno::makeAny( true ) );

    if(mnRadius)
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            try
            {
                xPropSet->setPropertyValue("CornerRadius", uno::makeAny( mnRadius ) );
            }
            catch(const uno::Exception&)
            {
                DBG_UNHANDLED_EXCEPTION( "xmloff", "setting corner radius");
            }
        }
    }

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLCaptionShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch (aIter.getToken())
    {
        case XML_ELEMENT(DRAW, XML_CAPTION_POINT_X):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maCaptionPoint.X, aIter.toView());
            break;
        case XML_ELEMENT(DRAW, XML_CAPTION_POINT_Y):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maCaptionPoint.Y, aIter.toView());
            break;
        case XML_ELEMENT(DRAW, XML_CORNER_RADIUS):
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRadius, aIter.toView());
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}


SdXMLGraphicObjectShapeContext::SdXMLGraphicObjectShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, false/*bTemporaryShape*/ )
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLGraphicObjectShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    if( aIter.getToken() == XML_ELEMENT(XLINK, XML_HREF) )
        maURL = aIter.toString();
    else
        return SdXMLShapeContext::processAttribute( aIter );
    return true;
}

void SdXMLGraphicObjectShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create graphic object shape
    OUString service;

    if( IsXMLToken( maPresentationClass, XML_GRAPHIC ) && GetImport().GetShapeImport()->IsPresentationShapesSupported() )
    {
        service = "com.sun.star.presentation.GraphicObjectShape";
    }
    else
    {
        service = "com.sun.star.drawing.GraphicObjectShape";
    }

    AddShape(service);

    if(!mxShape.is())
        return;

    SetStyle();
    SetLayer();

    uno::Reference< beans::XPropertySet > xPropset(mxShape, uno::UNO_QUERY);
    if(xPropset.is())
    {
        // since OOo 1.x had no line or fill style for graphics, but may create
        // documents with them, we have to override them here
        sal_Int32 nUPD, nBuildId;
        if( GetImport().getBuildIds( nUPD, nBuildId ) && (nUPD == 645) ) try
        {
            xPropset->setPropertyValue("FillStyle", Any( FillStyle_NONE ) );
            xPropset->setPropertyValue("LineStyle", Any( LineStyle_NONE ) );
        }
        catch(const Exception&)
        {
        }

        uno::Reference< beans::XPropertySetInfo > xPropsInfo( xPropset->getPropertySetInfo() );
        if( xPropsInfo.is() && xPropsInfo->hasPropertyByName("IsEmptyPresentationObject"))
            xPropset->setPropertyValue("IsEmptyPresentationObject", css::uno::makeAny( mbIsPlaceholder ) );

        if( !mbIsPlaceholder )
        {
            if( !maURL.isEmpty() )
            {
                uno::Reference<graphic::XGraphic> xGraphic = GetImport().loadGraphicByURL(maURL);
                if (xGraphic.is())
                {
                    xPropset->setPropertyValue("Graphic", uno::makeAny(xGraphic));
                }
            }
        }
    }

    if(mbIsUserTransformed)
    {
        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
        if(xProps.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
            if( xPropsInfo.is() )
            {
                if( xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                    xProps->setPropertyValue("IsPlaceholderDependent", css::uno::Any(false) );
            }
        }
    }

    // set pos, size, shear and rotate
    SetTransformation();

    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}

void SdXMLGraphicObjectShapeContext::endFastElement(sal_Int32 nElement)
{
    if (mxBase64Stream.is())
    {
        uno::Reference<graphic::XGraphic> xGraphic(GetImport().loadGraphicFromBase64(mxBase64Stream));
        if (xGraphic.is())
        {
            uno::Reference<beans::XPropertySet> xProperties(mxShape, uno::UNO_QUERY);
            if (xProperties.is())
            {
                xProperties->setPropertyValue("Graphic", uno::makeAny(xGraphic));
            }
        }
    }

    SdXMLShapeContext::endFastElement(nElement);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLGraphicObjectShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;

    if( nElement == XML_ELEMENT(OFFICE, XML_BINARY_DATA) )
    {
        if( maURL.isEmpty() && !mxBase64Stream.is() )
        {
            mxBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( mxBase64Stream.is() )
                xContext = new XMLBase64ImportContext( GetImport(),
                                                    mxBase64Stream );
        }
    }

    // delegate to parent class if no context could be created
    if (!xContext)
        xContext = SdXMLShapeContext::createFastChildContext(nElement,
                                                         xAttrList);

    if (!xContext)
        XMLOFF_WARN_UNKNOWN_ELEMENT("xmloff", nElement);

    return xContext;
}

SdXMLGraphicObjectShapeContext::~SdXMLGraphicObjectShapeContext()
{

}


SdXMLChartShapeContext::SdXMLChartShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape )
{
}

void SdXMLChartShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    const bool bIsPresentation = isPresentationShape();

    AddShape(
        bIsPresentation
        ? OUString("com.sun.star.presentation.ChartShape")
        : OUString("com.sun.star.drawing.OLE2Shape"));

    if(!mxShape.is())
        return;

    SetStyle();
    SetLayer();

    if( !mbIsPlaceholder )
    {
        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
        if(xProps.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
            if( xPropsInfo.is() && xPropsInfo->hasPropertyByName("IsEmptyPresentationObject"))
                xProps->setPropertyValue("IsEmptyPresentationObject", css::uno::Any(false) );

            uno::Any aAny;

            xProps->setPropertyValue("CLSID", Any(OUString("12DCAE26-281F-416F-a234-c3086127382e")) );

            aAny = xProps->getPropertyValue("Model");
            uno::Reference< frame::XModel > xChartModel;
            if( aAny >>= xChartModel )
            {
#ifndef ENABLE_WASM_STRIP_CHART
                // WASM_CHART change
                // TODO: Maybe use SdXMLGraphicObjectShapeContext completely instead
                // or try to create as mbIsPlaceholder object adding a Chart visuailzation
                // that should be available somehow
                mxChartContext.set( GetImport().GetChartImport()->CreateChartContext( GetImport(), xChartModel ) );
#endif
            }
        }
    }

    if(mbIsUserTransformed)
    {
        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
        if(xProps.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
            if( xPropsInfo.is() )
            {
                if( xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                    xProps->setPropertyValue("IsPlaceholderDependent", css::uno::Any(false) );
            }
        }
    }

    // set pos, size, shear and rotate
    SetTransformation();

    SdXMLShapeContext::startFastElement(nElement, xAttrList);

    if( mxChartContext.is() )
        mxChartContext->startFastElement( nElement, xAttrList );
}

void SdXMLChartShapeContext::endFastElement(sal_Int32 nElement)
{
    if( mxChartContext.is() )
        mxChartContext->endFastElement(nElement);

    SdXMLShapeContext::endFastElement(nElement);
}

void SdXMLChartShapeContext::characters( const OUString& rChars )
{
    if( mxChartContext.is() )
        mxChartContext->characters( rChars );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLChartShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( mxChartContext.is() )
        return mxChartContext->createFastChildContext( nElement, xAttrList );

    return nullptr;
}


SdXMLObjectShapeContext::SdXMLObjectShapeContext( SvXMLImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes)
: SdXMLShapeContext( rImport, xAttrList, rShapes, false/*bTemporaryShape*/ )
{
}

SdXMLObjectShapeContext::~SdXMLObjectShapeContext()
{
}

void SdXMLObjectShapeContext::startFastElement (sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/)
{
    // #96717# in theorie, if we don't have a URL we shouldn't even
    // export this OLE shape. But practically it's too risky right now
    // to change this so we better dispose this on load
    //if( !mbIsPlaceholder && ImpIsEmptyURL(maHref) )
    //  return;

    // #100592# this BugFix prevents that a shape is created. CL
    // is thinking about an alternative.
    // #i13140# Check for more than empty string in maHref, there are
    // other possibilities that maHref results in empty container
    // storage names
    if( !(GetImport().getImportFlags() & SvXMLImportFlags::EMBEDDED) && !mbIsPlaceholder && ImpIsEmptyURL(maHref) )
        return;

    OUString service("com.sun.star.drawing.OLE2Shape");

    bool bIsPresShape = !maPresentationClass.isEmpty() && GetImport().GetShapeImport()->IsPresentationShapesSupported();

    if( bIsPresShape )
    {
        if( IsXMLToken( maPresentationClass, XML_CHART ) )
        {
            service = "com.sun.star.presentation.ChartShape";
        }
        else if( IsXMLToken( maPresentationClass, XML_TABLE ) )
        {
            service = "com.sun.star.presentation.CalcShape";
        }
        else if( IsXMLToken( maPresentationClass, XML_OBJECT ) )
        {
            service = "com.sun.star.presentation.OLE2Shape";
        }
    }

    AddShape(service);

    if( !mxShape.is() )
        return;

    SetLayer();

    if(bIsPresShape)
    {
        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
        if(xProps.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
            if( xPropsInfo.is() )
            {
                if( !mbIsPlaceholder && xPropsInfo->hasPropertyByName("IsEmptyPresentationObject"))
                    xProps->setPropertyValue("IsEmptyPresentationObject", css::uno::Any(false) );

                if( mbIsUserTransformed && xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                    xProps->setPropertyValue("IsPlaceholderDependent", css::uno::Any(false) );
            }
        }
    }

    if( !mbIsPlaceholder && !maHref.isEmpty() )
    {
        uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );

        if( xProps.is() )
        {
            OUString aPersistName = GetImport().ResolveEmbeddedObjectURL( maHref, maCLSID );

            if ( GetImport().IsPackageURL( maHref ) )
            {
                static const OUStringLiteral  sURL( u"vnd.sun.star.EmbeddedObject:" );

                if ( aPersistName.startsWith( sURL ) )
                    aPersistName = aPersistName.copy( sURL.getLength() );

                xProps->setPropertyValue("PersistName",
                                          uno::makeAny( aPersistName ) );
            }
            else
            {
                // this is OOo link object
                xProps->setPropertyValue("LinkURL",
                                          uno::makeAny( aPersistName ) );
            }
        }
    }

    // set pos, size, shear and rotate
    SetTransformation();

    SetStyle();

    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

void SdXMLObjectShapeContext::endFastElement(sal_Int32 nElement)
{
    if (GetImport().isGeneratorVersionOlderThan(
                SvXMLImport::OOo_34x, SvXMLImport::LO_41x)) // < LO 4.0
    {
        // #i118485#
        // If it's an old file from us written before OOo3.4, we need to correct
        // FillStyle and LineStyle for OLE2 objects. The error was that the old paint
        // implementations just ignored added fill/linestyles completely, thus
        // those objects need to be corrected to not show blue and hairline which
        // always was the default, but would be shown now
        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);

        if( xProps.is() )
        {
            xProps->setPropertyValue("FillStyle", uno::makeAny(drawing::FillStyle_NONE));
            xProps->setPropertyValue("LineStyle", uno::makeAny(drawing::LineStyle_NONE));
        }
    }

    if( mxBase64Stream.is() )
    {
        OUString aPersistName( GetImport().ResolveEmbeddedObjectURLFromBase64() );
        static const OUStringLiteral  sURL( u"vnd.sun.star.EmbeddedObject:" );

        aPersistName = aPersistName.copy( sURL.getLength() );

        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
        if( xProps.is() )
            xProps->setPropertyValue("PersistName", uno::makeAny( aPersistName ) );
    }

    SdXMLShapeContext::endFastElement(nElement);
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLObjectShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() )
    {
        case XML_ELEMENT(DRAW, XML_CLASS_ID):
            maCLSID = aIter.toString();
            break;
        case XML_ELEMENT(XLINK, XML_HREF):
            maHref = aIter.toString();
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLObjectShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if(nElement == XML_ELEMENT(OFFICE, XML_BINARY_DATA))
    {
        mxBase64Stream = GetImport().GetStreamForEmbeddedObjectURLFromBase64();
        if( mxBase64Stream.is() )
            return new XMLBase64ImportContext( GetImport(), mxBase64Stream );
    }
    else if( nElement == XML_ELEMENT(OFFICE, XML_DOCUMENT) ||
             nElement == XML_ELEMENT(MATH, XML_MATH) )
    {
        rtl::Reference<XMLEmbeddedObjectImportContext> xEContext(
            new XMLEmbeddedObjectImportContext(GetImport(), nElement, xAttrList));
        maCLSID = xEContext->GetFilterCLSID();
        if( !maCLSID.isEmpty() )
        {
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if( xPropSet.is() )
            {
                xPropSet->setPropertyValue("CLSID", uno::makeAny( maCLSID ) );

                uno::Reference< lang::XComponent > xComp;
                xPropSet->getPropertyValue("Model") >>= xComp;
                SAL_WARN_IF( !xComp.is(), "xmloff", "no xModel for own OLE format" );
                xEContext->SetComponent(xComp);
            }
        }
        return xEContext;
    }

    // delegate to parent class if no context could be created
    return SdXMLShapeContext::createFastChildContext(nElement, xAttrList);
}

SdXMLAppletShapeContext::SdXMLAppletShapeContext( SvXMLImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes)
: SdXMLShapeContext( rImport, xAttrList, rShapes, false/*bTemporaryShape*/ ),
  mbIsScript( false )
{
}

SdXMLAppletShapeContext::~SdXMLAppletShapeContext()
{
}

void SdXMLAppletShapeContext::startFastElement (sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/)
{
    AddShape("com.sun.star.drawing.AppletShape");

    if( mxShape.is() )
    {
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();
        GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLAppletShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() )
    {
        case XML_ELEMENT(DRAW, XML_APPLET_NAME):
            maAppletName = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_CODE):
            maAppletCode = aIter.toString();
            break;
        case XML_ELEMENT(DRAW, XML_MAY_SCRIPT):
            mbIsScript = IsXMLToken( aIter, XML_TRUE );
            break;
        case XML_ELEMENT(XLINK, XML_HREF):
            maHref = GetImport().GetAbsoluteReference(aIter.toString());
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLAppletShapeContext::endFastElement(sal_Int32 nElement)
{
    uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
    if( xProps.is() )
    {
        if ( maSize.Width && maSize.Height )
        {
            // the visual area for applet must be set on loading
            awt::Rectangle aRect( 0, 0, maSize.Width, maSize.Height );
            xProps->setPropertyValue("VisibleArea", Any(aRect) );
        }

        if( maParams.hasElements() )
        {
            xProps->setPropertyValue("AppletCommands", Any(maParams) );
        }

        if( !maHref.isEmpty() )
        {
            xProps->setPropertyValue("AppletCodeBase", Any(maHref) );
        }

        if( !maAppletName.isEmpty() )
        {
            xProps->setPropertyValue("AppletName", Any(maAppletName) );
        }

        if( mbIsScript )
        {
            xProps->setPropertyValue("AppletIsScript", Any(mbIsScript) );

        }

        if( !maAppletCode.isEmpty() )
        {
            xProps->setPropertyValue("AppletCode", Any(maAppletCode) );
        }

        xProps->setPropertyValue("AppletDocBase", Any(GetImport().GetDocumentBase()) );

        SetThumbnail();
    }

    SdXMLShapeContext::endFastElement(nElement);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLAppletShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(DRAW, XML_PARAM) )
    {
        OUString aParamName, aParamValue;
        // now parse the attribute list and look for draw:name and draw:value
        for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        {
            if( aIter.getToken() == XML_ELEMENT(DRAW, XML_NAME) )
                aParamName = aIter.toString();
            if( aIter.getToken() == XML_ELEMENT(DRAW, XML_VALUE) )
                aParamValue = aIter.toString();
        }

        if( !aParamName.isEmpty() )
        {
            sal_Int32 nIndex = maParams.getLength();
            maParams.realloc( nIndex + 1 );
            auto pParams = maParams.getArray();
            pParams[nIndex].Name = aParamName;
            pParams[nIndex].Handle = -1;
            pParams[nIndex].Value <<= aParamValue;
            pParams[nIndex].State = beans::PropertyState_DIRECT_VALUE;
        }

        return new SvXMLImportContext( GetImport() );
    }

    return SdXMLShapeContext::createFastChildContext( nElement, xAttrList );
}


SdXMLPluginShapeContext::SdXMLPluginShapeContext( SvXMLImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes) :
SdXMLShapeContext( rImport, xAttrList, rShapes, false/*bTemporaryShape*/ ),
mbMedia( false )
{
}

SdXMLPluginShapeContext::~SdXMLPluginShapeContext()
{
}

void SdXMLPluginShapeContext::startFastElement (sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{

    // watch for MimeType attribute to see if we have a media object
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if( aIter.getToken() == XML_ELEMENT(DRAW, XML_MIME_TYPE) )
        {
            if( aIter.toString() == "application/vnd.sun.star.media" )
                mbMedia = true;
            // leave this loop
            break;
        }
    }

    OUString service;

    bool bIsPresShape = false;

    if( mbMedia )
    {
        service = "com.sun.star.drawing.MediaShape";

        bIsPresShape = !maPresentationClass.isEmpty() && GetImport().GetShapeImport()->IsPresentationShapesSupported();
        if( bIsPresShape )
        {
            if( IsXMLToken( maPresentationClass, XML_OBJECT ) )
            {
                service = "com.sun.star.presentation.MediaShape";
            }
        }
    }
    else
        service = "com.sun.star.drawing.PluginShape";

    AddShape(service);

    if( !mxShape.is() )
        return;

    SetLayer();

    if(bIsPresShape)
    {
        uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
        if(xProps.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
            if( xPropsInfo.is() )
            {
                if( !mbIsPlaceholder && xPropsInfo->hasPropertyByName("IsEmptyPresentationObject"))
                    xProps->setPropertyValue("IsEmptyPresentationObject", css::uno::Any(false) );

                if( mbIsUserTransformed && xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                    xProps->setPropertyValue("IsPlaceholderDependent", css::uno::Any(false) );
            }
        }
    }

    // set pos, size, shear and rotate
    SetTransformation();
    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

static OUString
lcl_GetMediaReference(SvXMLImport const& rImport, OUString const& rURL)
{
    if (rImport.IsPackageURL(rURL))
    {
        return "vnd.sun.star.Package:" + rURL;
    }
    else
    {
        return rImport.GetAbsoluteReference(rURL);
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLPluginShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() )
    {
        case XML_ELEMENT(DRAW, XML_MIME_TYPE):
            maMimeType = aIter.toString();
            break;
        case XML_ELEMENT(XLINK, XML_HREF):
            maHref = lcl_GetMediaReference(GetImport(), aIter.toString());
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLPluginShapeContext::endFastElement(sal_Int32 nElement)
{
    uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );

    if( xProps.is() )
    {
        if ( maSize.Width && maSize.Height )
        {
            static const OUStringLiteral sVisibleArea(  u"VisibleArea"  );
            uno::Reference< beans::XPropertySetInfo > aXPropSetInfo( xProps->getPropertySetInfo() );
            if ( !aXPropSetInfo.is() || aXPropSetInfo->hasPropertyByName( sVisibleArea ) )
            {
                // the visual area for a plugin must be set on loading
                awt::Rectangle aRect( 0, 0, maSize.Width, maSize.Height );
                xProps->setPropertyValue( sVisibleArea, Any(aRect) );
            }
        }

        if( !mbMedia )
        {
            // in case we have a plugin object
            if( maParams.hasElements() )
            {
                xProps->setPropertyValue("PluginCommands", Any(maParams) );
            }

            if( !maMimeType.isEmpty() )
            {
                xProps->setPropertyValue("PluginMimeType", Any(maMimeType) );
            }

            if( !maHref.isEmpty() )
            {
                xProps->setPropertyValue("PluginURL", Any(maHref) );
            }
        }
        else
        {
            // in case we have a media object
            xProps->setPropertyValue( "MediaURL", uno::makeAny(maHref));

            xProps->setPropertyValue("MediaMimeType", uno::makeAny(maMimeType) );

            for( const auto& rParam : std::as_const(maParams) )
            {
                const OUString& rName = rParam.Name;

                if( rName == "Loop" )
                {
                    OUString aValueStr;
                    rParam.Value >>= aValueStr;
                    xProps->setPropertyValue("Loop",
                        uno::makeAny( aValueStr == "true" ) );
                }
                else if( rName == "Mute" )
                {
                    OUString aValueStr;
                    rParam.Value >>= aValueStr;
                    xProps->setPropertyValue("Mute",
                        uno::makeAny( aValueStr == "true" ) );
                }
                else if( rName == "VolumeDB" )
                {
                    OUString aValueStr;
                    rParam.Value >>= aValueStr;
                    xProps->setPropertyValue("VolumeDB",
                                                uno::makeAny( static_cast< sal_Int16 >( aValueStr.toInt32() ) ) );
                }
                else if( rName == "Zoom" )
                {
                    OUString            aZoomStr;
                    media::ZoomLevel    eZoomLevel;

                    rParam.Value >>= aZoomStr;

                    if( aZoomStr == "25%" )
                        eZoomLevel = media::ZoomLevel_ZOOM_1_TO_4;
                    else if( aZoomStr == "50%" )
                        eZoomLevel = media::ZoomLevel_ZOOM_1_TO_2;
                    else if( aZoomStr == "100%" )
                        eZoomLevel = media::ZoomLevel_ORIGINAL;
                    else if( aZoomStr == "200%" )
                        eZoomLevel = media::ZoomLevel_ZOOM_2_TO_1;
                    else if( aZoomStr == "400%" )
                        eZoomLevel = media::ZoomLevel_ZOOM_4_TO_1;
                    else if( aZoomStr == "fit" )
                        eZoomLevel = media::ZoomLevel_FIT_TO_WINDOW;
                    else if( aZoomStr == "fixedfit" )
                        eZoomLevel = media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT;
                    else if( aZoomStr == "fullscreen" )
                        eZoomLevel = media::ZoomLevel_FULLSCREEN;
                    else
                        eZoomLevel = media::ZoomLevel_NOT_AVAILABLE;

                    xProps->setPropertyValue("Zoom", uno::makeAny( eZoomLevel ) );
                }
            }
        }

        SetThumbnail();
    }

    SdXMLShapeContext::endFastElement(nElement);
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLPluginShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( nElement == XML_ELEMENT(DRAW, XML_PARAM) )
    {
        OUString aParamName, aParamValue;
        // now parse the attribute list and look for draw:name and draw:value
        for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
        {
            if( aIter.getToken() == XML_ELEMENT(DRAW, XML_NAME) )
                aParamName = aIter.toString();
            else if( aIter.getToken() == XML_ELEMENT(DRAW, XML_VALUE) )
                aParamValue = aIter.toString();
        }

        if( !aParamName.isEmpty() )
        {
            sal_Int32 nIndex = maParams.getLength();
            maParams.realloc( nIndex + 1 );
            auto pParams = maParams.getArray();
            pParams[nIndex].Name = aParamName;
            pParams[nIndex].Handle = -1;
            pParams[nIndex].Value <<= aParamValue;
            pParams[nIndex].State = beans::PropertyState_DIRECT_VALUE;
        }

        return new SvXMLImportContext( GetImport() );
    }

    return SdXMLShapeContext::createFastChildContext( nElement, xAttrList );
}


SdXMLFloatingFrameShapeContext::SdXMLFloatingFrameShapeContext( SvXMLImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes)
: SdXMLShapeContext( rImport, xAttrList, rShapes, false/*bTemporaryShape*/ )
{
}

SdXMLFloatingFrameShapeContext::~SdXMLFloatingFrameShapeContext()
{
}

void SdXMLFloatingFrameShapeContext::startFastElement (sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/)
{
    AddShape("com.sun.star.drawing.FrameShape");

    if( !mxShape.is() )
        return;

    SetLayer();

    // set pos, size, shear and rotate
    SetTransformation();

    uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
    if( xProps.is() )
    {
        if( !maFrameName.isEmpty() )
        {
            xProps->setPropertyValue("FrameName", Any(maFrameName) );
        }

        if( !maHref.isEmpty() )
        {
            xProps->setPropertyValue("FrameURL", Any(maHref) );
        }
    }

    SetStyle();

    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLFloatingFrameShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    switch( aIter.getToken() )
    {
        case XML_ELEMENT(DRAW, XML_FRAME_NAME):
            maFrameName = aIter.toString();
            break;
        case XML_ELEMENT(XLINK, XML_HREF):
            maHref = GetImport().GetAbsoluteReference(aIter.toString());
            break;
        default:
            return SdXMLShapeContext::processAttribute( aIter );
    }
    return true;
}

void SdXMLFloatingFrameShapeContext::endFastElement(sal_Int32 nElement)
{
    uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );

    if( xProps.is() )
    {
        if ( maSize.Width && maSize.Height )
        {
            // the visual area for a floating frame must be set on loading
            awt::Rectangle aRect( 0, 0, maSize.Width, maSize.Height );
            xProps->setPropertyValue("VisibleArea", Any(aRect) );
        }
    }

    SetThumbnail();
    SdXMLShapeContext::endFastElement(nElement);
}


SdXMLFrameShapeContext::SdXMLFrameShapeContext( SvXMLImport& rImport,
        const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes > const & rShapes,
        bool bTemporaryShape)
: SdXMLShapeContext( rImport, xAttrList, rShapes, bTemporaryShape ),
    mbSupportsReplacement( false )
{
    uno::Reference < util::XCloneable > xClone( xAttrList, uno::UNO_QUERY );
    if( xClone.is() )
        mxAttrList.set( xClone->createClone(), uno::UNO_QUERY );
    else
        mxAttrList = new sax_fastparser::FastAttributeList(xAttrList);
}

SdXMLFrameShapeContext::~SdXMLFrameShapeContext()
{
}

void SdXMLFrameShapeContext::removeGraphicFromImportContext(const SvXMLImportContext& rContext)
{
    const SdXMLGraphicObjectShapeContext* pSdXMLGraphicObjectShapeContext = dynamic_cast< const SdXMLGraphicObjectShapeContext* >(&rContext);

    if(!pSdXMLGraphicObjectShapeContext)
        return;

    try
    {
        uno::Reference< container::XChild > xChild(pSdXMLGraphicObjectShapeContext->getShape(), uno::UNO_QUERY_THROW);

        uno::Reference< drawing::XShapes > xParent(xChild->getParent(), uno::UNO_QUERY_THROW);

        // remove from parent
        xParent->remove(pSdXMLGraphicObjectShapeContext->getShape());

        // dispose
        uno::Reference< lang::XComponent > xComp(pSdXMLGraphicObjectShapeContext->getShape(), UNO_QUERY);

        if(xComp.is())
        {
            xComp->dispose();
        }
    }
    catch( uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION( "xmloff", "Error in cleanup of multiple graphic object import." );
    }
}

namespace
{
uno::Reference<beans::XPropertySet> getGraphicPropertySetFromImportContext(const SvXMLImportContext& rContext)
{
    uno::Reference<beans::XPropertySet> aPropertySet;
    const SdXMLGraphicObjectShapeContext* pSdXMLGraphicObjectShapeContext = dynamic_cast<const SdXMLGraphicObjectShapeContext*>(&rContext);

    if (pSdXMLGraphicObjectShapeContext)
        aPropertySet.set(pSdXMLGraphicObjectShapeContext->getShape(), uno::UNO_QUERY);

    return aPropertySet;
}

} // end anonymous namespace

uno::Reference<graphic::XGraphic> SdXMLFrameShapeContext::getGraphicFromImportContext(const SvXMLImportContext& rContext) const
{
    uno::Reference<graphic::XGraphic> xGraphic;
    try
    {
        const uno::Reference<beans::XPropertySet> xPropertySet = getGraphicPropertySetFromImportContext(rContext);

        if (xPropertySet.is())
        {
            xPropertySet->getPropertyValue("Graphic") >>= xGraphic;
        }
    }
    catch( uno::Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("xmloff", "Error in cleanup of multiple graphic object import.");
    }

    return xGraphic;
}

OUString SdXMLFrameShapeContext::getGraphicPackageURLFromImportContext(const SvXMLImportContext& rContext) const
{
    OUString aRetval;
    const SdXMLGraphicObjectShapeContext* pSdXMLGraphicObjectShapeContext = dynamic_cast< const SdXMLGraphicObjectShapeContext* >(&rContext);

    if(pSdXMLGraphicObjectShapeContext)
    {
        try
        {
            const uno::Reference< beans::XPropertySet > xPropSet(pSdXMLGraphicObjectShapeContext->getShape(), uno::UNO_QUERY_THROW);

            xPropSet->getPropertyValue("GraphicStreamURL") >>= aRetval;
        }
        catch( uno::Exception& )
        {
            DBG_UNHANDLED_EXCEPTION( "xmloff", "Error in cleanup of multiple graphic object import." );
        }
    }

    return aRetval;
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLFrameShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    SvXMLImportContextRef xContext;
    if( !mxImplContext.is() )
    {
        SvXMLShapeContext* pShapeContext = XMLShapeImportHelper::CreateFrameChildContext(
                        GetImport(), nElement, xAttrList, mxShapes, mxAttrList );

        xContext = pShapeContext;

        // propagate the hyperlink to child context
        if ( !msHyperlink.isEmpty() )
            pShapeContext->setHyperlink( msHyperlink );

        auto nToken = nElement & TOKEN_MASK;
        // Ignore gltf model if necessary and so the fallback image will be imported
        if( nToken == XML_PLUGIN )
        {
            SdXMLPluginShapeContext* pPluginContext = dynamic_cast<SdXMLPluginShapeContext*>(pShapeContext);
            if( pPluginContext && pPluginContext->getMimeType() == "model/vnd.gltf+json" )
            {
                mxImplContext = nullptr;
                return new SvXMLImportContext(GetImport());
            }
        }

        mxImplContext = xContext;
        mbSupportsReplacement = (nToken == XML_OBJECT ) || (nToken == XML_OBJECT_OLE);
        setSupportsMultipleContents(nToken == XML_IMAGE);

        if(getSupportsMultipleContents() && dynamic_cast< SdXMLGraphicObjectShapeContext* >(xContext.get()))
        {
            if ( !maShapeId.isEmpty() )
                GetImport().getInterfaceToIdentifierMapper().reserveIdentifier( maShapeId );

            addContent(*mxImplContext);
        }
    }
    else if(getSupportsMultipleContents() && nElement == XML_ELEMENT(DRAW, XML_IMAGE))
    {
        // read another image
        xContext = XMLShapeImportHelper::CreateFrameChildContext(
            GetImport(), nElement, xAttrList, mxShapes, mxAttrList);
        mxImplContext = xContext;

        if(dynamic_cast< SdXMLGraphicObjectShapeContext* >(xContext.get()))
        {
            addContent(*mxImplContext);
        }
    }
    else if( mbSupportsReplacement && !mxReplImplContext.is() &&
             nElement == XML_ELEMENT(DRAW, XML_IMAGE) )
    {
        // read replacement image
        SvXMLImportContext *pImplContext = mxImplContext.get();
        SdXMLShapeContext *pSContext =
            dynamic_cast<SdXMLShapeContext*>( pImplContext  );
        if( pSContext )
        {
            uno::Reference < beans::XPropertySet > xPropSet(
                    pSContext->getShape(), uno::UNO_QUERY );
            if( xPropSet.is() )
            {
                xContext = new XMLReplacementImageContext( GetImport(),
                                    nElement, xAttrList, xPropSet );
                mxReplImplContext = xContext;
            }
        }
    }
    else if( nElement == XML_ELEMENT(SVG, XML_TITLE) || // #i68101#
             nElement == XML_ELEMENT(SVG_COMPAT, XML_TITLE) ||
             nElement == XML_ELEMENT(SVG, XML_DESC) ||
             nElement == XML_ELEMENT(SVG_COMPAT, XML_DESC) ||
             nElement == XML_ELEMENT(OFFICE, XML_EVENT_LISTENERS) ||
             nElement == XML_ELEMENT(DRAW, XML_GLUE_POINT) ||
             nElement == XML_ELEMENT(DRAW, XML_THUMBNAIL) )
    {
        if (getSupportsMultipleContents())
        {   // tdf#103567 ensure props are set on surviving shape
            // note: no more draw:image can be added once we get here
            mxImplContext = solveMultipleImages();
        }
        SvXMLImportContext *pImplContext = mxImplContext.get();
        xContext = static_cast<SvXMLImportContext*>(dynamic_cast<SdXMLShapeContext&>(*pImplContext).createFastChildContext( nElement,
                                                                        xAttrList ).get());
    }
    else if ( nElement == XML_ELEMENT(DRAW, XML_IMAGE_MAP) )
    {
        if (getSupportsMultipleContents())
        {   // tdf#103567 ensure props are set on surviving shape
            // note: no more draw:image can be added once we get here
            mxImplContext = solveMultipleImages();
        }
        SdXMLShapeContext *pSContext = dynamic_cast< SdXMLShapeContext* >( mxImplContext.get() );
        if( pSContext )
        {
            uno::Reference < beans::XPropertySet > xPropSet( pSContext->getShape(), uno::UNO_QUERY );
            if (xPropSet.is())
            {
                xContext = new XMLImageMapContext(GetImport(), xPropSet);
            }
        }
    }
    else if ( nElement == XML_ELEMENT(LO_EXT, XML_SIGNATURELINE) )
    {
        SdXMLShapeContext* pSContext = dynamic_cast<SdXMLShapeContext*>(mxImplContext.get());
        if (pSContext)
        {
            uno::Reference<beans::XPropertySet> xPropSet(pSContext->getShape(), uno::UNO_QUERY);
            if (xPropSet.is())
            {
                xContext = new SignatureLineContext(GetImport(), nElement, xAttrList,
                                                    pSContext->getShape());
            }
        }
    }
    else if ( nElement == XML_ELEMENT(LO_EXT, XML_QRCODE))
    {
        SdXMLShapeContext* pSContext = dynamic_cast<SdXMLShapeContext*>(mxImplContext.get());
        if (pSContext)
        {
            uno::Reference<beans::XPropertySet> xPropSet(pSContext->getShape(), uno::UNO_QUERY);
            if (xPropSet.is())
            {
                xContext = new QRCodeContext(GetImport(), nElement, xAttrList,
                                                    pSContext->getShape());
            }
        }
    }

    return xContext;
}

void SdXMLFrameShapeContext::startFastElement (sal_Int32 /*nElement*/,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& /*xAttrList*/)
{
    // ignore
}

void SdXMLFrameShapeContext::endFastElement(sal_Int32 nElement)
{
    // solve if multiple image child contexts were imported
    SvXMLImportContextRef const pSelectedContext(solveMultipleImages());
    const SdXMLGraphicObjectShapeContext* pShapeContext(
        dynamic_cast<const SdXMLGraphicObjectShapeContext*>(pSelectedContext.get()));
    if ( pShapeContext )
    {
        assert( mxImplContext.is() );
        const uno::Reference< uno::XInterface > xShape( pShapeContext->getShape() );
        GetImport().getInterfaceToIdentifierMapper().registerReservedReference( maShapeId, xShape );
    }

    if( !mxImplContext.is() )
    {
        // now check if this is an empty presentation object
        for( auto& aIter : sax_fastparser::castToFastAttributeList(mxAttrList) )
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT(PRESENTATION, XML_PLACEHOLDER):
                    mbIsPlaceholder = IsXMLToken( aIter, XML_TRUE );
                    break;
                case XML_ELEMENT(PRESENTATION, XML_CLASS):
                    maPresentationClass = aIter.toString();
                    break;
                default:;
            }
        }

        if( (!maPresentationClass.isEmpty()) && mbIsPlaceholder )
        {
            uno::Reference< xml::sax::XFastAttributeList> xEmpty;

            enum XMLTokenEnum eToken = XML_TEXT_BOX;

            if( IsXMLToken( maPresentationClass, XML_GRAPHIC ) )
            {
                eToken = XML_IMAGE;

            }
            else if( IsXMLToken( maPresentationClass, XML_PAGE ) )
            {
                eToken = XML_PAGE_THUMBNAIL;
            }
            else if( IsXMLToken( maPresentationClass, XML_CHART ) ||
                     IsXMLToken( maPresentationClass, XML_TABLE ) ||
                     IsXMLToken( maPresentationClass, XML_OBJECT ) )
            {
                eToken = XML_OBJECT;
            }

            auto x = XML_ELEMENT(DRAW, eToken);
            mxImplContext = XMLShapeImportHelper::CreateFrameChildContext(
                    GetImport(), x, mxAttrList, mxShapes, xEmpty );

            if( mxImplContext.is() )
            {
                mxImplContext->startFastElement( x, mxAttrList );
                mxImplContext->endFastElement(x);
            }
        }
    }

    mxImplContext = nullptr;
    SdXMLShapeContext::endFastElement(nElement);
}

bool SdXMLFrameShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    bool bId( false );

    switch ( aIter.getToken() )
    {
        case XML_ELEMENT(DRAW, XML_ID):
        case XML_ELEMENT(DRAW_EXT, XML_ID):
        case XML_ELEMENT(NONE, XML_ID):
        case XML_ELEMENT(XML, XML_ID) :
            bId = true;
            break;
        default:;
    }

    if ( bId )
        return SdXMLShapeContext::processAttribute( aIter );
    return true; // deliberately ignoring other attributes
}


SdXMLCustomShapeContext::SdXMLCustomShapeContext(
    SvXMLImport& rImport,
    const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes > const & rShapes)
:   SdXMLShapeContext( rImport, xAttrList, rShapes, false/*bTemporaryShape*/ )
{
    // See the XMLTextFrameContext ctor, a frame has Writer content (and not
    // editeng) if its autostyle has a parent style. Do the same for shapes as well.
    for( auto& aIter : sax_fastparser::castToFastAttributeList(xAttrList) )
    {
        if (aIter.getToken() == XML_ELEMENT(DRAW, XML_STYLE_NAME))
        {
            OUString aStyleName = aIter.toString();
            if(!aStyleName.isEmpty())
            {
                rtl::Reference<XMLTextImportHelper> xTxtImport = GetImport().GetTextImport();
                XMLPropStyleContext* pStyle = xTxtImport->FindAutoFrameStyle(aStyleName);
                // Note that this an API name, so intentionally not localized.
                if (pStyle && pStyle->GetParentName() == "Frame")
                {
                    mbTextBox = true;
                    break;
                }
            }
        }
    }
}

SdXMLCustomShapeContext::~SdXMLCustomShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLCustomShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    if( aIter.getToken() == XML_ELEMENT(DRAW, XML_ENGINE) )
    {
        maCustomShapeEngine = aIter.toString();
    }
    else if (aIter.getToken() == XML_ELEMENT(DRAW, XML_DATA) )
    {
        maCustomShapeData = aIter.toString();
    }
    else
        return SdXMLShapeContext::processAttribute( aIter );
    return true;
}

void SdXMLCustomShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.CustomShape");
    if ( !mxShape.is() )
        return;

    // Add, set Style and properties from base shape
    SetStyle();
    SetLayer();

    // set pos, size, shear and rotate
    SetTransformation();

    try
    {
        uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
        if( xPropSet.is() )
        {
            if ( !maCustomShapeEngine.isEmpty() )
            {
                xPropSet->setPropertyValue( EASGet( EAS_CustomShapeEngine ), Any(maCustomShapeEngine) );
            }
            if ( !maCustomShapeData.isEmpty() )
            {
                xPropSet->setPropertyValue( EASGet( EAS_CustomShapeData ), Any(maCustomShapeData) );
            }
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION( "xmloff", "setting enhanced customshape geometry" );
    }
    SdXMLShapeContext::startFastElement(nElement, xAttrList);
}

void SdXMLCustomShapeContext::endFastElement(sal_Int32 nElement)
{
    // Customshapes remember mirror state in its enhanced geometry.
    // SetTransformation() in StartElement() may have applied mirroring, but that is not yet
    // contained. Merge that information here before writing the property.
    if(!maUsedTransformation.isIdentity())
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;

        maUsedTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

        if (aScale.getX() < 0.0)
        {
            static const OUStringLiteral sName(u"MirroredX");
            //fdo#84043 Merge, if property exists, otherwise append it
            auto aI = std::find_if(maCustomShapeGeometry.begin(), maCustomShapeGeometry.end(),
                [](beans::PropertyValue& rValue) { return rValue.Name == sName; });
            if (aI != maCustomShapeGeometry.end())
            {
                beans::PropertyValue& rItem = *aI;
                bool bMirroredX = *o3tl::doAccess<bool>(rItem.Value);
                rItem.Value <<= !bMirroredX;
                rItem.Handle = -1;
                rItem.State = beans::PropertyState_DIRECT_VALUE;
            }
            else
            {
                beans::PropertyValue* pItem;
                maCustomShapeGeometry.emplace_back();
                pItem = &maCustomShapeGeometry.back();
                pItem->Name = sName;
                pItem->Handle = -1;
                pItem->Value <<= true;
                pItem->State = beans::PropertyState_DIRECT_VALUE;
            }
        }

        if (aScale.getY() < 0.0)
        {
            static const OUStringLiteral sName(u"MirroredY");
            //fdo#84043 Merge, if property exists, otherwise append it
            auto aI = std::find_if(maCustomShapeGeometry.begin(), maCustomShapeGeometry.end(),
                [](beans::PropertyValue& rValue) { return rValue.Name == sName; });
            if (aI != maCustomShapeGeometry.end())
            {
                beans::PropertyValue& rItem = *aI;
                bool bMirroredY = *o3tl::doAccess<bool>(rItem.Value);
                rItem.Value <<= !bMirroredY;
                rItem.Handle = -1;
                rItem.State = beans::PropertyState_DIRECT_VALUE;
            }
            else
            {
                beans::PropertyValue* pItem;
                maCustomShapeGeometry.emplace_back();
                pItem = &maCustomShapeGeometry.back();
                pItem->Name = sName;
                pItem->Handle = -1;
                pItem->Value <<= true;
                pItem->State = beans::PropertyState_DIRECT_VALUE;
            }
        }
    }

    if ( !maCustomShapeGeometry.empty() )
    {
        // converting the vector to a sequence
        uno::Sequence< beans::PropertyValue > aSeq( comphelper::containerToSequence(maCustomShapeGeometry) );

        try
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
            if( xPropSet.is() )
            {
                xPropSet->setPropertyValue( "CustomShapeGeometry", Any(aSeq) );
            }
        }
        catch(const uno::Exception&)
        {
            DBG_UNHANDLED_EXCEPTION( "xmloff", "setting enhanced customshape geometry" );
        }

        sal_Int32 nUPD;
        sal_Int32 nBuild;
        if (GetImport().getBuildIds(nUPD, nBuild))
        {
            if( ((nUPD >= 640 && nUPD <= 645) || (nUPD == 680)) && (nBuild <= 9221) )
            {
                Reference< drawing::XEnhancedCustomShapeDefaulter > xDefaulter( mxShape, UNO_QUERY );
                if( xDefaulter.is() )
                {
                    xDefaulter->createCustomShapeDefaults( "" );
                }
            }
        }
    }

    SdXMLShapeContext::endFastElement(nElement);

    // tdf#98163 call a custom slot to be able to reset the UNO API
    // implementations held on the SdrObjects of type
    // SdrObjCustomShape - those tend to linger until the entire file
    // is loaded. For large files with a lot of these, 32bit systems
    // may crash due to being out of resources after ca. 4200
    // Outliners and VirtualDevices used there as RefDevice
    try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);

        if(xPropSet.is())
        {
            xPropSet->setPropertyValue(
                "FlushCustomShapeUnoApiObjects", css::uno::Any(true));
        }
    }
    catch(const uno::Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("xmloff", "flushing after load");
    }
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLCustomShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const uno::Reference< xml::sax::XFastAttributeList>& xAttrList )
{
    css::uno::Reference< css::xml::sax::XFastContextHandler > xContext;
    if ( nElement == XML_ELEMENT(DRAW, XML_ENHANCED_GEOMETRY) )
    {
        uno::Reference< beans::XPropertySet > xPropSet( mxShape,uno::UNO_QUERY );
        if ( xPropSet.is() )
            xContext = new XMLEnhancedCustomShapeContext( GetImport(), mxShape, maCustomShapeGeometry );
    }
    // delegate to parent class if no context could be created
    if (!xContext)
        xContext = SdXMLShapeContext::createFastChildContext( nElement,
                                                         xAttrList);
    return xContext;
}

SdXMLTableShapeContext::SdXMLTableShapeContext( SvXMLImport& rImport, const css::uno::Reference< css::xml::sax::XFastAttributeList>& xAttrList, css::uno::Reference< css::drawing::XShapes > const & rShapes )
: SdXMLShapeContext( rImport, xAttrList, rShapes, false )
{
}

SdXMLTableShapeContext::~SdXMLTableShapeContext()
{
}

void SdXMLTableShapeContext::startFastElement (sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList)
{
    OUString service("com.sun.star.drawing.TableShape");

    bool bIsPresShape = !maPresentationClass.isEmpty() && GetImport().GetShapeImport()->IsPresentationShapesSupported();
    if( bIsPresShape )
    {
        if( IsXMLToken( maPresentationClass, XML_TABLE ) )
        {
            service = "com.sun.star.presentation.TableShape";
        }
    }

    AddShape(service);

    if( !mxShape.is() )
        return;

    SetLayer();

    uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);

    if(bIsPresShape && xProps.is())
    {
        uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
        if( xPropsInfo.is() )
        {
            if( !mbIsPlaceholder && xPropsInfo->hasPropertyByName("IsEmptyPresentationObject"))
                xProps->setPropertyValue("IsEmptyPresentationObject", css::uno::Any(false) );

            if( mbIsUserTransformed && xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                xProps->setPropertyValue("IsPlaceholderDependent", css::uno::Any(false) );
        }
    }

    SetStyle();

    if( xProps.is() )
    {
        if( !msTemplateStyleName.isEmpty() ) try
        {
            Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetImport().GetModel(), UNO_QUERY_THROW );
            Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
            Reference< XNameAccess > xTableFamily( xFamilies->getByName( "table" ), UNO_QUERY_THROW );
            Reference< XStyle > xTableStyle( xTableFamily->getByName( msTemplateStyleName ), UNO_QUERY_THROW );
            xProps->setPropertyValue("TableTemplate", Any( xTableStyle ) );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("xmloff.draw");
        }

        const XMLPropertyMapEntry* pEntry = &aXMLTableShapeAttributes[0];
        for( int i = 0; pEntry->msApiName && (i < 6); i++, pEntry++ )
        {
            try
            {
                const OUString sAPIPropertyName( pEntry->msApiName, pEntry->nApiNameLength, RTL_TEXTENCODING_ASCII_US );
                xProps->setPropertyValue( sAPIPropertyName, Any( maTemplateStylesUsed[i] ) );
            }
            catch(const Exception&)
            {
                DBG_UNHANDLED_EXCEPTION("xmloff.draw");
            }
        }
    }

    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );

    const rtl::Reference< XMLTableImport >& xTableImport( GetImport().GetShapeImport()->GetShapeTableImport() );
    if( xTableImport.is() && xProps.is() )
    {
        uno::Reference< table::XColumnRowRange > xColumnRowRange(
            xProps->getPropertyValue("Model"), uno::UNO_QUERY );

        if( xColumnRowRange.is() )
            mxTableImportContext = xTableImport->CreateTableContext( xColumnRowRange );

        if( mxTableImportContext.is() )
            mxTableImportContext->startFastElement( nElement, xAttrList );
    }
}

void SdXMLTableShapeContext::endFastElement(sal_Int32 nElement)
{
    if( mxTableImportContext.is() )
        mxTableImportContext->endFastElement(nElement);

    SdXMLShapeContext::endFastElement(nElement);

    if( mxShape.is() )
    {
        // set pos, size, shear and rotate
        SetTransformation();
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
bool SdXMLTableShapeContext::processAttribute( const sax_fastparser::FastAttributeList::FastAttributeIter & aIter )
{
    auto nElement = aIter.getToken();
    if( IsTokenInNamespace(nElement, XML_NAMESPACE_TABLE) )
    {
        if( (nElement & TOKEN_MASK) == XML_TEMPLATE_NAME )
        {
            msTemplateStyleName = aIter.toString();
        }
        else
        {
            int i = 0;
            const XMLPropertyMapEntry* pEntry = &aXMLTableShapeAttributes[0];
            while( pEntry->msApiName && (i < 6) )
            {
                if( (nElement & TOKEN_MASK) == pEntry->meXMLName )
                {
                    if( IsXMLToken( aIter, XML_TRUE ) )
                        maTemplateStylesUsed[i] = true;
                    break;
                }
                pEntry++;
                i++;
            }
        }
    }
    return SdXMLShapeContext::processAttribute( aIter );
}

css::uno::Reference< css::xml::sax::XFastContextHandler > SdXMLTableShapeContext::createFastChildContext(
    sal_Int32 nElement,
    const css::uno::Reference< css::xml::sax::XFastAttributeList >& xAttrList )
{
    if( mxTableImportContext.is() && IsTokenInNamespace(nElement, XML_NAMESPACE_TABLE) )
        return mxTableImportContext->createFastChildContext(nElement, xAttrList);
    return SdXMLShapeContext::createFastChildContext(nElement, xAttrList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
