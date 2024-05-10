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

#include <sal/config.h>
#include <sal/log.hxx>

#include <filter/msfilter/util.hxx>
#include <o3tl/string_view.hxx>
#include <o3tl/any.hxx>
#include <oox/core/xmlfilterbase.hxx>
#include <oox/export/shapes.hxx>
#include <oox/export/utils.hxx>
#include <oox/token/namespaces.hxx>
#include <oox/token/relationship.hxx>
#include <oox/token/tokens.hxx>

#include <initializer_list>
#include <string_view>

#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/document/XExporter.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterPair.hpp>
#include <com/sun/star/drawing/EnhancedCustomShapeParameterType.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <com/sun/star/embed/XEmbedPersist.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/text/XSimpleText.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/table/XTable.hpp>
#include <com/sun/star/table/XMergeableCell.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>
#include <com/sun/star/drawing/XGluePointsSupplier.hpp>
#include <com/sun/star/container/XIdentifierAccess.hpp>
#include <com/sun/star/table/BorderLineStyle.hpp>
#include <tools/globname.hxx>
#include <comphelper/classids.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/storagehelper.hxx>
#include <sot/exchange.hxx>
#include <utility>
#include <vcl/graph.hxx>
#include <vcl/outdev.hxx>
#include <filter/msfilter/escherex.hxx>
#include <svx/svdoashp.hxx>
#include <svx/svdoole2.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <oox/export/chartexport.hxx>
#include <oox/mathml/imexport.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <oox/export/DMLPresetShapeExport.hxx>

#include <frozen/bits/defines.h>
#include <frozen/bits/elsa_std.h>
#include <frozen/set.h>
#include <frozen/unordered_map.h>


using namespace ::css;
using namespace ::css::beans;
using namespace ::css::uno;
using namespace ::css::drawing;
using namespace ::css::table;
using namespace ::css::container;
using namespace ::css::document;
using namespace ::css::text;

using ::css::io::XOutputStream;
using ::css::chart2::XChartDocument;
using ::css::frame::XModel;

using ::oox::core::XmlFilterBase;
using ::sax_fastparser::FSHelperPtr;


namespace oox {

static void lcl_ConvertProgID(std::u16string_view rProgID,
    OUString & o_rMediaType, OUString & o_rRelationType, OUString & o_rFileExtension)
{
    if (rProgID == u"Excel.Sheet.12")
    {
        o_rMediaType = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
        o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
        o_rFileExtension = "xlsx";
    }
    else if (o3tl::starts_with(rProgID, u"Excel.SheetBinaryMacroEnabled.12") )
    {
        o_rMediaType = "application/vnd.ms-excel.sheet.binary.macroEnabled.12";
        o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
        o_rFileExtension = "xlsb";
    }
    else if (o3tl::starts_with(rProgID, u"Excel.SheetMacroEnabled.12"))
    {
        o_rMediaType = "application/vnd.ms-excel.sheet.macroEnabled.12";
        o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
        o_rFileExtension = "xlsm";
    }
    else if (o3tl::starts_with(rProgID, u"Excel.Sheet"))
    {
        o_rMediaType = "application/vnd.ms-excel";
        o_rRelationType = oox::getRelationship(Relationship::OLEOBJECT);
        o_rFileExtension = "xls";
    }
    else if (rProgID == u"PowerPoint.Show.12")
    {
        o_rMediaType = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
        o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
        o_rFileExtension = "pptx";
    }
    else if (rProgID == u"PowerPoint.ShowMacroEnabled.12")
    {
        o_rMediaType = "application/vnd.ms-powerpoint.presentation.macroEnabled.12";
        o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
        o_rFileExtension = "pptm";
    }
    else if (o3tl::starts_with(rProgID, u"PowerPoint.Show"))
    {
        o_rMediaType = "application/vnd.ms-powerpoint";
        o_rRelationType = oox::getRelationship(Relationship::OLEOBJECT);
        o_rFileExtension = "ppt";
    }
    else if (o3tl::starts_with(rProgID, u"PowerPoint.Slide.12"))
    {
       o_rMediaType = "application/vnd.openxmlformats-officedocument.presentationml.slide";
       o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
       o_rFileExtension = "sldx";
    }
    else if (rProgID == u"PowerPoint.SlideMacroEnabled.12")
    {
       o_rMediaType = "application/vnd.ms-powerpoint.slide.macroEnabled.12";
       o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
       o_rFileExtension = "sldm";
    }
    else if (rProgID == u"Word.DocumentMacroEnabled.12")
    {
        o_rMediaType = "application/vnd.ms-word.document.macroEnabled.12";
        o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
        o_rFileExtension = "docm";
    }
    else if (rProgID == u"Word.Document.12")
    {
        o_rMediaType = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
        o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
        o_rFileExtension = "docx";
    }
    else if (rProgID == u"Word.Document.8")
    {
        o_rMediaType = "application/msword";
        o_rRelationType = oox::getRelationship(Relationship::OLEOBJECT);
        o_rFileExtension = "doc";
    }
    else if (rProgID == u"Excel.Chart.8")
    {
        o_rMediaType = "application/vnd.ms-excel";
        o_rRelationType = oox::getRelationship(Relationship::OLEOBJECT);
        o_rFileExtension = "xls";
    }
    else if (rProgID == u"AcroExch.Document.11")
    {
        o_rMediaType = "application/pdf";
        o_rRelationType = oox::getRelationship(Relationship::OLEOBJECT);
        o_rFileExtension = "pdf";
    }
    else
    {
        o_rMediaType = "application/vnd.openxmlformats-officedocument.oleObject";
        o_rRelationType = oox::getRelationship(Relationship::OLEOBJECT);
        o_rFileExtension = "bin";
    }
}

static uno::Reference<io::XInputStream> lcl_StoreOwnAsOOXML(
    uno::Reference<uno::XComponentContext> const& xContext,
    uno::Reference<embed::XEmbeddedObject> const& xObj,
    char const*& o_rpProgID,
    OUString & o_rMediaType, OUString & o_rRelationType, OUString & o_rSuffix)
{
    static struct {
        struct {
            sal_uInt32 n1;
            sal_uInt16 n2, n3;
            sal_uInt8 b8, b9, b10, b11, b12, b13, b14, b15;
        } ClassId;
        char const* pFilterName;
        char const* pMediaType;
        char const* pProgID;
        char const* pSuffix;
    } const s_Mapping[] = {
        { {SO3_SW_CLASSID_60}, "MS Word 2007 XML", "application/vnd.openxmlformats-officedocument.wordprocessingml.document", "Word.Document.12", "docx" },
        { {SO3_SC_CLASSID_60}, "Calc MS Excel 2007 XML", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", "Excel.Sheet.12", "xlsx" },
        { {SO3_SIMPRESS_CLASSID_60}, "Impress MS PowerPoint 2007 XML", "application/vnd.openxmlformats-officedocument.presentationml.presentation", "PowerPoint.Show.12", "pptx" },
        // FIXME: Draw does not appear to have a MSO format export filter?
//            { {SO3_SDRAW_CLASSID}, "", "", "", "" },
        { {SO3_SCH_CLASSID_60}, "unused", "", "", "" },
        { {SO3_SM_CLASSID_60}, "unused", "", "", "" },
    };

    const char * pFilterName(nullptr);
    SvGlobalName const classId(xObj->getClassID());
    for (auto & i : s_Mapping)
    {
        auto const& rId(i.ClassId);
        SvGlobalName const temp(rId.n1, rId.n2, rId.n3, rId.b8, rId.b9, rId.b10, rId.b11, rId.b12, rId.b13, rId.b14, rId.b15);
        if (temp == classId)
        {
            assert(SvGlobalName(SO3_SCH_CLASSID_60) != classId); // chart should be written elsewhere!
            assert(SvGlobalName(SO3_SM_CLASSID_60) != classId); // formula should be written elsewhere!
            pFilterName = i.pFilterName;
            o_rMediaType = OUString::createFromAscii(i.pMediaType);
            o_rpProgID = i.pProgID;
            o_rSuffix = OUString::createFromAscii(i.pSuffix);
            o_rRelationType = oox::getRelationship(Relationship::PACKAGE);
            break;
        }
    }

    if (!pFilterName)
    {
        SAL_WARN("oox.shape", "oox::GetOLEObjectStream: unknown ClassId " << classId.GetHexName());
        return nullptr;
    }

    if (embed::EmbedStates::LOADED == xObj->getCurrentState())
    {
        xObj->changeState(embed::EmbedStates::RUNNING);
    }
    // use a temp stream - while it would work to store directly to a
    // fragment stream, an error during export means we'd have to delete it
    uno::Reference<io::XStream> const xTempStream(
        xContext->getServiceManager()->createInstanceWithContext(
            u"com.sun.star.comp.MemoryStream"_ustr, xContext),
        uno::UNO_QUERY_THROW);
    uno::Sequence<beans::PropertyValue> args( comphelper::InitPropertySequence({
            { "OutputStream", Any(xTempStream->getOutputStream()) },
            { "FilterName", Any(OUString::createFromAscii(pFilterName)) }
        }));
    uno::Reference<frame::XStorable> xStorable(xObj->getComponent(), uno::UNO_QUERY);
    try
    {
        xStorable->storeToURL(u"private:stream"_ustr, args);
    }
    catch (uno::Exception const&)
    {
        TOOLS_WARN_EXCEPTION("oox.shape", "oox::GetOLEObjectStream");
        return nullptr;
    }
    xTempStream->getOutputStream()->closeOutput();
    return xTempStream->getInputStream();
}

uno::Reference<io::XInputStream> GetOLEObjectStream(
        uno::Reference<uno::XComponentContext> const& xContext,
        uno::Reference<embed::XEmbeddedObject> const& xObj,
        std::u16string_view i_rProgID,
        OUString & o_rMediaType,
        OUString & o_rRelationType,
        OUString & o_rSuffix,
        const char *& o_rpProgID)
{
    uno::Reference<io::XInputStream> xInStream;
    try
    {
        uno::Reference<document::XStorageBasedDocument> const xParent(
            uno::Reference<container::XChild>(xObj, uno::UNO_QUERY_THROW)->getParent(),
            uno::UNO_QUERY_THROW);
        uno::Reference<embed::XStorage> const xParentStorage(xParent->getDocumentStorage());
        OUString const entryName(
            uno::Reference<embed::XEmbedPersist>(xObj, uno::UNO_QUERY_THROW)->getEntryName());

        if (xParentStorage->isStreamElement(entryName))
        {
            lcl_ConvertProgID(i_rProgID, o_rMediaType, o_rRelationType, o_rSuffix);
            xInStream = xParentStorage->cloneStreamElement(entryName)->getInputStream();
            // TODO: make it possible to take the sMediaType from the stream
        }
        else // the object is ODF - either the whole document is
        {    // ODF, or the OLE was edited so it was converted to ODF
            xInStream = lcl_StoreOwnAsOOXML(xContext, xObj,
                    o_rpProgID, o_rMediaType, o_rRelationType, o_rSuffix);
        }
    }
    catch (uno::Exception const&)
    {
        TOOLS_WARN_EXCEPTION("oox.shape", "oox::GetOLEObjectStream");
    }
    return xInStream;
}

} // namespace oox

namespace oox::drawingml {

ShapeExport::ShapeExport( sal_Int32 nXmlNamespace, FSHelperPtr pFS, ShapeHashMap* pShapeMap, XmlFilterBase* pFB, DocumentType eDocumentType, DMLTextExport* pTextExport, bool bUserShapes )
    : DrawingML( std::move(pFS), pFB, eDocumentType, pTextExport )
    , m_nEmbeddedObjects(0)
    , mnShapeIdMax( 1 )
    , mbUserShapes( bUserShapes )
    , mnXmlNamespace( nXmlNamespace )
    , maMapModeSrc( MapUnit::Map100thMM )
    , maMapModeDest( MapUnit::MapInch, Point(), Fraction( 1, 576 ), Fraction( 1, 576 ) )
    , mpShapeMap( pShapeMap ? pShapeMap : &maShapeMap )
{
    mpURLTransformer = std::make_shared<URLTransformer>();
}

void ShapeExport::SetURLTranslator(const std::shared_ptr<URLTransformer>& pTransformer)
{
    mpURLTransformer = pTransformer;
}

awt::Size ShapeExport::MapSize( const awt::Size& rSize ) const
{
    Size aRetSize( OutputDevice::LogicToLogic( Size( rSize.Width, rSize.Height ), maMapModeSrc, maMapModeDest ) );

    if ( !aRetSize.Width() )
        aRetSize.AdjustWidth( 1 );
    if ( !aRetSize.Height() )
        aRetSize.AdjustHeight( 1 );
    return awt::Size( aRetSize.Width(), aRetSize.Height() );
}

static bool IsNonEmptySimpleText(const Reference<XInterface>& xIface)
{
    if (Reference<XSimpleText> xText{ xIface, UNO_QUERY })
        return xText->getString().getLength();

    return false;
}

bool ShapeExport::NonEmptyText( const Reference< XInterface >& xIface )
{
    Reference< XPropertySet > xPropSet( xIface, UNO_QUERY );

    if( xPropSet.is() )
    {
        Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();
        if ( xPropSetInfo.is() )
        {
            if ( xPropSetInfo->hasPropertyByName( u"IsEmptyPresentationObject"_ustr ) )
            {
                bool bIsEmptyPresObj = false;
                if ( xPropSet->getPropertyValue( u"IsEmptyPresentationObject"_ustr ) >>= bIsEmptyPresObj )
                {
                    SAL_INFO("oox.shape", "empty presentation object " << bIsEmptyPresObj << " , props:");
                    if( bIsEmptyPresObj )
                       return true;
                }
            }

            if ( xPropSetInfo->hasPropertyByName( u"IsPresentationObject"_ustr ) )
            {
                bool bIsPresObj = false;
                if ( xPropSet->getPropertyValue( u"IsPresentationObject"_ustr ) >>= bIsPresObj )
                {
                    SAL_INFO("oox.shape", "presentation object " << bIsPresObj << ", props:");
                    if( bIsPresObj )
                       return true;
                }
            }
        }
    }

    return IsNonEmptySimpleText(xIface);
}

static void AddExtLst(FSHelperPtr const& pFS, Reference<XPropertySet> const& xShape)
{
    if (xShape->getPropertySetInfo()->hasPropertyByName(u"Decorative"_ustr)
        && xShape->getPropertyValue(u"Decorative"_ustr).get<bool>())
    {
        pFS->startElementNS(XML_a, XML_extLst);
//            FSNS(XML_xmlns, XML_a), GetExport().GetFilter().getNamespaceURL(OOX_NS(dml)));
        pFS->startElementNS(XML_a, XML_ext,
            // MSO uses this "URI" which is obviously not a URI
            XML_uri, "{C183D7F6-B498-43B3-948B-1728B52AA6E4}");
        pFS->singleElementNS(XML_adec, XML_decorative,
            FSNS(XML_xmlns, XML_adec), "http://schemas.microsoft.com/office/drawing/2017/decorative",
            XML_val, "1");
        pFS->endElementNS(XML_a, XML_ext);
        pFS->endElementNS(XML_a, XML_extLst);
    }
}

ShapeExport& ShapeExport::WritePolyPolygonShape( const Reference< XShape >& xShape, const bool bClosed )
{
    SAL_INFO("oox.shape", "write polypolygon shape");

    FSHelperPtr pFS = GetFS();
    pFS->startElementNS(mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp));

