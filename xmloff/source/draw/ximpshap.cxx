/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <cassert>

#include <tools/debug.hxx>
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
#include <comphelper/extract.hxx>
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
#include <com/sun/star/util/XCloneable.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include "xexptran.hxx"
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/HomogenMatrix3.hpp>

#include <sax/tools/converter.hxx>

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

    
    
    if( rURL.equalsAscii( "#./" ) )
        return true;

    return false;
}

TYPEINIT1( SvXMLShapeContext, SvXMLImportContext );
TYPEINIT1( SdXMLShapeContext, SvXMLShapeContext );

SdXMLShapeContext::SdXMLShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
    : SvXMLShapeContext( rImport, nPrfx, rLocalName, bTemporaryShape )
    , mxShapes( rShapes )
    , mxAttrList(xAttrList)
    , mbListContextPushed( false )
    , mnStyleFamily(XML_STYLE_FAMILY_SD_GRAPHICS_ID)
    , mnClass(0)
    , mbIsPlaceholder(sal_False)
    , mbClearDefaultAttributes( true )
    , mbIsUserTransformed(sal_False)
    , mnZOrder(-1)
    , maSize(1, 1)
    , maPosition(0, 0)
    , maUsedTransformation()
    , mbVisible(true)
    , mbPrintable(true)
    , mbHaveXmlId(false)
{
}

SdXMLShapeContext::~SdXMLShapeContext()
{
}

SvXMLImportContext *SdXMLShapeContext::CreateChildContext( sal_uInt16 p_nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = NULL;

    
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
        
        if( !mxCursor.is() )
        {
            uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
            if( xText.is() )
            {
                UniReference < XMLTextImportHelper > xTxtImport =
                    GetImport().GetTextImport();
                mxOldCursor = xTxtImport->GetCursor();
                mxCursor = xText->createTextCursor();
                if( mxCursor.is() )
                {
                    xTxtImport->SetCursor( mxCursor );
                }

                
                
                xTxtImport->PushListContext();
                mbListContextPushed = true;
            }
        }

        
        if( mxCursor.is() )
        {
            pContext = GetImport().GetTextImport()->CreateTextChildContext(
                GetImport(), p_nPrefix, rLocalName, xAttrList );
        }
    }

    
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext( p_nPrefix, rLocalName, xAttrList );

    return pContext;
}

