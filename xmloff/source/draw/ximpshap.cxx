/*************************************************************************
 *
 *  $RCSfile: ximpshap.cxx,v $
 *
 *  $Revision: 1.87 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 08:50:35 $
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

#pragma hdrstop

#include <tools/debug.hxx>

#ifndef _COM_SUN_STAR_DRAWING_XGLUEPOINTSSUPPLIER_HPP_
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XIDENTIFIERACCESS_HPP_
#include <com/sun/star/container/XIdentifierAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_GLUEPOINT2_HPP_
#include <com/sun/star/drawing/GluePoint2.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_ALIGNMENT_HPP_
#include <com/sun/star/drawing/Alignment.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_ESCAPEDIRECTION_HPP_
#include <com/sun/star/drawing/EscapeDirection.hpp>
#endif

#ifndef _COM_SUN_STAR_MEDIA_ZOOMLEVEL_HPP_
#include <com/sun/star/media/ZoomLevel.hpp>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _XIMPSHAPE_HXX
#include "ximpshap.hxx"
#endif

#ifndef _XMLOFF_XMLBASE64IMPORTCONTEXT_HXX
#include "XMLBase64ImportContext.hxx"
#endif

#ifndef _XMLOFF_XMLSHAPESTYLECONTEXT_HXX
#include "XMLShapeStyleContext.hxx"
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include "xmluconv.hxx"
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_AWT_XCONTROLMODEL_HPP_
#include <com/sun/star/awt/XControlModel.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_XCONTROLSHAPE_HPP_
#include <com/sun/star/drawing/XControlShape.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif

#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLONEABLE_HPP_
#include <com/sun/star/util/XCloneable.hpp>
#endif

#ifndef _XEXPTRANSFORM_HXX
#include "xexptran.hxx"
#endif

#ifndef _COM_SUN_STAR_DRAWING_POLYPOLYGONBEZIERCOORDS_HPP_
#include <com/sun/star/drawing/PolyPolygonBezierCoords.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_CONNECTORTYPE_HPP_
#include <com/sun/star/drawing/ConnectorType.hpp>
#endif

#ifndef _COM_SUN_STAR_DRAWING_HOMOGENMATRIX3_HPP_
#include <com/sun/star/drawing/HomogenMatrix3.hpp>
#endif

#ifndef _XMLOFF_PROPERTYSETMERGER_HXX_
#include "PropertySetMerger.hxx"
#endif

#ifndef _XMLOFF_FAMILIES_HXX_
#include "families.hxx"
#endif

#ifndef _XIMPSTYLE_HXX
#include "ximpstyl.hxx"
#endif

#ifndef _XMLOFF_XMLNMSPE_HXX
#include"xmlnmspe.hxx"
#endif

#ifndef _XMLOFF_XMLTOKEN_HXX
#include "xmltoken.hxx"
#endif

#ifndef _ENHANCED_CUSTOMSHAPE_TOKEN_HXX
#include "EnhancedCustomShapeToken.hxx"
#endif

#ifndef _XMLOFF_XMLIMAGEMAPCONTEXT_HXX_
#include "XMLImageMapContext.hxx"
#endif

#ifndef _SDPROPLS_HXX
#include "sdpropls.hxx"
#endif

#ifndef _XMLOFF_EVENTIMP_HXX
#include "eventimp.hxx"
#endif

#ifndef _XMLOFF_XIMPCUSTOMSHAPE_HXX_
#include "ximpcustomshape.hxx"
#endif

#ifndef _XMLOFF_XMLEMBEDDEDOBJECTIMPORTCONTEXT_HXX
#include "XMLEmbeddedObjectImportContext.hxx"
#endif

using namespace ::rtl;
using namespace ::com::sun::star;
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool ImpIsEmptyURL( const ::rtl::OUString& rURL )
{
    if( rURL.getLength() == 0 )
        return true;

    // #i13140# Also compare against 'toplevel' URLs. which also
    // result in empty filename strings.
    if( 0 == rURL.compareToAscii( "#./" ) )
        return true;

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SvXMLShapeContext, SvXMLImportContext );
TYPEINIT1( SdXMLShapeContext, SvXMLShapeContext );

SdXMLShapeContext::SdXMLShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SvXMLShapeContext( rImport, nPrfx, rLocalName ),
    mxShapes( rShapes ),
    mnStyleFamily(XML_STYLE_FAMILY_SD_GRAPHICS_ID),
    mbIsPlaceholder(FALSE),
    mbIsUserTransformed(FALSE),
    mxAttrList(xAttrList),
    mnZOrder(-1),
    mnShapeId(-1),
    maPosition(0, 0),
    maSize(1, 1)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLShapeContext::~SdXMLShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext *SdXMLShapeContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = NULL;

    if( nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) )
    {
        pContext = new SdXMLEventsContext( GetImport(), nPrefix, rLocalName, xAttrList, mxShape );
    }
    else if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_GLUE_POINT ) )
    {
        addGluePoint( xAttrList );
    }
    else if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_THUMBNAIL ) )
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
                UniReference < XMLTextImportHelper > xTxtImport =
                    GetImport().GetTextImport();
                mxOldCursor = xTxtImport->GetCursor();
                mxCursor = xText->createTextCursor();
                if( mxCursor.is() )
                {
                    xTxtImport->SetCursor( mxCursor );
                }

                // remember old list item and block (#91964#) and reset them
                // for the text frame
                mxOldListBlock = xTxtImport->_GetListBlock();
                mxOldListItem = xTxtImport->_GetListItem();
                xTxtImport->_SetListBlock( NULL );
                xTxtImport->_SetListItem( NULL );
            }
        }

        // if we have a text cursor, lets  try to import some text
        if( mxCursor.is() )
        {
            pContext = GetImport().GetTextImport()->CreateTextChildContext(
                GetImport(), nPrefix, rLocalName, xAttrList );
        }
    }

    // call parent for content
    if(!pContext)
        pContext = SvXMLImportContext::CreateChildContext( nPrefix, rLocalName, xAttrList );

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
                GetImport().GetMM100UnitConverter().convertMeasure(aGluePoint.Position.X, sValue);
            }
            else if( IsXMLToken( aLocalName, XML_Y ) )
            {
                GetImport().GetMM100UnitConverter().convertMeasure(aGluePoint.Position.Y, sValue);
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
                USHORT eKind;
                if( SvXMLUnitConverter::convertEnum( eKind, sValue, aXML_GlueAlignment_EnumMap ) )
                {
                    aGluePoint.PositionAlignment = (drawing::Alignment)eKind;
                    aGluePoint.IsRelative = sal_False;
                }
            }
            else if( IsXMLToken( aLocalName, XML_ESCAPE_DIRECTION ) )
            {
                USHORT eKind;
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
        catch( uno::Exception& )
        {
            DBG_ERROR( "exception during setting of glue points!");
        }
    }
}
//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
}

void SdXMLShapeContext::EndElement()
{
    if(mxCursor.is())
    {
        // delete addition newline
        const OUString aEmpty;
        mxCursor->gotoEnd( sal_False );
        mxCursor->goLeft( 1, sal_True );
        mxCursor->setString( aEmpty );

        // reset cursor
        GetImport().GetTextImport()->ResetCursor();
    }

    if(mxOldCursor.is())
        GetImport().GetTextImport()->SetCursor( mxOldCursor );

    // reinstall old list item (if necessary) #91964#
    if ( mxOldListBlock.Is() )
    {
        GetImport().GetTextImport()->_SetListBlock( mxOldListBlock );
        GetImport().GetTextImport()->_SetListItem( mxOldListItem );
    }

    if( mxLockable.is() )
        mxLockable->removeActionLock();
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::AddShape(uno::Reference< drawing::XShape >& xShape)
{
    if(xShape.is())
    {
        // set shape local
        mxShape = xShape;

        if(maShapeName.getLength())
        {
            uno::Reference< container::XNamed > xNamed( mxShape, uno::UNO_QUERY );
            if( xNamed.is() )
                xNamed->setName( maShapeName );
        }

        UniReference< XMLShapeImportHelper > xImp( GetImport().GetShapeImport() );
        xImp->addShape( xShape, mxAttrList, mxShapes );

        // #107848#
        if(!GetImport().HasTextImport()
            || !GetImport().GetTextImport()->IsInsideDeleteContext())
        {
            xImp->shapeWithZIndexAdded( xShape, mnZOrder );
        }

        if( mnShapeId != -1 )
            xImp->createShapeId( xShape, mnShapeId );

        // #91065# count only if counting for shape import is enabled
        if(GetImport().GetShapeImport()->IsHandleProgressBarEnabled())
        {
            // #80365# increment progress bar at load once for each draw object
            GetImport().GetProgressBarHelper()->Increment();
        }
    }

    mxLockable = uno::Reference< document::XActionLockable >::query( xShape );

    if( mxLockable.is() )
        mxLockable->addActionLock();

}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::AddShape(const char* pServiceName )
{
    uno::Reference< lang::XMultiServiceFactory > xServiceFact(GetImport().GetModel(), uno::UNO_QUERY);
    if(xServiceFact.is())
    {
        uno::Reference< drawing::XShape > xShape(xServiceFact->createInstance(OUString::createFromAscii(pServiceName)), uno::UNO_QUERY);
        if( xShape.is() )
            AddShape( xShape );
    }
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLShapeContext::SetTransformation()
{
    if(mxShape.is())
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if(xPropSet.is())
        {
            Matrix3D aTransformation;

            if(maSize.Width != 1 || maSize.Height != 1)
            {
                // take care there are no zeros used by error
                if(0 == maSize.Width)
                    maSize.Width = 1;
                if(0 == maSize.Height)
                    maSize.Height = 1;

                // set global size. This should always be used.
                aTransformation.Scale(maSize.Width, maSize.Height);
            }

            if(maPosition.X != 0 || maPosition.Y != 0)
            {
                // if global position is used, add it to transformation
                aTransformation.Translate(maPosition.X, maPosition.Y);
            }

            if(mnTransform.NeedsAction())
            {
                // transformation is used, apply to object.
                // NOTICE: The transformation is applied AFTER evtl. used
                // global positioning and scaling is used, so any shear or
                // rotate used herein is applied around the (0,0) position
                // of the PAGE object !!!
                Matrix3D aMat;
                mnTransform.GetFullTransform(aMat);

                // now add to transformation
                aTransformation *= aMat;
            }

            // now set transformation for this object
            uno::Any aAny;
            drawing::HomogenMatrix3 aMatrix;

            aMatrix.Line1.Column1 = aTransformation[0].X();
            aMatrix.Line1.Column2 = aTransformation[0].Y();
            aMatrix.Line1.Column3 = aTransformation[0].W();

            aMatrix.Line2.Column1 = aTransformation[1].X();
            aMatrix.Line2.Column2 = aTransformation[1].Y();
            aMatrix.Line2.Column3 = aTransformation[1].W();

            aMatrix.Line3.Column1 = aTransformation[2].X();
            aMatrix.Line3.Column2 = aTransformation[2].Y();
            aMatrix.Line3.Column3 = aTransformation[2].W();

            aAny <<= aMatrix;

            xPropSet->setPropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Transformation")), aAny);
        }
    }
}

//////////////////////////////////////////////////////////////////////////////

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

            // set style on shape
            if(maDrawStyleName.getLength() == 0)
                break;

            const SvXMLStyleContext* pStyle = 0L;
            sal_Bool bAutoStyle(FALSE);

            if(GetImport().GetShapeImport()->GetAutoStylesContext())
                pStyle = GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(mnStyleFamily, maDrawStyleName);

            if(pStyle)
                bAutoStyle = TRUE;

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
                    aStyleName = pDocStyle->GetParent();
                }
            }

            if( !xStyle.is() && aStyleName.getLength() )
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
                                sal_Int32 nPos = aStyleName.lastIndexOf( sal_Unicode('-') );
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
                                xFamilies->getByName( OUString( RTL_CONSTASCII_USTRINGPARAM( "graphics" ) ) ) >>= xFamily;
                                aStyleName = GetImport().GetStyleDisplayName(
                                    XML_STYLE_FAMILY_SD_GRAPHICS_ID,
                                    aStyleName );
                            }

                            if( xFamily.is() )
                                xFamily->getByName( aStyleName ) >>= xStyle;
                        }
                    }
                }
                catch( uno::Exception& )
                {
                    DBG_ERROR( "could not find style for shape!" );
                }
            }

            if( bSupportsStyle && xStyle.is() )
            {
                try
                {
                    // set style on object
                    uno::Any aAny;
                    aAny <<= xStyle;
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("Style")), aAny);
                }
                catch( uno::Exception& )
                {
                    DBG_ERROR( "could not find style for shape!" );
                }
            }

            // if this is an auto style, set its properties
            if(bAutoStyle && pDocStyle)
            {
                // set PropertySet on object
                pDocStyle->FillPropertySet(xPropSet);
            }

        } while(0);

        // try to set text auto style
        do
        {
            // set style on shape
            if( 0 == maTextStyleName.getLength() )
                break;

            if( NULL == GetImport().GetShapeImport()->GetAutoStylesContext())
                break;

            XMLPropStyleContext* pStyle = PTR_CAST( XMLPropStyleContext, GetImport().GetShapeImport()->GetAutoStylesContext()->FindStyleChildContext(XML_STYLE_FAMILY_TEXT_PARAGRAPH, maTextStyleName) );
            if( pStyle == NULL )
                break;

            // set PropertySet on object
            pStyle->FillPropertySet(xPropSet);

        } while(0);
    }
    catch( uno::Exception& )
    {
    }
}

void SdXMLShapeContext::SetLayer()
{
    if( maLayerName.getLength() )
    {
        try
        {
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if(xPropSet.is() )
            {
                uno::Any aAny;
                aAny <<= maLayerName;

                xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("LayerName")), aAny);
                return;
            }
        }
        catch( uno::Exception e )
        {
        }
    }
}

void SdXMLShapeContext::SetThumbnail()
{
    if( 0 == maThumbnailURL.getLength() )
        return;

    try
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
        if( !xPropSet.is() )
            return;

        const OUString sProperty(RTL_CONSTASCII_USTRINGPARAM("ThumbnailGraphicURL"));

        uno::Reference< beans::XPropertySetInfo > xPropSetInfo( xPropSet->getPropertySetInfo() );
        if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName( sProperty ) )
        {
            // load the thumbnail graphic and export it to a wmf stream so we can set
            // it at the api

            const OUString aInternalURL( GetImport().ResolveGraphicObjectURL( maThumbnailURL, sal_False ) );
            xPropSet->setPropertyValue( sProperty, uno::makeAny( aInternalURL ) );
        }
    }
    catch( uno::Exception e )
    {
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_ZINDEX ) )
        {
            mnZOrder = rValue.toInt32();
        }
        else if( IsXMLToken( rLocalName, XML_ID ) )
        {
            mnShapeId = rValue.toInt32();
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
            GetImport().GetMM100UnitConverter().convertMeasure(maPosition.X, rValue);
        }
        else if( IsXMLToken( rLocalName, XML_Y ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maPosition.Y, rValue);
        }
        else if( IsXMLToken( rLocalName, XML_WIDTH ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maSize.Width, rValue);
        }
        else if( IsXMLToken( rLocalName, XML_HEIGHT ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maSize.Height, rValue);
        }
        else if( IsXMLToken( rLocalName, XML_TRANSFORM ) )
        {
            // because of #85127# take svg:transform into account and hanle like
            // draw:transform for compatibility
            mnTransform.SetString(rValue, GetImport().GetMM100UnitConverter());
        }
    }
}

sal_Bool SdXMLShapeContext::isPresentationShape() const
{
    return (XML_STYLE_FAMILY_SD_PRESENTATION_ID == mnStyleFamily) &&
           maPresentationClass.getLength() &&
           (const_cast<SdXMLShapeContext*>(this))->GetImport().GetShapeImport()->IsPresentationShapesSupported();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLRectShapeContext, SdXMLShapeContext );

SdXMLRectShapeContext::SdXMLRectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnRadius( 0L )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLRectShapeContext::~SdXMLRectShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLRectShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CORNER_RADIUS ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnRadius, rValue);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

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
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("CornerRadius")), uno::makeAny( mnRadius ) );
                }
                catch( uno::Exception& )
                {
                    DBG_ERROR( "exception during setting of corner radius!");
                }
            }
        }
        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////3////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLLineShapeContext, SdXMLShapeContext );

SdXMLLineShapeContext::SdXMLLineShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnX1( 0L ),
    mnY1( 0L ),
    mnX2( 1L ),
    mnY2( 1L )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLLineShapeContext::~SdXMLLineShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLLineShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_X1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnX1, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnY1, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_X2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnX2, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnY2, rValue);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLLineShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // #85920# use SetTransformation() to handle import of simple lines.
    // This is necessary to kake into account all anchor positions and
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
            uno::Any aAny;

            *pInnerSequence = awt::Point( mnX1 - aTopLeft.X, mnY1 - aTopLeft.Y);
            pInnerSequence++;
            *pInnerSequence = awt::Point( mnX2 - aTopLeft.X, mnY2 - aTopLeft.Y);

            aAny <<= aPolyPoly;
            xPropSet->setPropertyValue(
                OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry")), aAny);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLEllipseShapeContext, SdXMLShapeContext );

SdXMLEllipseShapeContext::SdXMLEllipseShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnCX( 0L ),
    mnCY( 0L ),
    mnRX( 1L ),
    mnRY( 1L ),
    meKind( drawing::CircleKind_FULL ),
    mnStartAngle( 0 ),
    mnEndAngle( 0 )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLEllipseShapeContext::~SdXMLEllipseShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLEllipseShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_SVG == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_RX ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnRX, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_RY ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnRY, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_CX ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnCX, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_CY ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnCY, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_R ) )
        {
            // single radius, it's a circle and both radii are the same
            GetImport().GetMM100UnitConverter().convertMeasure(mnRX, rValue);
            mnRY = mnRX;
            return;
        }
    }
    else if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_KIND ) )
        {
            USHORT eKind;
            if( SvXMLUnitConverter::convertEnum( eKind, rValue, aXML_CircleKind_EnumMap ) )
            {
                meKind = eKind;
            }
            return;
        }
        if( IsXMLToken( rLocalName, XML_START_ANGLE ) )
        {
            double dStartAngle;
            if( SvXMLUnitConverter::convertDouble( dStartAngle, rValue ) )
                mnStartAngle = (sal_Int32)(dStartAngle * 100.0);
            return;
        }
        if( IsXMLToken( rLocalName, XML_END_ANGLE ) )
        {
            double dEndAngle;
            if( SvXMLUnitConverter::convertDouble( dEndAngle, rValue ) )
                mnEndAngle = (sal_Int32)(dEndAngle * 100.0);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLEllipseShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create rectangle shape
    AddShape("com.sun.star.drawing.EllipseShape");
    if(mxShape.is())
    {
        // Add, set Style and properties from base shape
        SetStyle();
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        if( meKind != drawing::CircleKind_FULL )
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape, uno::UNO_QUERY );
            if( xPropSet.is() )
            {
                uno::Any aAny;
                aAny <<= (drawing::CircleKind)meKind;
                xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleKind")), aAny );

                aAny <<= mnStartAngle;
                xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleStartAngle")), aAny );

                aAny <<= mnEndAngle;
                xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CircleEndAngle")), aAny );
            }
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPolygonShapeContext, SdXMLShapeContext );

SdXMLPolygonShapeContext::SdXMLPolygonShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes, sal_Bool bClosed)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mbClosed( bClosed )
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLPolygonShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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

//////////////////////////////////////////////////////////////////////////////

SdXMLPolygonShapeContext::~SdXMLPolygonShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

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
            if(maPoints.getLength() && maViewBox.getLength())
            {
                SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
                awt::Size aSize(aViewBox.GetWidth(), aViewBox.GetHeight());
                awt::Point aPosition(aViewBox.GetX(), aViewBox.GetY());
                SdXMLImExPointsElement aPoints(maPoints, aViewBox,
                    aPosition, aSize, GetImport().GetMM100UnitConverter());

                uno::Any aAny;
                aAny <<= aPoints.GetPointSequenceSequence();
                xPropSet->setPropertyValue(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry")), aAny);
            }
        }

        // set pos, size, shear and rotate and get copy of matrix
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPathShapeContext, SdXMLShapeContext );

SdXMLPathShapeContext::SdXMLPathShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mbClosed( TRUE )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPathShapeContext::~SdXMLPathShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLPathShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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

//////////////////////////////////////////////////////////////////////////////

void SdXMLPathShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create polygon shape
    if(maD.getLength())
    {
        // prepare some of the parameters
        SdXMLImExViewBox aViewBox(maViewBox, GetImport().GetMM100UnitConverter());
        awt::Size aSize(aViewBox.GetWidth(), aViewBox.GetHeight());
        awt::Point aPosition(aViewBox.GetX(), aViewBox.GetY());
        SdXMLImExSvgDElement aPoints(maD, aViewBox,
            aPosition, aSize, GetImport().GetMM100UnitConverter());

        char* pService;
        // now create shape
        if(aPoints.IsCurve())
        {
            if(aPoints.IsClosed())
            {
                pService = "com.sun.star.drawing.ClosedBezierShape";
            }
            else
            {
                pService = "com.sun.star.drawing.OpenBezierShape";
            }
        }
        else
        {
            if(aPoints.IsClosed())
            {
                pService = "com.sun.star.drawing.PolyPolygonShape";
            }
            else
            {
                pService = "com.sun.star.drawing.PolyLineShape";
            }
        }

        // Add, set Style and properties from base shape
        AddShape(pService);

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

                // set svg:d
                if(maD.getLength())
                {
                    if(aPoints.IsCurve())
                    {
                        drawing::PolyPolygonBezierCoords aSourcePolyPolygon(
                            aPoints.GetPointSequenceSequence(),
                            aPoints.GetFlagSequenceSequence());

                        aAny <<= aSourcePolyPolygon;
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry")), aAny);
                    }
                    else
                    {
                        aAny <<= aPoints.GetPointSequenceSequence();
                        xPropSet->setPropertyValue(
                            OUString(RTL_CONSTASCII_USTRINGPARAM("Geometry")), aAny);
                    }
                }
            }

            // set pos, size, shear and rotate
            SetTransformation();

            SdXMLShapeContext::StartElement(xAttrList);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLTextBoxShapeContext, SdXMLShapeContext );

SdXMLTextBoxShapeContext::SdXMLTextBoxShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mnRadius(0)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLTextBoxShapeContext::~SdXMLTextBoxShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLTextBoxShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CORNER_RADIUS ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnRadius, rValue);
            return;
        }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLTextBoxShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>&)
{
    // create textbox shape
    sal_Bool bIsPresShape = sal_False;
    bool bClearText = false;

    char *pService = NULL;

    if( isPresentationShape() )
    {
        // check if the current document supports presentation shapes
        if( GetImport().GetShapeImport()->IsPresentationShapesSupported() )
        {
            if( IsXMLToken( maPresentationClass, XML_PRESENTATION_SUBTITLE ))
            {
                // XmlShapeTypePresSubtitleShape
                pService = "com.sun.star.presentation.SubtitleShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_OUTLINE ) )
            {
                // XmlShapeTypePresOutlinerShape
                pService = "com.sun.star.presentation.OutlinerShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_NOTES ) )
            {
                // XmlShapeTypePresNotesShape
                pService = "com.sun.star.presentation.NotesShape";
            }
            else if( IsXMLToken( maPresentationClass, XML_HEADER ) )
            {
                // XmlShapeTypePresHeaderShape
                pService = "com.sun.star.presentation.HeaderShape";
                bClearText = true;
            }
            else if( IsXMLToken( maPresentationClass, XML_FOOTER ) )
            {
                // XmlShapeTypePresFooterShape
                pService = "com.sun.star.presentation.FooterShape";
                bClearText = true;
            }
            else if( IsXMLToken( maPresentationClass, XML_PAGE_NUMBER ) )
            {
                // XmlShapeTypePresSlideNumberShape
                pService = "com.sun.star.presentation.SlideNumberShape";
                bClearText = true;
            }
            else if( IsXMLToken( maPresentationClass, XML_DATE_TIME ) )
            {
                // XmlShapeTypePresDateTimeShape
                pService = "com.sun.star.presentation.DateTimeShape";
                bClearText = true;
            }
            else //  IsXMLToken( maPresentationClass, XML_PRESENTATION_TITLE ) )
            {
                // XmlShapeTypePresTitleTextShape
                pService = "com.sun.star.presentation.TitleTextShape";
            }
            bIsPresShape = TRUE;
        }
    }

    if( NULL == pService )
    {
        // normal text shape
        pService = "com.sun.star.drawing.TextShape";
    }

    // Add, set Style and properties from base shape
    AddShape(pService);

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
                    if( !mbIsPlaceholder && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( sal_False ) );

                    if( mbIsUserTransformed && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") ), ::cppu::bool2any( sal_False ) );
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
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("CornerRadius")), uno::makeAny( mnRadius ) );
                }
                catch( uno::Exception& )
                {
                    DBG_ERROR( "exception during setting of corner radius!");
                }
            }
        }

        SdXMLShapeContext::StartElement(mxAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLControlShapeContext, SdXMLShapeContext );

SdXMLControlShapeContext::SdXMLControlShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLControlShapeContext::~SdXMLControlShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLControlShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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
        DBG_ASSERT( maFormId.getLength(), "draw:control without a form:id attribute!" );
        if( maFormId.getLength() )
        {
#ifndef SVX_LIGHT
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
#endif // #ifndef SVX_LIGHT
        }

        SetStyle();
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLConnectorShapeContext, SdXMLShapeContext );

SdXMLConnectorShapeContext::SdXMLConnectorShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    maStart(0,0),
    maEnd(1,1),
    mnType( (USHORT)drawing::ConnectorType_STANDARD ),
    mnStartShapeId(-1),
    mnStartGlueId(-1),
    mnEndShapeId(-1),
    mnEndGlueId(-1),
    mnDelta1(0),
    mnDelta2(0),
    mnDelta3(0)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLConnectorShapeContext::~SdXMLConnectorShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLConnectorShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_DRAW:
    {
        if( IsXMLToken( rLocalName, XML_START_SHAPE ) )
        {
            mnStartShapeId = rValue.toInt32();
            return;
        }
        if( IsXMLToken( rLocalName, XML_START_GLUE_POINT ) )
        {
            mnStartGlueId = rValue.toInt32();
            return;
        }
        if( IsXMLToken( rLocalName, XML_END_SHAPE ) )
        {
            mnEndShapeId = rValue.toInt32();
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
                GetImport().GetMM100UnitConverter().convertMeasure(mnDelta1, aToken);
                if( aTokenEnum.getNextToken( aToken ) )
                {
                    GetImport().GetMM100UnitConverter().convertMeasure(mnDelta2, aToken);
                    if( aTokenEnum.getNextToken( aToken ) )
                    {
                        GetImport().GetMM100UnitConverter().convertMeasure(mnDelta3, aToken);
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
    }
    case XML_NAMESPACE_SVG:
    {
        if( IsXMLToken( rLocalName, XML_X1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maStart.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maStart.Y, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_X2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maEnd.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maEnd.Y, rValue);
            return;
        }
    }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLConnectorShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // #107928#
    // For security reasons, do not add empty connectors. There may have been an error in EA2
    // that created empty, far set off connectors (e.g. 63 meters below top of document). This
    // is not guaranteed, but it's definitely safe to not add empty connectors.
    sal_Bool bDoAdd(sal_True);

    if(    -1 == mnStartShapeId
        && -1 == mnEndShapeId
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
        // create Connector shape
        // add, set style and properties from base shape
        AddShape("com.sun.star.drawing.ConnectorShape");
        if(mxShape.is())
        {
            // add connection ids
            if( mnStartShapeId != -1 )
                GetImport().GetShapeImport()->addShapeConnection( mxShape, sal_True, mnStartShapeId, mnStartGlueId );
            if( mnEndShapeId != -1 )
                GetImport().GetShapeImport()->addShapeConnection( mxShape, sal_False, mnEndShapeId, mnEndGlueId );

            uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
            if( xProps.is() )
            {
                uno::Any aAny;
                aAny <<= maStart;
                xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition")), aAny);

                aAny <<= maEnd;
                xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition")), aAny );

                aAny <<= (drawing::ConnectorType)mnType;
                xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeKind")), aAny );

                aAny <<= mnDelta1;
                xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine1Delta")), aAny );

                aAny <<= mnDelta2;
                xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine2Delta")), aAny );

                aAny <<= mnDelta3;
                xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EdgeLine3Delta")), aAny );
            }
            SetStyle();
            SetLayer();

            SdXMLShapeContext::StartElement(xAttrList);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLMeasureShapeContext, SdXMLShapeContext );

SdXMLMeasureShapeContext::SdXMLMeasureShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    maStart(0,0),
    maEnd(1,1)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLMeasureShapeContext::~SdXMLMeasureShapeContext()
{
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLMeasureShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    switch( nPrefix )
    {
    case XML_NAMESPACE_SVG:
    {
        if( IsXMLToken( rLocalName, XML_X1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maStart.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y1 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maStart.Y, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_X2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maEnd.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_Y2 ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maEnd.Y, rValue);
            return;
        }
    }
    }

    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

//////////////////////////////////////////////////////////////////////////////

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
            uno::Any aAny;
            aAny <<= maStart;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("StartPosition")), aAny);

            aAny <<= maEnd;
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("EndPosition")), aAny );
        }

        // delete pre created fields
        uno::Reference< text::XText > xText( mxShape, uno::UNO_QUERY );
        if( xText.is() )
        {
            const OUString aEmpty( RTL_CONSTASCII_USTRINGPARAM( " " ) );
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
        xCursor->goRight( 1, sal_True );
        xCursor->setString( aEmpty );
    }
    while(0);

    SdXMLShapeContext::EndElement();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPageShapeContext, SdXMLShapeContext );

SdXMLPageShapeContext::SdXMLPageShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ), mnPageNumber(0)
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLPageShapeContext::~SdXMLPageShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLPageShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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

//////////////////////////////////////////////////////////////////////////////

void SdXMLPageShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Page shape
    // add, set style and properties from base shape

    // #86163# take into account which type of PageShape needs to
    // be constructed. It's an pres shape if presentation:XML_CLASS == XML_PRESENTATION_PAGE.
    sal_Bool bIsPresentation = maPresentationClass.getLength() &&
           GetImport().GetShapeImport()->IsPresentationShapesSupported();

    uno::Reference< lang::XServiceInfo > xInfo( mxShapes, uno::UNO_QUERY );
    const sal_Bool bIsOnHandoutPage = xInfo.is() && xInfo->supportsService( OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.presentation.HandoutMasterPage")) );

    if( bIsOnHandoutPage )
    {
        AddShape("com.sun.star.presentation.HandoutShape");
    }
    else
    {
        if(bIsPresentation && !IsXMLToken( maPresentationClass, XML_PRESENTATION_PAGE ) )
        {
            bIsPresentation = FALSE;
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
            const OUString aPageNumberStr(RTL_CONSTASCII_USTRINGPARAM("PageNumber"));
            if( xPropSetInfo.is() && xPropSetInfo->hasPropertyByName(aPageNumberStr))
                xPropSet->setPropertyValue(aPageNumberStr, uno::makeAny( mnPageNumber ));
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLCaptionShapeContext, SdXMLShapeContext );

SdXMLCaptionShapeContext::SdXMLCaptionShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    // #86616# for correct edge rounding import mnRadius needs to be initialized
    mnRadius( 0L )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLCaptionShapeContext::~SdXMLCaptionShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLCaptionShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    // create Caption shape
    // add, set style and properties from base shape
    AddShape("com.sun.star.drawing.CaptionShape");
    if( mxShape.is() )
    {
        SetStyle();
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
        if( xProps.is() )
            xProps->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("CaptionPoint")), uno::makeAny( maCaptionPoint ) );

        if(mnRadius)
        {
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if(xPropSet.is())
            {
                try
                {
                    xPropSet->setPropertyValue(OUString(RTL_CONSTASCII_USTRINGPARAM("CornerRadius")), uno::makeAny( mnRadius ) );
                }
                catch( uno::Exception& )
                {
                    DBG_ERROR( "exception during setting of corner radius!");
                }
            }
        }

        SdXMLShapeContext::StartElement(xAttrList);
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLCaptionShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    if( XML_NAMESPACE_DRAW == nPrefix )
    {
        if( IsXMLToken( rLocalName, XML_CAPTION_POINT_X ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maCaptionPoint.X, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_CAPTION_POINT_Y ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(maCaptionPoint.Y, rValue);
            return;
        }
        if( IsXMLToken( rLocalName, XML_CORNER_RADIUS ) )
        {
            GetImport().GetMM100UnitConverter().convertMeasure(mnRadius, rValue);
            return;
        }
    }
    SdXMLShapeContext::processAttribute( nPrefix, rLocalName, rValue );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLGraphicObjectShapeContext, SdXMLShapeContext );

SdXMLGraphicObjectShapeContext::SdXMLGraphicObjectShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    maURL()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLGraphicObjectShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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

//////////////////////////////////////////////////////////////////////////////

void SdXMLGraphicObjectShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    // create graphic object shape
    char *pService;

    if( IsXMLToken( maPresentationClass, XML_GRAPHIC ) && GetImport().GetShapeImport()->IsPresentationShapesSupported() )
    {
        pService = "com.sun.star.presentation.GraphicObjectShape";
    }
    else
    {
        pService = "com.sun.star.drawing.GraphicObjectShape";
    }

    AddShape( pService );

    if(mxShape.is())
    {
        SetStyle();
        SetLayer();
        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
        if(xProps.is())
        {
            uno::Reference< beans::XPropertySetInfo > xPropsInfo( xProps->getPropertySetInfo() );
            if( xPropsInfo.is() && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( mbIsPlaceholder ) );

            if( !mbIsPlaceholder )
            {
                if( maURL.getLength() )
                {
                    uno::Any aAny;
                    aAny <<= GetImport().ResolveGraphicObjectURL( maURL, sal_True );
                    try
                    {
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicURL") ), aAny );
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicStreamURL") ), aAny );
                    }
                    catch (lang::IllegalArgumentException const &)
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
                    if( xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") ), ::cppu::bool2any( sal_False ) );
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
        if( sURL.getLength() )
        {
            try
            {
                uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
                if(xProps.is())
                {
                    const uno::Any aAny( uno::makeAny( sURL ) );
                    xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicURL") ), aAny );
                    xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("GraphicStreamURL") ), aAny );
                }
            }
            catch (lang::IllegalArgumentException const &)
            {
            }
        }
    }

    SdXMLShapeContext::EndElement();
}


//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLGraphicObjectShapeContext::CreateChildContext(
    USHORT nPrefix, const ::rtl::OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = NULL;

    if ( (XML_NAMESPACE_DRAW == nPrefix) &&
         IsXMLToken( rLocalName, XML_IMAGE_MAP ) )
    {
        uno::Reference< beans::XPropertySet > xPropSet(mxShape,uno::UNO_QUERY);
        if (xPropSet.is())
        {
            pContext = new XMLImageMapContext(GetImport(), nPrefix,
                                              rLocalName, xPropSet);
        }
    }
    else if( (XML_NAMESPACE_OFFICE == nPrefix) &&
             xmloff::token::IsXMLToken( rLocalName, xmloff::token::XML_BINARY_DATA ) )
    {
        if( !maURL.getLength() && !mxBase64Stream.is() )
        {
            mxBase64Stream = GetImport().GetStreamForGraphicObjectURLFromBase64();
            if( mxBase64Stream.is() )
                pContext = new XMLBase64ImportContext( GetImport(), nPrefix,
                                                    rLocalName, xAttrList,
                                                    mxBase64Stream );
        }
    }

    // delegate to parent class if no context could be created
    if ( NULL == pContext )
        pContext = SdXMLShapeContext::CreateChildContext(nPrefix, rLocalName,
                                                         xAttrList);

    return pContext;
}

//////////////////////////////////////////////////////////////////////////////

SdXMLGraphicObjectShapeContext::~SdXMLGraphicObjectShapeContext()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLChartShapeContext, SdXMLShapeContext );

SdXMLChartShapeContext::SdXMLChartShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
    mpChartContext( NULL )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLChartShapeContext::~SdXMLChartShapeContext()
{
    if( mpChartContext )
        delete mpChartContext;
}

//////////////////////////////////////////////////////////////////////////////

void SdXMLChartShapeContext::StartElement(const uno::Reference< xml::sax::XAttributeList>& xAttrList)
{
    const sal_Bool bIsPresentation = isPresentationShape();

    AddShape( bIsPresentation ? "com.sun.star.presentation.ChartShape" : "com.sun.star.drawing.OLE2Shape" );

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
                if( xPropsInfo.is() && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                    xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( sal_False ) );

                uno::Any aAny;

                const OUString aCLSID( RTL_CONSTASCII_USTRINGPARAM("12DCAE26-281F-416F-a234-c3086127382e"));

                aAny <<= aCLSID;
                xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID") ), aAny );

#ifndef SVX_LIGHT
                aAny = xProps->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ) );
                uno::Reference< frame::XModel > xChartModel;
                if( aAny >>= xChartModel )
                {
                    mpChartContext = GetImport().GetChartImport()->CreateChartContext( GetImport(), XML_NAMESPACE_SVG, GetXMLToken(XML_CHART), xChartModel, xAttrList );
                }
#endif
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
                    if( xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") ), ::cppu::bool2any( sal_False ) );
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

void SdXMLChartShapeContext::Characters( const ::rtl::OUString& rChars )
{
    if( mpChartContext )
        mpChartContext->Characters( rChars );
}

SvXMLImportContext * SdXMLChartShapeContext::CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    if( mpChartContext )
        return mpChartContext->CreateChildContext( nPrefix, rLocalName, xAttrList );

    return NULL;
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLObjectShapeContext, SdXMLShapeContext );

SdXMLObjectShapeContext::SdXMLObjectShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

SdXMLObjectShapeContext::~SdXMLObjectShapeContext()
{
}

void SdXMLObjectShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& )
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
    if( !(GetImport().getImportFlags() & IMPORT_EMBEDDED) && !mbIsPlaceholder && ImpIsEmptyURL(maHref) )
        return;

    char* pService = "com.sun.star.drawing.OLE2Shape";

    sal_Bool bIsPresShape = maPresentationClass.getLength() && GetImport().GetShapeImport()->IsPresentationShapesSupported();
;

    if( bIsPresShape )
    {
        if( IsXMLToken( maPresentationClass, XML_PRESENTATION_CHART ) )
        {
            pService = "com.sun.star.presentation.ChartShape";
        }
        else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_TABLE ) )
        {
            pService = "com.sun.star.presentation.TableShape";
        }
        else if( IsXMLToken( maPresentationClass, XML_PRESENTATION_OBJECT ) )
        {
            pService = "com.sun.star.presentation.OLE2Shape";
        }
    }

    AddShape( pService );

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
                    if( !mbIsPlaceholder && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsEmptyPresentationObject") ), ::cppu::bool2any( sal_False ) );

                    if( mbIsUserTransformed && xPropsInfo->hasPropertyByName(OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") )))
                        xProps->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsPlaceholderDependent") ), ::cppu::bool2any( sal_False ) );
                }
            }
        }

        if( !mbIsPlaceholder && maHref.getLength() )
        {
            uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );

            if( xProps.is() )
            {
                uno::Any        aAny;
                OUString        aPersistName( GetImport().ResolveEmbeddedObjectURL( maHref, maCLSID ) );
                const OUString  sURL(RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.EmbeddedObject:" ));

                aAny <<= ( aPersistName = aPersistName.copy( sURL.getLength() ) );
                xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "PersistName" ) ), aAny );
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
    // #100592#
    if( mxBase64Stream.is() )
    {
        OUString aPersistName( GetImport().ResolveEmbeddedObjectURL( maHref, OUString() ) );
        const OUString  sURL(RTL_CONSTASCII_USTRINGPARAM( "vnd.sun.star.EmbeddedObject:" ));

        aPersistName = aPersistName.copy( sURL.getLength() );

        uno::Reference< beans::XPropertySet > xProps(mxShape, uno::UNO_QUERY);
        if( xProps.is() )
            xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "PersistName" ) ), uno::makeAny( aPersistName ) );
    }

    SdXMLShapeContext::EndElement();
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLObjectShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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
    USHORT nPrefix, const ::rtl::OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    // #100592#
    SvXMLImportContext* pContext = NULL;

    if((XML_NAMESPACE_OFFICE == nPrefix) && IsXMLToken(rLocalName, XML_BINARY_DATA))
    {
        maHref = OUString( RTL_CONSTASCII_USTRINGPARAM( "#Obj12345678" ) );
        mxBase64Stream =    GetImport().ResolveEmbeddedObjectURLFromBase64( maHref );
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
        if( maCLSID.getLength() != 0 )
        {
            uno::Reference< beans::XPropertySet > xPropSet(mxShape, uno::UNO_QUERY);
            if( xPropSet.is() )
            {
                xPropSet->setPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("CLSID") ), uno::makeAny( maCLSID ) );

                uno::Reference< lang::XComponent > xComp;
                xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Model") ) ) >>= xComp;
                DBG_ASSERT( xComp.is(), "no xModel for own OLE format" );
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

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLAppletShapeContext, SdXMLShapeContext );

SdXMLAppletShapeContext::SdXMLAppletShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
  mbIsScript( sal_False )
{
}

SdXMLAppletShapeContext::~SdXMLAppletShapeContext()
{
}

void SdXMLAppletShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    char* pService = "com.sun.star.drawing.AppletShape";
    AddShape( pService );

    if( mxShape.is() )
    {
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();
        GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLAppletShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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

        if( maParams.getLength() )
        {
            aAny <<= maParams;
            xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletCommands" ) ), aAny );
        }

        if( maHref.getLength() )
        {
            aAny <<= maHref;
            xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletCodeBase" ) ), aAny );
        }

        if( maAppletName.getLength() )
        {
            aAny <<= maAppletName;
            xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletName" ) ), aAny );
        }

        if( mbIsScript )
        {
            aAny <<= mbIsScript;
            xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletIsScript" ) ), aAny );

        }

        if( maAppletCode.getLength() )
        {
            aAny <<= maAppletCode;
            xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "AppletCode" ) ), aAny );
        }

        SetThumbnail();
    }

    SdXMLShapeContext::EndElement();
}

SvXMLImportContext * SdXMLAppletShapeContext::CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = NULL;

    if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_PARAM ) )
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

        if( aParamName.getLength() )
        {
            sal_Int32 nIndex = maParams.getLength();
            maParams.realloc( nIndex + 1 );
            maParams[nIndex].Name = aParamName;
            maParams[nIndex].Handle = -1;
            maParams[nIndex].Value <<= aParamValue;
            maParams[nIndex].State = beans::PropertyState_DIRECT_VALUE;
        }

        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return SdXMLShapeContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLPluginShapeContext, SdXMLShapeContext );

SdXMLPluginShapeContext::SdXMLPluginShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes) :
SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes ),
mbMedia( false )
{
}

SdXMLPluginShapeContext::~SdXMLPluginShapeContext()
{
}

void SdXMLPluginShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList)
{
    // watch for MimeType attribute to see if we have a media object
    for( sal_Int16 n = 0, nAttrCount = ( xAttrList.is() ? xAttrList->getLength() : 0 ); n < nAttrCount; ++n )
    {
        OUString    aLocalName;
        sal_uInt16  nPrefix = GetImport().GetNamespaceMap().GetKeyByAttrName( xAttrList->getNameByIndex( n ), &aLocalName );

        if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( aLocalName, XML_MIME_TYPE ) )
        {
            if( 0 == xAttrList->getValueByIndex( n ).compareToAscii( "application/vnd.sun.star.media" ) )
                mbMedia = true;

            // leave this loop
            n = nAttrCount - 1;
        }
    }

    char* pService;

    if( mbMedia )
        pService = "com.sun.star.drawing.MediaShape";
    else
        pService = "com.sun.star.drawing.PluginShape";

    AddShape( pService );

    if( mxShape.is() )
    {
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();
        GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLPluginShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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
            maHref = GetImport().GetAbsoluteReference(rValue);
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

        if( !mbMedia )
        {
            // in case we have a plugin object
            if( maParams.getLength() )
            {
                aAny <<= maParams;
                xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "PluginCommands" ) ), aAny );
            }

            if( maMimeType.getLength() )
            {
                aAny <<= maMimeType;
                xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "PluginMimeType" ) ), aAny );
            }

            if( maHref.getLength() )
            {
                aAny <<= maHref;
                xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "PluginURL" ) ), aAny );
            }
        }
        else
        {
            // in case we have a media object
            xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "MediaURL" ) ), uno::makeAny( maHref ) );

            for( sal_Int32 nParam = 0; nParam < maParams.getLength(); ++nParam )
            {
                const OUString& rName = maParams[ nParam ].Name;

                if( 0 == rName.compareToAscii( "Loop" ) )
                {
                    OUString aValueStr;
                    maParams[ nParam ].Value >>= aValueStr;
                    xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Loop" ) ),
                        uno::makeAny( static_cast< sal_Bool >( 0 == aValueStr.compareToAscii( "true" ) ) ) );
                }
                else if( 0 == rName.compareToAscii( "Mute" ) )
                {
                    OUString aValueStr;
                    maParams[ nParam ].Value >>= aValueStr;
                    xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Mute" ) ),
                        uno::makeAny( static_cast< sal_Bool >( 0 == aValueStr.compareToAscii( "true" ) ) ) );
                }
                else if( 0 == rName.compareToAscii( "VolumeDB" ) )
                {
                    OUString aValueStr;
                    maParams[ nParam ].Value >>= aValueStr;
                    xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "VolumeDB" ) ),
                                                uno::makeAny( static_cast< sal_Int16 >( aValueStr.toInt32() ) ) );
                }
                else if( 0 == rName.compareToAscii( "Zoom" ) )
                {
                    OUString            aZoomStr;
                    media::ZoomLevel    eZoomLevel;

                    maParams[ nParam ].Value >>= aZoomStr;

                    if( 0 == aZoomStr.compareToAscii( "25%" ) )
                        eZoomLevel = media::ZoomLevel_ZOOM_1_TO_4;
                    else if( 0 == aZoomStr.compareToAscii( "50%" ) )
                        eZoomLevel = media::ZoomLevel_ZOOM_1_TO_2;
                    else if( 0 == aZoomStr.compareToAscii( "100%" ) )
                        eZoomLevel = media::ZoomLevel_ORIGINAL;
                    else if( 0 == aZoomStr.compareToAscii( "200%" ) )
                        eZoomLevel = media::ZoomLevel_ZOOM_2_TO_1;
                    else if( 0 == aZoomStr.compareToAscii( "400%" ) )
                        eZoomLevel = media::ZoomLevel_ZOOM_4_TO_1;
                    else if( 0 == aZoomStr.compareToAscii( "fit" ) )
                        eZoomLevel = media::ZoomLevel_FIT_TO_WINDOW;
                    else if( 0 == aZoomStr.compareToAscii( "fixedfit" ) )
                        eZoomLevel = media::ZoomLevel_FIT_TO_WINDOW_FIXED_ASPECT;
                    else if( 0 == aZoomStr.compareToAscii( "fullscreen" ) )
                        eZoomLevel = media::ZoomLevel_FULLSCREEN;
                    else
                        eZoomLevel = media::ZoomLevel_NOT_AVAILABLE;

                    xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "Zoom" ) ), uno::makeAny( eZoomLevel ) );
                }
            }
        }

        SetThumbnail();
    }

    SdXMLShapeContext::EndElement();
}

SvXMLImportContext * SdXMLPluginShapeContext::CreateChildContext( USHORT nPrefix, const ::rtl::OUString& rLocalName, const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = NULL;

    if( nPrefix == XML_NAMESPACE_DRAW && IsXMLToken( rLocalName, XML_PARAM ) )
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

            if( aParamName.getLength() )
            {
                sal_Int32 nIndex = maParams.getLength();
                maParams.realloc( nIndex + 1 );
                maParams[nIndex].Name = aParamName;
                maParams[nIndex].Handle = -1;
                maParams[nIndex].Value <<= aParamValue;
                maParams[nIndex].State = beans::PropertyState_DIRECT_VALUE;
            }
        }

        return new SvXMLImportContext( GetImport(), nPrefix, rLocalName );
    }

    return SdXMLShapeContext::CreateChildContext( nPrefix, rLocalName, xAttrList );
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLFloatingFrameShapeContext, SdXMLShapeContext );

SdXMLFloatingFrameShapeContext::SdXMLFloatingFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

SdXMLFloatingFrameShapeContext::~SdXMLFloatingFrameShapeContext()
{
}

void SdXMLFloatingFrameShapeContext::StartElement( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& )
{
    char* pService = "com.sun.star.drawing.FrameShape";
    AddShape( pService );

    if( mxShape.is() )
    {
        SetLayer();

        // set pos, size, shear and rotate
        SetTransformation();

        uno::Reference< beans::XPropertySet > xProps( mxShape, uno::UNO_QUERY );
        if( xProps.is() )
        {
            uno::Any aAny;

            if( maFrameName.getLength() )
            {
                aAny <<= maFrameName;
                xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FrameName" ) ), aAny );
            }

            if( maHref.getLength() )
            {
                aAny <<= maHref;
                xProps->setPropertyValue( OUString( RTL_CONSTASCII_USTRINGPARAM( "FrameURL" ) ), aAny );
            }
        }

        SetStyle();

        GetImport().GetShapeImport()->finishShape( mxShape, mxAttrList, mxShapes );
    }
}

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLFloatingFrameShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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
    SetThumbnail();
    SdXMLShapeContext::EndElement();
}

//////////////////////////////////////////////////////////////////////////////

TYPEINIT1( SdXMLFrameShapeContext, SdXMLShapeContext );

SdXMLFrameShapeContext::SdXMLFrameShapeContext( SvXMLImport& rImport, sal_uInt16 nPrfx,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
        com::sun::star::uno::Reference< com::sun::star::drawing::XShapes >& rShapes)
: SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
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

SvXMLImportContext *SdXMLFrameShapeContext::CreateChildContext( USHORT nPrefix,
    const OUString& rLocalName,
    const uno::Reference< xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext * pContext = 0;

    if( !mxImplContext.Is() )
    {
        pContext = GetImport().GetShapeImport()->CreateFrameChildContext(
                        GetImport(), nPrefix, rLocalName, xAttrList, mxShapes, mxAttrList );
    }
    else if( (nPrefix == XML_NAMESPACE_OFFICE && IsXMLToken( rLocalName, XML_EVENT_LISTENERS ) ) ||
             (nPrefix == XML_NAMESPACE_DRAW && (IsXMLToken( rLocalName, XML_GLUE_POINT ) ||
                                                IsXMLToken( rLocalName, XML_THUMBNAIL ) ) ) )
    {
        SvXMLImportContext *pImplContext = &mxImplContext;
        pContext = PTR_CAST( SdXMLShapeContext, pImplContext )->CreateChildContext( nPrefix,
                                                                        rLocalName, xAttrList );
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
    mxImplContext = 0;
    SdXMLShapeContext::EndElement();
}

void SdXMLFrameShapeContext::processAttribute( sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
{
    // ignore
}

TYPEINIT1( SdXMLCustomShapeContext, SdXMLShapeContext );

SdXMLCustomShapeContext::SdXMLCustomShapeContext(
    SvXMLImport& rImport,
    sal_uInt16 nPrfx,
    const OUString& rLocalName,
    const com::sun::star::uno::Reference< com::sun::star::xml::sax::XAttributeList>& xAttrList,
    uno::Reference< drawing::XShapes >& rShapes)
:   SdXMLShapeContext( rImport, nPrfx, rLocalName, xAttrList, rShapes )
{
}

//////////////////////////////////////////////////////////////////////////////

SdXMLCustomShapeContext::~SdXMLCustomShapeContext()
{
}

//////////////////////////////////////////////////////////////////////////////

// this is called from the parent group for each unparsed attribute in the attribute list
void SdXMLCustomShapeContext::processAttribute( sal_uInt16 nPrefix, const ::rtl::OUString& rLocalName, const ::rtl::OUString& rValue )
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

//////////////////////////////////////////////////////////////////////////////

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
                if ( maCustomShapeEngine.getLength() )
                {
                    uno::Any aAny;
                    aAny <<= maCustomShapeEngine;
                    xPropSet->setPropertyValue( EASGet( EAS_CustomShapeEngine ), aAny );
                }
                if ( maCustomShapeData.getLength() )
                {
                    uno::Any aAny;
                    aAny <<= maCustomShapeData;
                    xPropSet->setPropertyValue( EASGet( EAS_CustomShapeData ), aAny );
                }
            }
        }
        catch( uno::Exception& )
        {
            DBG_ERROR( "could not set enhanced customshape geometry" );
        }
        SdXMLShapeContext::StartElement(xAttrList);
    }
}

void SdXMLCustomShapeContext::EndElement()
{
    if ( maCustomShapeGeometry.size() )
    {
        const rtl::OUString sCustomShapeGeometry    ( RTL_CONSTASCII_USTRINGPARAM( "CustomShapeGeometry" ) );

        // converting the vector to a sequence
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
        catch( uno::Exception& )
        {
            DBG_ERROR( "could not set enhanced customshape geometry" );
        }
    }

    SdXMLShapeContext::EndElement();
}

//////////////////////////////////////////////////////////////////////////////

SvXMLImportContext* SdXMLCustomShapeContext::CreateChildContext(
    USHORT nPrefix, const ::rtl::OUString& rLocalName,
    const uno::Reference<xml::sax::XAttributeList>& xAttrList )
{
    SvXMLImportContext* pContext = NULL;
    if ( XML_NAMESPACE_DRAW == nPrefix )
    {
        if ( IsXMLToken( rLocalName, XML_ENHANCED_GEOMETRY ) )
        {
            uno::Reference< beans::XPropertySet > xPropSet( mxShape,uno::UNO_QUERY );
            if ( xPropSet.is() )
                pContext = new XMLEnhancedCustomShapeContext( GetImport(), nPrefix, rLocalName, maCustomShapeGeometry );
        }
    }
    // delegate to parent class if no context could be created
    if ( NULL == pContext )
        pContext = SdXMLShapeContext::CreateChildContext( nPrefix, rLocalName,
                                                         xAttrList);
    return pContext;
}