    awt::Point aPos = xShape->getPosition();
    // Position is relative to group for child elements in Word, but absolute in API.
    if (GetDocumentType() == DOCUMENT_DOCX && !mbUserShapes && m_xParent.is())
    {
        awt::Point aParentPos = m_xParent->getPosition();
        aPos.X -= aParentPos.X;
        aPos.Y -= aParentPos.Y;
    }
    awt::Size aSize = xShape->getSize();
    tools::Rectangle aRect(Point(aPos.X, aPos.Y), Size(aSize.Width, aSize.Height));

#if OSL_DEBUG_LEVEL > 0
    tools::PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon(xShape);
    awt::Size size = MapSize( awt::Size( aRect.GetWidth(), aRect.GetHeight() ) );
    SAL_INFO("oox.shape", "poly count " << aPolyPolygon.Count());
    SAL_INFO("oox.shape", "size: " << size.Width << " x " << size.Height);
#endif

    Reference<XPropertySet> const xProps(xShape, UNO_QUERY);
    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
    {
        pFS->startElementNS(mnXmlNamespace, XML_nvSpPr);
        pFS->startElementNS(mnXmlNamespace, XML_cNvPr,
                              XML_id, OString::number(GetNewShapeID(xShape)),
                              XML_name, GetShapeName(xShape));
        AddExtLst(pFS, xProps);
        pFS->endElementNS(mnXmlNamespace, XML_cNvPr);
    }
    pFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr);
    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
    {
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }

    // visual shape properties
    pFS->startElementNS(mnXmlNamespace, XML_spPr);
    WriteTransformation( xShape, aRect, XML_a );
    WritePolyPolygon(xShape, bClosed);
    if( xProps.is() ) {
        if( bClosed )
            WriteFill(xProps, aSize);
        WriteOutline( xProps );
    }

    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp) );

    return *this;
}

ShapeExport& ShapeExport::WriteClosedPolyPolygonShape( const Reference< XShape >& xShape )
{
    return WritePolyPolygonShape( xShape, true );
}

ShapeExport& ShapeExport::WriteOpenPolyPolygonShape( const Reference< XShape >& xShape )
{
    return WritePolyPolygonShape( xShape, false );
}

ShapeExport& ShapeExport::WriteGroupShape(const uno::Reference<drawing::XShape>& xShape)
{
    FSHelperPtr pFS = GetFS();

    sal_Int32 nGroupShapeToken = XML_grpSp;
    if (GetDocumentType() == DOCUMENT_DOCX && !mbUserShapes)
    {
        if (!m_xParent.is())
            nGroupShapeToken = XML_wgp; // toplevel
        else
            mnXmlNamespace = XML_wpg;
    }

    pFS->startElementNS(mnXmlNamespace, nGroupShapeToken);

    // non visual properties
    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
    {
        pFS->startElementNS(mnXmlNamespace, XML_nvGrpSpPr);
        pFS->startElementNS(mnXmlNamespace, XML_cNvPr,
                XML_id, OString::number(GetNewShapeID(xShape)),
                XML_name, GetShapeName(xShape));
        uno::Reference<beans::XPropertySet> const xShapeProps(xShape, uno::UNO_QUERY_THROW);
        AddExtLst(pFS, xShapeProps);
        pFS->endElementNS(mnXmlNamespace, XML_cNvPr);
        pFS->singleElementNS(mnXmlNamespace, XML_cNvGrpSpPr);
        WriteNonVisualProperties(xShape );
        pFS->endElementNS(mnXmlNamespace, XML_nvGrpSpPr);
    }
    else
        pFS->singleElementNS(mnXmlNamespace, XML_cNvGrpSpPr);

    // visual properties
    pFS->startElementNS(mnXmlNamespace, XML_grpSpPr);
    WriteShapeTransformation(xShape, XML_a, false, false, true);
    pFS->endElementNS(mnXmlNamespace, XML_grpSpPr);

    uno::Reference<drawing::XShapes> xGroupShape(xShape, uno::UNO_QUERY_THROW);
    uno::Reference<drawing::XShape> xParent = m_xParent;
    m_xParent = xShape;
    for (sal_Int32 i = 0; i < xGroupShape->getCount(); ++i)
    {
        uno::Reference<drawing::XShape> xChild(xGroupShape->getByIndex(i), uno::UNO_QUERY_THROW);
        sal_Int32 nSavedNamespace = mnXmlNamespace;

        uno::Reference<lang::XServiceInfo> xServiceInfo(xChild, uno::UNO_QUERY_THROW);
        if (GetDocumentType() == DOCUMENT_DOCX && !mbUserShapes)
        {
            // tdf#128820: WriteGraphicObjectShapePart calls WriteTextShape for non-empty simple
            // text objects, which needs writing into wps::wsp element, so make sure to use wps
            // namespace for those objects
            if (xServiceInfo->supportsService(u"com.sun.star.drawing.GraphicObjectShape"_ustr)
                && !IsNonEmptySimpleText(xChild))
                mnXmlNamespace = XML_pic;
            else
                mnXmlNamespace = XML_wps;
        }
        WriteShape(xChild);

        mnXmlNamespace = nSavedNamespace;
    }
    m_xParent = std::move(xParent);

    pFS->endElementNS(mnXmlNamespace, nGroupShapeToken);
    return *this;
}
namespace
{

constexpr frozen::set<std::u16string_view, 57> constDenySet(
{
    u"block-arc",
    u"rectangle",
    u"ellipse",
    u"ring",
    u"can",
    u"cube",
    u"paper",
    u"frame",
    u"forbidden",
    u"smiley",
    u"sun",
    u"flower",
    u"bracket-pair",
    u"brace-pair",
    u"quad-bevel",
    u"round-rectangular-callout",
    u"rectangular-callout",
    u"round-callout",
    u"cloud-callout",
    u"line-callout-1",
    u"line-callout-2",
    u"line-callout-3",
    u"paper",
    u"vertical-scroll",
    u"horizontal-scroll",
    u"mso-spt34",
    u"mso-spt75",
    u"mso-spt164",
    u"mso-spt180",
    u"flowchart-process",
    u"flowchart-alternate-process",
    u"flowchart-decision",
    u"flowchart-data",
    u"flowchart-predefined-process",
    u"flowchart-internal-storage",
    u"flowchart-document",
    u"flowchart-multidocument",
    u"flowchart-terminator",
    u"flowchart-preparation",
    u"flowchart-manual-input",
    u"flowchart-manual-operation",
    u"flowchart-connector",
    u"flowchart-off-page-connector",
    u"flowchart-card",
    u"flowchart-punched-tape",
    u"flowchart-summing-junction",
    u"flowchart-or",
    u"flowchart-collate",
    u"flowchart-sort",
    u"flowchart-extract",
    u"flowchart-merge",
    u"flowchart-stored-data",
    u"flowchart-delay",
    u"flowchart-sequential-access",
    u"flowchart-magnetic-disk",
    u"flowchart-direct-access-storage",
    u"flowchart-display"
});

constexpr frozen::set<std::u16string_view, 5> constAllowSet(
{
    u"heart",
    u"puzzle",
    u"col-60da8460",
    u"col-502ad400",
    u"sinusoid"
});

} // end anonymous namespace

static bool lcl_IsOnDenylist(OUString const & rShapeType)
{
    return constDenySet.find(rShapeType) != constDenySet.end();
}

static bool lcl_IsOnAllowlist(OUString const & rShapeType)
{
    return constAllowSet.find(rShapeType) != constAllowSet.end();
}

static bool lcl_GetHandlePosition( sal_Int32 &nValue, const EnhancedCustomShapeParameter &rParam, const Sequence< EnhancedCustomShapeAdjustmentValue > &rSeq)
{
    bool bAdj = false;
    if ( rParam.Value.getValueTypeClass() == TypeClass_DOUBLE )
    {
        double fValue(0.0);
        if ( rParam.Value >>= fValue )
            nValue = static_cast<sal_Int32>(fValue);
    }
    else
        rParam.Value >>= nValue;

    if ( rParam.Type == EnhancedCustomShapeParameterType::ADJUSTMENT)
    {
        bAdj = true;
        sal_Int32 nIdx = nValue;
        if ( nIdx < rSeq.getLength() )
        {
            if ( rSeq[ nIdx ] .Value.getValueTypeClass() == TypeClass_DOUBLE )
            {
                double fValue(0.0);
                rSeq[ nIdx ].Value >>= fValue;
                nValue = fValue;

            }
            else
            {
                rSeq[ nIdx ].Value >>= nValue;
            }
        }
    }
    return bAdj;
}

static void lcl_AnalyzeHandles( const uno::Sequence<beans::PropertyValues> & rHandles,
        std::vector< std::pair< sal_Int32, sal_Int32> > &rHandlePositionList,
        const Sequence< EnhancedCustomShapeAdjustmentValue > &rSeq)
{
    for ( const Sequence< PropertyValue >& rPropSeq : rHandles )
    {
        static constexpr OUStringLiteral sPosition( u"Position"  );
        bool bPosition = false;
        EnhancedCustomShapeParameterPair aPosition;
        for ( const PropertyValue& rPropVal: rPropSeq )
        {
            if ( rPropVal.Name == sPosition )
            {
                if ( rPropVal.Value >>= aPosition )
                    bPosition = true;
            }
        }
        if ( bPosition )
        {
            sal_Int32 nXPosition = 0;
            sal_Int32 nYPosition = 0;
            // For polar handles, nXPosition is radius and nYPosition is angle
            lcl_GetHandlePosition( nXPosition, aPosition.First , rSeq );
            lcl_GetHandlePosition( nYPosition, aPosition.Second, rSeq );
            rHandlePositionList.emplace_back( nXPosition, nYPosition );
        }
    }
}

static void lcl_AppendAdjustmentValue( std::vector< std::pair< sal_Int32, sal_Int32> > &rAvList, sal_Int32 nAdjIdx, sal_Int32 nValue )
{
    rAvList.emplace_back( nAdjIdx , nValue );
}

static sal_Int32 lcl_NormalizeAngle( sal_Int32 nAngle )
{
    nAngle = nAngle % 360;
    return nAngle < 0 ? ( nAngle + 360 ) : nAngle ;
}

static sal_Int32 lcl_CircleAngle2CustomShapeEllipseAngleOOX(const sal_Int32 nInternAngle, const sal_Int32 nWidth, const sal_Int32 nHeight)
{
    if (nWidth != 0 || nHeight != 0)
    {
        double fAngle = basegfx::deg2rad<100>(nInternAngle); // intern 1/100 deg to rad
        fAngle = atan2(nHeight * sin(fAngle), nWidth * cos(fAngle)); // circle to ellipse
        fAngle = basegfx::rad2deg<60000>(fAngle); // rad to OOXML angle unit
        sal_Int32 nAngle = basegfx::fround(fAngle); // normalize
        nAngle = nAngle % 21600000;
        return nAngle < 0 ? (nAngle + 21600000) : nAngle;
    }
    else // should be handled by caller, dummy value
        return 0;
}

static OUString lcl_GetTarget(const css::uno::Reference<css::frame::XModel>& xModel,
                              std::u16string_view rURL)
{
    Reference<drawing::XDrawPagesSupplier> xDPS(xModel, uno::UNO_QUERY_THROW);
    Reference<drawing::XDrawPages> xDrawPages(xDPS->getDrawPages(), uno::UNO_SET_THROW);
    sal_uInt32 nPageCount = xDrawPages->getCount();
    OUString sTarget;

    for (sal_uInt32 i = 0; i < nPageCount; ++i)
    {
        Reference<XDrawPage> xDrawPage;
        xDrawPages->getByIndex(i) >>= xDrawPage;
        Reference<container::XNamed> xNamed(xDrawPage, UNO_QUERY);
        if (!xNamed)
            continue;
        OUString sSlideName = "#" + xNamed->getName();
        if (rURL == sSlideName)
        {
            sTarget = "slide" + OUString::number(i + 1) + ".xml";
            break;
        }
    }

    return sTarget;
}