void SdXMLShapeContext::addGluePoint( const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    
    if( !mxGluePoints.is() )
    {
        uno::Reference< drawing::XGluePointsSupplier > xSupplier( mxShape, uno::UNO_QUERY );
        if( !xSupplier.is() )
            return;

        mxGluePoints = uno::Reference< container::XIdentifierContainer >::query( xSupplier->getGluePoints() );

        if( !mxGluePoints.is() )
            return;
    }

    drawing::GluePoint2 aGluePoint;
    aGluePoint.IsUserDefined = sal_True;
    aGluePoint.Position.X = 0;
    aGluePoint.Position.Y = 0;
    aGluePoint.Escape = drawing::EscapeDirection_SMART;
    aGluePoint.PositionAlignment = drawing::Alignment_CENTER;
    aGluePoint.IsRelative = sal_True;

    sal_Int32 nId = -1;

    
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
                    aGluePoint.IsRelative = sal_False;
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
        
        const OUString aEmpty;
        mxCursor->gotoEnd( sal_False );
        mxCursor->goLeft( 1, sal_True );
        mxCursor->setString( aEmpty );

        
        GetImport().GetTextImport()->ResetCursor();
    }

    if(mxOldCursor.is())
        GetImport().GetTextImport()->SetCursor( mxOldCursor );

    
    if (mbListContextPushed) {
        GetImport().GetTextImport()->PopListContext();
    }

    if( !msHyperlink.isEmpty() ) try
    {
        uno::Reference< beans::XPropertySet > xProp( mxShape, uno::UNO_QUERY );

        OUString sLink(  "Hyperlink"  );
        if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName( sLink ) )
            xProp->setPropertyValue( sLink, uno::Any( msHyperlink ) );
        Reference< XEventsSupplier > xEventsSupplier( mxShape, UNO_QUERY );
        const OUString sBookmark(  "Bookmark"  );

        if( xEventsSupplier.is() )
        {
            const OUString sEventType(  "EventType"  );
            const OUString sClickAction(  "ClickAction"  );

            Reference< XNameReplace > xEvents( xEventsSupplier->getEvents(), UNO_QUERY_THROW );

            uno::Sequence< beans::PropertyValue > aProperties( 3 );
            aProperties[0].Name = sEventType;
            aProperties[0].Handle = -1;
            aProperties[0].Value <<= OUString( "Presentation" );
            aProperties[0].State = beans::PropertyState_DIRECT_VALUE;

            aProperties[1].Name = sClickAction;
            aProperties[1].Handle = -1;
            aProperties[1].Value <<= ::com::sun::star::presentation::ClickAction_DOCUMENT;
            aProperties[1].State = beans::PropertyState_DIRECT_VALUE;

            aProperties[2].Name = sBookmark;
            aProperties[2].Handle = -1;
            aProperties[2].Value <<= msHyperlink;
            aProperties[2].State = beans::PropertyState_DIRECT_VALUE;

            const OUString sAPIEventName(  "OnClick"  );
            xEvents->replaceByName( sAPIEventName, Any( aProperties ) );
        }
        else
        {
            
            Reference< beans::XPropertySet > xSet( mxShape, UNO_QUERY_THROW );
            xSet->setPropertyValue( sBookmark, Any( msHyperlink ) );
            xSet->setPropertyValue("OnClick", Any( ::com::sun::star::presentation::ClickAction_DOCUMENT ) );
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
        
        mxShape = xShape;

        if(!maShapeName.isEmpty())
        {
            uno::Reference< container::XNamed > xNamed( mxShape, uno::UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( maShapeName );
        }

        UniReference< XMLShapeImportHelper > xImp( GetImport().GetShapeImport() );
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
                xSet->setPropertyValue("Visible", uno::Any( sal_False ) );

            if( !mbPrintable )
                xSet->setPropertyValue("Printable", uno::Any( sal_False ) );
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

        if( !maShapeId.isEmpty() )
        {
            uno::Reference< uno::XInterface > xRef( static_cast<uno::XInterface *>(xShape.get()) );
            GetImport().getInterfaceToIdentifierMapper().registerReference( maShapeId, xRef );
        }

        
        if(GetImport().GetShapeImport()->IsHandleProgressBarEnabled())
        {
            
            GetImport().GetProgressBarHelper()->Increment();
        }
    }

    mxLockable = uno::Reference< document::XActionLockable >::query( xShape );

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
                xShape = uno::Reference< drawing::XShape >(xServiceFact->createInstance("com.sun.star.drawing.temporaryForXMLImportOLE2Shape"), uno::UNO_QUERY);
            }
            else if (serviceName == "com.sun.star.drawing.GraphicObjectShape"
                     || serviceName == "com.sun.star.drawing.MediaShape"
                     || serviceName == "com.sun.star.presentation.MediaShape")
            {
                css::uno::Sequence<css::uno::Any> args(1);
                args[0] <<= GetImport().GetDocumentBase();
                xShape = css::uno::Reference<css::drawing::XShape>(
                    xServiceFact->createInstanceWithArguments(
                        serviceName, args),
                    css::uno::UNO_QUERY);
            }
            else
            {
                xShape = uno::Reference< drawing::XShape >(xServiceFact->createInstance(serviceName), uno::UNO_QUERY);
            }
            if( xShape.is() )
                AddShape( xShape );
        }
        catch(const uno::Exception& e)
        {
            uno::Sequence<OUString> aSeq( 1 );
            aSeq[0] = serviceName;
            GetImport().SetError( XMLERROR_FLAG_ERROR | XMLERROR_API,
                                  aSeq, e.Message, NULL );
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
                
                if(0 == maSize.Width)
                    maSize.Width = 1;
                if(0 == maSize.Height)
                    maSize.Height = 1;

                
                maUsedTransformation.scale(maSize.Width, maSize.Height);
            }

            if(maPosition.X != 0 || maPosition.Y != 0)
            {
                
                maUsedTransformation.translate(maPosition.X, maPosition.Y);
            }

            if(mnTransform.NeedsAction())
            {
                
                
                
                
                
                ::basegfx::B2DHomMatrix aMat;
                mnTransform.GetFullTransform(aMat);

                
                maUsedTransformation *= aMat;
            }

            
            uno::Any aAny;
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

            aAny <<= aMatrix;

            xPropSet->setPropertyValue(
                OUString("Transformation"), aAny);
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
            XMLPropStyleContext* pDocStyle = NULL;

            
            if(maDrawStyleName.isEmpty())
                break;

            const SvXMLStyleContext* pStyle = 0L;
            sal_Bool bAutoStyle(sal_False);

            if(GetImport().GetShapeImport()->GetAutoStylesContext())
                pStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(mnStyleFamily, maDrawStyleName);

            if(pStyle)
                bAutoStyle = sal_True;

            if(!pStyle && GetImport().GetShapeImport()->GetStylesContext())
                pStyle = GetImport().GetShapeImport()->GetStylesContext()->FindStyleChildContext(mnStyleFamily, maDrawStyleName);

            OUString aStyleName = maDrawStyleName;
            uno::Reference< style::XStyle > xStyle;

            if( pStyle && pStyle->ISA(XMLShapeStyleContext) )
            {
                pDocStyle = PTR_CAST( XMLShapeStyleContext, pStyle );

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
                    
                    uno::Any aAny;
                    aAny <<= xStyle;
                    xPropSet->setPropertyValue("Style", aAny);
                }
                catch(const uno::Exception&)
                {
                    OSL_FAIL( "could not find style for shape!" );
                }
            }

            
            if(bAutoStyle && pDocStyle)
            {
                
                pDocStyle->FillPropertySet(xPropSet);
            }

        } while(false);

        
        do
        {
            
            if( maTextStyleName.isEmpty() )
                break;

            if( NULL == GetImport().GetShapeImport()->GetAutoStylesContext())
                break;

            const SvXMLStyleContext* pTempStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(XML_STYLE_FAMILY_TEXT_PARAGRAPH, maTextStyleName);
            XMLPropStyleContext* pStyle = PTR_CAST( XMLPropStyleContext, pTempStyle ); 
            if( pStyle == NULL )
                break;

            
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
                uno::Any aAny;
                aAny <<= maLayerName;

                xPropSet->setPropertyValue("LayerName", aAny);
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

        const OUString sProperty("ThumbnailGraphicURL");

        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName( sProperty ) )
        {
            
            

            const OUString aInternalURL( GetImport().ResolveGraphicObjectURL( maThumbnailURL, sal_False ) );
            xPropSet->setPropertyValue( sProperty, uno::makeAny( aInternalURL ) );
        }
    }
    catch(const uno::Exception&)
    {
    }
}


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
            if (!mbHaveXmlId) { maShapeId = rValue; };
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
            
            
            mnTransform.SetString(rValue, GetImport().GetMM100UnitConverter());
        }

        
        else if( IsXMLToken( rLocalName, XML_TITLE ) )
        {
            maShapeTitle = rValue;
        }
        else if( IsXMLToken( rLocalName, XML_DESC ) )
        {
            maShapeDescription = rValue;
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

sal_Bool SdXMLShapeContext::isPresentationShape() const
{
    if( !maPresentationClass.isEmpty() && (const_cast<SdXMLShapeContext*>(this))->GetImport().GetShapeImport()->IsPresentationShapesSupported() )
    {
        if(XML_STYLE_FAMILY_SD_PRESENTATION_ID == mnStyleFamily)
        {
            return sal_True;
        }

        if( IsXMLToken( maPresentationClass, XML_HEADER ) || IsXMLToken( maPresentationClass, XML_FOOTER ) ||
            IsXMLToken( maPresentationClass, XML_PAGE_NUMBER ) || IsXMLToken( maPresentationClass, XML_DATE_TIME ) )
        {
            return sal_True;
        }
    }

    return sal_False;
}

void SdXMLShapeContext::onDemandRescueUsefulDataFromTemporary( const SvXMLImportContext& rCandidate )
{
    const SdXMLShapeContext* pCandidate = dynamic_cast< const SdXMLShapeContext* >(&rCandidate);

    if(!mxGluePoints.is() && pCandidate)
    {
        
        uno::Reference< drawing::XGluePointsSupplier > xSourceSupplier( pCandidate->getShape(), uno::UNO_QUERY );
        if( !xSourceSupplier.is() )
            return;

        uno::Reference< container::XIdentifierAccess > xSourceGluePoints( xSourceSupplier->getGluePoints(), uno::UNO_QUERY );
        if( !xSourceGluePoints.is() )
            return;

        uno::Sequence< sal_Int32 > aSourceIdSequence( xSourceGluePoints->getIdentifiers() );
        const sal_Int32 nSourceCount(aSourceIdSequence.getLength());
        UniReference< XMLShapeImportHelper > xSourceShapeImportHelper(const_cast< SdXMLShapeContext* >(pCandidate)->GetImport().GetShapeImport());

        if(nSourceCount)
        {
            
            uno::Reference< drawing::XGluePointsSupplier > xSupplier( mxShape, uno::UNO_QUERY );
            if( !xSupplier.is() )
                return;

            mxGluePoints = uno::Reference< container::XIdentifierContainer >::query( xSupplier->getGluePoints() );

            if( !mxGluePoints.is() )
                return;

            drawing::GluePoint2 aSourceGluePoint;

            for( sal_Int32 nSourceIndex(0); nSourceIndex < nSourceCount; nSourceIndex++ )
            {
                const sal_Int32 nSourceIdentifier = aSourceIdSequence[nSourceIndex];

                
                if((xSourceGluePoints->getByIdentifier( nSourceIdentifier ) >>= aSourceGluePoint)
                    && aSourceGluePoint.IsUserDefined)
                {
                    
                    const sal_Int32 nDestinnationId = xSourceShapeImportHelper->findGluePointMapping(
                        pCandidate->getShape(),
                        nSourceIdentifier );

                    if(-1 != nSourceIdentifier)
                    {
                        
                        
                        
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

TYPEINIT1( SdXMLRectShapeContext, SdXMLShapeContext );

SdXMLRectShapeContext::SdXMLRectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    mnRadius( 0L )
{
}

SdXMLRectShapeContext::~SdXMLRectShapeContext()
{
}


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
    
    AddShape("com.sun.star.drawing.RectangleShape");
    if(mxShape.is())
    {
        
        SetStyle();
        SetLayer();

        
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

TYPEINIT1( SdXMLLineShapeContext, SdXMLShapeContext );

SdXMLLineShapeContext::SdXMLLineShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
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
    
    
    
    
    AddShape("com.sun.star.drawing.PolyLineShape");

    if(mxShape.is())
    {
        
        SetStyle();
        SetLayer();

        
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

        
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            drawing::PointSequenceSequence aPolyPoly(1L);
            drawing::PointSequence* pOuterSequence = aPolyPoly.getArray();
            pOuterSequence->realloc(2L);
            awt::Point* pInnerSequence = pOuterSequence->getArray();
            uno::Any aAny;

            *pInnerSequence = awt::Point( mnX1 - aTopLeft.X, mnY1 - aTopLeft.Y);
            pInnerSequence++;
            *pInnerSequence = awt::Point( mnX2 - aTopLeft.X, mnY2 - aTopLeft.Y);

            aAny <<= aPolyPoly;
            xPropSet->setPropertyValue(
                OUString("Geometry"), aAny);
        }

        
        maSize.Width = aBottomRight.X - aTopLeft.X;
        maSize.Height = aBottomRight.Y - aTopLeft.Y;
        maPosition.X = aTopLeft.X;
        maPosition.Y = aTopLeft.Y;

        
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

TYPEINIT1( SdXMLEllipseShapeContext, SdXMLShapeContext );

SdXMLEllipseShapeContext::SdXMLEllipseShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
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
    
    AddShape("com.sun.star.drawing.EllipseShape");
    if(mxShape.is())
    {
        
        SetStyle();
        SetLayer();

        if(mnCX != 0 || mnCY != 0 || mnRX != 1 || mnRY != 1)
        {
            
            maSize.Width = 2 * mnRX;
            maSize.Height = 2 * mnRY;
            maPosition.X = mnCX - mnRX;
            maPosition.Y = mnCY - mnRY;
        }

        
        SetTransformation();

        if( meKind != drawing::CircleKind_FULL )
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
            if( xPropSet.is() )
            {
                uno::Any aAny;
                aAny <<= (drawing::CircleKind)meKind;
                xPropSet->setPropertyValue("CircleKind", aAny );

                aAny <<= mnStartAngle;
                xPropSet->setPropertyValue("CircleStartAngle", aAny );

                aAny <<= mnEndAngle;
                xPropSet->setPropertyValue("CircleEndAngle", aAny );
            }
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

TYPEINIT1( SdXMLPolygonShapeContext, SdXMLShapeContext );

SdXMLPolygonShapeContext::SdXMLPolygonShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes, sal_Bool bClosed, sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    mbClosed( bClosed )
{
}


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
    
    if(mbClosed)
        AddShape("com.sun.star.drawing.PolyPolygonShape");
    else
        AddShape("com.sun.star.drawing.PolyLineShape");

    if( mxShape.is() )
    {
        SetStyle();
        SetLayer();

        
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            
            if(!maPoints.isEmpty() && !maViewBox.isEmpty())
            {
                const SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
                basegfx::B2DVector aSize(aViewBox.GetWidth(), aViewBox.GetHeight());

                
                
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

                        com::sun::star::drawing::PointSequenceSequence aPointSequenceSequence;
                        uno::Any aAny;

                        basegfx::tools::B2DPolyPolygonToUnoPointSequenceSequence(basegfx::B2DPolyPolygon(aPolygon), aPointSequenceSequence);
                        aAny <<= aPointSequenceSequence;
                        xPropSet->setPropertyValue(OUString("Geometry"), aAny);
                    }
                }
            }
        }

        
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

TYPEINIT1( SdXMLPathShapeContext, SdXMLShapeContext );

SdXMLPathShapeContext::SdXMLPathShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLPathShapeContext::~SdXMLPathShapeContext()
{
}


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
    
    if(!maD.isEmpty())
    {
        const SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
        basegfx::B2DVector aSize(aViewBox.GetWidth(), aViewBox.GetHeight());

        
        
        if(maSize.Width != 0 && maSize.Height != 0)
        {
            aSize = basegfx::B2DVector(maSize.Width, maSize.Height);
        }

        basegfx::B2DPolyPolygon aPolyPolygon;

        if(basegfx::tools::importFromSvgD(aPolyPolygon, maD, GetImport().needFixPositionAfterZ(), 0))
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

                
                AddShape(service);

                
                
                
                if( mxShape.is() )
                {
                    SetStyle();
                    SetLayer();

                    
                    uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);

                    if(xPropSet.is())
                    {
                        uno::Any aAny;

                        
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

                        xPropSet->setPropertyValue(OUString("Geometry"), aAny);
                    }

                    
                    SetTransformation();

                    SdXMLShapeContext::StartElement(xAttrList);
                }
            }
        }
    }
}

TYPEINIT1( SdXMLTextBoxShapeContext, SdXMLShapeContext );

SdXMLTextBoxShapeContext::SdXMLTextBoxShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    mnRadius(0)
{
}

SdXMLTextBoxShapeContext::~SdXMLTextBoxShapeContext()
{
}


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
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

void SdXMLTextBoxShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>&)
{
    
    sal_Bool bIsPresShape = sal_False;
    bool bClearText = false;

    OUString service;

    if( isPresentationShape() )
    {
        
        if( GetImport().GetShapeImport()->IsPresentationShapesSupported() )
        {
            if( IsXMLToken( maPresentationClass, XML_PRESENTATION_SUBTITLE ))
            {
                
                service = "com.sun.star.presentation.SubtitleShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_OUTLINE ) )
            {
                
                service = "com.sun.star.presentation.OutlinerShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_NOTES ) )
            {
                
                service = "com.sun.star.presentation.NotesShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_HEADER ) )
            {
                
                service = "com.sun.star.presentation.HeaderShape";
                bClearText = true;
            }
            else if( IsXMLToken( maPresentationClass, XML_FOOTER ) )
            {
                
                service = "com.sun.star.presentation.FooterShape";
                bClearText = true;
            }
            else if( IsXMLToken( maPresentationClass, XML_PAGE_NUMBER ) )
            {
                
                service = "com.sun.star.presentation.SlideNumberShape";
                bClearText = true;
            }
            else if( IsXMLToken( maPresentationClass, XML_DATE_TIME ) )
            {
                
                service = "com.sun.star.presentation.DateTimeShape";
                bClearText = true;
            }
            else 
            {
                
                service = "com.sun.star.presentation.TitleTextShape";
            }
            bIsPresShape = sal_True;
        }
    }

    if( service.isEmpty() )
    {
        
        service = "com.sun.star.drawing.TextShape";
    }

    
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
                        xProps->setPropertyValue("IsEmptyPresentationObject", ::cppu::bool2any( false ) );

                    if( mbIsUserTransformed && xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                        xProps->setPropertyValue("IsPlaceholderDependent", ::cppu::bool2any( false ) );
                }
            }
        }

        if( bClearText )
        {
            uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
            OUString aEmpty;
            xText->setString( aEmpty );
        }

        









        
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

        SdXMLShapeContext::StartElement(mxAttrList);
    }
}

TYPEINIT1( SdXMLControlShapeContext, SdXMLShapeContext );

SdXMLControlShapeContext::SdXMLControlShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLControlShapeContext::~SdXMLControlShapeContext()
{
}


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
    
    
    AddShape("com.sun.star.drawing.ControlShape");
    if( mxShape.is() )
    {
        DBG_ASSERT( !maFormId.isEmpty(), "draw:control without a form:id attribute!" );
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

        
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

TYPEINIT1( SdXMLConnectorShapeContext, SdXMLShapeContext );

SdXMLConnectorShapeContext::SdXMLConnectorShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
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
    if ( getBuildIds( nUPD, nBuildId ) &&
       ( ( nUPD == 641 ) || ( nUPD == 645 ) || ( nUPD == 680 ) || ( nUPD == 300 ) ||
         ( nUPD == 310 ) || ( nUPD == 320 ) || ( nUPD == 330 ) || ( nUPD == 340 ) ||
         ( nUPD == 350 && nBuildId < 202 ) ) )
    {
        bWrongPositionAfterZ = true;
    }
    return bWrongPositionAfterZ;
}



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
            SvXMLUnitConverter::convertEnum( mnType, rValue, aXML_ConnectionKind_EnumMap );
            return;
        }
        
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

            if(basegfx::tools::importFromSvgD(aPolyPolygon, rValue, GetImport().needFixPositionAfterZ(), 0))
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
    
    
    
    
    sal_Bool bDoAdd(sal_True);

    if(    maStartShapeId.isEmpty()
        && maEndShapeId.isEmpty()
        && maStart.X == maEnd.X
        && maStart.Y == maEnd.Y
        && 0 == mnDelta1
        && 0 == mnDelta2
        && 0 == mnDelta3
        )
    {
        bDoAdd = sal_False;
    }

    if(bDoAdd)
    {
        
        
        AddShape("com.sun.star.drawing.ConnectorShape");
        if(mxShape.is())
        {
            
            
            if(mnTransform.NeedsAction())
            {
                
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

            
            if( !maStartShapeId.isEmpty() )
                GetImport().GetShapeImport()->addShapeConnection( mxShape, sal_True, maStartShapeId, mnStartGlueId );
            if( !maEndShapeId.isEmpty() )
                GetImport().GetShapeImport()->addShapeConnection( mxShape, sal_False, maEndShapeId, mnEndGlueId );

            uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
            if( xProps.is() )
            {
                uno::Any aAny;
                aAny <<= maStart;
                xProps->setPropertyValue("StartPosition", aAny);

                aAny <<= maEnd;
                xProps->setPropertyValue("EndPosition", aAny );

                aAny <<= (drawing::ConnectorType)mnType;
                xProps->setPropertyValue("EdgeKind", aAny );

                aAny <<= mnDelta1;
                xProps->setPropertyValue("EdgeLine1Delta", aAny );

                aAny <<= mnDelta2;
                xProps->setPropertyValue("EdgeLine2Delta", aAny );

                aAny <<= mnDelta3;
                xProps->setPropertyValue("EdgeLine3Delta", aAny );
            }
            SetStyle();
            SetLayer();

            if ( maPath.hasValue() )
            {
                
                
                
                
                bool bApplySVGD( true );
                if ( uno::Reference< text::XTextDocument >(GetImport().GetModel(), uno::UNO_QUERY).is() )
                {
                    sal_Int32 nUPD( 0 );
                    sal_Int32 nBuild( 0 );
                    const bool bBuildIdFound = GetImport().getBuildIds( nUPD, nBuild );
                    if ( GetImport().IsTextDocInOOoFileFormat() ||
                         ( bBuildIdFound &&
                           ( ( nUPD == 641 ) || ( nUPD == 645 ) ||  
                             ( nUPD == 680 ) ||                     
                             ( nUPD == 300 ) ||                     
                             ( nUPD == 310 ) ||                     
                             ( nUPD == 320 ) ) ) )                  
                    {
                        bApplySVGD = false;
                    }
                }

                if ( bApplySVGD )
                {
                    assert(maPath.getValueType() == ::getCppuType((const drawing::PolyPolygonBezierCoords*)0));
                    xProps->setPropertyValue("PolyPolygonBezier", maPath);
                }
            }

            SdXMLShapeContext::StartElement(xAttrList);
        }
    }
}

TYPEINIT1( SdXMLMeasureShapeContext, SdXMLShapeContext );

SdXMLMeasureShapeContext::SdXMLMeasureShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    maStart(0,0),
    maEnd(1,1)
{
}

SdXMLMeasureShapeContext::~SdXMLMeasureShapeContext()
{
}


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
    
    
    AddShape("com.sun.star.drawing.MeasureShape");
    if(mxShape.is())
    {
        SetStyle();
        SetLayer();

        uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
        if( xProps.is() )
        {
            uno::Any aAny;
            aAny <<= maStart;
            xProps->setPropertyValue("StartPosition", aAny);

            aAny <<= maEnd;
            xProps->setPropertyValue("EndPosition", aAny );
        }

        
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
        
        uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
        if( !xText.is() )
            break;

        uno::Reference< text::XTextCursor > xCursor( xText->createTextCursor() );
        if( !xCursor.is() )
            break;

        const OUString aEmpty;
        xCursor->collapseToStart();
        xCursor->goRight( 1, sal_True );
        xCursor->setString( aEmpty );
    }
    while(false);

    SdXMLShapeContext::EndElement();
}

