/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>

#include "check.hxx"
#include "plugin.hxx"

/**
comparing floating point numbers using == or != is a bad idea.
*/

namespace {

class FpComparison:
    public RecursiveASTVisitor<FpComparison>, public loplugin::Plugin
{
public:
    explicit FpComparison(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        vclFloatDevicePixel = compiler.getPreprocessor()
            .getIdentifierInfo("VCL_FLOAT_DEVICE_PIXEL")->hasMacroDefinition();
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinaryOperator(const BinaryOperator* );
    bool TraverseFunctionDecl(FunctionDecl* );
    bool TraverseCXXMethodDecl(CXXMethodDecl* );
private:
    bool ignore(FunctionDecl* );
    enum class EState { None, TraverseProcess, TraverseIgnore };
    EState meState = EState::None;
    bool vclFloatDevicePixel;
};

bool FpComparison::TraverseFunctionDecl(FunctionDecl* function)
{
    bool bIgnore = ignore(function);
    meState = bIgnore ? EState::TraverseIgnore : EState::TraverseProcess;
    bool bRet = RecursiveASTVisitor::TraverseFunctionDecl(function);
    meState = EState::None;
    return bRet;
}

bool FpComparison::TraverseCXXMethodDecl(CXXMethodDecl* function)
{
    bool bIgnore = ignore(function);
    meState = bIgnore ? EState::TraverseIgnore : EState::TraverseProcess;
    bool bRet = RecursiveASTVisitor::TraverseCXXMethodDecl(function);
    meState = EState::None;
    return bRet;
}

bool FpComparison::ignore(FunctionDecl* function)
{
    if (ignoreLocation(function)) {
        return true;
    }
    // we assume that these modules know what they are doing with FP stuff
    StringRef aFileName = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(function->getLocStart()));
    if (loplugin::hasPathnamePrefix(aFileName, SRCDIR "/sc/")) {
        return true;
    }
    if (!function->doesThisDeclarationHaveABody()) {
        return true;
    }
     // Ignore operator== and operator!=
    if (function->getOverloadedOperator() == OO_EqualEqual
        || function->getOverloadedOperator() == OO_ExclaimEqual) {
        return true;
    }
    // ignore known good functions
    loplugin::DeclCheck dc(function);
    if ((dc.Function("approxEqual").Namespace("math").Namespace("rtl")
         .GlobalNamespace())
        || dc.Function("doubleToString").AnonymousNamespace().GlobalNamespace()
        || dc.Function("stringToDouble").AnonymousNamespace().GlobalNamespace()
        || dc.Function("rtl_math_round").GlobalNamespace()
        || dc.Function("rtl_math_approxEqual").GlobalNamespace()
        || dc.Function("rtl_math_approxValue").GlobalNamespace()
        || dc.Function("rtl_math_asinh").GlobalNamespace()
        || dc.Function("rtl_math_acosh").GlobalNamespace()
        || dc.Function("_equalSequence").Namespace("cppu").GlobalNamespace()
            // cppu/source/uno/eq.hxx
        || dc.Function("_equalData").Namespace("cppu").GlobalNamespace()
            // cppu/source/uno/eq.hxx
        || dc.Function("equalFont").Namespace("xmlscript").GlobalNamespace()
            // xmlscript/source/xmldlg_imexp/xmldlg_export.cxx
        || (dc.Function("initialize").Class("Impl2").AnonymousNamespace()
            .GlobalNamespace())
            // testtools/source/bridgetest/constructors.cxx
        || (dc.Function("initialize").Class("Impl").AnonymousNamespace()
            .GlobalNamespace())
            // testtools/source/bridgetest/constructors.cxx
        || dc.Function("lok_approxEqual").AnonymousNamespace().GlobalNamespace()
            // libreofficekit/source/gtk/lokdocview.cxx
        // These might need fixing:
        || (dc.Function("getSmallestDistancePointToPolygon").Namespace("tools")
            .Namespace("basegfx").GlobalNamespace())
            // basegfx/source/polygon/b2dpolygontools.cxx
        || (dc.Function("getSmallestDistancePointToPolyPolygon")
            .Namespace("tools").Namespace("basegfx").GlobalNamespace())
            // basegfx/source/polygon/b2dpolypolygontools.cxx
        || dc.Function("performTest").Namespace("bridge_test").GlobalNamespace()
            // testtools/source/bridgetest/bridgetest.cxx
        || dc.Function("equals").Namespace("bridge_test").GlobalNamespace()
        || (dc.Function("lcl_getNANInsteadDBL_MIN").AnonymousNamespace()
            .GlobalNamespace())
            // chart2/source/controller/chartapiwrapper/ChartDataWrapper.cxx
        //TODO:
        || dc.Function("ImpCheckCondition").Class("SvNumberformat").GlobalNamespace()
        || dc.Function("GetTimeFormat").Class("SvNumberFormatter").GlobalNamespace()
        || dc.Function("GuessDateTimeFormat").Class("SvNumberFormatter").GlobalNamespace()
        || dc.Function("GetEditFormat").Class("SvNumberFormatter").GlobalNamespace()
        || dc.Function("getSmallestDistancePointToBezierSegment").Class("B2DCubicBezier").Namespace("basegfx").GlobalNamespace()
        || dc.Function("getLength").Class("B3DVector").Namespace("basegfx").GlobalNamespace()
        || dc.Function("getXZLength").Class("B3DVector").Namespace("basegfx").GlobalNamespace()
        || dc.Function("getYZLength").Class("B3DVector").Namespace("basegfx").GlobalNamespace()
        || dc.Function("impSolve").Class("solver").AnonymousNamespace().Namespace("basegfx").GlobalNamespace()
        || dc.Function("getB2DPolyPolygon").Class("solver").AnonymousNamespace().Namespace("basegfx").GlobalNamespace()
        || dc.Function("getNormal").Class("CoordinateDataArray3D").GlobalNamespace()
        || dc.Function("append").Class("ImplPolygon").AnonymousNamespace().Namespace("basegfx").GlobalNamespace()
        || dc.Function("finish").Class("ImplPolygon").AnonymousNamespace().Namespace("basegfx").GlobalNamespace()
        || dc.Function("convertMeasure").Class("Converter").Namespace("sax").GlobalNamespace()
        || dc.Function("convertDouble").Class("Converter").Namespace("sax").GlobalNamespace()
        || dc.Function("convertDuration").Class("Converter").Namespace("sax").GlobalNamespace()
        || dc.Function("Frustum").Class("B3dTransformationSet").GlobalNamespace()
        || dc.Function("Ortho").Class("B3dTransformationSet").GlobalNamespace()
        || dc.Function("SetRatio").Class("B3dTransformationSet").GlobalNamespace()
        || dc.Function("SetDeviceRectangle").Class("B3dTransformationSet").GlobalNamespace()
        || dc.Function("HSBtoRGB").Class("Color").GlobalNamespace()
        || dc.Function("Normalize").Class("Vector2D").Namespace("tools").GlobalNamespace()
        || dc.Function("setAcceleration").Class("AnimationNode").Namespace("animcore").GlobalNamespace()
        || dc.Function("setDecelerate").Class("AnimationNode").Namespace("animcore").GlobalNamespace()
        || dc.Function("setVolume").Class("AnimationNode").Namespace("animcore").GlobalNamespace()
        || dc.Function("setIterateInterval").Class("AnimationNode").Namespace("animcore").GlobalNamespace()
        || dc.Function("createElement").Struct("Style").Namespace("xmlscript").GlobalNamespace()
        || dc.Function("set_property").Class("Window").Namespace("vcl").GlobalNamespace()
        || dc.Function("Adjust").Class("Bitmap").GlobalNamespace()
        || dc.Function("ImplWriteActions").Class("SVMConverter").GlobalNamespace()
        || dc.Function("Mirror").Class("GDIMetaFile").GlobalNamespace()
        || dc.Function("Adjust").Class("GDIMetaFile").GlobalNamespace()
        || dc.Function("Scale").Class("MetaCommentAction").GlobalNamespace()
        || dc.Function("ImplGetGamma").Class("PNGReaderImpl").Namespace("vcl").GlobalNamespace()
        || dc.Function("playMetafile").Class("PDFWriterImpl").Namespace("vcl").GlobalNamespace()
        || dc.Function("invert").Class("Matrix3").Namespace("vcl").GlobalNamespace()
        || dc.Function("emitTilings").Class("PDFWriterImpl").Namespace("vcl").GlobalNamespace()
        || dc.Function("drawHorizontalGlyphs").Class("PDFWriterImpl").Namespace("vcl").GlobalNamespace()
        || dc.Function("ImplWrite").Class("EMFWriter").GlobalNamespace()
        || dc.Function("WriteRecords").Class("WMFWriter").GlobalNamespace()
        || dc.Function("convertOneTypeEsc").Class("CffSubsetterContext").GlobalNamespace()
        || dc.Function("ApplyGlyphTransform").Class("FreetypeFont").GlobalNamespace()
        || dc.Function("JoinVerticalClipRectangles").Class("PrinterGfx").Namespace("psp").GlobalNamespace()
        || dc.Function("PSSetLineWidth").Class("PrinterGfx").Namespace("psp").GlobalNamespace()
        || dc.Function("DrawEPS").Class("PrinterGfx").Namespace("psp").GlobalNamespace()
        || dc.Function("addDrawRectangle").Class("RenderList").GlobalNamespace()
        || dc.Function("addDrawPolyPolygon").Class("RenderList").GlobalNamespace()
        || dc.Function("addDrawPolyLine").Class("RenderList").GlobalNamespace()
        || dc.Function("ApplyMatrix").Class("OpenGLProgram").GlobalNamespace()
        || dc.Function("GuessWidth").Class("BorderWidthImpl").GlobalNamespace()
        || dc.Function("ImplSetValue").Class("FormattedField").GlobalNamespace()
        || dc.Function("IsAdjusted").Class("GraphicAttr").GlobalNamespace()
        || dc.Operator(OO_Call).Struct("SpriteWeakOrder").Namespace("canvas").GlobalNamespace()
        || dc.Function("setAlpha").Class("CanvasCustomSpriteHelper").Namespace("canvas").GlobalNamespace()
        || dc.Function("setPriority").Class("CanvasCustomSpriteHelper").Namespace("canvas").GlobalNamespace()
        || dc.Function("createFillGraphicAttribute").Class("SdrFillGraphicAttribute").Namespace("attribute").Namespace("drawinglayer").GlobalNamespace()
        || dc.Function("create2DDecomposition").Class("ScenePrimitive2D").Namespace("primitive2d").Namespace("drawinglayer").GlobalNamespace()
        || dc.Function("createAtom").Class("SvgLinearGradientPrimitive2D").Namespace("primitive2d").Namespace("drawinglayer").GlobalNamespace()
        || dc.Function("createAtom").Class("SvgRadialGradientPrimitive2D").Namespace("primitive2d").Namespace("drawinglayer").GlobalNamespace()
        || dc.Function("FoldConstantsBinaryNode").Class("SbiExprNode").GlobalNamespace()
        || dc.Function("Format").Class("SbxValue").GlobalNamespace()
        || dc.Function("Compare").Class("SbxValue").GlobalNamespace()
        || dc.Function("SelectPlayToolBoxItem").Class("MediaControlBase").Namespace("avmedia").GlobalNamespace()
        || dc.Function("convertDateTime").Class("SvXMLUnitConverter").GlobalNamespace()
        || dc.Function("exportAudio").Class("AnimationsExporterImpl").Namespace("xmloff").GlobalNamespace()
        || dc.Function("StartElement").Class("XMLEnhancedCustomShapeContext").GlobalNamespace()
        || dc.Function("SetString").Class("SdXMLImExTransform2D").GlobalNamespace()
        || dc.Function("SetString").Class("SdXMLImExTransform3D").GlobalNamespace()
        || dc.Function("ExportPart_Impl").Class("SvXMLNumFmtExport").GlobalNamespace()
        || dc.Function("AddNumber").Class("SvXMLNumFormatContext").GlobalNamespace()
        || dc.Function("GetColorData").Class("EnhancedCustomShape2d").GlobalNamespace()
        || dc.Function("AdaptObjColor").Class("EnhancedCustomShape2d").GlobalNamespace()
        || dc.Function("RotateScene").Class("E3dScene").GlobalNamespace()
        || dc.Function("createViewIndependentPrimitive2DSequence").Class("ViewContactOfSdrCaptionObj").Namespace("contact").Namespace("sdr").GlobalNamespace()
        || dc.Function("PostItemChange").Class("E3dSceneProperties").Namespace("properties").Namespace("sdr").GlobalNamespace()
        || dc.Function("NbcRotate").Class("SdrObject").GlobalNamespace()
        || dc.Function("TakeObjNameSingul").Class("SdrPathObj").GlobalNamespace()
        || dc.Function("NbcInsPoint").Class("SdrPathObj").GlobalNamespace()
        || dc.Function("setValue").Class("Cell").Namespace("table").Namespace("sdr").GlobalNamespace()
        || dc.Function("implSetDepth").Class("ExtrusionDepthWindow").Namespace("svx").GlobalNamespace()
        || dc.Function("PointsToBezier").Class("XPolygon").GlobalNamespace()
        || dc.Function("SetPosition").Class("Svx3DLightControl").GlobalNamespace()
        || dc.Function("SetRotation").Class("Svx3DLightControl").GlobalNamespace()
        || dc.Function("PlayToolBoxSelectHdl").Class("MediaPlaybackPanel").Namespace("sidebar").Namespace("svx").GlobalNamespace()
        || dc.Function("CreateGraphicProperties").Class("EscherPropertyContainer").GlobalNamespace()
        || dc.Function("renderSprite").Class("CanvasCustomSprite").Namespace("oglcanvas").GlobalNamespace()
        || dc.Operator(OO_Call).Struct("SpriteComparator").AnonymousNamespace().Namespace("oglcanvas").GlobalNamespace()
        || dc.Function("isHorizontalAxis").Class("TickFactory2D").Namespace("chart").GlobalNamespace()
        || dc.Function("isVerticalAxis").Class("TickFactory2D").Namespace("chart").GlobalNamespace()
        || dc.Function("getDistanceAxisTickToText").Class("TickFactory2D").Namespace("chart").GlobalNamespace()
        || dc.Function("calculateExplicitIncrementAndScaleForLogarithmic").Class("ScaleAutomatism").Namespace("chart").GlobalNamespace()
        || dc.Function("calculateExplicitIncrementAndScaleForLinear").Class("ScaleAutomatism").Namespace("chart").GlobalNamespace()
        || dc.Function("makeTickmarkPropertiesForComplexCategories").Struct("AxisProperties").Namespace("chart").GlobalNamespace()
        || dc.Function("createShapes").Class("BarChart").Namespace("chart").GlobalNamespace()
        || dc.Function("transform").Class("Linear3DTransformation").Namespace("chart").GlobalNamespace()
        || dc.Function("CalculateCubicSplines").Class("SplineCalculater").Namespace("chart").GlobalNamespace()
        || dc.Function("setDiagramPositioning").Class("DiagramHelper").Namespace("chart").GlobalNamespace()
        || dc.Function("centerGrow").Class("RelativePositionHelper").Namespace("chart").GlobalNamespace()
        || dc.Function("pushToPropMap").Struct("FillProperties").Namespace("drawingml").Namespace("oox").GlobalNamespace()
        || dc.Function("convertFromProperties").Class("AxFontDataModel").Namespace("ole").Namespace("oox").GlobalNamespace()
        || dc.Function("isNotANumber").Class("ChartDataWrapper").Namespace("wrapper").Namespace("chart").GlobalNamespace()
        || dc.Function("Reset").Class("ErrorBarResources").Namespace("chart").GlobalNamespace()
        || dc.Function("ApplySpecialItem").Class("AxisItemConverter").Namespace("wrapper").Namespace("chart").GlobalNamespace()
        || dc.Function("ApplySpecialItem").Class("DataPointItemConverter").Namespace("wrapper").Namespace("chart").GlobalNamespace()
        || dc.Function("ApplySpecialItem").Class("TitleItemConverter").Namespace("wrapper").Namespace("chart").GlobalNamespace()
        || dc.Function("ApplySpecialItem").Class("TextLabelItemConverter").Namespace("wrapper").Namespace("chart").GlobalNamespace()
        || dc.Function("operate").Class("OOp_COMPARE").Namespace("file").Namespace("connectivity").GlobalNamespace()
        || dc.Function("SetValues").Class("ColorFieldControl").Namespace("cui").GlobalNamespace()
        || dc.Function("SetValue").Class("ColorSliderControl").Namespace("cui").GlobalNamespace()
        || dc.Function("Write").Class("ORTFImportExport").Namespace("dbaui").GlobalNamespace()
        || dc.Function("appendRow").Class("ORTFImportExport").Namespace("dbaui").GlobalNamespace()
        || dc.Function("WriteCell").Class("OHTMLImportExport").Namespace("dbaui").GlobalNamespace()
        || dc.Function("getBold").Class("VbaFontBase").GlobalNamespace()
        || dc.Function("ModifyHdl").Class("SaneDlg").GlobalNamespace()
        || dc.Function("EstablishNumericOption").Class("SaneDlg").GlobalNamespace()
        || dc.Function("translatePropertiesToItems").Class("ControlCharacterDialog").Namespace("pcr").GlobalNamespace()
        || dc.Function("writeMatrix").Class("Tag").Namespace("swf").GlobalNamespace()
        || dc.Function("Impl_writeActions").Class("Writer").Namespace("swf").GlobalNamespace()
        || dc.Function("Impl_quadBezierApprox").Class("Writer").Namespace("swf").GlobalNamespace()
        || dc.Function("hasGradientOpacity").Struct("AnnotatingVisitor").AnonymousNamespace().Namespace("svgi").GlobalNamespace()
        || dc.Function("getOdfColor").Struct("AnnotatingVisitor").AnonymousNamespace().Namespace("svgi").GlobalNamespace()
        || dc.Function("writeStyle").Struct("AnnotatingVisitor").AnonymousNamespace().Namespace("svgi").GlobalNamespace()
        || dc.Operator(OO_Call).Struct("ShapeWritingVisitor").AnonymousNamespace().Namespace("svgi").GlobalNamespace()
        || dc.Function("SvgDashArray2Odf").Struct("OfficeStylesWritingVisitor").Namespace("svgi").GlobalNamespace()
        || dc.Function("ImplWriteMask").Class("SVGActionWriter").GlobalNamespace()
        || dc.Function("Factor").Class("FormulaCompiler").Namespace("formula").GlobalNamespace()
        || dc.Function("setDateTime").Class("Calendar_gregorian").Namespace("i18n").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()
        || dc.Function("setLocalDateTime").Class("Calendar_gregorian").Namespace("i18n").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()
        || dc.Function("compareSubstring").Class("ChapterCollator").Namespace("i18n").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()
        || dc.Function("ToXml").Class("XFPadding").GlobalNamespace()
        || dc.Function("Equal").Class("XFCellStyle").GlobalNamespace()
        || dc.Function("Equal").Class("XFParaStyle").GlobalNamespace()
        || dc.Function("GetAnimationEffect").Class("EffectMigration").Namespace("sd").GlobalNamespace()
        || dc.Function("SetAnimationSpeed").Class("EffectMigration").Namespace("sd").GlobalNamespace()
        || dc.Function("GetAnimationSpeed").Class("EffectMigration").Namespace("sd").GlobalNamespace()
        || dc.Function("calculateIterateDuration").Class("CustomAnimationEffect").Namespace("sd").GlobalNamespace()
        || dc.Function("setDuration").Class("CustomAnimationEffect").Namespace("sd").GlobalNamespace()
        || dc.Function("replaceNode").Class("CustomAnimationEffect").Namespace("sd").GlobalNamespace()
        || dc.Function("setIterateInterval").Class("CustomAnimationEffect").Namespace("sd").GlobalNamespace()
        || dc.Function("append").Class("EffectSequenceHelper").Namespace("sd").GlobalNamespace()
        || dc.Function("replace").Class("EffectSequenceHelper").Namespace("sd").GlobalNamespace()
        || dc.Function("createTextGroupParagraphEffects").Class("EffectSequenceHelper").Namespace("sd").GlobalNamespace()
        || dc.Function("setTextGrouping").Class("EffectSequenceHelper").Namespace("sd").GlobalNamespace()
        || dc.Function("setTextGroupingAuto").Class("EffectSequenceHelper").Namespace("sd").GlobalNamespace()
        || dc.Function("SetPresentationPenWidth").Class("SdOptionsMisc").GlobalNamespace()
        || dc.Function("VarLook").Class("SwCalc").GlobalNamespace()
        || dc.Function("Prim").Class("SwCalc").GlobalNamespace()
        || dc.Function("keycompare").Struct("SwSortElement").GlobalNamespace()
        || dc.Function("FormatValue").Class("SwDBField").GlobalNamespace()
        || dc.Function("Evaluate").Class("SwDBField").GlobalNamespace()
        || dc.Function("GetValue").Class("SwTableBox").GlobalNamespace()
        || dc.Function("Modify").Class("SwTableBoxFormat").GlobalNamespace()
        || dc.Function("isNotANumber").Class("SwXTextTable").GlobalNamespace()
        || dc.Function("GetNextToken").Class("CSS1Parser").GlobalNamespace()
        || dc.Function("update").Class("FontStylePropertyBox").Namespace("sd").GlobalNamespace()
        || dc.Function("implMenuSelectHdl").Class("FontStylePropertyBox").Namespace("sd").GlobalNamespace()
        || dc.Function("update").Class("CustomAnimationEffectTabPage").Namespace("sd").GlobalNamespace()
        || dc.Function("update").Class("CustomAnimationDurationTabPage").Namespace("sd").GlobalNamespace()
        || dc.Function("update").Class("CustomAnimationTextAnimTabPage").Namespace("sd").GlobalNamespace()
        || dc.Function("FillCalcWithMergeData").Class("SwDBManager").GlobalNamespace()
        || dc.Function("compareWith").Struct("TransitionEffect").Namespace("impl").Namespace("sd").GlobalNamespace()
        || dc.Function("changeSelection").Class("CustomAnimationPane").Namespace("sd").GlobalNamespace()
        || dc.Function("ContextMenuHdl").Class("SlideshowImpl").Namespace("sd").GlobalNamespace()
        || dc.Function("FormatBox").Class("DocxAttributeOutput").GlobalNamespace()
        || dc.Function("GetString").Class("Complex").Namespace("analysis").Namespace("sca").GlobalNamespace()
        || dc.Function("getDelta").Class("AnalysisAddIn").GlobalNamespace()
        || dc.Function("DataToDoc").Class("SwInsertDBColAutoPilot").GlobalNamespace()
        || dc.Function("convertAnimateValue").Class("AnimationExporter").Namespace("ppt").GlobalNamespace()
        || dc.Function("GetId").Class("PPTExBulletProvider").GlobalNamespace()
        || dc.Function("SetThumbPosition").Class("PresenterScrollBar").Namespace("presenter").Namespace("sdext").GlobalNamespace()
        || dc.Function("SetTotalSize").Class("PresenterScrollBar").Namespace("presenter").Namespace("sdext").GlobalNamespace()
        || dc.Function("SetThumbSize").Class("PresenterScrollBar").Namespace("presenter").Namespace("sdext").GlobalNamespace()
        || dc.Function("Layout").Class("PresenterNotesView").Namespace("presenter").Namespace("sdext").GlobalNamespace()
        || dc.Function("compare").Struct("lessThanShape").Class("Shape").Namespace("internal").Namespace("slideshow").GlobalNamespace()
        || dc.Operator(OO_Call).Struct("lessThanArea").Class("HyperlinkArea").Namespace("internal").Namespace("slideshow").GlobalNamespace()
        || dc.Function("viewsChanged").Class("PointerSymbol").Namespace("internal").Namespace("slideshow").GlobalNamespace()
        || dc.Function("RetrieveAttrs").Struct("SmXMLContext_Helper").GlobalNamespace()
        || dc.Function("CompareImpl").Class("SortedResultSet").GlobalNamespace()
        || dc.Function("emit").Struct("PDFNumber").Namespace("pdfparse").GlobalNamespace()
        || dc.Function("visit").Class("DrawXmlOptimizer").Namespace("pdfi").GlobalNamespace()
        || dc.Function("optimizeTextElements").Class("DrawXmlOptimizer").Namespace("pdfi").GlobalNamespace()
        || dc.Function("init").Class("DrawXmlFinalizer").Namespace("pdfi").GlobalNamespace()
        || dc.Function("visit").Class("DrawXmlFinalizer").Namespace("pdfi").GlobalNamespace()
        || dc.Function("GetState").Class("OReportController").Namespace("rptui").GlobalNamespace()
        || dc.Function("isFormatCommandEnabled").Class("OReportController").Namespace("rptui").GlobalNamespace()
        || dc.Function("resolveUnderlines").Struct("PageElement").Namespace("pdfi").GlobalNamespace()
        || dc.Function("visit").Class("WriterXmlOptimizer").Namespace("pdfi").GlobalNamespace()
        || dc.Function("optimizeTextElements").Class("WriterXmlOptimizer").Namespace("pdfi").GlobalNamespace()
        || dc.Function("drawGlyphs").Class("PDFIProcessor").Namespace("pdfi").GlobalNamespace()
        || dc.Function("LTypeToDXFLineInfo").Class("DXF2GDIMetaFile").GlobalNamespace()
        || dc.Function("DrawEntities").Class("DXF2GDIMetaFile").GlobalNamespace()
        || dc.Function("ImplWriteActions").Class("PSWriter").GlobalNamespace()
        || dc.Function("ImplWriteLineInfo").Class("PSWriter").GlobalNamespace()
        || dc.Function("ImplInsert").Class("CGMBitmap").GlobalNamespace()
        || dc.Function("GetNext").Class("CGMBitmap").GlobalNamespace()
        || dc.Function("drawPolyLine").Class("X11SalGraphicsImpl").GlobalNamespace()
        || dc.Function("testRefresh").Class("XDataPilotTable").Namespace("apitest").GlobalNamespace()
        || dc.Function("testTitleManualLayoutXLSX").Class("Chart2ExportTest").GlobalNamespace()
        || dc.Function("testPlotAreaManualLayoutXLSX").Class("Chart2ExportTest").GlobalNamespace()
        || dc.Function("testLegendManualLayoutXLSX").Class("Chart2ExportTest").GlobalNamespace()
        || dc.Function("SetScreenNumber").Class("AquaSalFrame").GlobalNamespace()
        || (vclFloatDevicePixel
            && (dc.Function("Justify").Class("GenericSalLayout").GlobalNamespace()
                || dc.Function("AdjustLayout").Class("MultiSalLayout").GlobalNamespace())))
    {
        return true;
    }
//    cout << "xxx " + function->getQualifiedNameAsString() << endl;
    return false;
}

bool isZeroConstant(ASTContext& context, const Expr* expr)
{
    if (!expr->getType()->isFloatingType()) {
        return false;
    }
    // prevent clang crash
    if (!context.getLangOpts().CPlusPlus) {
        return false;
    }
    APValue result;
    if (!expr->isCXX11ConstantExpr(context, &result)) {
        return false;
    }
    assert(result.isFloat());
    return result.getFloat().isZero();
}
bool FpComparison::VisitBinaryOperator(const BinaryOperator* binaryOp)
{
    if (meState != EState::TraverseProcess || ignoreLocation(binaryOp)) {
        return true;
    }
    if (binaryOp->getOpcode() != BO_EQ && binaryOp->getOpcode() != BO_NE) {
        return true;
    }
    // comparison with zero is valid
    if (isZeroConstant(compiler.getASTContext(), binaryOp->getLHS())
        || isZeroConstant(compiler.getASTContext(), binaryOp->getRHS()))
    {
        return true;
    }
    QualType LHSStrippedType = binaryOp->getLHS()->IgnoreParenImpCasts()->getType();
    QualType RHSStrippedType = binaryOp->getRHS()->IgnoreParenImpCasts()->getType();
    if (LHSStrippedType->isFloatingType() && RHSStrippedType->isFloatingType()) {
        report(
            DiagnosticsEngine::Warning, "floating-point comparison",
            binaryOp->getSourceRange().getBegin())
          << binaryOp->getSourceRange();
    }
    return true;
}


loplugin::Plugin::Registration< FpComparison > X("fpcomparison", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