ShapeExport& ShapeExport::WriteCustomShape( const Reference< XShape >& xShape )
{
    SAL_INFO("oox.shape", "write custom shape");
    Reference< XPropertySet > rXPropSet( xShape, UNO_QUERY );
    // First check, if this is a Fontwork-shape. For DrawingML, such a shape is a
    // TextBox shape with body property prstTxWarp.
    if (IsFontworkShape(rXPropSet))
    {
        ShapeExport::WriteTextShape(xShape); // qualifier to prevent PowerPointShapeExport
        return *this;
    }

    bool bHasGeometrySeq(false);
    Sequence< PropertyValue > aGeometrySeq;
    OUString sShapeType(u"non-primitive"_ustr); // default in ODF
    if (GetProperty(rXPropSet, u"CustomShapeGeometry"_ustr))
    {
        SAL_INFO("oox.shape", "got custom shape geometry");
        if (mAny >>= aGeometrySeq)
        {
            bHasGeometrySeq = true;
            SAL_INFO("oox.shape", "got custom shape geometry sequence");
            for (const PropertyValue& rProp : aGeometrySeq)
            {
                SAL_INFO("oox.shape", "geometry property: " << rProp.Name);
                if (rProp.Name == "Type")
                    rProp.Value >>= sShapeType;
            }
        }
    }

    bool bPredefinedHandlesUsed = true;
    bool bHasHandles = false;

    ShapeFlag nMirrorFlags = ShapeFlag::NONE;
    MSO_SPT eShapeType = EscherPropertyContainer::GetCustomShapeType( xShape, nMirrorFlags, sShapeType );
    assert(dynamic_cast< SdrObjCustomShape* >(SdrObject::getSdrObjectFromXShape(xShape)) && "Not a SdrObjCustomShape (!)");
    SdrObjCustomShape& rSdrObjCustomShape(static_cast< SdrObjCustomShape& >(*SdrObject::getSdrObjectFromXShape(xShape)));
    const bool bIsDefaultObject(
        EscherPropertyContainer::IsDefaultObject(
            rSdrObjCustomShape,
            eShapeType));
    OString sPresetShape = msfilter::util::GetOOXMLPresetGeometry(sShapeType);
    SAL_INFO("oox.shape", "custom shape type: " << sShapeType << " ==> " << sPresetShape);

    sal_Int32 nAdjustmentValuesIndex = -1;
    awt::Rectangle aViewBox;
    uno::Sequence<beans::PropertyValues> aHandles;

    bool bFlipH = false;
    bool bFlipV = false;

    if (bHasGeometrySeq)
    {
        for (int i = 0; i < aGeometrySeq.getLength(); i++)
        {
                const PropertyValue& rProp = aGeometrySeq[ i ];
                SAL_INFO("oox.shape", "geometry property: " << rProp.Name);

                if ( rProp.Name == "MirroredX" )
                    rProp.Value >>= bFlipH;

                if ( rProp.Name == "MirroredY" )
                    rProp.Value >>= bFlipV;
                if ( rProp.Name == "AdjustmentValues" )
                    nAdjustmentValuesIndex = i;
                else if ( rProp.Name == "Handles" )
                {
                    rProp.Value >>= aHandles;
                    if ( aHandles.hasElements() )
                        bHasHandles = true;
                    if( !bIsDefaultObject )
                        bPredefinedHandlesUsed = false;
                    // TODO: update nAdjustmentsWhichNeedsToBeConverted here
                }
                else if ( rProp.Name == "ViewBox" )
                    rProp.Value >>= aViewBox;
        }
    }

    FSHelperPtr pFS = GetFS();
    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
    {
        bool bUseBackground = false;
        if (GetProperty(rXPropSet, u"FillUseSlideBackground"_ustr))
            mAny >>= bUseBackground;
        if (bUseBackground)
            mpFS->startElementNS(mnXmlNamespace, XML_sp, XML_useBgFill, "1");
        else
            mpFS->startElementNS(mnXmlNamespace, XML_sp);

        bool isVisible = true ;
        if( GetProperty(rXPropSet, u"Visible"_ustr))
        {
            mAny >>= isVisible;
        }
        pFS->startElementNS( mnXmlNamespace, XML_nvSpPr );
        pFS->startElementNS(
            mnXmlNamespace, XML_cNvPr, XML_id,
            OString::number(GetShapeID(xShape) == -1 ? GetNewShapeID(xShape) : GetShapeID(xShape)),
            XML_name, GetShapeName(xShape), XML_hidden, sax_fastparser::UseIf("1", !isVisible));

        if( GetProperty(rXPropSet, u"URL"_ustr) )
        {
            OUString sURL;
            mAny >>= sURL;
            if( !sURL.isEmpty() )
            {
                OUString sRelId = mpFB->addRelation( mpFS->getOutputStream(),
                        oox::getRelationship(Relationship::HYPERLINK),
                        mpURLTransformer->getTransformedString(sURL),
                        mpURLTransformer->isExternalURL(sURL));

                mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId);
            }
        }

        OUString sBookmark;
        if (GetProperty(rXPropSet, u"Bookmark"_ustr))
            mAny >>= sBookmark;

        if (GetProperty(rXPropSet, u"OnClick"_ustr))
        {
            OUString sPPAction;
            presentation::ClickAction eClickAction = presentation::ClickAction_NONE;
            mAny >>= eClickAction;
            if (eClickAction != presentation::ClickAction_NONE)
            {
                switch (eClickAction)
                {
                    case presentation::ClickAction_STOPPRESENTATION:
                        sPPAction = "ppaction://hlinkshowjump?jump=endshow";
                        break;
                    case presentation::ClickAction_NEXTPAGE:
                        sPPAction = "ppaction://hlinkshowjump?jump=nextslide";
                        break;
                    case presentation::ClickAction_LASTPAGE:
                        sPPAction = "ppaction://hlinkshowjump?jump=lastslide";
                        break;
                    case presentation::ClickAction_PREVPAGE:
                        sPPAction = "ppaction://hlinkshowjump?jump=previousslide";
                        break;
                    case presentation::ClickAction_FIRSTPAGE:
                        sPPAction = "ppaction://hlinkshowjump?jump=firstslide";
                        break;
                    case presentation::ClickAction_BOOKMARK:
                        sBookmark = "#" + sBookmark;
                        break;
                    default:
                        break;
                }
            }
            if (!sPPAction.isEmpty())
                pFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), "", XML_action,
                                     sPPAction);
        }
        if (!sBookmark.isEmpty())
        {
            bool bExtURL = URLTransformer().isExternalURL(sBookmark);
            sBookmark = bExtURL ? sBookmark : lcl_GetTarget(GetFB()->getModel(), sBookmark);

            OUString sRelId
                = mpFB->addRelation(mpFS->getOutputStream(),
                                    bExtURL ? oox::getRelationship(Relationship::HYPERLINK)
                                            : oox::getRelationship(Relationship::SLIDE),
                                    sBookmark, bExtURL);
            if (bExtURL)
                mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId);
            else
                mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId,
                                      XML_action, "ppaction://hlinksldjump");
        }
        AddExtLst(pFS, rXPropSet);
        pFS->endElementNS(mnXmlNamespace, XML_cNvPr);
        pFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr);
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }
    else
    {
        pFS->startElementNS(mnXmlNamespace, XML_wsp);
        if (m_xParent.is())
        {
            pFS->startElementNS(mnXmlNamespace, XML_cNvPr, XML_id,
                                OString::number(GetShapeID(xShape) == -1 ? GetNewShapeID(xShape)
                                                                         : GetShapeID(xShape)),
                                XML_name, GetShapeName(xShape));

            if (GetProperty(rXPropSet, u"Hyperlink"_ustr))
            {
                OUString sURL;
                mAny >>= sURL;
                if (!sURL.isEmpty())
                {
                    OUString sRelId = mpFB->addRelation(
                        mpFS->getOutputStream(), oox::getRelationship(Relationship::HYPERLINK),
                        mpURLTransformer->getTransformedString(sURL),
                        mpURLTransformer->isExternalURL(sURL));

                    mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId);
                }
            }
            AddExtLst(pFS, rXPropSet);
            pFS->endElementNS(mnXmlNamespace, XML_cNvPr);
        }
        pFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr);
    }

    // visual shape properties
    pFS->startElementNS(mnXmlNamespace, XML_spPr);

    // we export non-primitive shapes to custom geometry
    // we also export non-ooxml shapes which have handles/equations to custom geometry, because
    // we cannot convert ODF equations to DrawingML equations. TODO: see what binary DOC export filter does.
    // but our WritePolyPolygon()/WriteCustomGeometry() functions are incomplete, therefore we use a denylist
    // we use a allowlist for shapes where mapping to MSO preset shape is not optimal
    bool bCustGeom = true;
    bool bOnDenylist = false;
    if( sShapeType == "ooxml-non-primitive" )
        bCustGeom = true;
    else if( sShapeType.startsWith("ooxml") )
        bCustGeom = false;
    else if( lcl_IsOnAllowlist(sShapeType) )
        bCustGeom = true;
    else if( lcl_IsOnDenylist(sShapeType) )
    {
        bCustGeom = false;
        bOnDenylist = true;
    }

    bool bPresetWriteSuccessful = false;
    // Let the custom shapes what has name and preset information in OOXML, to be written
    // as preset ones with parameters. Try that with this converter class.
    if (!sShapeType.startsWith("ooxml") && sShapeType != "non-primitive" && !mbUserShapes
        && xShape->getShapeType() == "com.sun.star.drawing.CustomShape"
        && !lcl_IsOnAllowlist(sShapeType))
    {
        DMLPresetShapeExporter aCustomShapeConverter(this, xShape);
        bPresetWriteSuccessful = aCustomShapeConverter.WriteShape();
    }
    // If preset writing has problems try to write the shape as it done before
    if (bPresetWriteSuccessful)
        ;// Already written do nothing.
    else if (bCustGeom)
    {
        WriteShapeTransformation( xShape, XML_a, bFlipH, bFlipV );
        bool bSuccess = WriteCustomGeometry(xShape, rSdrObjCustomShape);
        // In case of Writer, the parent element is <wps:spPr>, and there the <a:custGeom> element
        // is not optional.
        if (!bSuccess && GetDocumentType() == DOCUMENT_DOCX)
        {
            WriteEmptyCustomGeometry();
        }
    }
    else if (bOnDenylist && bHasHandles && nAdjustmentValuesIndex !=-1 && !sShapeType.startsWith("mso-spt"))
    {
        WriteShapeTransformation( xShape, XML_a, bFlipH, bFlipV );
        Sequence< EnhancedCustomShapeAdjustmentValue > aAdjustmentSeq;
        std::vector< std::pair< sal_Int32, sal_Int32> > aHandlePositionList;
        std::vector< std::pair< sal_Int32, sal_Int32> > aAvList;
        aGeometrySeq[ nAdjustmentValuesIndex ].Value >>= aAdjustmentSeq ;

        lcl_AnalyzeHandles( aHandles, aHandlePositionList, aAdjustmentSeq );

        sal_Int32 nXPosition = 0;
        sal_Int32 nYPosition = 0;
        if ( !aHandlePositionList.empty() )
        {
            nXPosition = aHandlePositionList[0].first ;
            nYPosition = aHandlePositionList[0].second ;
        }
        switch( eShapeType )
        {
            case mso_sptBorderCallout1:
            {
                sal_Int32 adj3 =  double(nYPosition)/aViewBox.Height *100000;
                sal_Int32 adj4 =  double(nXPosition)/aViewBox.Width *100000;
                lcl_AppendAdjustmentValue( aAvList, 1, 18750 );
                lcl_AppendAdjustmentValue( aAvList, 2, -8333 );
                lcl_AppendAdjustmentValue( aAvList, 3, adj3 );
                lcl_AppendAdjustmentValue( aAvList, 4, adj4 );
                break;
            }
            case mso_sptBorderCallout2:
            {
                sal_Int32 adj5 =  double(nYPosition)/aViewBox.Height *100000;
                sal_Int32 adj6 =  double(nXPosition)/aViewBox.Width *100000;
                sal_Int32 adj3 =  18750;
                sal_Int32 adj4 =  -16667;
                lcl_AppendAdjustmentValue( aAvList, 1, 18750 );
                lcl_AppendAdjustmentValue( aAvList, 2, -8333 );
                if ( aHandlePositionList.size() > 1 )
                {
                    nXPosition = aHandlePositionList[1].first ;
                    nYPosition = aHandlePositionList[1].second ;
                    adj3 =  double(nYPosition)/aViewBox.Height *100000;
                    adj4 =  double(nXPosition)/aViewBox.Width *100000;
                }
                lcl_AppendAdjustmentValue( aAvList, 3, adj3 );
                lcl_AppendAdjustmentValue( aAvList, 4, adj4 );
                lcl_AppendAdjustmentValue( aAvList, 5, adj5 );
                lcl_AppendAdjustmentValue( aAvList, 6, adj6 );
                break;
            }
            case mso_sptWedgeRectCallout:
            case mso_sptWedgeRRectCallout:
            case mso_sptWedgeEllipseCallout:
            case mso_sptCloudCallout:
            {
                sal_Int32 adj1 =  (double(nXPosition)/aViewBox.Width -0.5) *100000;
                sal_Int32 adj2 =  (double(nYPosition)/aViewBox.Height -0.5) *100000;
                lcl_AppendAdjustmentValue( aAvList, 1, adj1 );
                lcl_AppendAdjustmentValue( aAvList, 2, adj2 );
                if ( eShapeType == mso_sptWedgeRRectCallout)
                {
                    lcl_AppendAdjustmentValue( aAvList, 3, 16667);
                }

                break;
            }
            case mso_sptFoldedCorner:
            {
                sal_Int32 adj =  double( aViewBox.Width - nXPosition) / std::min( aViewBox.Width,aViewBox.Height ) * 100000;
                lcl_AppendAdjustmentValue( aAvList, 0, adj );
                break;
            }
            case mso_sptDonut:
            case mso_sptSun:
            case mso_sptMoon:
            case mso_sptNoSmoking:
            case mso_sptHorizontalScroll:
            case mso_sptBevel:
            case mso_sptBracketPair:
            {
                sal_Int32 adj =  double( nXPosition )/aViewBox.Width*100000 ;
                lcl_AppendAdjustmentValue( aAvList, 0, adj );
                break;
            }
            case mso_sptCan:
            case mso_sptCube:
            case mso_sptBracePair:
            case mso_sptVerticalScroll:
            {
                sal_Int32 adj =  double( nYPosition )/aViewBox.Height *100000 ;
                lcl_AppendAdjustmentValue( aAvList, 0, adj );
                break;
            }
            case mso_sptSmileyFace:
            {
                sal_Int32 adj =  double( nYPosition )/aViewBox.Height *100000 - 76458.0;
                lcl_AppendAdjustmentValue( aAvList, 0, adj );
                break;
            }
            case mso_sptBlockArc:
            {
                sal_Int32 nRadius = 50000 * ( 1 - double(nXPosition) / 10800);
                sal_Int32 nAngleStart = lcl_NormalizeAngle( nYPosition );
                sal_Int32 nAngleEnd = lcl_NormalizeAngle( 180 - nAngleStart );
                lcl_AppendAdjustmentValue( aAvList, 1, 21600000 / 360 * nAngleStart );
                lcl_AppendAdjustmentValue( aAvList, 2, 21600000 / 360 * nAngleEnd );
                lcl_AppendAdjustmentValue( aAvList, 3, nRadius );
                break;
            }
            // case mso_sptNil:
            // case mso_sptBentConnector3:
            // case mso_sptBorderCallout3:
            default:
            {
                if ( sPresetShape == "frame" )
                {
                    sal_Int32 adj1 =  double( nYPosition )/aViewBox.Height *100000 ;
                    lcl_AppendAdjustmentValue( aAvList, 1, adj1 );
                }
                break;
            }
        }
        WritePresetShape( sPresetShape  , aAvList );
    }
    else // preset geometry
    {
        WriteShapeTransformation( xShape, XML_a, bFlipH, bFlipV );
        if( nAdjustmentValuesIndex != -1 )
        {
            WritePresetShape( sPresetShape, eShapeType, bPredefinedHandlesUsed,
                              aGeometrySeq[ nAdjustmentValuesIndex ] );
        }
        else
            WritePresetShape( sPresetShape );
    }
    if( rXPropSet.is() )
    {
        WriteFill(rXPropSet, xShape->getSize());
        WriteOutline( rXPropSet );
        WriteShapeEffects( rXPropSet );

        bool bHas3DEffectinShape = false;
        uno::Sequence<beans::PropertyValue> grabBag;
        rXPropSet->getPropertyValue(u"InteropGrabBag"_ustr) >>= grabBag;

        for (auto const& it : grabBag)
            if (it.Name == "3DEffectProperties")
                bHas3DEffectinShape = true;

        if( bHas3DEffectinShape)
            Write3DEffects( rXPropSet, /*bIsText=*/false );
    }

    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    pFS->startElementNS(mnXmlNamespace, XML_style);
    WriteShapeStyle( rXPropSet );
    pFS->endElementNS( mnXmlNamespace, XML_style );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp) );

    return *this;
}