TYPEINIT1( SdXMLPageShapeContext, SdXMLShapeContext );

SdXMLPageShapeContext::SdXMLPageShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ), mnPageNumber(0)
{
    mbClearDefaultAttributes = false;
}

SdXMLPageShapeContext::~SdXMLPageShapeContext()
{
}


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
    
    

    
    
    sal_Bool bIsPresentation = !maPresentationClass.isEmpty() &&
           GetImport().GetShapeImport()->IsPresentationShapesSupported();

    uno::Reference< lang::XServiceInfo > xInfo( mxShapes, uno::UNO_QUERY );
    const sal_Bool bIsOnHandoutPage = xInfo.is() && xInfo->supportsService("com.sun.star.presentation.HandoutMasterPage");

    if( bIsOnHandoutPage )
    {
        AddShape("com.sun.star.presentation.HandoutShape");
    }
    else
    {
        if(bIsPresentation && !IsXMLToken( maPresentationClass, XML_PRESENTATION_PAGE ) )
        {
            bIsPresentation = sal_False;
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

TYPEINIT1( SdXMLCaptionShapeContext, SdXMLShapeContext );

SdXMLCaptionShapeContext::SdXMLCaptionShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    
    mnRadius( 0L )
{
}

SdXMLCaptionShapeContext::~SdXMLCaptionShapeContext()
{
}

void SdXMLCaptionShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    
    
    AddShape("com.sun.star.drawing.CaptionShape");
    if( mxShape.is() )
    {
        SetStyle();
        SetLayer();

        uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );

        
        
        
        
        
        sal_Bool bIsAutoGrowWidth = sal_False;
        if ( xProps.is() )
        {
            uno::Any aAny( xProps->getPropertyValue("TextAutoGrowWidth") );
            aAny >>= bIsAutoGrowWidth;

            if ( bIsAutoGrowWidth )
                xProps->setPropertyValue("TextAutoGrowWidth", uno::makeAny( sal_False ) );
        }

        
        SetTransformation();
        if( xProps.is() )
            xProps->setPropertyValue("CaptionPoint", uno::makeAny( maCaptionPoint ) );

        if ( bIsAutoGrowWidth )
            xProps->setPropertyValue("TextAutoGrowWidth", uno::makeAny( sal_True ) );

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

TYPEINIT1( SdXMLGraphicObjectShapeContext, SdXMLShapeContext );

SdXMLGraphicObjectShapeContext::SdXMLGraphicObjectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    maURL()
{
}


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

void SdXMLGraphicObjectShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    
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
                xPropset->setPropertyValue("IsEmptyPresentationObject", ::cppu::bool2any( mbIsPlaceholder ) );

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
                        xProps->setPropertyValue("IsPlaceholderDependent", ::cppu::bool2any( false ) );
                }
            }
        }

        
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
    SvXMLImportContext* pContext = NULL;

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

    
    if ( NULL == pContext )
        pContext = SdXMLShapeContext::CreateChildContext(nPrefix, rLocalName,
                                                         xAttrList);

    return pContext;
}

