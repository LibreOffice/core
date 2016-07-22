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

#include <cassert>

#include <tools/debug.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/document/XEventsSupplier.hpp>
#include <com/sun/star/container/XNameReplace.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/LineStyle.hpp>
#include <xmloff/unointerfacetouniqueidentifiermapper.hxx>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/container/XIdentifierAccess.hpp>
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
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/awt/XControlModel.hpp>
#include <com/sun/star/drawing/XControlShape.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include "xexptran.hxx"
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>

#include <sax/tools/converter.hxx>
#include <comphelper/sequence.hxx>

#include "PropertySetMerger.hxx"
#include <xmloff/families.hxx>
#include "ximpstyl.hxx"
#include"xmloff/xmlnmspe.hxx"
#include <xmloff/xmltoken.hxx>
#include "EnhancedCustomShapeToken.hxx"
#include "XMLReplacementImageContext.hxx"
#include "XMLImageMapContext.hxx"
#include "sdpropls.hxx"
#include "eventimp.hxx"
#include "descriptionimp.hxx"
#include "ximpcustomshape.hxx"
#include "XMLEmbeddedObjectImportContext.hxx"
#include <xmloff/xmlerror.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <com/sun/star/drawing/XEnhancedCustomShapeDefaulter.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/vector/b2dvector.hxx>

#include <config_features.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::drawing;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::xmloff::token;
using namespace ::xmloff::EnhancedCustomShapeToken;

SvXMLEnumMapEntry aXML_GlueAlignment_EnumMap[] =
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
    { XML_TOKEN_INVALID, 0 }
};

SvXMLEnumMapEntry aXML_GlueEscapeDirection_EnumMap[] =
{
    { XML_AUTO,         drawing::EscapeDirection_SMART },
    { XML_LEFT,         drawing::EscapeDirection_LEFT },
    { XML_RIGHT,        drawing::EscapeDirection_RIGHT },
    { XML_UP,           drawing::EscapeDirection_UP },
    { XML_DOWN,         drawing::EscapeDirection_DOWN },
    { XML_HORIZONTAL,   drawing::EscapeDirection_HORIZONTAL },
    { XML_VERTICAL,     drawing::EscapeDirection_VERTICAL },
    { XML_TOKEN_INVALID, 0 }
};

static bool ImpIsEmptyURL( const OUString& rURL )
{
    if( rURL.isEmpty() )
        return true;

    // #i13140# Also compare against 'toplevel' URLs. which also
    // result in empty filename strings.
    if( rURL == "#./" )
        return true;

    return false;
}


SdXMLShapeContext::SdXMLShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
    : SvXMLShapeContext( rImport, nPrfx, rLocalName, bTemporaryShape )
    , mxShapes( rShapes )
    , mxAttrList(xAttrList)
    , mbListContextPushed( false )
    , mnStyleFamily(XML_STYLE_FAMILY_SD_GRAPHICS_ID)
    , mbIsPlaceholder(false)
    , mbClearDefaultAttributes( true )
    , mbIsUserTransformed(false)
    , mnZOrder(-1)
    , maSize(1, 1)
    , mnRelWidth(0)
    , mnRelHeight(0)
    , maPosition(0, 0)
    , maUsedTransformation()
    , mbVisible(true)
    , mbPrintable(true)
    , mbHaveXmlId(false)
    , mbTextBox(false)
{
}

SdXMLShapeContext::~SdXMLShapeContext()
{
}

SvXMLImportContext *SdXMLShapeContext::CreateChildContext( sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = nullptr;

    // #i68101#
    if( p_nPrefix == XML_NAMESPACE_SVG &&
        (IsXMLToken( rLocalName, XML_TITLE ) || IsXMLToken( rLocalName, XML_DESC ) ) )
    {
        pContext = new SdXMLDescriptionContext( GetImport(), p_nPrefix, rLocalName, xAttrList, mxShape );
    }
    else if( p_nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
    {
        pContext = new SdXMLEventsContext( GetImport(), p_nPrefix, rLocalName, xAttrList, mxShape );
    }
    else if( p_nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_GLUE_POINT ) )
    {
        addGluePoint( xAttrList );
    }
    else if( p_nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_THUMBNAIL ) )
    {
        // search attributes for xlink:href
        sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        for(sal_Int16 i=0; i < nAttrCount; i++)
        {
            OUString sAttrName = xAttrList->getNameByIndex( i );
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );

            if( nPrefix == XML_NAMESPACE_XLINK )
            {
                if( IsXMLToken( aLocalName, XML_HREF ) )
                {
                    maThumbnailURL = xAttrList->getValueByIndex( i );
                    break;
                }
            }
        }
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
            pContext = GetImport().GetTextImport()->CreateTextChildContext(
                GetImport(), p_nPrefix, rLocalName, xAttrList,
                ( mbTextBox ? XMLTextType::TextBox : XMLTextType::Shape ) );
        }
    }

    // call parent for content
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext( p_nPrefix, rLocalName, xAttrList );

    return pContext;
}

void SdXMLShapeContext::addGluePoint( const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    // get the glue points container for this shape if its not already there
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
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for(sal_Int16 i=0; i < nAttrCount; i++)
    {
        OUString sAttrName = xAttrList->getNameByIndex( i );
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( sAttrName, &aLocalName );
        const OUString sValue( xAttrList->getValueByIndex( i ) );

        if( nPrefix == XML_NAMESPACE_SVG )
        {
            if( IsXMLToken( aLocalName, XML_X ) )
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aGluePoint.Position.X, sValue);
            }
            else if( IsXMLToken( aLocalName, XML_Y ) )
            {
                GetImport().GetMM100UnitConverter().convertMeasureToCore(
                        aGluePoint.Position.Y, sValue);
            }
        }
        else if( nPrefix == XML_NAMESPACE_DRAW )
        {
            if( IsXMLToken( aLocalName, XML_ID ) )
            {
                nId = sValue.toInt32();
            }
            else if( IsXMLToken( aLocalName, XML_ALIGN ) )
            {
                sal_uInt16 eKind;
                if( SvXMLUnitConverter::convertEnum( eKind, sValue, aXML_GlueAlignment_EnumMap ) )
                {
                    aGluePoint.PositionAlignment = (drawing::Alignment)eKind;
                    aGluePoint.IsRelative = false;
                }
            }
            else if( IsXMLToken( aLocalName, XML_ESCAPE_DIRECTION ) )
            {
                sal_uInt16 eKind;
                if( SvXMLUnitConverter::convertEnum( eKind, sValue, aXML_GlueEscapeDirection_EnumMap ) )
                {
                    aGluePoint.Escape = (drawing::EscapeDirection)eKind;
                }
            }
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
            OSL_FAIL( "exception during setting of glue points!");
        }
    }
}

void SdXMLShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>&)
{
    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

void SdXMLShapeContext::EndElement()
{
    if(mxCursor.is())
    {
        // delete addition newline
        const OUString aEmpty;
        mxCursor->gotoEnd( false );
        mxCursor->goLeft( 1, true );
        mxCursor->setString( aEmpty );

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
            Reference< XNameReplace > xEvents( xEventsSupplier->getEvents(), UNO_QUERY_THROW );

            uno::Sequence< beans::PropertyValue > aProperties( 3 );
            aProperties[0].Name = "EventType";
            aProperties[0].Handle = -1;
            aProperties[0].Value <<= OUString( "Presentation" );
            aProperties[0].State = beans::PropertyState_DIRECT_VALUE;

            aProperties[1].Name = "ClickAction";
            aProperties[1].Handle = -1;
            aProperties[1].Value <<= css::presentation::ClickAction_DOCUMENT;
            aProperties[1].State = beans::PropertyState_DIRECT_VALUE;

            aProperties[2].Name = "Bookmark";
            aProperties[2].Handle = -1;
            aProperties[2].Value <<= msHyperlink;
            aProperties[2].State = beans::PropertyState_DIRECT_VALUE;

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
        OSL_FAIL("xmloff::SdXMLShapeContext::EndElement(), exception caught while setting hyperlink!");
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
            OSL_FAIL( "SdXMLShapeContext::AddShape(), exception caught!" );
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
    if(xServiceFact.is())
    {
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
                css::uno::Sequence<css::uno::Any> args(1);
                args[0] <<= GetImport().GetDocumentBase();
                xShape.set( xServiceFact->createInstanceWithArguments(serviceName, args),
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
}

void SdXMLShapeContext::SetTransformation()
{
    if(mxShape.is())
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
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
            drawing::HomogenMatrix3 aMatrix;

            aMatrix.Line1.Column1 = maUsedTransformation.get(0, 0);
            aMatrix.Line1.Column2 = maUsedTransformation.get(0, 1);
            aMatrix.Line1.Column3 = maUsedTransformation.get(0, 2);

            aMatrix.Line2.Column1 = maUsedTransformation.get(1, 0);
            aMatrix.Line2.Column2 = maUsedTransformation.get(1, 1);
            aMatrix.Line2.Column3 = maUsedTransformation.get(1, 2);

            aMatrix.Line3.Column1 = maUsedTransformation.get(2, 0);
            aMatrix.Line3.Column2 = maUsedTransformation.get(2, 1);
            aMatrix.Line3.Column3 = maUsedTransformation.get(2, 2);

            xPropSet->setPropertyValue("Transformation", Any(aMatrix));
        }
    }
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
            XMLPropStyleContext* pDocStyle = nullptr;

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

            if( pStyle && dynamic_cast<const XMLShapeStyleContext*>( pStyle ) !=  nullptr)
            {
                pDocStyle = const_cast<XMLShapeStyleContext*>(dynamic_cast<const XMLShapeStyleContext*>( pStyle ) );

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

                            if( XML_STYLE_FAMILY_SD_PRESENTATION_ID == mnStyleFamily )
                            {
                                aStyleName = GetImport().GetStyleDisplayName(
                                    XML_STYLE_FAMILY_SD_PRESENTATION_ID,
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
                                // get graphics familie
                                xFamilies->getByName("graphics") >>= xFamily;
                                aStyleName = GetImport().GetStyleDisplayName(
                                    XML_STYLE_FAMILY_SD_GRAPHICS_ID,
                                    aStyleName );
                            }

                            if( xFamily.is() )
                                xFamily->getByName( aStyleName ) >>= xStyle;
                        }
                    }
                }
                catch(const uno::Exception&)
                {
                    OSL_FAIL( "could not find style for shape!" );
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
                    OSL_FAIL( "could not find style for shape!" );
                }
            }

            // if this is an auto style, set its properties
            if(bAutoStyle && pDocStyle)
            {
                // set PropertySet on object
                pDocStyle->FillPropertySet(xPropSet);
            }

            // Writer shapes: if this one has a TextBox, set it here.
            uno::Reference<beans::XPropertySetInfo> xPropertySetInfo = xPropSet->getPropertySetInfo();
            if (xPropertySetInfo->hasPropertyByName("TextBox"))
                xPropSet->setPropertyValue("TextBox", uno::makeAny(mbTextBox));

        } while(false);

        // try to set text auto style
        do
        {
            // set style on shape
            if( maTextStyleName.isEmpty() )
                break;

            if( nullptr == GetImport().GetShapeImport()->GetAutoStylesContext())
                break;

            const SvXMLStyleContext* pTempStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(XML_STYLE_FAMILY_TEXT_PARAGRAPH, maTextStyleName);
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
    if( !maLayerName.isEmpty() )
    {
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
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName( "ThumbnailGraphicURL" ) )
        {
            // load the thumbnail graphic and export it to a wmf stream so we can set
            // it at the api

            const OUString aInternalURL( GetImport().ResolveGraphicObjectURL( maThumbnailURL, false ) );
            xPropSet->setPropertyValue( "ThumbnailGraphicURL", uno::makeAny( aInternalURL ) );
        }
    }
    catch(const uno::Exception&)
    {
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( (XML_NAMESPACE_DRAW == nPrefix) || (XML_NAMESPACE_DRAW_EXT == nPrefix) )
    {
        if( IsXMLToken( rLocalName, XML_ZINDEX ) )
        {
            mnZOrder = rValue.toInt32();
        }
        else if( IsXMLToken( rLocalName, XML_ID ) )
        {
            if (!mbHaveXmlId) { maShapeId = rValue; }
        }
        else if( IsXMLToken( rLocalName, XML_NAME ) )
        {
            maShapeName = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_STYLE_NAME ) )
        {
            maDrawStyleName = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_TEXT_STYLE_NAME ) )
        {
            maTextStyleName = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_LAYER ) )
        {
            maLayerName = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_TRANSFORM ) )
        {
            mnTransform.SetString(rValue, GetImport().GetMM100UnitConverter());
        }
        else if( IsXMLToken( rLocalName, XML_DISPLAY ) )
        {
            mbVisible = IsXMLToken( rValue, XML_ALWAYS ) || IsXMLToken( rValue, XML_SCREEN );
            mbPrintable = IsXMLToken( rValue, XML_ALWAYS ) || IsXMLToken( rValue, XML_PRINTER );
        }
    }
    else if( XML_NAMESPACE_PRESENTATION == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_USER_TRANSFORMED ) )
        {
            mbIsUserTransformed = IsXMLToken( rValue, XML_TRUE );
        }
        else if( IsXMLToken( rLocalName, XML_PLACEHOLDER ) )
        {
            mbIsPlaceholder = IsXMLToken( rValue, XML_TRUE );
            if( mbIsPlaceholder )
                mbClearDefaultAttributes = false;
        }
        else if( IsXMLToken( rLocalName, XML_CLASS ) )
        {
            maPresentationClass = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_STYLE_NAME ) )
        {
            maDrawStyleName = rValue;
            mnStyleFamily = XML_STYLE_FAMILY_SD_PRESENTATION_ID;
        }
    }
    else if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_X ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maPosition.X, rValue);
        }
        else if( IsXMLToken( rLocalName, XML_Y ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maPosition.Y, rValue);
        }
        else if( IsXMLToken( rLocalName, XML_WIDTH ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maSize.Width, rValue);
            if( maSize.Width > 0 )
                maSize.Width += 1;
            else if( maSize.Width < 0 )
                maSize.Width -= 1;
        }
        else if( IsXMLToken( rLocalName, XML_HEIGHT ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maSize.Height, rValue);
            if( maSize.Height > 0 )
                maSize.Height += 1;
            else if( maSize.Height < 0 )
                maSize.Height -= 1;
        }
        else if( IsXMLToken( rLocalName, XML_TRANSFORM ) )
        {
            // because of #85127# take svg:transform into account and handle like
            // draw:transform for compatibility
            mnTransform.SetString(rValue, GetImport().GetMM100UnitConverter());
        }

        // #i68101#
        else if( IsXMLToken( rLocalName, XML_TITLE ) )
        {
            maShapeTitle = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_DESC ) )
        {
            maShapeDescription = rValue;
        }
    }
    else if (nPrefix == XML_NAMESPACE_STYLE)
    {
        sal_Int32 nTmp;
        if (IsXMLToken(rLocalName, XML_REL_WIDTH))
        {
            if (sax::Converter::convertPercent(nTmp, rValue))
                mnRelWidth = static_cast<sal_Int16>(nTmp);
        }
        else if (IsXMLToken(rLocalName, XML_REL_HEIGHT))
        {
            if (sax::Converter::convertPercent(nTmp, rValue))
                mnRelHeight = static_cast<sal_Int16>(nTmp);
        }
    }
    else if( (XML_NAMESPACE_NONE == nPrefix) || (XML_NAMESPACE_XML == nPrefix) )
    {
        if( IsXMLToken( rLocalName, XML_ID ) )
        {
            maShapeId = rValue;
            mbHaveXmlId = true;
        }
    }
}