ShapeExport& ShapeExport::WriteEllipseShape( const Reference< XShape >& xShape )
{
    SAL_INFO("oox.shape", "write ellipse shape");

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS(mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp));

    // TODO: connector ?

    Reference<XPropertySet> const xProps(xShape, UNO_QUERY);
    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
    {
        pFS->startElementNS(mnXmlNamespace, XML_nvSpPr);
        pFS->startElementNS(mnXmlNamespace, XML_cNvPr,
                XML_id, OString::number(GetNewShapeID(xShape)),
                XML_name, GetShapeName(xShape));
        AddExtLst(pFS, xProps);
        pFS->endElementNS(mnXmlNamespace, XML_cNvPr);
        pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr );
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }
    else
        pFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr);

    CircleKind  eCircleKind(CircleKind_FULL);
    if (xProps.is())
        xProps->getPropertyValue(u"CircleKind"_ustr ) >>= eCircleKind;

    // visual shape properties
    pFS->startElementNS( mnXmlNamespace, XML_spPr );
    WriteShapeTransformation( xShape, XML_a );

    if (CircleKind_FULL == eCircleKind)
        WritePresetShape("ellipse"_ostr);
    else
    {
        sal_Int32 nStartAngleIntern(9000);
        sal_Int32 nEndAngleIntern(0);
        if (xProps.is())
        {
           xProps->getPropertyValue(u"CircleStartAngle"_ustr ) >>= nStartAngleIntern;
           xProps->getPropertyValue(u"CircleEndAngle"_ustr) >>= nEndAngleIntern;
        }
        std::vector< std::pair<sal_Int32,sal_Int32>> aAvList;
        awt::Size aSize = xShape->getSize();
        if (aSize.Width != 0 || aSize.Height != 0)
        {
            // Our arc has 90° up, OOXML has 90° down, so mirror it.
            // API angles are 1/100 degree.
            sal_Int32 nStartAngleOOXML(lcl_CircleAngle2CustomShapeEllipseAngleOOX(36000 - nEndAngleIntern, aSize.Width, aSize.Height));
            sal_Int32 nEndAngleOOXML(lcl_CircleAngle2CustomShapeEllipseAngleOOX(36000 - nStartAngleIntern, aSize.Width, aSize.Height));
            lcl_AppendAdjustmentValue( aAvList, 1, nStartAngleOOXML);
            lcl_AppendAdjustmentValue( aAvList, 2, nEndAngleOOXML);
        }
        switch (eCircleKind)
        {
            case CircleKind_ARC :
                WritePresetShape("arc"_ostr, aAvList);
            break;
            case CircleKind_SECTION :
                WritePresetShape("pie"_ostr, aAvList);
            break;
            case CircleKind_CUT :
                WritePresetShape("chord"_ostr, aAvList);
            break;
        default :
            WritePresetShape("ellipse"_ostr);
        }
    }
    if( xProps.is() )
    {
        if (CircleKind_ARC == eCircleKind)
        {
            // An arc in ODF is never filled, even if a fill style other than
            // "none" is set. OOXML arc can be filled, so set fill explicit to
            // NONE, otherwise some hidden or inherited filling is shown.
            FillStyle eFillStyle(FillStyle_NONE);
            uno::Any aNewValue;
            aNewValue <<= eFillStyle;
            xProps->setPropertyValue(u"FillStyle"_ustr, aNewValue);
        }
        WriteFill( xProps );
        WriteOutline( xProps );
    }
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp) );

    return *this;
}

ShapeExport& ShapeExport::WriteGraphicObjectShape( const Reference< XShape >& xShape )
{
    WriteGraphicObjectShapePart( xShape );

    return *this;
}

void ShapeExport::WriteGraphicObjectShapePart( const Reference< XShape >& xShape, const Graphic* pGraphic )
{
    SAL_INFO("oox.shape", "write graphic object shape");

    if (IsNonEmptySimpleText(xShape))
    {
        SAL_INFO("oox.shape", "graphicObject: wrote only text");

        WriteTextShape(xShape);

        return;
    }

    SAL_INFO("oox.shape", "graphicObject without text");

    uno::Reference<graphic::XGraphic> xGraphic;
    OUString sMediaURL;

    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );

    if (pGraphic)
    {
        xGraphic.set(pGraphic->GetXGraphic());
    }
    else if (xShapeProps.is() && xShapeProps->getPropertySetInfo()->hasPropertyByName(u"Graphic"_ustr))
    {
        xShapeProps->getPropertyValue(u"Graphic"_ustr) >>= xGraphic;
    }

    // tdf#155903 Only for PPTX, Microsoft does not support this feature in Word and Excel.
    bool bHasMediaURL = GetDocumentType() == DOCUMENT_PPTX && xShapeProps.is()
                        && xShapeProps->getPropertySetInfo()->hasPropertyByName(u"MediaURL"_ustr)
                        && (xShapeProps->getPropertyValue(u"MediaURL"_ustr) >>= sMediaURL);

    if (!xGraphic.is() && !bHasMediaURL)
    {
        SAL_INFO("oox.shape", "no graphic or media URL found");
        return;
    }

    FSHelperPtr pFS = GetFS();
    XmlFilterBase* pFB = GetFB();

    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
        pFS->startElementNS(mnXmlNamespace, XML_pic);
    else
        pFS->startElementNS(mnXmlNamespace, XML_pic,
            FSNS(XML_xmlns, XML_pic), pFB->getNamespaceURL(OOX_NS(dmlPicture)));

    pFS->startElementNS(mnXmlNamespace, XML_nvPicPr);

    presentation::ClickAction eClickAction = presentation::ClickAction_NONE;
    OUString sDescr, sURL, sBookmark, sPPAction;
    bool bHaveDesc;

    if ( ( bHaveDesc = GetProperty( xShapeProps, u"Description"_ustr ) ) )
        mAny >>= sDescr;
    if ( GetProperty( xShapeProps, u"URL"_ustr ) )
        mAny >>= sURL;
    if (GetProperty(xShapeProps, u"Bookmark"_ustr))
        mAny >>= sBookmark;
    if (GetProperty(xShapeProps, u"OnClick"_ustr))
        mAny >>= eClickAction;

    pFS->startElementNS( mnXmlNamespace, XML_cNvPr,
                          XML_id,     OString::number(GetNewShapeID(xShape)),
                          XML_name,   GetShapeName(xShape),
                          XML_descr,  sax_fastparser::UseIf(sDescr, bHaveDesc));

    if (eClickAction != presentation::ClickAction_NONE)
    {
        switch (eClickAction)
        {
            case presentation::ClickAction_STOPPRESENTATION:
                sPPAction = "ppaction://hlinkshowjump?jump=endshow";
                break;
            case presentation::ClickAction_NEXTPAGE:
                sPPAction = "ppaction://hlinkshowjump?jump=nextslide";
                break;
            case presentation::ClickAction_LASTPAGE:
                sPPAction = "ppaction://hlinkshowjump?jump=lastslide";
                break;
            case presentation::ClickAction_PREVPAGE:
                sPPAction = "ppaction://hlinkshowjump?jump=previousslide";
                break;
            case presentation::ClickAction_FIRSTPAGE:
                sPPAction = "ppaction://hlinkshowjump?jump=firstslide";
                break;
            case presentation::ClickAction_BOOKMARK:
                sBookmark = "#" + sBookmark;
                break;
            default:
                break;
        }
    }

    // OOXTODO: //cNvPr children: XML_extLst, XML_hlinkHover
    if (bHasMediaURL || !sPPAction.isEmpty())
        pFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), "", XML_action,
                             bHasMediaURL ? u"ppaction://media"_ustr : sPPAction);
    if( !sURL.isEmpty() )
    {
        OUString sRelId = mpFB->addRelation( mpFS->getOutputStream(),
                oox::getRelationship(Relationship::HYPERLINK),
                mpURLTransformer->getTransformedString(sURL),
                mpURLTransformer->isExternalURL(sURL));

        mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId);
    }

    if (!sBookmark.isEmpty())
    {
        bool bExtURL = URLTransformer().isExternalURL(sBookmark);
        sBookmark = bExtURL ? sBookmark : lcl_GetTarget(GetFB()->getModel(), sBookmark);

        OUString sRelId = mpFB->addRelation(mpFS->getOutputStream(),
                                            bExtURL ? oox::getRelationship(Relationship::HYPERLINK)
                                                    : oox::getRelationship(Relationship::SLIDE),
                                            sBookmark, bExtURL);

        if (bExtURL)
            mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId);
        else
            mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId, XML_action,
                                  "ppaction://hlinksldjump");
    }
    AddExtLst(pFS, xShapeProps);
    pFS->endElementNS(mnXmlNamespace, XML_cNvPr);

    pFS->singleElementNS(mnXmlNamespace, XML_cNvPicPr
                         // OOXTODO: XML_preferRelativeSize
                        );
    if (bHasMediaURL)
        WriteMediaNonVisualProperties(xShape);
    else
        WriteNonVisualProperties(xShape);

    pFS->endElementNS( mnXmlNamespace, XML_nvPicPr );

    pFS->startElementNS(mnXmlNamespace, XML_blipFill);

    if (xGraphic.is())
    {
        WriteXGraphicBlip(xShapeProps, xGraphic, mbUserShapes);
    }
    else if (bHasMediaURL)
    {
        Reference<graphic::XGraphic> xFallbackGraphic;
        if (xShapeProps->getPropertySetInfo()->hasPropertyByName(u"FallbackGraphic"_ustr))
            xShapeProps->getPropertyValue(u"FallbackGraphic"_ustr) >>= xFallbackGraphic;

        WriteXGraphicBlip(xShapeProps, xFallbackGraphic, mbUserShapes);
    }

    if (xGraphic.is())
    {
        WriteSrcRectXGraphic(xShapeProps, xGraphic);
    }

    // now we stretch always when we get pGraphic (when changing that
    // behavior, test n#780830 for regression, where the OLE sheet might get tiled
    bool bStretch = false;
    if( !pGraphic && GetProperty( xShapeProps, u"FillBitmapStretch"_ustr ) )
        mAny >>= bStretch;

    if ( pGraphic || bStretch )
        pFS->singleElementNS(XML_a, XML_stretch);

    if (bHasMediaURL)
    {
        // Graphic of media shapes is always stretched.
        pFS->startElementNS(XML_a, XML_stretch);
        pFS->singleElementNS(XML_a, XML_fillRect);
        pFS->endElementNS(XML_a, XML_stretch);
    }

    pFS->endElementNS( mnXmlNamespace, XML_blipFill );

    // visual shape properties
    pFS->startElementNS(mnXmlNamespace, XML_spPr);
    bool bFlipH = false;
    if( xShapeProps->getPropertySetInfo()->hasPropertyByName(u"IsMirrored"_ustr) )
    {
        xShapeProps->getPropertyValue(u"IsMirrored"_ustr) >>= bFlipH;
    }
    WriteShapeTransformation( xShape, XML_a, bFlipH, false, false, false, true );
    WritePresetShape( "rect"_ostr );
    // graphic object can come with the frame (bnc#654525)
    WriteOutline( xShapeProps );

    WriteShapeEffects( xShapeProps );
    Write3DEffects( xShapeProps, /*bIsText=*/false );

    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    pFS->endElementNS( mnXmlNamespace, XML_pic );
}

static void lcl_Rotate(sal_Int32 nAngle, Point center, awt::Point& pt)
{
    sal_Int16 nCos, nSin;
    switch (nAngle)
    {
        case 90:
            nCos = 0;
            nSin = 1;
            break;
        case 180:
            nCos = -1;
            nSin = 0;
            break;
        case 270:
            nCos = 0;
            nSin = -1;
            break;
        default:
            return;
    }
    sal_Int32 x = pt.X - center.X();
    sal_Int32 y = pt.Y - center.Y();
    pt.X = center.X() + x * nCos - y * nSin;
    pt.Y = center.Y() + y * nCos + x * nSin;
}