SdXMLGraphicObjectShapeContext::~SdXMLGraphicObjectShapeContext()
{

}

TYPEINIT1( SdXMLChartShapeContext, SdXMLShapeContext );

SdXMLChartShapeContext::SdXMLChartShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    mpChartContext( NULL )
{
}

SdXMLChartShapeContext::~SdXMLChartShapeContext()
{
    if( mpChartContext )
        delete mpChartContext;
}

void SdXMLChartShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    const sal_Bool bIsPresentation = isPresentationShape();

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
                    xProps->setPropertyValue("IsEmptyPresentationObject", ::cppu::bool2any( false ) );

                uno::Any aAny;

                const OUString aCLSID( "12DCAE26-281F-416F-a234-c3086127382e");

                aAny <<= aCLSID;
                xProps->setPropertyValue("CLSID", aAny );

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
                        xProps->setPropertyValue("IsPlaceholderDependent", ::cppu::bool2any( false ) );
                }
            }
        }

        
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
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    if( mpChartContext )
        return mpChartContext->CreateChildContext( nPrefix, rLocalName, xAttrList );

    return NULL;
}

TYPEINIT1( SdXMLObjectShapeContext, SdXMLShapeContext );

SdXMLObjectShapeContext::SdXMLObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLObjectShapeContext::~SdXMLObjectShapeContext()
{
}

void SdXMLObjectShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    
    
    
    
    

    
    
    
    
    
    if( !(GetImport().getImportFlags() & IMPORT_EMBEDDED) && !mbIsPlaceholder && ImpIsEmptyURL(maHref) )
        return;

    OUString service("com.sun.star.drawing.OLE2Shape");

    sal_Bool bIsPresShape = !maPresentationClass.isEmpty() && GetImport().GetShapeImport()->IsPresentationShapesSupported();

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
                        xProps->setPropertyValue("IsEmptyPresentationObject", ::cppu::bool2any( false ) );

                    if( mbIsUserTransformed && xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                        xProps->setPropertyValue("IsPlaceholderDependent", ::cppu::bool2any( false ) );
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
                    
                    xProps->setPropertyValue("LinkURL",
                                              uno::makeAny( aPersistName ) );
                }
            }
        }

        
        SetTransformation();

        SetStyle();

        GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
    }
}

void SdXMLObjectShapeContext::EndElement()
{
    if (GetImport().isGeneratorVersionOlderThan(
                SvXMLImport::OOo_34x, SvXMLImport::LO_4x))
    {
        
        
        
        
        
        
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
    
    SvXMLImportContext* pContext = NULL;

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
                DBG_ASSERT( xComp.is(), "no xModel for own OLE format" );
                pEContext->SetComponent( xComp );
            }
        }
        pContext = pEContext;
    }

    
    if(!pContext)
        pContext = SdXMLShapeContext::CreateChildContext(nPrefix, rLocalName, xAttrList);

    return pContext;
}