bool SdXMLShapeContext::isPresentationShape() const
{
    if( !maPresentationClass.isEmpty() && (const_cast<SdXMLShapeContext*>(this))->GetImport().GetShapeImport()->IsPresentationShapesSupported() )
    {
        if(XML_STYLE_FAMILY_SD_PRESENTATION_ID == mnStyleFamily)
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

void SdXMLShapeContext::onDemandRescueUsefulDataFromTemporary( const SvXMLImportContext& rCandidate )
{
    const SdXMLShapeContext* pCandidate = dynamic_cast< const SdXMLShapeContext* >(&rCandidate);

    if(!mxGluePoints.is() && pCandidate)
    {
        // try to rescue GluePoints from rCandidate to local if we not yet have GluePoints by copying them
        uno::Reference< drawing::XGluePointsSupplier > xSourceSupplier( pCandidate->getShape(), uno::UNO_QUERY );
        if( !xSourceSupplier.is() )
            return;

        uno::Reference< container::XIdentifierAccess > xSourceGluePoints( xSourceSupplier->getGluePoints(), uno::UNO_QUERY );
        if( !xSourceGluePoints.is() )
            return;

        uno::Sequence< sal_Int32 > aSourceIdSequence( xSourceGluePoints->getIdentifiers() );
        const sal_Int32 nSourceCount(aSourceIdSequence.getLength());
        rtl::Reference< XMLShapeImportHelper > xSourceShapeImportHelper(const_cast< SdXMLShapeContext* >(pCandidate)->GetImport().GetShapeImport());

        if(nSourceCount)
        {
            // rCandidate has GluePoints; prepare the GluePoint container for the local shape
            uno::Reference< drawing::XGluePointsSupplier > xSupplier( mxShape, uno::UNO_QUERY );
            if( !xSupplier.is() )
                return;

            mxGluePoints.set( xSupplier->getGluePoints(), UNO_QUERY );

            if( !mxGluePoints.is() )
                return;

            drawing::GluePoint2 aSourceGluePoint;

            for( sal_Int32 nSourceIndex(0); nSourceIndex < nSourceCount; nSourceIndex++ )
            {
                const sal_Int32 nSourceIdentifier = aSourceIdSequence[nSourceIndex];

                // loop over GluePoints which are UserDefined (avoid the auto mapped ones)
                if((xSourceGluePoints->getByIdentifier( nSourceIdentifier ) >>= aSourceGluePoint)
                    && aSourceGluePoint.IsUserDefined)
                {
                    // get original mappingID back, this is the draw:id imported with a draw:glue-point
                    const sal_Int32 nDestinnationId = xSourceShapeImportHelper->findGluePointMapping(
                        pCandidate->getShape(),
                        nSourceIdentifier );

                    if(-1 != nSourceIdentifier)
                    {
                        // if we got that we are able to add a copy of that GluePoint to the local
                        // context and xShape since we have all information that the source shape
                        // and context had at import time
                        try
                        {
                            const sal_Int32 nInternalId = mxGluePoints->insert( uno::makeAny( aSourceGluePoint ) );
                            GetImport().GetShapeImport()->addGluePointMapping( mxShape, nDestinnationId, nInternalId );
                        }
                        catch (const uno::Exception& e)
                        {
                            SAL_WARN("xmloff", "exception during setting of glue points: " << e.Message);
                        }
                    }
                }
            }
        }
    }
}


SdXMLRectShapeContext::SdXMLRectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    mnRadius( 0L )
{
}

SdXMLRectShapeContext::~SdXMLRectShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLRectShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CORNER_RADIUS ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRadius, rValue);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLRectShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.RectangleShape");
    if(mxShape.is())
    {
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
                    OSL_FAIL( "exception during setting of corner radius!");
                }
            }
        }
        SdXMLShapeContext::StartElement(xAttrList);
    }
}


SdXMLLineShapeContext::SdXMLLineShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    mnX1( 0L ),
    mnY1( 0L ),
    mnX2( 1L ),
    mnY2( 1L )
{
}

SdXMLLineShapeContext::~SdXMLLineShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLLineShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_X1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnX1, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnY1, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_X2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnX2, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnY2, rValue);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLLineShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // #85920# use SetTransformation() to handle import of simple lines.
    // This is necessary to take into account all anchor positions and
    // other things. All shape imports use the same import schemata now.
    // create necessary shape (Line Shape)
    AddShape("com.sun.star.drawing.PolyLineShape");

    if(mxShape.is())
    {
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
            drawing::PointSequenceSequence aPolyPoly(1L);
            drawing::PointSequence* pOuterSequence = aPolyPoly.getArray();
            pOuterSequence->realloc(2L);
            awt::Point* pInnerSequence = pOuterSequence->getArray();

            *pInnerSequence = awt::Point( mnX1 - aTopLeft.X, mnY1 - aTopLeft.Y);
            pInnerSequence++;
            *pInnerSequence = awt::Point( mnX2 - aTopLeft.X, mnY2 - aTopLeft.Y);

            xPropSet->setPropertyValue("Geometry", Any(aPolyPoly));
        }

        // set sizes for transformation
        maSize.Width = aBottomRight.X - aTopLeft.X;
        maSize.Height = aBottomRight.Y - aTopLeft.Y;
        maPosition.X = aTopLeft.X;
        maPosition.Y = aTopLeft.Y;

        // set pos, size, shear and rotate and get copy of matrix
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}


SdXMLEllipseShapeContext::SdXMLEllipseShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    mnCX( 0L ),
    mnCY( 0L ),
    mnRX( 1L ),
    mnRY( 1L ),
    meKind( drawing::CircleKind_FULL ),
    mnStartAngle( 0 ),
    mnEndAngle( 0 )
{
}

SdXMLEllipseShapeContext::~SdXMLEllipseShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLEllipseShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_RX ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRX, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_RY ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRY, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_CX ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnCX, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_CY ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnCY, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_R ) )
        {
            // single radius, it's a circle and both radii are the same
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRX, rValue);
            mnRY = mnRX;
            return;
        }
    }
    else if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_KIND ) )
        {
            sal_uInt16 eKind;
            if( SvXMLUnitConverter::convertEnum( eKind, rValue, aXML_CircleKind_EnumMap ) )
            {
                meKind = eKind;
            }
            return;
        }
        if( IsXMLToken( rLocalName, XML_START_ANGLE ) )
        {
            double dStartAngle;
            if (::sax::Converter::convertDouble( dStartAngle, rValue ))
                mnStartAngle = (sal_Int32)(dStartAngle * 100.0);
            return;
        }
        if( IsXMLToken( rLocalName, XML_END_ANGLE ) )
        {
            double dEndAngle;
            if (::sax::Converter::convertDouble( dEndAngle, rValue ))
                mnEndAngle = (sal_Int32)(dEndAngle * 100.0);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLEllipseShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.EllipseShape");
    if(mxShape.is())
    {
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
                xPropSet->setPropertyValue("CircleKind", Any( (drawing::CircleKind)meKind) );
                xPropSet->setPropertyValue("CircleStartAngle", Any(mnStartAngle) );
                xPropSet->setPropertyValue("CircleEndAngle", Any(mnEndAngle) );
            }
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}


SdXMLPolygonShapeContext::SdXMLPolygonShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes, bool bClosed, bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    mbClosed( bClosed )
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLPolygonShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_VIEWBOX ) )
        {
            maViewBox = rValue;
            return;
        }
    }
    else if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_POINTS ) )
        {
            maPoints = rValue;
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

SdXMLPolygonShapeContext::~SdXMLPolygonShapeContext()
{
}

void SdXMLPolygonShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // Add, set Style and properties from base shape
    if(mbClosed)
        AddShape("com.sun.star.drawing.PolyPolygonShape");
    else
        AddShape("com.sun.star.drawing.PolyLineShape");

    if( mxShape.is() )
    {
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

                if(basegfx::tools::importFromSvgPoints(aPolygon, maPoints))
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
                                basegfx::tools::createSourceRangeTargetRangeTransform(
                                    aSourceRange,
                                    aTargetRange));
                        }

                        css::drawing::PointSequenceSequence aPointSequenceSequence;
                        basegfx::tools::B2DPolyPolygonToUnoPointSequenceSequence(basegfx::B2DPolyPolygon(aPolygon), aPointSequenceSequence);
                        xPropSet->setPropertyValue("Geometry", Any(aPointSequenceSequence));
                    }
                }
            }
        }

        // set pos, size, shear and rotate and get copy of matrix
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}