static void lcl_FlipHFlipV(tools::Polygon aPoly, sal_Int32 nAngle, bool& rFlipH, bool& rFlipV)
{
    Point aStart = aPoly[0];
    Point aEnd = aPoly[aPoly.GetSize() - 1];

    if (aStart.X() > aEnd.X() && aStart.Y() > aEnd.Y())
    {
        if (nAngle)
        {
            if (nAngle == 90)
                rFlipH = true;
            if (nAngle == 270)
                rFlipV = true;
        }
        else // 0°
        {
            rFlipH = true;
            rFlipV = true;
        }
    }

    if (aStart.X() < aEnd.X() && aStart.Y() < aEnd.Y())
    {
        if (nAngle)
        {
            if (nAngle != 270)
            {
                rFlipH = true;
                rFlipV = true;
            }
            else
                rFlipH = true;
        }
    }

    if (aStart.Y() < aEnd.Y() && aStart.X() > aEnd.X())
    {
        if (nAngle)
        {
            if (nAngle == 180)
                rFlipV = true;
            if (nAngle == 270)
            {
                rFlipV = true;
                rFlipH = true;
            }
        }
        else // 0°
        {
            rFlipH = true;
        }
    }

    if (aStart.Y() > aEnd.Y() && aStart.X() < aEnd.X())
    {
        if (nAngle)
        {
            if (nAngle == 90)
            {
                rFlipH = true;
                rFlipV = true;
            }
            if (nAngle == 180)
                rFlipH = true;
        }
        else // 0°
            rFlipV = true;
    }
}

static sal_Int32 lcl_GetAngle(tools::Polygon aPoly)
{
    sal_Int32 nAngle;
    Point aStartPoint = aPoly[0];
    Point aEndPoint = aPoly[aPoly.GetSize() - 1];
    if (aStartPoint.X() == aPoly[1].X())
    {
        if ((aStartPoint.X() < aEndPoint.X() && aStartPoint.Y() > aEndPoint.Y())
            || (aStartPoint.X() > aEndPoint.X() && aStartPoint.Y() < aEndPoint.Y()))
        {
            nAngle = 90;
        }
        else
            nAngle = 270;
    }
    else
    {
        if (aStartPoint.X() > aPoly[1].X())
            nAngle = 180;
        else
            nAngle = 0;
    }

    return nAngle;
}

// Adjust value decide the position, where the connector should turn.
static void lcl_GetConnectorAdjustValue(const Reference<XShape>& xShape, tools::Polygon aPoly,
                                        ConnectorType eConnectorType,
                                        std::vector<std::pair<sal_Int32, sal_Int32>>& rAvList)
{
    Reference<XPropertySet> xShapeProps(xShape, UNO_QUERY);
    bool bIsOOXMLCurve(false);
    xShapeProps->getPropertyValue(u"EdgeOOXMLCurve"_ustr) >>= bIsOOXMLCurve;
    sal_Int32 nAdjCount = 0;
    if (eConnectorType == ConnectorType_CURVE)
    {
        if (bIsOOXMLCurve)
        {
            nAdjCount = (aPoly.GetSize() - 4) / 3;
        }
        else if (aPoly.GetSize() == 4)
        {
            if ((aPoly[0].X() == aPoly[1].X() && aPoly[2].X() == aPoly[3].X())
                || (aPoly[0].Y() == aPoly[1].Y() && aPoly[2].Y() == aPoly[3].Y()))
            {
                nAdjCount = 1; // curvedConnector3, control vectors parallel
            }
            else
                nAdjCount = 0; // curvedConnector2, control vectors orthogonal
        }
        else if (aPoly.GetSize() > 4)
        {
            if ((aPoly[2].X() == aPoly[3].X() && aPoly[3].X() == aPoly[4].X())
                || (aPoly[2].Y() == aPoly[3].Y() && aPoly[3].Y() == aPoly[4].Y()))
            {
                nAdjCount = 3; // curvedConnector5
            }
            else
                nAdjCount = 2; // curvedConnector4
        }
    }
    else
    {
        switch (aPoly.GetSize())
        {
            case 3:
                nAdjCount = 0; // bentConnector2
                break;
            case 4:
                nAdjCount = 1; // bentConnector3
                break;
            case 5:
                nAdjCount = 2; // bentConnector4
                break;
            case 6:
                nAdjCount = 3; // bentConnector5
                break;
        }
    }

    if (nAdjCount)
    {
        sal_Int32 nAdjustValue;
        Point aStart = aPoly[0];
        Point aEnd = aPoly[aPoly.GetSize() - 1];

        for (sal_Int32 i = 1; i <= nAdjCount; ++i)
        {
            Point aPt = aPoly[i];

            if (aEnd.Y() == aStart.Y())
                aEnd.setY(aStart.Y() + 1);
            if (aEnd.X() == aStart.X())
                aEnd.setX(aStart.X() + 1);

            bool bVertical = aPoly[1].X() - aStart.X() != 0 ? true : false;
            // vertical and horizon alternate
            if (i % 2 == 1)
                bVertical = !bVertical;

            if (eConnectorType == ConnectorType_CURVE)
            {
                if (bIsOOXMLCurve)
                {
                    aPt = aPoly[3 *  i];
                }
                else
                {
                    awt::Size aSize = xShape->getSize();
                    awt::Point aShapePosition = xShape->getPosition();
                    tools::Rectangle aBoundRect = aPoly.GetBoundRect();

                    if (bVertical)
                    {
                        if ((aBoundRect.GetSize().Height() - aSize.Height) == 1)
                            aPt.setY(aPoly[i + 1].Y());
                        else if (aStart.Y() > aPt.Y())
                            aPt.setY(aShapePosition.Y);
                        else
                            aPt.setY(aShapePosition.Y + aSize.Height);
                    }
                    else
                    {
                        if ((aBoundRect.GetSize().Width() - aSize.Width) == 1)
                            aPt.setX(aPoly[i + 1].X());
                        else if (aStart.X() > aPt.X())
                            aPt.setX(aShapePosition.X);
                        else
                            aPt.setX(aShapePosition.X + aSize.Width);
                    }
                }
            }

            if (bVertical)
                nAdjustValue = ((aPt.Y() - aStart.Y()) * 100000) / (aEnd.Y() - aStart.Y());
            else
                nAdjustValue = ((aPt.X() - aStart.X()) * 100000) / (aEnd.X() - aStart.X());

            rAvList.emplace_back(i, nAdjustValue);
        }
    }
}

static sal_Int32 lcl_GetGluePointId(const Reference<XShape>& xShape, sal_Int32 nGluePointId)
{
    if (nGluePointId > 3)
        return nGluePointId - 4;
    else
    {
        bool bFlipH = false;
        bool bFlipV = false;
        Reference<XPropertySet> xShapeProps(xShape, UNO_QUERY);
        if (xShapeProps.is() && xShapeProps->getPropertySetInfo()
                && xShapeProps->getPropertySetInfo()->hasPropertyByName(u"CustomShapeGeometry"_ustr))
        {
            Sequence<PropertyValue> aGeometrySeq;
            xShapeProps->getPropertyValue(u"CustomShapeGeometry"_ustr) >>= aGeometrySeq;
            for (int i = 0; i < aGeometrySeq.getLength(); i++)
            {
                const PropertyValue& rProp = aGeometrySeq[i];
                if (rProp.Name == "MirroredX")
                    rProp.Value >>= bFlipH;

                if (rProp.Name == "MirroredY")
                    rProp.Value >>= bFlipV;
            }
        }

        if ((!bFlipH && !bFlipV) || (bFlipH && bFlipV))
        {
            // change id of the bounding box (1 <-> 3)
            if (nGluePointId == 1)
                nGluePointId = 3; // Right
            else if (nGluePointId == 3)
                nGluePointId = 1; // Left
        }
    }

    return nGluePointId;
}

ShapeExport& ShapeExport::WriteConnectorShape( const Reference< XShape >& xShape )
{
    bool bFlipH = false;
    bool bFlipV = false;
    sal_Int32 nAngle = 0;
    sal_Int32 nStartGlueId = 0;
    sal_Int32 nEndGlueId = 0;

    SAL_INFO("oox.shape", "write connector shape");

    FSHelperPtr pFS = GetFS();

    OUString sGeometry;
    std::vector<std::pair<sal_Int32, sal_Int32>> aAdjustValueList;
    Reference< XPropertySet > rXPropSet( xShape, UNO_QUERY );
    Reference< XPropertyState > rXPropState( xShape, UNO_QUERY );
    awt::Point aStartPoint, aEndPoint;
    Reference< XShape > rXShapeA;
    Reference< XShape > rXShapeB;
    PropertyState eState;
    ConnectorType eConnectorType = ConnectorType_STANDARD;
    if (GetProperty(rXPropSet, u"EdgeKind"_ustr))
        mAny >>= eConnectorType;

    switch( eConnectorType ) {
        case ConnectorType_CURVE:
            sGeometry = "curvedConnector";
            break;
        case ConnectorType_LINES:
        case ConnectorType_STANDARD:
            sGeometry = "bentConnector";
            break;
        default:
        case ConnectorType_LINE:
            sGeometry = "straightConnector1";
            break;
    }

    if (GetPropertyAndState( rXPropSet, rXPropState, u"EdgeStartPoint"_ustr, eState ) && eState == beans::PropertyState_DIRECT_VALUE )
    {
        mAny >>= aStartPoint;
        if (GetPropertyAndState( rXPropSet, rXPropState, u"EdgeEndPoint"_ustr, eState ) && eState == beans::PropertyState_DIRECT_VALUE )
            mAny >>= aEndPoint;
    }
    if (GetProperty(rXPropSet, u"EdgeStartConnection"_ustr))
        mAny >>= rXShapeA;
    if (GetProperty(rXPropSet, u"EdgeEndConnection"_ustr))
        mAny >>= rXShapeB;

    if (GetProperty(rXPropSet, u"StartGluePointIndex"_ustr))
        mAny >>= nStartGlueId;
    if (nStartGlueId != -1)
        nStartGlueId = lcl_GetGluePointId(rXShapeA, nStartGlueId);

    if (GetProperty(rXPropSet, u"EndGluePointIndex"_ustr))
        mAny >>= nEndGlueId;
    if (nEndGlueId != -1)
        nEndGlueId = lcl_GetGluePointId(rXShapeB, nEndGlueId);

    // Position is relative to group in Word, but relative to anchor of group in API.
    if (GetDocumentType() == DOCUMENT_DOCX && !mbUserShapes && m_xParent.is())
    {
        awt::Point aParentPos = m_xParent->getPosition();
        aStartPoint.X -= aParentPos.X;
        aStartPoint.Y -= aParentPos.Y;
        aEndPoint.X -= aParentPos.X;
        aEndPoint.Y -= aParentPos.Y;
    }
    EscherConnectorListEntry aConnectorEntry( xShape, aStartPoint, rXShapeA, aEndPoint, rXShapeB );

    if (eConnectorType != ConnectorType_LINE)
    {
        tools::PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon(xShape);
        if (aPolyPolygon.Count() > 0)
        {
            tools::Polygon aPoly = aPolyPolygon.GetObject(0);
            lcl_GetConnectorAdjustValue(xShape, aPoly, eConnectorType, aAdjustValueList);
            nAngle = lcl_GetAngle(aPoly);
            lcl_FlipHFlipV(aPoly, nAngle, bFlipH, bFlipV);
            if (nAngle)
            {
                Point center((aEndPoint.X + aStartPoint.X) / 2, (aEndPoint.Y + aStartPoint.Y) / 2);
                lcl_Rotate(nAngle, center, aStartPoint);
                lcl_Rotate(nAngle, center, aEndPoint);
                nAngle *= 60000;
            }
            sGeometry = sGeometry + OUString::number(aAdjustValueList.size() + 2);
        }
    }

    tools::Rectangle aRect( Point( aStartPoint.X, aStartPoint.Y ), Point( aEndPoint.X, aEndPoint.Y ) );
    if( aRect.getOpenWidth() < 0 ) {
        aRect.SetLeft(aEndPoint.X);
        aRect.setWidth( aStartPoint.X - aEndPoint.X );
        if (eConnectorType == ConnectorType_LINE)
            bFlipH = true;
    }

    if( aRect.getOpenHeight() < 0 ) {
        aRect.SetTop(aEndPoint.Y);
        aRect.setHeight( aStartPoint.Y - aEndPoint.Y );
        if (eConnectorType == ConnectorType_LINE)
            bFlipV = true;
    }

    // tdf#99810 connector shape (cxnSp) is not valid with namespace 'wps'
    const auto nShapeNode = (mnXmlNamespace == XML_wps ? XML_wsp : XML_cxnSp);
    pFS->startElementNS(mnXmlNamespace, nShapeNode);

    if (mnXmlNamespace == XML_wps)
    {
        // non visual connector shape drawing properties
        pFS->singleElementNS(mnXmlNamespace, XML_cNvCnPr);
    }
    else
    {
        // non visual shape properties
        pFS->startElementNS(mnXmlNamespace, XML_nvCxnSpPr);
        pFS->startElementNS(mnXmlNamespace, XML_cNvPr,
            XML_id, OString::number(GetNewShapeID(xShape)),
            XML_name, GetShapeName(xShape));
        AddExtLst(pFS, rXPropSet);
        pFS->endElementNS(mnXmlNamespace, XML_cNvPr);
        // non visual connector shape drawing properties
        pFS->startElementNS(mnXmlNamespace, XML_cNvCxnSpPr);

        if (GetShapeID(rXShapeA) == -1)
            GetNewShapeID(rXShapeA);
        if (GetShapeID(rXShapeB) == -1)
            GetNewShapeID(rXShapeB);
        WriteConnectorConnections(nStartGlueId, nEndGlueId, GetShapeID(rXShapeA), GetShapeID(rXShapeB));
        pFS->endElementNS(mnXmlNamespace, XML_cNvCxnSpPr);
        if (GetDocumentType() == DOCUMENT_PPTX)
            pFS->singleElementNS(mnXmlNamespace, XML_nvPr);
        pFS->endElementNS(mnXmlNamespace, XML_nvCxnSpPr);
    }

    // visual shape properties
    pFS->startElementNS(mnXmlNamespace, XML_spPr);
    WriteTransformation( xShape, aRect, XML_a, bFlipH, bFlipV, nAngle );
    // TODO: write adjustments (ppt export doesn't work well there either)
    WritePresetShape( sGeometry.toUtf8(), aAdjustValueList);
    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
        WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // connector shape (cxnSp) cannot contain text (txBody) (according to schema)
    if( nShapeNode != XML_cxnSp )
    {
        // write text
        WriteTextBox( xShape, mnXmlNamespace );
    }

    pFS->endElementNS(mnXmlNamespace, nShapeNode);

    return *this;
}