TYPEINIT1( SdXMLAppletShapeContext, SdXMLShapeContext );

SdXMLAppletShapeContext::SdXMLAppletShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
  mbIsScript( sal_False )
{
}

SdXMLAppletShapeContext::~SdXMLAppletShapeContext()
{
}

void SdXMLAppletShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    AddShape("com.sun.star.drawing.AppletShape");

    if( mxShape.is() )
    {
        SetLayer();

        
        SetTransformation();
        GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
    }
}


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
        uno::Any aAny;

        if ( maSize.Width && maSize.Height )
        {
            
            awt::Rectangle aRect( 0, 0, maSize.Width, maSize.Height );
            aAny <<= aRect;
            xProps->setPropertyValue("VisibleArea", aAny );
        }

        if( maParams.getLength() )
        {
            aAny <<= maParams;
            xProps->setPropertyValue("AppletCommands", aAny );
        }

        if( !maHref.isEmpty() )
        {
            aAny <<= maHref;
            xProps->setPropertyValue("AppletCodeBase", aAny );
        }

        if( !maAppletName.isEmpty() )
        {
            aAny <<= maAppletName;
            xProps->setPropertyValue("AppletName", aAny );
        }

        if( mbIsScript )
        {
            aAny <<= mbIsScript;
            xProps->setPropertyValue("AppletIsScript", aAny );

        }

        if( !maAppletCode.isEmpty() )
        {
            aAny <<= maAppletCode;
            xProps->setPropertyValue("AppletCode", aAny );
        }

        aAny <<= OUString( GetImport().GetDocumentBase() );
        xProps->setPropertyValue("AppletDocBase", aAny );

        SetThumbnail();
    }

    SdXMLShapeContext::EndElement();
}

SvXMLImportContext * SdXMLAppletShapeContext::CreateChildContext( sal_uInt16 p_nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    if( p_nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_PARAM ) )
    {
        OUString aParamName, aParamValue;
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        
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

TYPEINIT1( SdXMLPluginShapeContext, SdXMLShapeContext );

SdXMLPluginShapeContext::SdXMLPluginShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape) :
SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
mbMedia( false )
{
}

SdXMLPluginShapeContext::~SdXMLPluginShapeContext()
{
}

void SdXMLPluginShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList)
{
    
    for( sal_Int16 n = 0, nAttrCount = ( xAttrList.is() ? xAttrList->getLength() : 0 ); n < nAttrCount; ++n )
    {
        OUString    aLocalName;
        sal_uInt16  nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( n ), &aLocalName );

        if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( aLocalName, XML_MIME_TYPE ) )
        {
            if( xAttrList->getValueByIndex( n ).equalsAscii( "application/vnd.sun.star.media" ) )
                mbMedia = true;

            
            n = nAttrCount - 1;
        }
    }

    OUString service;

    sal_Bool bIsPresShape = sal_False;

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
                        xProps->setPropertyValue("IsEmptyPresentationObject", ::cppu::bool2any( false ) );

                    if( mbIsUserTransformed && xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                        xProps->setPropertyValue("IsPlaceholderDependent", ::cppu::bool2any( false ) );
                }
            }
        }

        
        SetTransformation();
        GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
    }
}