SdXMLPathShapeContext::SdXMLPathShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLPathShapeContext::~SdXMLPathShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLPathShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_VIEWBOX ) )
        {
            maViewBox = rValue;
            return;
        }
        else if( IsXMLToken( rLocalName, XML_D ) )
        {
            maD = rValue;
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLPathShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create polygon shape
    if(!maD.isEmpty())
    {
        const SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
        basegfx::B2DVector aSize(aViewBox.GetWidth(), aViewBox.GetHeight());

        // Is this correct? It overrides ViewBox stuff; OTOH it makes no
        // sense to have the geometry content size different from object size
        if(maSize.Width != 0 && maSize.Height != 0)
        {
            aSize = basegfx::B2DVector(maSize.Width, maSize.Height);
        }

        basegfx::B2DPolyPolygon aPolyPolygon;

        if(basegfx::tools::importFromSvgD(aPolyPolygon, maD, GetImport().needFixPositionAfterZ(), nullptr))
        {
            if(aPolyPolygon.count())
            {
                const basegfx::B2DRange aSourceRange(
                    aViewBox.GetX(), aViewBox.GetY(),
                    aViewBox.GetX() + aViewBox.GetWidth(), aViewBox.GetY() + aViewBox.GetHeight());
                const basegfx::B2DRange aTargetRange(
                    aViewBox.GetX(), aViewBox.GetY(),
                    aViewBox.GetX() + aSize.getX(), aViewBox.GetY() + aSize.getY());

                if(!aSourceRange.equal(aTargetRange))
                {
                    aPolyPolygon.transform(
                        basegfx::tools::createSourceRangeTargetRangeTransform(
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
                if( mxShape.is() )
                {
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

                            basegfx::tools::B2DPolyPolygonToUnoPolyPolygonBezierCoords(
                                aPolyPolygon,
                                aSourcePolyPolygon);
                            aAny <<= aSourcePolyPolygon;
                        }
                        else
                        {
                            drawing::PointSequenceSequence aSourcePolyPolygon;

                            basegfx::tools::B2DPolyPolygonToUnoPointSequenceSequence(
                                aPolyPolygon,
                                aSourcePolyPolygon);
                            aAny <<= aSourcePolyPolygon;
                        }

                        xPropSet->setPropertyValue("Geometry", aAny);
                    }

                    // set pos, size, shear and rotate
                    SetTransformation();

                    SdXMLShapeContext::StartElement(xAttrList);
                }
            }
        }
    }
}


SdXMLTextBoxShapeContext::SdXMLTextBoxShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, false/*bTemporaryShape*/ ),
    mnRadius(0),
    maChainNextName("")
{
}

SdXMLTextBoxShapeContext::~SdXMLTextBoxShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLTextBoxShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CORNER_RADIUS ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRadius, rValue);
            return;
        }

        if( IsXMLToken( rLocalName, XML_CHAIN_NEXT_NAME ) )
        {
            maChainNextName = rValue;
            return;
        }

    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLTextBoxShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>&)
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
            if( IsXMLToken( maPresentationClass, XML_PRESENTATION_SUBTITLE ))
            {
                // XmlShapeTypePresSubtitleShape
                service = "com.sun.star.presentation.SubtitleShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_OUTLINE ) )
            {
                // XmlShapeTypePresOutlinerShape
                service = "com.sun.star.presentation.OutlinerShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_NOTES ) )
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
            else //  IsXMLToken( maPresentationClass, XML_PRESENTATION_TITLE ) )
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

    if( mxShape.is() )
    {
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
            OUString aEmpty;
            xText->setString( aEmpty );
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
                    OSL_FAIL( "exception during setting of corner radius!");
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
                    OSL_FAIL( "exception during setting of name of next chain link!");
                }
            }
        }

        SdXMLShapeContext::StartElement(mxAttrList);
    }
}


SdXMLControlShapeContext::SdXMLControlShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLControlShapeContext::~SdXMLControlShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLControlShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CONTROL ) )
        {
            maFormId = rValue;
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLControlShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Control shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.ControlShape");
    if( mxShape.is() )
    {
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

        SdXMLShapeContext::StartElement(xAttrList);
    }
}


SdXMLConnectorShapeContext::SdXMLConnectorShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    maStart(0,0),
    maEnd(1,1),
    mnType( (sal_uInt16)drawing::ConnectorType_STANDARD ),
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
void SdXMLConnectorShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_DRAW:
    {
        if( IsXMLToken( rLocalName, XML_START_SHAPE ) )
        {
            maStartShapeId = rValue;
            return;
        }
        if( IsXMLToken( rLocalName, XML_START_GLUE_POINT ) )
        {
            mnStartGlueId = rValue.toInt32();
            return;
        }
        if( IsXMLToken( rLocalName, XML_END_SHAPE ) )
        {
            maEndShapeId = rValue;
            return;
        }
        if( IsXMLToken( rLocalName, XML_END_GLUE_POINT ) )
        {
            mnEndGlueId = rValue.toInt32();
            return;
        }
        if( IsXMLToken( rLocalName, XML_LINE_SKEW ) )
        {
            SvXMLTokenEnumerator aTokenEnum( rValue );
            OUString aToken;
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
            return;
        }
        if( IsXMLToken( rLocalName, XML_TYPE ) )
        {
            (void)SvXMLUnitConverter::convertEnum( mnType, rValue, aXML_ConnectionKind_EnumMap );
            return;
        }
        // #121965# draw:transform may be used in ODF1.2, e.g. exports from MS seem to use these
        else if( IsXMLToken( rLocalName, XML_TRANSFORM ) )
        {
            mnTransform.SetString(rValue, GetImport().GetMM100UnitConverter());
        }
    }
    break;

    case XML_NAMESPACE_SVG:
    {
        if( IsXMLToken( rLocalName, XML_X1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maStart.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maStart.Y, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_X2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maEnd.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maEnd.Y, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_D ) )
        {
            basegfx::B2DPolyPolygon aPolyPolygon;

            if(basegfx::tools::importFromSvgD(aPolyPolygon, rValue, GetImport().needFixPositionAfterZ(), nullptr))
            {
                if(aPolyPolygon.count())
                {
                    drawing::PolyPolygonBezierCoords aSourcePolyPolygon;

                    basegfx::tools::B2DPolyPolygonToUnoPolyPolygonBezierCoords(
                        aPolyPolygon,
                        aSourcePolyPolygon);
                    maPath <<= aSourcePolyPolygon;
                }
            }
        }
    }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLConnectorShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
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

    if(bDoAdd)
    {
        // create Connector shape
        // add, set style and properties from base shape
        AddShape("com.sun.star.drawing.ConnectorShape");
        if(mxShape.is())
        {
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
                xProps->setPropertyValue("EdgeKind", Any((drawing::ConnectorType)mnType) );
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

            SdXMLShapeContext::StartElement(xAttrList);
        }
    }
}


SdXMLMeasureShapeContext::SdXMLMeasureShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    maStart(0,0),
    maEnd(1,1)
{
}

SdXMLMeasureShapeContext::~SdXMLMeasureShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLMeasureShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_SVG:
    {
        if( IsXMLToken( rLocalName, XML_X1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maStart.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maStart.Y, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_X2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maEnd.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maEnd.Y, rValue);
            return;
        }
    }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLMeasureShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Measure shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.MeasureShape");
    if(mxShape.is())
    {
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
            const OUString aEmpty(  " "  );
            xText->setString( aEmpty );
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

void SdXMLMeasureShapeContext::EndElement()
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

        const OUString aEmpty;
        xCursor->collapseToStart();
        xCursor->goRight( 1, true );
        xCursor->setString( aEmpty );
    }
    while(false);

    SdXMLShapeContext::EndElement();
}


SdXMLPageShapeContext::SdXMLPageShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ), mnPageNumber(0)
{
    mbClearDefaultAttributes = false;
}

SdXMLPageShapeContext::~SdXMLPageShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLPageShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_PAGE_NUMBER ) )
        {
            mnPageNumber = rValue.toInt32();
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLPageShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Page shape
    // add, set style and properties from base shape

    // #86163# take into account which type of PageShape needs to
    // be constructed. It's an pres shape if presentation:XML_CLASS == XML_PRESENTATION_PAGE.
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
        if(bIsPresentation && !IsXMLToken( maPresentationClass, XML_PRESENTATION_PAGE ) )
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

    if(mxShape.is())
    {
        SetStyle();
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
            const OUString aPageNumberStr("PageNumber");
            if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(aPageNumberStr))
                xPropSet->setPropertyValue(aPageNumberStr, uno::makeAny( mnPageNumber ));
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}


SdXMLCaptionShapeContext::SdXMLCaptionShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    // #86616# for correct edge rounding import mnRadius needs to be initialized
    mnRadius( 0L )
{
}

SdXMLCaptionShapeContext::~SdXMLCaptionShapeContext()
{
}

void SdXMLCaptionShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Caption shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.CaptionShape");
    if( mxShape.is() )
    {
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
                    OSL_FAIL( "exception during setting of corner radius!");
                }
            }
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLCaptionShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CAPTION_POINT_X ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maCaptionPoint.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_CAPTION_POINT_Y ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    maCaptionPoint.Y, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_CORNER_RADIUS ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasureToCore(
                    mnRadius, rValue);
            return;
        }
    }
    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}