ShapeExport& ShapeExport::WriteLineShape( const Reference< XShape >& xShape )
{
    bool bFlipH = false;
    bool bFlipV = false;

    SAL_INFO("oox.shape", "write line shape");

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS(mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp));

    tools::PolyPolygon aPolyPolygon = EscherPropertyContainer::GetPolyPolygon( xShape );
    if( aPolyPolygon.Count() == 1 && aPolyPolygon[ 0 ].GetSize() == 2)
    {
        const tools::Polygon& rPoly = aPolyPolygon[ 0 ];

        bFlipH = ( rPoly[ 0 ].X() > rPoly[ 1 ].X() );
        bFlipV = ( rPoly[ 0 ].Y() > rPoly[ 1 ].Y() );
    }

    Reference<XPropertySet> const xShapeProps(xShape, UNO_QUERY);
    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
    {
        pFS->startElementNS(mnXmlNamespace, XML_nvSpPr);
        pFS->startElementNS(mnXmlNamespace, XML_cNvPr,
                              XML_id, OString::number(GetNewShapeID(xShape)),
                              XML_name, GetShapeName(xShape));
        AddExtLst(pFS, xShapeProps);
        pFS->endElementNS(mnXmlNamespace, XML_cNvPr);
    }
    pFS->singleElementNS( mnXmlNamespace, XML_cNvSpPr );
    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
    {
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }

    // visual shape properties
    pFS->startElementNS(mnXmlNamespace, XML_spPr);
    WriteShapeTransformation( xShape, XML_a, bFlipH, bFlipV, true);
    WritePresetShape( "line"_ostr );
    if( xShapeProps.is() )
        WriteOutline( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    //write style
    pFS->startElementNS(mnXmlNamespace, XML_style);
    WriteShapeStyle( xShapeProps );
    pFS->endElementNS( mnXmlNamespace, XML_style );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp) );

    return *this;
}

ShapeExport& ShapeExport::WriteNonVisualDrawingProperties( const Reference< XShape >& xShape, const char* pName )
{
    FSHelperPtr pFS = GetFS();

    Reference<XPropertySet> const xShapeProps(xShape, UNO_QUERY);
    pFS->startElementNS(mnXmlNamespace, XML_cNvPr,
                              XML_id, OString::number(GetNewShapeID(xShape)),
                              XML_name, pName );
    AddExtLst(pFS, xShapeProps);
    pFS->endElementNS(mnXmlNamespace, XML_cNvPr);

    return *this;
}

ShapeExport& ShapeExport::WriteNonVisualProperties( const Reference< XShape >& )
{
    // Override to generate //nvPr elements.
    return *this;
}

ShapeExport& ShapeExport::WriteRectangleShape( const Reference< XShape >& xShape )
{
    SAL_INFO("oox.shape", "write rectangle shape");

    FSHelperPtr pFS = GetFS();

    pFS->startElementNS(mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp));

    sal_Int32 nRadius = 0;

    Reference< XPropertySet > xShapeProps( xShape, UNO_QUERY );
    if( xShapeProps.is() )
    {
        xShapeProps->getPropertyValue( u"CornerRadius"_ustr ) >>= nRadius;
    }

    if( nRadius )
    {
        nRadius = MapSize( awt::Size( nRadius, 0 ) ).Width;
    }
    //TODO: use nRadius value more precisely than just deciding whether to use
    // "rect" or "roundRect" preset shape below

    // non visual shape properties
    if (GetDocumentType() == DOCUMENT_DOCX && !mbUserShapes)
        pFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr);
    pFS->startElementNS(mnXmlNamespace, XML_nvSpPr);
    pFS->startElementNS(mnXmlNamespace, XML_cNvPr,
                          XML_id, OString::number(GetNewShapeID(xShape)),
                          XML_name, GetShapeName(xShape));
    AddExtLst(pFS, xShapeProps);
    pFS->endElementNS(mnXmlNamespace, XML_cNvPr);
    pFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr);
    WriteNonVisualProperties( xShape );
    pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );

    // visual shape properties
    pFS->startElementNS(mnXmlNamespace, XML_spPr);
    WriteShapeTransformation( xShape, XML_a );
    WritePresetShape( nRadius == 0 ? "rect" : "roundRect" );
    Reference< XPropertySet > xProps( xShape, UNO_QUERY );
    if( xProps.is() )
    {
        WriteFill( xProps );
        WriteOutline( xProps );
    }
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    // write text
    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp) );

    return *this;
}


typedef ShapeExport& (ShapeExport::*ShapeConverter)( const Reference< XShape >& );
typedef std::unordered_map< const char*, ShapeConverter, rtl::CStringHash, rtl::CStringEqual> NameToConvertMapType;

namespace
{

constexpr auto constMap = frozen::make_unordered_map<std::u16string_view, ShapeConverter>(
{
    { u"com.sun.star.drawing.CaptionShape", &ShapeExport::WriteTextShape },
    { u"com.sun.star.drawing.ClosedBezierShape", &ShapeExport::WriteClosedPolyPolygonShape },
    { u"com.sun.star.drawing.ConnectorShape", &ShapeExport::WriteConnectorShape },
    { u"com.sun.star.drawing.CustomShape", &ShapeExport::WriteCustomShape },
    { u"com.sun.star.drawing.EllipseShape", &ShapeExport::WriteEllipseShape },
    { u"com.sun.star.drawing.GraphicObjectShape", &ShapeExport::WriteGraphicObjectShape },
    { u"com.sun.star.drawing.LineShape", &ShapeExport::WriteLineShape },
    { u"com.sun.star.drawing.MediaShape", &ShapeExport::WriteGraphicObjectShape },
    { u"com.sun.star.drawing.OpenBezierShape", &ShapeExport::WriteOpenPolyPolygonShape },
    { u"com.sun.star.drawing.PolyPolygonShape", &ShapeExport::WriteClosedPolyPolygonShape },
    { u"com.sun.star.drawing.PolyLineShape", &ShapeExport::WriteOpenPolyPolygonShape },
    { u"com.sun.star.drawing.RectangleShape", &ShapeExport::WriteRectangleShape },
    { u"com.sun.star.drawing.OLE2Shape", &ShapeExport::WriteOLE2Shape },
    { u"com.sun.star.drawing.TableShape", &ShapeExport::WriteTableShape },
    { u"com.sun.star.drawing.TextShape", &ShapeExport::WriteTextShape },
    { u"com.sun.star.drawing.GroupShape", &ShapeExport::WriteGroupShape },
    { u"com.sun.star.presentation.GraphicObjectShape", &ShapeExport::WriteGraphicObjectShape },
    { u"com.sun.star.presentation.MediaShape", &ShapeExport::WriteGraphicObjectShape },
    { u"com.sun.star.presentation.ChartShape", &ShapeExport::WriteOLE2Shape },
    { u"com.sun.star.presentation.OLE2Shape", &ShapeExport::WriteOLE2Shape },
    { u"com.sun.star.presentation.TableShape", &ShapeExport::WriteTableShape },
    { u"com.sun.star.presentation.TextShape", &ShapeExport::WriteTextShape },
    { u"com.sun.star.presentation.DateTimeShape", &ShapeExport::WriteTextShape },
    { u"com.sun.star.presentation.FooterShape", &ShapeExport::WriteTextShape },
    { u"com.sun.star.presentation.HeaderShape", &ShapeExport::WriteTextShape },
    { u"com.sun.star.presentation.NotesShape", &ShapeExport::WriteTextShape },
    { u"com.sun.star.presentation.OutlinerShape", &ShapeExport::WriteTextShape },
    { u"com.sun.star.presentation.SlideNumberShape", &ShapeExport::WriteTextShape },
    { u"com.sun.star.presentation.TitleTextShape", &ShapeExport::WriteTextShape },
});

} // end anonymous namespace

ShapeExport& ShapeExport::WriteShape( const Reference< XShape >& xShape )
{
    if (!xShape)
        throw lang::IllegalArgumentException();

    OUString sShapeType = xShape->getShapeType();
    SAL_INFO("oox.shape", "write shape: " << sShapeType);
    auto aConverterIterator = constMap.find(sShapeType);
    if (aConverterIterator == constMap.end())
    {
        SAL_INFO("oox.shape", "unknown shape");
        return WriteUnknownShape( xShape );
    }

    if (GetDocumentType() == DOCUMENT_PPTX)
    {
        Reference< XPropertySet > xShapeProperties(xShape, UNO_QUERY);
        if (xShapeProperties && xShapeProperties->getPropertySetInfo()
            && xShapeProperties->getPropertySetInfo()->hasPropertyByName(u"IsPresentationObject"_ustr)
            && xShapeProperties->getPropertyValue(u"IsPresentationObject"_ustr).hasValue())
            mbPlaceholder = xShapeProperties->getPropertyValue(u"IsPresentationObject"_ustr).get<bool>();
    }

    (this->*(aConverterIterator->second))(xShape);

    return *this;
}

static bool lcl_isTextBox(const Reference<XInterface>& xIface)
{
    uno::Reference<beans::XPropertySet> xPropertySet(xIface, uno::UNO_QUERY);
    if (!xPropertySet.is())
        return false;
    uno::Reference<beans::XPropertySetInfo> xPropertySetInfo = xPropertySet->getPropertySetInfo();
    if (!xPropertySetInfo->hasPropertyByName(u"TextBox"_ustr))
       return false;
    css::uno::Any aTextBox(xPropertySet->getPropertyValue(u"TextBox"_ustr));
    if (!aTextBox.hasValue())
       return false;
    return aTextBox.get<bool>();
}

ShapeExport& ShapeExport::WriteTextBox( const Reference< XInterface >& xIface, sal_Int32 nXmlNamespace, bool bWritePropertiesAsLstStyles )
{
    // In case this shape has an associated textbox, then export that, and we're done.
    if (GetDocumentType() == DOCUMENT_DOCX && !mbUserShapes && GetTextExport())
    {
        if (lcl_isTextBox(xIface))
        {
            GetTextExport()->WriteTextBox(uno::Reference<drawing::XShape>(xIface, uno::UNO_QUERY_THROW));
            WriteText( xIface, /*bBodyPr=*/true, /*bText=*/false, /*nXmlNamespace=*/nXmlNamespace );
            return *this;
        }
    }

    Reference< XText > xXText( xIface, UNO_QUERY );
    if( (NonEmptyText( xIface ) || GetDocumentType() == DOCUMENT_PPTX)
        && xXText.is() )
    {
        FSHelperPtr pFS = GetFS();

        pFS->startElementNS(nXmlNamespace,
                            (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_txBody : XML_txbx));
        WriteText(xIface, /*bBodyPr=*/(GetDocumentType() != DOCUMENT_DOCX || mbUserShapes), /*bText=*/true,
                  /*nXmlNamespace=*/0, /*bWritePropertiesAsLstStyles=*/bWritePropertiesAsLstStyles);
        pFS->endElementNS( nXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_txBody : XML_txbx) );
        if (GetDocumentType() == DOCUMENT_DOCX && !mbUserShapes)
            WriteText( xIface, /*bBodyPr=*/true, /*bText=*/false, /*nXmlNamespace=*/nXmlNamespace );
    }
    else if (GetDocumentType() == DOCUMENT_DOCX && !mbUserShapes)
        mpFS->singleElementNS(nXmlNamespace, XML_bodyPr);

    return *this;
}