static OUString
lcl_GetMediaReference(SvXMLImport const& rImport, OUString const& rURL)
{
    if (rImport.IsPackageURL(rURL))
    {
        return OUString( "vnd.sun.star.Package:") + rURL;
    }
    else
    {
        return rImport.GetAbsoluteReference(rURL);
    }
}


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
        uno::Any aAny;

        if ( maSize.Width && maSize.Height )
        {
            const OUString sVisibleArea(  "VisibleArea"  );
            uno::Reference< beans::XPropertySetInfo > aXPropSetInfo( xProps->getPropertySetInfo() );
            if ( !aXPropSetInfo.is() || aXPropSetInfo->hasPropertyByName( sVisibleArea ) )
            {
                
                awt::Rectangle aRect( 0, 0, maSize.Width, maSize.Height );
                aAny <<= aRect;
                xProps->setPropertyValue( sVisibleArea, aAny );
            }
        }

        if( !mbMedia )
        {
            
            if( maParams.getLength() )
            {
                aAny <<= maParams;
                xProps->setPropertyValue("PluginCommands", aAny );
            }

            if( !maMimeType.isEmpty() )
            {
                aAny <<= maMimeType;
                xProps->setPropertyValue("PluginMimeType", aAny );
            }

            if( !maHref.isEmpty() )
            {
                aAny <<= maHref;
                xProps->setPropertyValue("PluginURL", aAny );
            }
        }
        else
        {
            
            xProps->setPropertyValue(
                    OUString("MediaURL"),
                    uno::makeAny(maHref));

            for( sal_Int32 nParam = 0; nParam < maParams.getLength(); ++nParam )
            {
                const OUString& rName = maParams[ nParam ].Name;

                if( rName.equalsAscii( "Loop" ) )
                {
                    OUString aValueStr;
                    maParams[ nParam ].Value >>= aValueStr;
                    xProps->setPropertyValue("Loop",
                        uno::makeAny( static_cast< sal_Bool >( aValueStr.equalsAscii( "true" ) ) ) );
                }
                else if( rName.equalsAscii( "Mute" ) )
                {
                    OUString aValueStr;
                    maParams[ nParam ].Value >>= aValueStr;
                    xProps->setPropertyValue("Mute",
                        uno::makeAny( static_cast< sal_Bool >( aValueStr.equalsAscii( "true" ) ) ) );
                }
                else if( rName.equalsAscii( "VolumeDB" ) )
                {
                    OUString aValueStr;
                    maParams[ nParam ].Value >>= aValueStr;
                    xProps->setPropertyValue("VolumeDB",
                                                uno::makeAny( static_cast< sal_Int16 >( aValueStr.toInt32() ) ) );
                }
                else if( rName.equalsAscii( "Zoom" ) )
                {
                    OUString            aZoomStr;
                    media::ZoomLevel    eZoomLevel;

                    maParams[ nParam ].Value >>= aZoomStr;

                    if( aZoomStr.equalsAscii( "25%" ) )
                        eZoomLevel = media::ZoomLevel_ZOOM_1_TO_4;
                    else if( aZoomStr.equalsAscii( "50%" ) )
                        eZoomLevel = media::ZoomLevel_ZOOM_1_TO_2;
                    else if( aZoomStr.equalsAscii( "100%" ) )
                        eZoomLevel = media::ZoomLevel_ORIGINAL;
                    else if( aZoomStr.equalsAscii( "200%" ) )
                        eZoomLevel = media::ZoomLevel_ZOOM_2_TO_1;
                    else if( aZoomStr.equalsAscii( "400%" ) )
                        eZoomLevel = media::ZoomLevel_ZOOM_4_TO_1;
                    else if( aZoomStr.equalsAscii( "fit" ) )
                        eZoomLevel = media::ZoomLevel_FIT_TO_WINDOW;
                    else if( aZoomStr.equalsAscii( "fixedfit" ) )
                        eZoomLevel = media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT;
                    else if( aZoomStr.equalsAscii( "fullscreen" ) )
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

SvXMLImportContext * SdXMLPluginShapeContext::CreateChildContext( sal_uInt16 p_nPrefix, const OUString& rLocalName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    if( p_nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_PARAM ) )
    {
        OUString aParamName, aParamValue;
        const sal_Int16 nAttrCount = xAttrList.is() ? xAttrList->getLength() : 0;
        
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

TYPEINIT1( SdXMLFloatingFrameShapeContext, SdXMLShapeContext );

SdXMLFloatingFrameShapeContext::SdXMLFloatingFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLFloatingFrameShapeContext::~SdXMLFloatingFrameShapeContext()
{
}

void SdXMLFloatingFrameShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    AddShape("com.sun.star.drawing.FrameShape");

    if( mxShape.is() )
    {
        SetLayer();

        
        SetTransformation();

        uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
        if( xProps.is() )
        {
            uno::Any aAny;

            if( !maFrameName.isEmpty() )
            {
                aAny <<= maFrameName;
                xProps->setPropertyValue("FrameName", aAny );
            }

            if( !maHref.isEmpty() )
            {
                aAny <<= maHref;
                xProps->setPropertyValue("FrameURL", aAny );
            }
        }

        SetStyle();

        GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
    }
}


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
            
            awt::Rectangle aRect( 0, 0, maSize.Width, maSize.Height );
            uno::Any aAny;
            aAny <<= aRect;
            xProps->setPropertyValue("VisibleArea", aAny );
        }
    }

    SetThumbnail();
    SdXMLShapeContext::EndElement();
}

TYPEINIT1( SdXMLFrameShapeContext, SdXMLShapeContext );

SdXMLFrameShapeContext::SdXMLFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes,
        sal_Bool bTemporaryShape)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape ),
    MultiImageImportHelper(),
    mbSupportsReplacement( sal_False ),
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
                    
                    xParent->remove(pSdXMLGraphicObjectShapeContext->getShape());

                    
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
    SvXMLImportContext * pContext = 0;

    if( !mxImplContext.Is() )
    {

        SvXMLShapeContext* pShapeContext= GetImport().GetShapeImport()->CreateFrameChildContext(
                        GetImport(), nPrefix, rLocalName, xAttrList, mxShapes, mxAttrList );

        pContext = pShapeContext;

        
        if ( !msHyperlink.isEmpty() )
            pShapeContext->setHyperlink( msHyperlink );

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
        
        SvXMLImportContext *pImplContext = &mxImplContext;
        SdXMLShapeContext *pSContext =
            PTR_CAST( SdXMLShapeContext, pImplContext );
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
            ( nPrefix == XML_NAMESPACE_SVG &&   
                (IsXMLToken( rLocalName, XML_TITLE ) || IsXMLToken( rLocalName, XML_DESC ) ) ) ||
             (nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) ) ||
             (nPrefix == XML_NAMESPACE_DRAW && (IsXMLToken( rLocalName, XML_GLUE_POINT ) ||
                                                IsXMLToken( rLocalName, XML_THUMBNAIL ) ) ) )
    {
        SvXMLImportContext *pImplContext = &mxImplContext;
        pContext = PTR_CAST( SdXMLShapeContext, pImplContext )->CreateChildContext( nPrefix,
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

    
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}

void SdXMLFrameShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>&)
{
    
}