SdXMLGraphicObjectShapeContext::SdXMLGraphicObjectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, false/*bTemporaryShape*/ ),
    maURL()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLGraphicObjectShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_XLINK == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_HREF ) )
        {
            maURL = rValue;
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLGraphicObjectShapeContext::StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& )
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

    if(mxShape.is())
    {
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
                    uno::Any aAny;
                    aAny <<= GetImport().ResolveGraphicObjectURL( maURL, GetImport().isGraphicLoadOnDemandSupported() );
                    try
                    {
                        xPropset->setPropertyValue("GraphicURL", aAny );
                        xPropset->setPropertyValue("GraphicStreamURL", aAny );
                    }
                    catch (const lang::IllegalArgumentException&)
                    {
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

        SdXMLShapeContext::StartElement(mxAttrList);
    }
}

void SdXMLGraphicObjectShapeContext::EndElement()
{
    if( mxBase64Stream.is() )
    {
        OUString sURL( GetImport().ResolveGraphicObjectURLFromBase64( mxBase64Stream ) );
        if( !sURL.isEmpty() )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
                if(xProps.is())
                {
                    const uno::Any aAny( uno::makeAny( sURL ) );
                    xProps->setPropertyValue("GraphicURL", aAny );
                    xProps->setPropertyValue("GraphicStreamURL", aAny );
                }
            }
            catch (const lang::IllegalArgumentException&)
            {
            }
        }
    }

    SdXMLShapeContext::EndElement();
}

SvXMLImportContext* SdXMLGraphicObjectShapeContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;

    if( (XML_NAMESPACE_OFFICE == nPrefix) &&
             xmloff::token::IsXMLToken( rLocalName, xmloff::token::XML_BINARY_DATA ) )
    {
        if( maURL.isEmpty() && !mxBase64Stream.is() )
        {
            mxBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( mxBase64Stream.is() )
                pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    mxBase64Stream );
        }
    }

    // delegate to parent class if no context could be created
    if ( nullptr == pContext )
        pContext = SdXMLShapeContext::CreateChildContext(nPrefix, rLocalName,
                                                         xAttrList);

    return pContext;
}

SdXMLGraphicObjectShapeContext::~SdXMLGraphicObjectShapeContext()
{

}


SdXMLChartShapeContext::SdXMLChartShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    mpChartContext( nullptr )
{
}

SdXMLChartShapeContext::~SdXMLChartShapeContext()
{
    delete mpChartContext;
}

void SdXMLChartShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    const bool bIsPresentation = isPresentationShape();

    AddShape(
        bIsPresentation
        ? OUString("com.sun.star.presentation.ChartShape")
        : OUString("com.sun.star.drawing.OLE2Shape"));

    if(mxShape.is())
    {
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

                const OUString aCLSID( "12DCAE26-281F-416F-a234-c3086127382e");

                xProps->setPropertyValue("CLSID", Any(aCLSID) );

                aAny = xProps->getPropertyValue("Model");
                uno::Reference< frame::XModel > xChartModel;
                if( aAny >>= xChartModel )
                {
                    mpChartContext = GetImport().GetChartImport()->CreateChartContext( GetImport(), XML_NAMESPACE_SVG, GetXMLToken(XML_CHART), xChartModel, xAttrList );
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

        SdXMLShapeContext::StartElement(xAttrList);

        if( mpChartContext )
            mpChartContext->StartElement( xAttrList );
    }
}

void SdXMLChartShapeContext::EndElement()
{
    if( mpChartContext )
        mpChartContext->EndElement();

    SdXMLShapeContext::EndElement();
}

void SdXMLChartShapeContext::Characters( const OUString& rChars )
{
    if( mpChartContext )
        mpChartContext->Characters( rChars );
}

SvXMLImportContext * SdXMLChartShapeContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    if( mpChartContext )
        return mpChartContext->CreateChildContext( nPrefix, rLocalName, xAttrList );

    return nullptr;
}


SdXMLObjectShapeContext::SdXMLObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, false/*bTemporaryShape*/ )
{
}

SdXMLObjectShapeContext::~SdXMLObjectShapeContext()
{
}

void SdXMLObjectShapeContext::StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& )
{
    // #96717# in theorie, if we don't have a url we shouldn't even
    // export this ole shape. But practical its to risky right now
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
        if( IsXMLToken( maPresentationClass, XML_PRESENTATION_CHART ) )
        {
            service = "com.sun.star.presentation.ChartShape";
        }
        else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_TABLE ) )
        {
            service = "com.sun.star.presentation.CalcShape";
        }
        else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_OBJECT ) )
        {
            service = "com.sun.star.presentation.OLE2Shape";
        }
    }

    AddShape(service);

    if( mxShape.is() )
    {
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
                    const OUString  sURL( "vnd.sun.star.EmbeddedObject:" );

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
}

void SdXMLObjectShapeContext::EndElement()
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
        const OUString  sURL( "vnd.sun.star.EmbeddedObject:" );

        aPersistName = aPersistName.copy( sURL.getLength() );

        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
        if( xProps.is() )
            xProps->setPropertyValue("PersistName", uno::makeAny( aPersistName ) );
    }

    SdXMLShapeContext::EndElement();
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLObjectShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_DRAW:
        if( IsXMLToken( rLocalName, XML_CLASS_ID ) )
        {
            maCLSID = rValue;
            return;
        }
        break;
    case XML_NAMESPACE_XLINK:
        if( IsXMLToken( rLocalName, XML_HREF ) )
        {
            maHref = rValue;
            return;
        }
        break;
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

SvXMLImportContext* SdXMLObjectShapeContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;

    if((XML_NAMESPACE_OFFICE == nPrefix) && IsXMLToken(rLocalName, XML_BINARY_DATA))
    {
        mxBase64Stream = GetImport().GetStreamForEmbeddedObjectURLFromBase64();
        if( mxBase64Stream.is() )
            pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                rLocalName, xAttrList,
                                                mxBase64Stream );
    }
    else if( ((XML_NAMESPACE_OFFICE == nPrefix) && IsXMLToken(rLocalName, XML_DOCUMENT)) ||
                ((XML_NAMESPACE_MATH == nPrefix) && IsXMLToken(rLocalName, XML_MATH)) )
    {
        XMLEmbeddedObjectImportContext *pEContext =
            new XMLEmbeddedObjectImportContext( GetImport(), nPrefix,
                                                rLocalName, xAttrList );
        maCLSID = pEContext->GetFilterCLSID();
        if( !maCLSID.isEmpty() )
        {
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if( xPropSet.is() )
            {
                xPropSet->setPropertyValue("CLSID", uno::makeAny( maCLSID ) );

                uno::Reference< lang::XComponent > xComp;
                xPropSet->getPropertyValue("Model") >>= xComp;
                SAL_WARN_IF( !xComp.is(), "xmloff", "no xModel for own OLE format" );
                pEContext->SetComponent( xComp );
            }
        }
        pContext = pEContext;
    }

    // delegate to parent class if no context could be created
    if(!pContext)
        pContext = SdXMLShapeContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}


SdXMLAppletShapeContext::SdXMLAppletShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, false/*bTemporaryShape*/ ),
  mbIsScript( false )
{
}

SdXMLAppletShapeContext::~SdXMLAppletShapeContext()
{
}

void SdXMLAppletShapeContext::StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& )
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
void SdXMLAppletShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_DRAW:
        if( IsXMLToken( rLocalName, XML_APPLET_NAME ) )
        {
            maAppletName = rValue;
            return;
        }
        if( IsXMLToken( rLocalName, XML_CODE ) )
        {
            maAppletCode = rValue;
            return;
        }
        if( IsXMLToken( rLocalName, XML_MAY_SCRIPT ) )
        {
            mbIsScript = IsXMLToken( rValue, XML_TRUE );
            return;
        }
        break;
    case XML_NAMESPACE_XLINK:
        if( IsXMLToken( rLocalName, XML_HREF ) )
        {
            maHref = GetImport().GetAbsoluteReference(rValue);
            return;
        }
        break;
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLAppletShapeContext::EndElement()
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

        if( maParams.getLength() )
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

    SdXMLShapeContext::EndElement();
}