void ShapeExport::WriteTable( const Reference< XShape >& rXShape  )
{
    Reference< XTable > xTable;
    Reference< XPropertySet > xPropSet( rXShape, UNO_QUERY );

    mpFS->startElementNS(XML_a, XML_graphic);
    mpFS->startElementNS(XML_a, XML_graphicData,
                         XML_uri, "http://schemas.openxmlformats.org/drawingml/2006/table");

    if ( xPropSet.is() && ( xPropSet->getPropertyValue( u"Model"_ustr ) >>= xTable ) )
    {
        mpFS->startElementNS(XML_a, XML_tbl);
        mpFS->startElementNS(XML_a, XML_tblPr);
        WriteShapeEffects(xPropSet);
        mpFS->endElementNS(XML_a, XML_tblPr);

        Reference< container::XIndexAccess > xColumns( xTable->getColumns(), UNO_QUERY_THROW );
        Reference< container::XIndexAccess > xRows( xTable->getRows(), UNO_QUERY_THROW );
        sal_uInt16 nRowCount = static_cast< sal_uInt16 >( xRows->getCount() );
        sal_uInt16 nColumnCount = static_cast< sal_uInt16 >( xColumns->getCount() );

        mpFS->startElementNS(XML_a, XML_tblGrid);

        for ( sal_Int32 x = 0; x < nColumnCount; x++ )
        {
            Reference< XPropertySet > xColPropSet( xColumns->getByIndex( x ), UNO_QUERY_THROW );
            sal_Int32 nWidth(0);
            xColPropSet->getPropertyValue( u"Width"_ustr ) >>= nWidth;

            mpFS->singleElementNS(XML_a, XML_gridCol,
                                  XML_w, OString::number(oox::drawingml::convertHmmToEmu(nWidth)));
        }

        mpFS->endElementNS( XML_a, XML_tblGrid );

        // map for holding the transpose index of the merged cells and pair<parentTransposeIndex, parentCell>
        typedef std::unordered_map<sal_Int32, std::pair<sal_Int32, Reference< XMergeableCell> > > transposeTableMap;
        transposeTableMap mergedCellMap;

        for( sal_Int32 nRow = 0; nRow < nRowCount; nRow++ )
        {
            Reference< XPropertySet > xRowPropSet( xRows->getByIndex( nRow ), UNO_QUERY_THROW );
            sal_Int32 nRowHeight(0);

            xRowPropSet->getPropertyValue( u"Height"_ustr ) >>= nRowHeight;

            mpFS->startElementNS(XML_a, XML_tr,
                XML_h, OString::number(oox::drawingml::convertHmmToEmu(nRowHeight)));
            for( sal_Int32 nColumn = 0; nColumn < nColumnCount; nColumn++ )
            {
                Reference< XMergeableCell > xCell( xTable->getCellByPosition( nColumn, nRow ),
                                                   UNO_QUERY_THROW );
                sal_Int32 transposedIndexofCell = (nRow * nColumnCount) + nColumn;

                //assume we will open a cell, set to false below if we won't
                bool bCellOpened = true;

                if(xCell->getColumnSpan() > 1 && xCell->getRowSpan() > 1)
                {
                    // having both : horizontal and vertical merge
                    mpFS->startElementNS(XML_a, XML_tc,
                                         XML_gridSpan, OString::number(xCell->getColumnSpan()),
                                         XML_rowSpan, OString::number(xCell->getRowSpan()));
                    // since, XMergeableCell doesn't have the information about
                    // cell having hMerge or vMerge.
                    // So, Populating the merged cell map in-order to use it to
                    // decide the attribute for the individual cell.
                    for(sal_Int32 columnIndex = nColumn; columnIndex < nColumn+xCell->getColumnSpan(); ++columnIndex)
                    {
                        for(sal_Int32 rowIndex = nRow; rowIndex < nRow+xCell->getRowSpan(); ++rowIndex)
                        {
                            sal_Int32 transposeIndexForMergeCell =
                                (rowIndex * nColumnCount) + columnIndex;
                            mergedCellMap[transposeIndexForMergeCell] =
                                std::make_pair(transposedIndexofCell, xCell);
                        }
                    }

                }
                else if(xCell->getColumnSpan() > 1)
                {
                    // having : horizontal merge
                    mpFS->startElementNS(XML_a, XML_tc,
                                         XML_gridSpan, OString::number(xCell->getColumnSpan()));
                    for(sal_Int32 columnIndex = nColumn; columnIndex < nColumn + xCell->getColumnSpan(); ++columnIndex) {
                        sal_Int32 transposeIndexForMergeCell = (nRow*nColumnCount) + columnIndex;
                        mergedCellMap[transposeIndexForMergeCell] =
                            std::make_pair(transposedIndexofCell, xCell);
                    }
                }
                else if(xCell->getRowSpan() > 1)
                {
                    // having : vertical merge
                    mpFS->startElementNS(XML_a, XML_tc,
                                         XML_rowSpan, OString::number(xCell->getRowSpan()));

                    for(sal_Int32 rowIndex = nRow; rowIndex < nRow + xCell->getRowSpan(); ++rowIndex) {
                        sal_Int32 transposeIndexForMergeCell = (rowIndex*nColumnCount) + nColumn;
                        mergedCellMap[transposeIndexForMergeCell] =
                            std::make_pair(transposedIndexofCell, xCell);
                    }
                }
                else
                {
                    // now, the cell can be an independent cell or
                    // it can be a cell which is been merged to some parent cell
                    if(!xCell->isMerged())
                    {
                        // independent cell
                        mpFS->startElementNS(XML_a, XML_tc);
                    }
                    else
                    {
                        // it a merged cell to some parent cell
                        // find the parent cell for the current cell at hand
                        transposeTableMap::iterator it = mergedCellMap.find(transposedIndexofCell);
                        if(it != mergedCellMap.end())
                        {
                            sal_Int32 transposeIndexOfParent = it->second.first;
                            Reference< XMergeableCell > parentCell = it->second.second;
                            // finding the row and column index for the parent cell from transposed index
                            sal_Int32 parentColumnIndex = transposeIndexOfParent % nColumnCount;
                            sal_Int32 parentRowIndex = transposeIndexOfParent / nColumnCount;
                            if(nColumn == parentColumnIndex)
                            {
                                // the cell is vertical merge and it might have gridspan
                                if(parentCell->getColumnSpan() > 1)
                                {
                                    // vMerge and has gridSpan
                                    mpFS->startElementNS(XML_a, XML_tc,
                                                         XML_vMerge, OString::number(1),
                                                         XML_gridSpan, OString::number(xCell->getColumnSpan()));
                                }
                                else
                                {
                                    // only vMerge
                                    mpFS->startElementNS(XML_a, XML_tc,
                                                         XML_vMerge, OString::number(1));
                                }
                            }
                            else if(nRow == parentRowIndex)
                            {
                                // the cell is horizontal merge and it might have rowspan
                                if(parentCell->getRowSpan() > 1)
                                {
                                    // hMerge and has rowspan
                                    mpFS->startElementNS(XML_a, XML_tc,
                                                         XML_hMerge, OString::number(1),
                                                         XML_rowSpan, OString::number(xCell->getRowSpan()));
                                }
                                else
                                {
                                    // only hMerge
                                    mpFS->startElementNS(XML_a, XML_tc,
                                                         XML_hMerge, OString::number(1));
                                }
                            }
                            else
                            {
                                // has hMerge and vMerge
                                mpFS->startElementNS(XML_a, XML_tc,
                                                     XML_vMerge, OString::number(1),
                                                     XML_hMerge, OString::number(1));
                            }
                        }
                        else
                            bCellOpened = false;
                    }
                }

                if (bCellOpened)
                {
                    WriteTextBox( xCell, XML_a );

                    Reference< XPropertySet > xCellPropSet(xCell, UNO_QUERY_THROW);
                    WriteTableCellProperties(xCellPropSet);

                    mpFS->endElementNS( XML_a, XML_tc );
                }
            }

            mpFS->endElementNS( XML_a, XML_tr );
        }

        mpFS->endElementNS( XML_a, XML_tbl );
    }

    mpFS->endElementNS( XML_a, XML_graphicData );
    mpFS->endElementNS( XML_a, XML_graphic );
}

void ShapeExport::WriteTableCellProperties(const Reference< XPropertySet>& xCellPropSet)
{
    sal_Int32 nLeftMargin(0), nRightMargin(0);
    TextVerticalAdjust eVerticalAlignment;
    const char* sVerticalAlignment;

    Any aLeftMargin = xCellPropSet->getPropertyValue(u"TextLeftDistance"_ustr);
    aLeftMargin >>= nLeftMargin;

    Any aRightMargin = xCellPropSet->getPropertyValue(u"TextRightDistance"_ustr);
    aRightMargin >>= nRightMargin;

    Any aVerticalAlignment = xCellPropSet->getPropertyValue(u"TextVerticalAdjust"_ustr);
    aVerticalAlignment >>= eVerticalAlignment;
    sVerticalAlignment = GetTextVerticalAdjust(eVerticalAlignment);

    sal_Int32 nRotateAngle = 0;
    Any aRotateAngle = xCellPropSet->getPropertyValue(u"RotateAngle"_ustr);
    aRotateAngle >>= nRotateAngle;
    std::optional<OString> aTextVerticalValue = GetTextVerticalType(nRotateAngle);

    Sequence<PropertyValue> aGrabBag;
    if( !aTextVerticalValue &&
        (xCellPropSet->getPropertyValue(u"CellInteropGrabBag"_ustr) >>= aGrabBag) )
    {
        for (auto const& rIt : aGrabBag)
        {
            if (rIt.Name == "mso-tcPr-vert-value")
            {
                aTextVerticalValue = rIt.Value.get<OUString>().toUtf8();
                break;
            }
        }
    }

    mpFS->startElementNS(XML_a, XML_tcPr, XML_anchor, sVerticalAlignment,
    XML_vert, aTextVerticalValue,
    XML_marL, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nLeftMargin)), nLeftMargin > 0),
    XML_marR, sax_fastparser::UseIf(OString::number(oox::drawingml::convertHmmToEmu(nRightMargin)), nRightMargin > 0));

    // Write background fill for table cell.
    // TODO
    // tcW : Table cell width
    WriteTableCellBorders(xCellPropSet);
    DrawingML::WriteFill(xCellPropSet);
    mpFS->endElementNS( XML_a, XML_tcPr );
}

void ShapeExport::WriteBorderLine(const sal_Int32 xml_line_element, const BorderLine2& rBorderLine)
{
// While importing the table cell border line width, it converts EMU->Hmm then divided result by 2.
// To get original value of LineWidth need to multiple by 2.
    sal_Int32 nBorderWidth = rBorderLine.LineWidth;
    nBorderWidth *= 2;
    nBorderWidth = oox::drawingml::convertHmmToEmu( nBorderWidth );

    if ( nBorderWidth > 0 )
    {
        mpFS->startElementNS(XML_a, xml_line_element, XML_w, OString::number(nBorderWidth));
        if ( rBorderLine.Color == sal_Int32( COL_AUTO ) )
            mpFS->singleElementNS(XML_a, XML_noFill);
        else
            DrawingML::WriteSolidFill( ::Color(ColorTransparency, rBorderLine.Color) );

        OUString sBorderStyle;
        sal_Int16 nStyle = rBorderLine.LineStyle;
        mAny.setValue(&nStyle, cppu::UnoType<sal_Int16>::get());
        switch (*o3tl::doAccess<sal_Int16>(mAny))
        {
            case ::table::BorderLineStyle::SOLID:
                sBorderStyle = "solid";
                break;
            case ::table::BorderLineStyle::DOTTED:
                sBorderStyle = "dot";
                break;
            case ::table::BorderLineStyle::DASHED:
                sBorderStyle = "dash";
                break;
            case ::table::BorderLineStyle::DASH_DOT:
                sBorderStyle = "dashDot";
                break;
            case ::table::BorderLineStyle::DASH_DOT_DOT:
                sBorderStyle = "sysDashDotDot";
                break;
        }
        mpFS->singleElementNS(XML_a, XML_prstDash, XML_val, sBorderStyle);
        mpFS->endElementNS(XML_a, xml_line_element);
    }
    else if( nBorderWidth == 0)
    {
        mpFS->startElementNS(XML_a, xml_line_element);
        mpFS->singleElementNS(XML_a, XML_noFill);
        mpFS->endElementNS(XML_a, xml_line_element);
    }
}

void ShapeExport::WriteTableCellBorders(const Reference< XPropertySet>& xCellPropSet)
{
    BorderLine2 aBorderLine;

// lnL - Left Border Line Properties of table cell
    xCellPropSet->getPropertyValue(u"LeftBorder"_ustr) >>= aBorderLine;
    WriteBorderLine( XML_lnL, aBorderLine );

// lnR - Right Border Line Properties of table cell
    xCellPropSet->getPropertyValue(u"RightBorder"_ustr) >>= aBorderLine;
    WriteBorderLine( XML_lnR, aBorderLine );

// lnT - Top Border Line Properties of table cell
    xCellPropSet->getPropertyValue(u"TopBorder"_ustr) >>= aBorderLine;
    WriteBorderLine( XML_lnT, aBorderLine );

// lnB - Bottom Border Line Properties of table cell
    xCellPropSet->getPropertyValue(u"BottomBorder"_ustr) >>= aBorderLine;
    WriteBorderLine( XML_lnB, aBorderLine );
}

ShapeExport& ShapeExport::WriteTableShape( const Reference< XShape >& xShape )
{
    FSHelperPtr pFS = GetFS();

    pFS->startElementNS(mnXmlNamespace, XML_graphicFrame);

    pFS->startElementNS(mnXmlNamespace, XML_nvGraphicFramePr);

    Reference<XPropertySet> const xShapeProps(xShape, UNO_QUERY);
    pFS->startElementNS(mnXmlNamespace, XML_cNvPr,
                          XML_id, OString::number(GetNewShapeID(xShape)),
                          XML_name,   GetShapeName(xShape));
    AddExtLst(pFS, xShapeProps);
    pFS->endElementNS(mnXmlNamespace, XML_cNvPr);

    pFS->singleElementNS(mnXmlNamespace, XML_cNvGraphicFramePr);

    if( GetDocumentType() == DOCUMENT_PPTX )
        pFS->singleElementNS(mnXmlNamespace, XML_nvPr);
    pFS->endElementNS( mnXmlNamespace, XML_nvGraphicFramePr );

    WriteShapeTransformation( xShape, mnXmlNamespace );
    WriteTable( xShape );

    pFS->endElementNS( mnXmlNamespace, XML_graphicFrame );

    return *this;
}

ShapeExport& ShapeExport::WriteTextShape( const Reference< XShape >& xShape )
{
    FSHelperPtr pFS = GetFS();
    Reference<XPropertySet> xShapeProps(xShape, UNO_QUERY);

    pFS->startElementNS(mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp));

    // non visual shape properties
    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
    {
        pFS->startElementNS(mnXmlNamespace, XML_nvSpPr);
        pFS->startElementNS(mnXmlNamespace, XML_cNvPr,
                              XML_id, OString::number(GetNewShapeID(xShape)),
                              XML_name, GetShapeName(xShape));
        OUString sURL;
        if (GetProperty(xShapeProps, u"URL"_ustr))
            mAny >>= sURL;

        if (!sURL.isEmpty())
        {
            OUString sRelId = mpFB->addRelation(mpFS->getOutputStream(),
                    oox::getRelationship(Relationship::HYPERLINK),
                    mpURLTransformer->getTransformedString(sURL),
                    mpURLTransformer->isExternalURL(sURL));

            mpFS->singleElementNS(XML_a, XML_hlinkClick, FSNS(XML_r, XML_id), sRelId);
        }
        AddExtLst(pFS, xShapeProps);
        pFS->endElementNS(mnXmlNamespace, XML_cNvPr);
    }
    pFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr, XML_txBox, "1");
    if (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes)
    {
        WriteNonVisualProperties( xShape );
        pFS->endElementNS( mnXmlNamespace, XML_nvSpPr );
    }

    // visual shape properties
    pFS->startElementNS(mnXmlNamespace, XML_spPr);
    WriteShapeTransformation( xShape, XML_a );
    WritePresetShape( "rect"_ostr );
    uno::Reference<beans::XPropertySet> xPropertySet(xShape, UNO_QUERY);
    if (!IsFontworkShape(xShapeProps)) // Fontwork needs fill and outline in run properties instead.
    {
        WriteBlipOrNormalFill(xPropertySet, u"Graphic"_ustr, xShape->getSize());
        WriteOutline(xPropertySet);
        WriteShapeEffects(xPropertySet);
    }
    pFS->endElementNS( mnXmlNamespace, XML_spPr );

    WriteTextBox( xShape, mnXmlNamespace );

    pFS->endElementNS( mnXmlNamespace, (GetDocumentType() != DOCUMENT_DOCX || mbUserShapes ? XML_sp : XML_wsp) );

    return *this;
}