void SdXMLFrameShapeContext::EndElement()
{
    
    SvXMLImportContextRef const pSelectedContext(solveMultipleImages());
    const SdXMLGraphicObjectShapeContext* pShapeContext(
        dynamic_cast<const SdXMLGraphicObjectShapeContext*>(&pSelectedContext));
    if ( pShapeContext && !maShapeId.isEmpty() )
    {
        
        
        assert( mxImplContext.Is() );
        const uno::Reference< uno::XInterface > xShape( pShapeContext->getShape() );
        GetImport().getInterfaceToIdentifierMapper().registerReferenceAlways( maShapeId, xShape );
    }

    if( !mxImplContext.Is() )
    {
        
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

    mxImplContext = 0;
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

TYPEINIT1( SdXMLCustomShapeContext, SdXMLShapeContext );

SdXMLCustomShapeContext::SdXMLCustomShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes,
    sal_Bool bTemporaryShape)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, bTemporaryShape )
{
}

SdXMLCustomShapeContext::~SdXMLCustomShapeContext()
{
}


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
    
    AddShape("com.sun.star.drawing.CustomShape");
    if ( mxShape.is() )
    {
        
        SetStyle();
        SetLayer();

        
        SetTransformation();

        try
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
            if( xPropSet.is() )
            {
                if ( !maCustomShapeEngine.isEmpty() )
                {
                    uno::Any aAny;
                    aAny <<= maCustomShapeEngine;
                    xPropSet->setPropertyValue( EASGet( EAS_CustomShapeEngine ), aAny );
                }
                if ( !maCustomShapeData.isEmpty() )
                {
                    uno::Any aAny;
                    aAny <<= maCustomShapeData;
                    xPropSet->setPropertyValue( EASGet( EAS_CustomShapeData ), aAny );
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
    
    
    
    
    
    if(!maUsedTransformation.isIdentity())
    {
        basegfx::B2DVector aScale, aTranslate;
        double fRotate, fShearX;

        maUsedTransformation.decompose(aScale, aTranslate, fRotate, fShearX);

        bool bFlippedX(aScale.getX() < 0.0);
        bool bFlippedY(aScale.getY() < 0.0);

        if(bFlippedX && bFlippedY)
        {
            
            bFlippedX = bFlippedY = false;
        }

        if(bFlippedX || bFlippedY)
        {
            beans::PropertyValue aNewPoroperty;

            if(bFlippedX)
            {
                aNewPoroperty.Name = "MirroredX";
            }
            else
            {
                aNewPoroperty.Name = "MirroredY";
            }

            aNewPoroperty.Handle = -1;
            aNewPoroperty.Value <<= sal_True;
            aNewPoroperty.State = beans::PropertyState_DIRECT_VALUE;

            maCustomShapeGeometry.push_back(aNewPoroperty);
        }
    }

    if ( !maCustomShapeGeometry.empty() )
    {
        const OUString sCustomShapeGeometry    (  "CustomShapeGeometry"  );

        
        uno::Sequence< beans::PropertyValue > aSeq( maCustomShapeGeometry.size() );
        beans::PropertyValue* pValues = aSeq.getArray();
        std::vector< beans::PropertyValue >::const_iterator aIter( maCustomShapeGeometry.begin() );
        std::vector< beans::PropertyValue >::const_iterator aEnd( maCustomShapeGeometry.end() );
        while ( aIter != aEnd )
            *pValues++ = *aIter++;

        try
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
            if( xPropSet.is() )
            {
                uno::Any aAny;
                aAny <<= aSeq;
                xPropSet->setPropertyValue( sCustomShapeGeometry, aAny );
            }
        }
        catch(const uno::Exception&)
        {
            OSL_FAIL( "could not set enhanced customshape geometry" );
        }

        sal_Int32 nUPD( 0 );
        sal_Int32 nBuild( 0 );
        GetImport().getBuildIds( nUPD, nBuild );
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

    SdXMLShapeContext::EndElement();
}

SvXMLImportContext* SdXMLCustomShapeContext::CreateChildContext(
    sal_uInt16 nPrefix, const OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = NULL;
    if ( XML_NAMESPACE_DRAW == nPrefix )
    {
        if ( IsXMLToken( rLocalName, XML_ENHANCED_GEOMETRY ) )
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape,uno::UNO_QUERY );
            if ( xPropSet.is() )
                pContext = new XMLEnhancedCustomShapeContext( GetImport(), mxShape, nPrefix, rLocalName, maCustomShapeGeometry );
        }
    }
    
    if ( NULL == pContext )
        pContext = SdXMLShapeContext::CreateChildContext( nPrefix, rLocalName,
                                                         xAttrList);
    return pContext;
}

TYPEINIT1( SdXMLTableShapeContext, SdXMLShapeContext );

SdXMLTableShapeContext::SdXMLTableShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx, const OUString& rLocalName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList, com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes )
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes, sal_False )
{
    memset( &maTemplateStylesUsed, 0, sizeof( maTemplateStylesUsed ) );
}

SdXMLTableShapeContext::~SdXMLTableShapeContext()
{
}

void SdXMLTableShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
{
    OUString service("com.sun.star.drawing.TableShape");

    sal_Bool bIsPresShape = !maPresentationClass.isEmpty() && GetImport().GetShapeImport()->IsPresentationShapesSupported();
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
                        xProps->setPropertyValue("IsEmptyPresentationObject", ::cppu::bool2any( false ) );

                    if( mbIsUserTransformed && xPropsInfo->hasPropertyByName("IsPlaceholderDependent"))
                        xProps->setPropertyValue("IsPlaceholderDependent", ::cppu::bool2any( false ) );
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
        
        SetTransformation();
    }
}


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
                        maTemplateStylesUsed[i] = sal_True;
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