SvXMLImportContext * SdXMLAppletShapeContext::CreateChildContext( sal_uInt16 p_nPrefix, const OUString& rLocalName, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    if( p_nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_PARAM ) )
    {
        OUString aParamName, aParamValue;
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        // now parse the attribute list and look for draw:name and draw:value
        for(sal_Int16 a(0); a < nAttrCount; a++)
        {
            const OUString& rAttrName = xAttrList->getNameByIndex(a);
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);
            const OUString aValue( xAttrList->getValueByIndex(a) );

            if( nPrefix == XML_NAMESPACE_DRAW )
            {
                if( IsXMLToken( aLocalName, XML_NAME ) )
                {
                    aParamName = aValue;
                }
                else if( IsXMLToken( aLocalName, XML_VALUE ) )
                {
                    aParamValue = aValue;
                }
            }
        }

        if( !aParamName.isEmpty() )
        {
            sal_Int32 nIndex = maParams.getLength();
            maParams.realloc( nIndex + 1 );
            maParams[nIndex].Name = aParamName;
            maParams[nIndex].Handle = -1;
            maParams[nIndex].Value <<= aParamValue;
            maParams[nIndex].State = beans::PropertyState_DIRECT_VALUE;
        }

        return new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
    }

    return SdXMLShapeContext::CreateChildContext( p_nPrefix, rLocalName, xAttrList );
}


SdXMLPluginShapeContext::SdXMLPluginShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes) :
SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, false/*bTemporaryShape*/ ),
mbMedia( false )
{
}

SdXMLPluginShapeContext::~SdXMLPluginShapeContext()
{
}

void SdXMLPluginShapeContext::StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList)
{

    // watch for MimeType attribute to see if we have a media object
    for( sal_Int16 n = 0, nAttrCount = ( xAttrList.is() ? xAttrList->getLength() : 0 ); n < nAttrCount; ++n )
    {
        OUString    aLocalName;
        sal_uInt16  nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( n ), &aLocalName );

        if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( aLocalName, XML_MIME_TYPE ) )
        {
            if( xAttrList->getValueByIndex( n ) == "application/vnd.sun.star.media" )
                mbMedia = true;
#if HAVE_FEATURE_GLTF
            if( xAttrList->getValueByIndex( n ) == "model/vnd.gltf+json" )
                mbMedia = true;
#endif
            // leave this loop
            n = nAttrCount - 1;
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
            if( IsXMLToken( maPresentationClass, XML_PRESENTATION_OBJECT ) )
            {
                service = "com.sun.star.presentation.MediaShape";
            }
        }
    }
    else
        service = "com.sun.star.drawing.PluginShape";

    AddShape(service);

    if( mxShape.is() )
    {
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
void SdXMLPluginShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_DRAW:
        if( IsXMLToken( rLocalName, XML_MIME_TYPE ) )
        {
            maMimeType = rValue;
            return;
        }
        break;
    case XML_NAMESPACE_XLINK:
        if( IsXMLToken( rLocalName, XML_HREF ) )
        {
            maHref = lcl_GetMediaReference(GetImport(), rValue);
            return;
        }
        break;
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLPluginShapeContext::EndElement()
{
    uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );

    if( xProps.is() )
    {
        if ( maSize.Width && maSize.Height )
        {
            const OUString sVisibleArea(  "VisibleArea"  );
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
            if( maParams.getLength() )
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

            for( sal_Int32 nParam = 0; nParam < maParams.getLength(); ++nParam )
            {
                const OUString& rName = maParams[ nParam ].Name;

                if( rName == "Loop" )
                {
                    OUString aValueStr;
                    maParams[ nParam ].Value >>= aValueStr;
                    xProps->setPropertyValue("Loop",
                        uno::makeAny( aValueStr == "true" ) );
                }
                else if( rName == "Mute" )
                {
                    OUString aValueStr;
                    maParams[ nParam ].Value >>= aValueStr;
                    xProps->setPropertyValue("Mute",
                        uno::makeAny( aValueStr == "true" ) );
                }
                else if( rName == "VolumeDB" )
                {
                    OUString aValueStr;
                    maParams[ nParam ].Value >>= aValueStr;
                    xProps->setPropertyValue("VolumeDB",
                                                uno::makeAny( static_cast< sal_Int16 >( aValueStr.toInt32() ) ) );
                }
                else if( rName == "Zoom" )
                {
                    OUString            aZoomStr;
                    media::ZoomLevel    eZoomLevel;

                    maParams[ nParam ].Value >>= aZoomStr;

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

    SdXMLShapeContext::EndElement();
}

SvXMLImportContext * SdXMLPluginShapeContext::CreateChildContext( sal_uInt16 p_nPrefix, const OUString& rLocalName, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList )
{
    if( p_nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_PARAM ) )
    {
        OUString aParamName, aParamValue;
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        // now parse the attribute list and look for draw:name and draw:value
        for(sal_Int16 a(0); a < nAttrCount; a++)
        {
            const OUString& rAttrName = xAttrList->getNameByIndex(a);
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);
            const OUString aValue( xAttrList->getValueByIndex(a) );

            if( nPrefix == XML_NAMESPACE_DRAW )
            {
                if( IsXMLToken( aLocalName, XML_NAME ) )
                {
                    aParamName = aValue;
                }
                else if( IsXMLToken( aLocalName, XML_VALUE ) )
                {
                    aParamValue = aValue;
                }
            }

            if( !aParamName.isEmpty() )
            {
                sal_Int32 nIndex = maParams.getLength();
                maParams.realloc( nIndex + 1 );
                maParams[nIndex].Name = aParamName;
                maParams[nIndex].Handle = -1;
                maParams[nIndex].Value <<= aParamValue;
                maParams[nIndex].State = beans::PropertyState_DIRECT_VALUE;
            }
        }

        return new SvXMLImportContext( GetImport(), p_nPrefix, rLocalName );
    }

    return SdXMLShapeContext::CreateChildContext( p_nPrefix, rLocalName, xAttrList );
}


SdXMLFloatingFrameShapeContext::SdXMLFloatingFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, false/*bTemporaryShape*/ )
{
}

SdXMLFloatingFrameShapeContext::~SdXMLFloatingFrameShapeContext()
{
}

void SdXMLFloatingFrameShapeContext::StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& )
{
    AddShape("com.sun.star.drawing.FrameShape");

    if( mxShape.is() )
    {
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
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLFloatingFrameShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_DRAW:
        if( IsXMLToken( rLocalName, XML_FRAME_NAME ) )
        {
            maFrameName = rValue;
            return;
        }
        break;
    case XML_NAMESPACE_XLINK:
        if( IsXMLToken( rLocalName, XML_HREF ) )
        {
            maHref = GetImport().GetAbsoluteReference(rValue);
            return;
        }
        break;
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLFloatingFrameShapeContext::EndElement()
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
    SdXMLShapeContext::EndElement();
}


SdXMLFrameShapeContext::SdXMLFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
        css::uno::Reference< css::drawing::XShapes >& rShapes,
        bool bTemporaryShape)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    MultiImageImportHelper(),
    mbSupportsReplacement( false ),
    mxImplContext(),
    mxReplImplContext()
{
    uno::Reference < util::XCloneable > xClone( xAttrList, uno::UNO_QUERY );
    if( xClone.is() )
        mxAttrList.set( xClone->createClone(), uno::UNO_QUERY );
    else
        mxAttrList = new SvXMLAttributeList( xAttrList );

}

SdXMLFrameShapeContext::~SdXMLFrameShapeContext()
{
}

void SdXMLFrameShapeContext::removeGraphicFromImportContext(const SvXMLImportContext& rContext) const
{
    const SdXMLGraphicObjectShapeContext* pSdXMLGraphicObjectShapeContext = dynamic_cast< const SdXMLGraphicObjectShapeContext* >(&rContext);

    if(pSdXMLGraphicObjectShapeContext)
    {
        try
        {
            uno::Reference< container::XChild > xChild(pSdXMLGraphicObjectShapeContext->getShape(), uno::UNO_QUERY_THROW);

            if(xChild.is())
            {
                uno::Reference< drawing::XShapes > xParent(xChild->getParent(), uno::UNO_QUERY_THROW);

                if(xParent.is())
                {
                    // remove from parent
                    xParent->remove(pSdXMLGraphicObjectShapeContext->getShape());

                    // dispose
                    uno::Reference< lang::XComponent > xComp(pSdXMLGraphicObjectShapeContext->getShape(), UNO_QUERY);

                    if(xComp.is())
                    {
                        xComp->dispose();
                    }
                }
            }
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Error in cleanup of multiple graphic object import (!)" );
        }
    }
}