void ShapeExport::WriteMathShape(Reference<XShape> const& xShape)
{
    Reference<XPropertySet> const xPropSet(xShape, UNO_QUERY);
    assert(xPropSet.is());
    Reference<XModel> xMathModel;
    xPropSet->getPropertyValue(u"Model"_ustr) >>= xMathModel;
    assert(xMathModel.is());
    assert(GetDocumentType() != DOCUMENT_DOCX); // should be written in DocxAttributeOutput
    SAL_WARN_IF(GetDocumentType() == DOCUMENT_XLSX, "oox.shape", "Math export to XLSX isn't tested, should it happen here?");

    // ECMA standard does not actually allow oMath outside of
    // WordProcessingML so write a MCE like PPT 2010 does
    mpFS->startElementNS(XML_mc, XML_AlternateContent);
    mpFS->startElementNS(XML_mc, XML_Choice,
        FSNS(XML_xmlns, XML_a14), mpFB->getNamespaceURL(OOX_NS(a14)),
        XML_Requires, "a14");
    mpFS->startElementNS(mnXmlNamespace, XML_sp);
    mpFS->startElementNS(mnXmlNamespace, XML_nvSpPr);
    mpFS->startElementNS(mnXmlNamespace, XML_cNvPr,
         XML_id, OString::number(GetNewShapeID(xShape)),
         XML_name, GetShapeName(xShape));
    AddExtLst(mpFS, xPropSet);
    mpFS->endElementNS(mnXmlNamespace, XML_cNvPr);
    mpFS->singleElementNS(mnXmlNamespace, XML_cNvSpPr, XML_txBox, "1");
    mpFS->singleElementNS(mnXmlNamespace, XML_nvPr);
    mpFS->endElementNS(mnXmlNamespace, XML_nvSpPr);
    mpFS->startElementNS(mnXmlNamespace, XML_spPr);
    WriteShapeTransformation(xShape, XML_a);
    WritePresetShape("rect"_ostr);
    mpFS->endElementNS(mnXmlNamespace, XML_spPr);
    mpFS->startElementNS(mnXmlNamespace, XML_txBody);
    mpFS->startElementNS(XML_a, XML_bodyPr);
    mpFS->endElementNS(XML_a, XML_bodyPr);
    mpFS->startElementNS(XML_a, XML_p);
    mpFS->startElementNS(XML_a14, XML_m);

    oox::FormulaImExportBase *const pMagic(
        dynamic_cast<oox::FormulaImExportBase*>(xMathModel.get()));
    assert(pMagic);
    pMagic->writeFormulaOoxml(GetFS(), GetFB()->getVersion(), GetDocumentType(),
        FormulaImExportBase::eFormulaAlign::INLINE);

    mpFS->endElementNS(XML_a14, XML_m);
    mpFS->endElementNS(XML_a, XML_p);
    mpFS->endElementNS(mnXmlNamespace, XML_txBody);
    mpFS->endElementNS(mnXmlNamespace, XML_sp);
    mpFS->endElementNS(XML_mc, XML_Choice);
    mpFS->startElementNS(XML_mc, XML_Fallback);
    // TODO: export bitmap shape as fallback
    mpFS->endElementNS(XML_mc, XML_Fallback);
    mpFS->endElementNS(XML_mc, XML_AlternateContent);
}

ShapeExport& ShapeExport::WriteOLE2Shape( const Reference< XShape >& xShape )
{
    Reference< XPropertySet > xPropSet( xShape, UNO_QUERY );
    if (!xPropSet.is())
        return *this;

    enum { CHART, MATH, OTHER } eType(OTHER);
    OUString clsid;
    xPropSet->getPropertyValue(u"CLSID"_ustr) >>= clsid;
    if (!clsid.isEmpty())
    {
        SvGlobalName aClassID;
        bool const isValid = aClassID.MakeId(clsid);
        assert(isValid); (void)isValid;
        if (SotExchange::IsChart(aClassID))
            eType = CHART;
        else if (SotExchange::IsMath(aClassID))
            eType = MATH;
    }

    if (CHART == eType)
    {
        Reference< XChartDocument > xChartDoc;
        xPropSet->getPropertyValue(u"Model"_ustr) >>= xChartDoc;
        assert(xChartDoc.is());
        //export the chart
#if !ENABLE_WASM_STRIP_CHART
        // WASM_CHART change
        // TODO: With Chart extracted this cannot really happen since
        // no Chart could've been added at all
        ChartExport aChartExport( mnXmlNamespace, GetFS(), xChartDoc, GetFB(), GetDocumentType() );
        aChartExport.WriteChartObj( xShape, GetNewShapeID( xShape ), ++mnChartCount );
#endif
        return *this;
    }

    if (MATH == eType)
    {
        WriteMathShape(xShape);
        return *this;
    }

    uno::Reference<embed::XEmbeddedObject> const xObj(
        xPropSet->getPropertyValue(u"EmbeddedObject"_ustr), uno::UNO_QUERY);

    if (!xObj.is())
    {
        SAL_WARN("oox.shape", "ShapeExport::WriteOLE2Shape: no object");

        // tdf#152436 Export the preview graphic of the object if the object is missing.
        SdrObject* pSdrOLE2(SdrObject::getSdrObjectFromXShape(xShape));
        if (auto pOle2Obj = dynamic_cast<SdrOle2Obj*>(pSdrOLE2))
        {
            const Graphic* pGraphic = pOle2Obj->GetGraphic();
            if (pGraphic)
                WriteGraphicObjectShapePart(xShape, pGraphic);
        }

        return *this;
    }

    uno::Sequence<beans::PropertyValue> grabBag;
    OUString entryName;
    try
    {
        uno::Reference<beans::XPropertySet> const xParent(
            uno::Reference<container::XChild>(xObj, uno::UNO_QUERY_THROW)->getParent(),
            uno::UNO_QUERY_THROW);

        xParent->getPropertyValue(u"InteropGrabBag"_ustr) >>= grabBag;

        entryName = uno::Reference<embed::XEmbedPersist>(xObj, uno::UNO_QUERY_THROW)->getEntryName();
    }
    catch (uno::Exception const&)
    {
        TOOLS_WARN_EXCEPTION("oox.shape", "ShapeExport::WriteOLE2Shape");
        return *this;
    }

    OUString progID;

    for (auto const& it : grabBag)
    {
        if (it.Name == "EmbeddedObjects")
        {
            uno::Sequence<beans::PropertyValue> objects;
            it.Value >>= objects;
            for (auto const& object : objects)
            {
                if (object.Name == entryName)
                {
                    uno::Sequence<beans::PropertyValue> props;
                    object.Value >>= props;
                    for (auto const& prop : props)
                    {
                        if (prop.Name == "ProgID")
                        {
                            prop.Value >>= progID;
                            break;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }

    OUString sMediaType;
    OUString sRelationType;
    OUString sSuffix;
    const char * pProgID(nullptr);
    OString anotherProgID;

    uno::Reference<io::XInputStream> const xInStream =
        oox::GetOLEObjectStream(
            mpFB->getComponentContext(), xObj, progID,
            sMediaType, sRelationType, sSuffix, pProgID);

    OUString sURL;
    OUString sRelId;
    if (!xInStream.is())
    {
        xPropSet->getPropertyValue(u"LinkURL"_ustr) >>= sURL;
        if (sURL.isEmpty())
            return *this;

        sRelId = mpFB->addRelation(mpFS->getOutputStream(),
                                   oox::getRelationship(Relationship::OLEOBJECT), sURL, true);
    }
    else
    {
        if (!pProgID && !progID.isEmpty())
        {
            anotherProgID = OUStringToOString(progID, RTL_TEXTENCODING_UTF8);
            pProgID = anotherProgID.getStr();
        }

        assert(!sMediaType.isEmpty());
        assert(!sRelationType.isEmpty());
        assert(!sSuffix.isEmpty());

        OUString sNumber = OUString::number(++m_nEmbeddedObjects);
        OUString sFileName = u"embeddings/oleObject"_ustr + sNumber + u"."_ustr + sSuffix;
        OUString sFilePath = GetComponentDir() + u"/"_ustr + sFileName;
        uno::Reference<io::XOutputStream> const xOutStream(mpFB->openFragmentStream(sFilePath, sMediaType));
        assert(xOutStream.is()); // no reason why that could fail

        try
        {
            ::comphelper::OStorageHelper::CopyInputToOutput(xInStream, xOutStream);
        }
        catch (uno::Exception const&)
        {
            TOOLS_WARN_EXCEPTION("oox.shape", "ShapeExport::WriteOLEObject");
        }

        sRelId = mpFB->addRelation(
            mpFS->getOutputStream(), sRelationType,
            Concat2View(GetRelationCompPrefix() + sFileName));
    }

    sal_Int64 nAspect;
    bool bShowAsIcon = (xPropSet->getPropertyValue(u"Aspect"_ustr) >>= nAspect)
                       && nAspect == embed::Aspects::MSOLE_ICON;

    mpFS->startElementNS(mnXmlNamespace, XML_graphicFrame);

    mpFS->startElementNS(mnXmlNamespace, XML_nvGraphicFramePr);

    mpFS->startElementNS(mnXmlNamespace, XML_cNvPr,
                           XML_id,     OString::number(GetNewShapeID(xShape)),
                           XML_name,   GetShapeName(xShape));
    AddExtLst(mpFS, xPropSet);
    mpFS->endElementNS(mnXmlNamespace, XML_cNvPr);

    mpFS->singleElementNS(mnXmlNamespace, XML_cNvGraphicFramePr);

    if (GetDocumentType() == DOCUMENT_PPTX)
        mpFS->singleElementNS(mnXmlNamespace, XML_nvPr);
    mpFS->endElementNS( mnXmlNamespace, XML_nvGraphicFramePr );

    WriteShapeTransformation( xShape, mnXmlNamespace );

    mpFS->startElementNS(XML_a, XML_graphic);
    mpFS->startElementNS(XML_a, XML_graphicData,
                         XML_uri, "http://schemas.openxmlformats.org/presentationml/2006/ole");
    if (pProgID)
    {
        mpFS->startElementNS( mnXmlNamespace, XML_oleObj,
                          XML_showAsIcon, sax_fastparser::UseIf("1", bShowAsIcon),
                          XML_progId, pProgID,
                          FSNS(XML_r, XML_id), sRelId,
                          XML_spid, "" );
    }
    else
    {
        mpFS->startElementNS( mnXmlNamespace, XML_oleObj,
//?                                              XML_name, "Document",
                          XML_showAsIcon, sax_fastparser::UseIf("1", bShowAsIcon),
                          FSNS(XML_r, XML_id), sRelId,
                          // The spec says that this is a required attribute, but PowerPoint can only handle an empty value.
                          XML_spid, "" );
    }

    if (sURL.isEmpty())
        mpFS->singleElementNS(mnXmlNamespace, XML_embed);
    else
        mpFS->singleElementNS(mnXmlNamespace, XML_link, XML_updateAutomatic, "1");

    // pic element
    SdrObject* pSdrOLE2(SdrObject::getSdrObjectFromXShape(xShape));
    // The spec doesn't allow <p:pic> here, but PowerPoint requires it.
    bool const bEcma = mpFB->getVersion() == oox::core::ECMA_376_1ST_EDITION;
    if (bEcma)
        if (auto pOle2Obj = dynamic_cast<SdrOle2Obj*>(pSdrOLE2))
        {
            const Graphic* pGraphic = pOle2Obj->GetGraphic();
            if (pGraphic)
                WriteGraphicObjectShapePart( xShape, pGraphic );
        }

    mpFS->endElementNS( mnXmlNamespace, XML_oleObj );

    mpFS->endElementNS( XML_a, XML_graphicData );
    mpFS->endElementNS( XML_a, XML_graphic );

    mpFS->endElementNS( mnXmlNamespace, XML_graphicFrame );

    return *this;
}

ShapeExport& ShapeExport::WriteUnknownShape( const Reference< XShape >& )
{
    // Override this method to do something useful.
    return *this;
}

sal_Int32 ShapeExport::GetNewShapeID( const Reference< XShape >& rXShape )
{
    return GetNewShapeID( rXShape, GetFB() );
}

sal_Int32 ShapeExport::GetNewShapeID( const Reference< XShape >& rXShape, XmlFilterBase* pFB )
{
    if( !rXShape.is() )
        return -1;

    sal_Int32 nID = pFB->GetUniqueId();

    (*mpShapeMap)[ rXShape ] = nID;

    return nID;
}

sal_Int32 ShapeExport::GetShapeID( const Reference< XShape >& rXShape )
{
    return GetShapeID( rXShape, mpShapeMap );
}

sal_Int32 ShapeExport::GetShapeID( const Reference< XShape >& rXShape, ShapeHashMap* pShapeMap )
{
    if( !rXShape.is() )
        return -1;

    ShapeHashMap::const_iterator aIter = pShapeMap->find( rXShape );

    if( aIter == pShapeMap->end() )
        return -1;

    return aIter->second;
}

OUString ShapeExport::GetShapeName(const Reference<XShape>& xShape)
{
    Reference<XPropertySet> rXPropSet(xShape, UNO_QUERY);

    // Empty name keeps the object unnamed.
    OUString sName;

    if (GetProperty(rXPropSet, u"Name"_ustr))
        mAny >>= sName;
    return sName;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