OUString SdXMLFrameShapeContext::getGraphicURLFromImportContext(const SvXMLImportContext& rContext) const
{
    OUString aRetval;
    const SdXMLGraphicObjectShapeContext* pSdXMLGraphicObjectShapeContext = dynamic_cast< const SdXMLGraphicObjectShapeContext* >(&rContext);

    if(pSdXMLGraphicObjectShapeContext)
    {
        try
        {
            const uno::Reference< beans::XPropertySet > xPropSet(pSdXMLGraphicObjectShapeContext->getShape(), uno::UNO_QUERY_THROW);

            if(xPropSet.is())
            {
                xPropSet->getPropertyValue("GraphicStreamURL") >>= aRetval;

                if(!aRetval.getLength())
                {
                    // it maybe a link, try GraphicURL
                    xPropSet->getPropertyValue("GraphicURL") >>= aRetval;
                }
            }
        }
        catch( uno::Exception& )
        {
            OSL_FAIL( "Error in cleanup of multiple graphic object import (!)" );
        }
    }

    return aRetval;
}

SvXMLImportContext *SdXMLFrameShapeContext::CreateChildContext( sal_uInt16 nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = nullptr;

    if( !mxImplContext.Is() )
    {
        SvXMLShapeContext* pShapeContext= GetImport().GetShapeImport()->CreateFrameChildContext(
                        GetImport(), nPrefix, rLocalName, xAttrList, mxShapes, mxAttrList );

        pContext = pShapeContext;

        // propagate the hyperlink to child context
        if ( !msHyperlink.isEmpty() )
            pShapeContext->setHyperlink( msHyperlink );

#if !HAVE_FEATURE_GLTF
        // Ignore gltf model if necessary and so the fallback image will be imported
        if( IsXMLToken(rLocalName, XML_PLUGIN ) )
        {
            SdXMLPluginShapeContext* pPluginContext = dynamic_cast<SdXMLPluginShapeContext*>(pShapeContext);
            if( pPluginContext && pPluginContext->getMimeType() == "model/vnd.gltf+json" )
            {
                 mxImplContext = nullptr;
                 return this;
            }
        }
#endif

        mxImplContext = pContext;
        mbSupportsReplacement = IsXMLToken(rLocalName, XML_OBJECT ) || IsXMLToken(rLocalName, XML_OBJECT_OLE);
        setSupportsMultipleContents(IsXMLToken(rLocalName, XML_IMAGE));

        if(getSupportsMultipleContents() && dynamic_cast< SdXMLGraphicObjectShapeContext* >(pContext))
        {
            addContent(*mxImplContext);
        }
    }
    else if(getSupportsMultipleContents() && XML_NAMESPACE_DRAW == nPrefix && IsXMLToken(rLocalName, XML_IMAGE))
    {
        // read another image
        pContext = GetImport().GetShapeImport()->CreateFrameChildContext(
            GetImport(), nPrefix, rLocalName, xAttrList, mxShapes, mxAttrList);
        mxImplContext = pContext;

        if(dynamic_cast< SdXMLGraphicObjectShapeContext* >(pContext))
        {
            addContent(*mxImplContext);
        }
    }
    else if( mbSupportsReplacement && !mxReplImplContext &&
             XML_NAMESPACE_DRAW == nPrefix &&
             IsXMLToken( rLocalName, XML_IMAGE ) )
    {
        // read replacement image
        SvXMLImportContext *pImplContext = &mxImplContext;
        SdXMLShapeContext *pSContext =
            dynamic_cast<SdXMLShapeContext*>( pImplContext  );
        if( pSContext )
        {
            uno::Reference < beans::XPropertySet > xPropSet(
                    pSContext->getShape(), uno::UNO_QUERY );
            if( xPropSet.is() )
            {
                pContext = new XMLReplacementImageContext( GetImport(),
                                    nPrefix, rLocalName, xAttrList, xPropSet );
                mxReplImplContext = pContext;
            }
        }
    }
    else if(
            ( nPrefix == XML_NAMESPACE_SVG &&   // #i68101#
                (IsXMLToken( rLocalName, XML_TITLE ) || IsXMLToken( rLocalName, XML_DESC ) ) ) ||
             (nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) ) ||
             (nPrefix == XML_NAMESPACE_DRAW && (IsXMLToken( rLocalName, XML_GLUE_POINT ) ||
                                                IsXMLToken( rLocalName, XML_THUMBNAIL ) ) ) )
    {
        SvXMLImportContext *pImplContext = &mxImplContext;
        pContext = dynamic_cast<SdXMLShapeContext&>(*pImplContext).CreateChildContext( nPrefix,
                                                                        rLocalName, xAttrList );
    }
    else if ( (XML_NAMESPACE_DRAW == nPrefix) && IsXMLToken( rLocalName, XML_IMAGE_MAP ) )
    {
        SdXMLShapeContext *pSContext = dynamic_cast< SdXMLShapeContext* >( &mxImplContext );
        if( pSContext )
        {
            uno::Reference < beans::XPropertySet > xPropSet( pSContext->getShape(), uno::UNO_QUERY );
            if (xPropSet.is())
            {
                pContext = new XMLImageMapContext(GetImport(), nPrefix, rLocalName, xPropSet);
            }
        }
    }
    // call parent for content
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

void SdXMLFrameShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>&)
{
    // ignore
}

void SdXMLFrameShapeContext::EndElement()
{
    // solve if multiple image child contexts were imported
    SvXMLImportContextRef const pSelectedContext(solveMultipleImages());
    const SdXMLGraphicObjectShapeContext* pShapeContext(
        dynamic_cast<const SdXMLGraphicObjectShapeContext*>(&pSelectedContext));
    if ( pShapeContext && !maShapeId.isEmpty() )
    {
        // fdo#64512 and fdo#60075 - make sure *this* shape is
        // registered for given ID
        assert( mxImplContext.Is() );
        const uno::Reference< uno::XInterface > xShape( pShapeContext->getShape() );
        GetImport().getInterfaceToIdentifierMapper().registerReferenceAlways( maShapeId, xShape );
    }

    if( !mxImplContext.Is() )
    {
        // now check if this is an empty presentation object
        sal_Int16 nAttrCount = mxAttrList.is() ? mxAttrList->getLength() : 0;
        for(sal_Int16 a(0); a < nAttrCount; a++)
        {
            OUString aLocalName;
            sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(mxAttrList->getNameByIndex(a), &aLocalName);

            if( nPrefix == XML_NAMESPACE_PRESENTATION )
            {
                if( IsXMLToken( aLocalName, XML_PLACEHOLDER ) )
                {
                    mbIsPlaceholder = IsXMLToken( mxAttrList->getValueByIndex(a), XML_TRUE );
                }
                else if( IsXMLToken( aLocalName, XML_CLASS ) )
                {
                    maPresentationClass = mxAttrList->getValueByIndex(a);
                }
            }
        }

        if( (!maPresentationClass.isEmpty()) && mbIsPlaceholder )
        {
            uno::Reference< xml::sax::XAttributeList> xEmpty;

            enum XMLTokenEnum eToken = XML_TEXT_BOX;

            if( IsXMLToken( maPresentationClass, XML_GRAPHIC ) )
            {
                eToken = XML_IMAGE;

            }
            else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_PAGE ) )
            {
                eToken = XML_PAGE_THUMBNAIL;
            }
            else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_CHART ) ||
                     IsXMLToken( maPresentationClass, XML_PRESENTATION_TABLE ) ||
                     IsXMLToken( maPresentationClass, XML_PRESENTATION_OBJECT ) )
            {
                eToken = XML_OBJECT;
            }

            mxImplContext = GetImport().GetShapeImport()->CreateFrameChildContext(
                    GetImport(), XML_NAMESPACE_DRAW, GetXMLToken( eToken ), mxAttrList, mxShapes, xEmpty );

            if( mxImplContext.Is() )
            {
                mxImplContext->StartElement( mxAttrList );
                mxImplContext->EndElement();
            }
        }
    }

    mxImplContext = nullptr;
    SdXMLShapeContext::EndElement();
}

void SdXMLFrameShapeContext::processAttribute( sal_uInt16 nPrefix,
        const OUString& rLocalName, const OUString& rValue )
{
    bool bId( false );

    switch ( nPrefix )
    {
        case XML_NAMESPACE_DRAW :
        case XML_NAMESPACE_DRAW_EXT :
            bId = IsXMLToken( rLocalName, XML_ID );
            break;
        case XML_NAMESPACE_NONE :
        case XML_NAMESPACE_XML :
            bId = IsXMLToken( rLocalName, XML_ID );
            break;
    }

    if ( bId )
        SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}


SdXMLCustomShapeContext::SdXMLCustomShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, false/*bTemporaryShape*/ )
{
    // See the XMLTextFrameContext ctor, a frame has Writer content (and not
    // editeng) if its autostyle has a parent style. Do the same for shapes as well.
    sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
    for (sal_Int16 i=0; i < nAttrCount; ++i)
    {
        const OUString& rAttrName = xAttrList->getNameByIndex(i);
        OUString aLocalName;
        sal_uInt16 nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName(rAttrName, &aLocalName);
        if (nPrefix == XML_NAMESPACE_DRAW && IsXMLToken(aLocalName, XML_STYLE_NAME))
        {
            OUString aStyleName = xAttrList->getValueByIndex(i);
            if(!aStyleName.isEmpty())
            {
                rtl::Reference<XMLTextImportHelper> xTxtImport = GetImport().GetTextImport();
                XMLPropStyleContext* pStyle = xTxtImport->FindAutoFrameStyle(aStyleName);
                if (pStyle && !pStyle->GetParentName().isEmpty())
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
void SdXMLCustomShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_ENGINE ) )
        {
            maCustomShapeEngine = rValue;
            return;
        }
        if ( IsXMLToken( rLocalName, XML_DATA ) )
        {
            maCustomShapeData = rValue;
            return;
        }
    }
    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLCustomShapeContext::StartElement( const uno::Reference< xml::sax::XAttributeList >& xAttrList )
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.CustomShape");
    if ( mxShape.is() )
    {
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
            OSL_FAIL( "could not set enhanced customshape geometry" );
        }
        SdXMLShapeContext::StartElement(xAttrList);
    }
}

void SdXMLCustomShapeContext::EndElement()
{
    // for backward compatibility, the above SetTransformation() may alraedy have
    // applied a call to SetMirroredX/SetMirroredY. This is not yet added to the
    // beans::PropertyValues in maCustomShapeGeometry. When applying these now, this
    // would be lost again.
    // TTTT: Remove again after aw080
    if(!maUsedTransformation.isIdentity())
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;

        maUsedTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

        bool bFlippedX(aScale.getX() < 0.0);
        bool bFlippedY(aScale.getY() < 0.0);

        if(bFlippedX && bFlippedY)
        {
            // when both are used it is the same as 180 degree rotation; reset
            bFlippedX = bFlippedY = false;
        }

        if(bFlippedX || bFlippedY)
        {
            OUString sName;

            if(bFlippedX)
                sName = "MirroredX";
            else
                sName = "MirroredY";

            //fdo#84043 overwrite the property if it already exists, otherwise append it
            beans::PropertyValue* pItem;
            std::vector< beans::PropertyValue >::iterator aI(maCustomShapeGeometry.begin());
            std::vector< beans::PropertyValue >::iterator aE(maCustomShapeGeometry.end());
            while (aI != aE)
            {
                if (aI->Name == sName)
                    break;
                ++aI;
            }
            if (aI != aE)
            {
                beans::PropertyValue& rItem = *aI;
                pItem = &rItem;
            }
            else
            {
                maCustomShapeGeometry.push_back(beans::PropertyValue());
                pItem = &maCustomShapeGeometry.back();
            }

            pItem->Name = sName;
            pItem->Handle = -1;
            pItem->Value <<= true;
            pItem->State = beans::PropertyState_DIRECT_VALUE;
        }
    }

    if ( !maCustomShapeGeometry.empty() )
    {
        const OUString sCustomShapeGeometry    (  "CustomShapeGeometry"  );

        // converting the vector to a sequence
        uno::Sequence< beans::PropertyValue > aSeq( comphelper::containerToSequence(maCustomShapeGeometry) );

        try
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
            if( xPropSet.is() )
            {
                xPropSet->setPropertyValue( sCustomShapeGeometry, Any(aSeq) );
            }
        }
        catch(const uno::Exception&)
        {
            OSL_FAIL( "could not set enhanced customshape geometry" );
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
                    OUString aEmptyType;
                    xDefaulter->createCustomShapeDefaults( aEmptyType );
                }
            }
        }
    }

    SdXMLShapeContext::EndElement();

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
        OSL_FAIL("could not flush after load");
    }
}

SvXMLImportContext* SdXMLCustomShapeContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = nullptr;
    if ( XML_NAMESPACE_DRAW == nPrefix )
    {
        if ( IsXMLToken( rLocalName, XML_ENHANCED_GEOMETRY ) )
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape,uno::UNO_QUERY );
            if ( xPropSet.is() )
                pContext = new XMLEnhancedCustomShapeContext( GetImport(), mxShape, nPrefix, rLocalName, maCustomShapeGeometry );
        }
    }
    // delegate to parent class if no context could be created
    if ( nullptr == pContext )
        pContext = SdXMLShapeContext::CreateChildContext( nPrefix, rLocalName,
                                                         xAttrList);
    return pContext;
}


SdXMLTableShapeContext::SdXMLTableShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const css::uno::Reference< css::xml::sax::XAttributeList>& xAttrList, css::uno::Reference< css::drawing::XShapes >& rShapes )
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, false )
{
    memset( &maTemplateStylesUsed, 0, sizeof( maTemplateStylesUsed ) );
}

SdXMLTableShapeContext::~SdXMLTableShapeContext()
{
}

void SdXMLTableShapeContext::StartElement( const css::uno::Reference< css::xml::sax::XAttributeList >& xAttrList )
{
    OUString service("com.sun.star.drawing.TableShape");

    bool bIsPresShape = !maPresentationClass.isEmpty() && GetImport().GetShapeImport()->IsPresentationShapesSupported();
    if( bIsPresShape )
    {
        if( IsXMLToken( maPresentationClass, XML_PRESENTATION_TABLE ) )
        {
            service = "com.sun.star.presentation.TableShape";
        }
    }

    AddShape(service);

    if( mxShape.is() )
    {
        SetLayer();

        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);

        if(bIsPresShape)
        {
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

        SetStyle();

        if( xProps.is() )
        {
            if( !msTemplateStyleName.isEmpty() ) try
            {
                Reference< XStyleFamiliesSupplier > xFamiliesSupp( GetImport().GetModel(), UNO_QUERY_THROW );
                Reference< XNameAccess > xFamilies( xFamiliesSupp->getStyleFamilies() );
                const OUString sFamilyName( "table"  );
                Reference< XNameAccess > xTableFamily( xFamilies->getByName( sFamilyName ), UNO_QUERY_THROW );
                Reference< XStyle > xTableStyle( xTableFamily->getByName( msTemplateStyleName ), UNO_QUERY_THROW );
                xProps->setPropertyValue("TableTemplate", Any( xTableStyle ) );
            }
            catch(const Exception&)
            {
                OSL_FAIL("SdXMLTableShapeContext::StartElement(), exception caught!");
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
                    OSL_FAIL("SdXMLTableShapeContext::StartElement(), exception caught!");
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
                mxTableImportContext = xTableImport->CreateTableContext( GetPrefix(), GetLocalName(), xColumnRowRange );

            if( mxTableImportContext.Is() )
                mxTableImportContext->StartElement( xAttrList );
        }
    }
}

void SdXMLTableShapeContext::EndElement()
{
    if( mxTableImportContext.Is() )
        mxTableImportContext->EndElement();

    SdXMLShapeContext::EndElement();

    if( mxShape.is() )
    {
        // set pos, size, shear and rotate
        SetTransformation();
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLTableShapeContext::processAttribute( sal_uInt16 nPrefix, const OUString& rLocalName, const OUString& rValue )
{
    if( nPrefix == XML_NAMESPACE_TABLE )
    {
        if( IsXMLToken( rLocalName, XML_TEMPLATE_NAME ) )
        {
            msTemplateStyleName = rValue;
        }
        else
        {
            int i = 0;
            const XMLPropertyMapEntry* pEntry = &aXMLTableShapeAttributes[0];
            while( pEntry->msApiName && (i < 6) )
            {
                if( IsXMLToken( rLocalName, pEntry->meXMLName ) )
                {
                    if( IsXMLToken( rValue, XML_TRUE ) )
                        maTemplateStylesUsed[i] = true;
                    break;
                }
                pEntry++;
                i++;
            }
        }
    }
    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

SvXMLImportContext* SdXMLTableShapeContext::CreateChildContext( sal_uInt16 nPrefix, const OUString& rLocalName, const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    if( mxTableImportContext.Is() && (nPrefix == XML_NAMESPACE_TABLE) )
        return mxTableImportContext->CreateChildContext(nPrefix, rLocalName, xAttrList);
    else
        return SdXMLShapeContext::CreateChildContext(nPrefix, rLocalName, xAttrList);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
