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
#include <set>
#include <iostream>

#include "plugin.hxx"

/**
Find parameters that have no name, i.e. they are unused and we're worked around the "unused parameter" warning.

Most of these can be removed.

TODO look for places where we are working around the warning by doing
    (void) param1;
 */
namespace {

class CheckUnusedParams: public loplugin::FilteringPlugin<CheckUnusedParams> {
public:
    explicit CheckUnusedParams(loplugin::InstantiationData const & data):
        FilteringPlugin(data) {}
    void run() override;
    bool VisitFunctionDecl(FunctionDecl const *);
    bool VisitUnaryOperator(UnaryOperator const *);
    bool VisitInitListExpr(InitListExpr const *);
    bool VisitCallExpr(CallExpr const *);
    bool VisitBinaryOperator(BinaryOperator const *);
    bool VisitCXXConstructExpr(CXXConstructExpr const *);
private:
    void checkForFunctionDecl(Expr const *, bool bCheckOnly = false);
    std::set<FunctionDecl const *> m_addressOfSet;
    enum class PluginPhase { FindAddressOf, Warning };
    PluginPhase m_phase;
};

void CheckUnusedParams::run()
{
    StringRef fn(handler.getMainFileName());
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/"))
         return;
    // Taking pointer to function
    if (loplugin::isSamePathname(fn, SRCDIR "/l10ntools/source/xmlparse.cxx"))
         return;
    // macro magic which declares something needed by an external library
    if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/misc/gridprinter.cxx"))
         return;

    // valid test/qa code
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/compilerplugins/clang/test/"))
         return;
    if (loplugin::isSamePathname(fn, SRCDIR "/cppu/qa/test_reference.cxx"))
         return;

    // leave this alone for now
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/libreofficekit/"))
         return;
    // this has a certain pattern to its code which appears to include lots of unused params
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/xmloff/"))
         return;
    // I believe someone is busy working on this chunk of code
    if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/docshell/dataprovider.cxx"))
         return;
    // I think erack is working on stuff here
    if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/excel/xiformula.cxx"))
         return;
    // lots of callbacks here
    if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/lotus/op.cxx"))
         return;
    // template magic
    if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/html/htmlpars.cxx"))
         return;

    m_phase = PluginPhase::FindAddressOf;
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    m_phase = PluginPhase::Warning;
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
}

bool CheckUnusedParams::VisitUnaryOperator(UnaryOperator const * op) {
    if (op->getOpcode() != UO_AddrOf) {
        return true;
    }
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    checkForFunctionDecl(op->getSubExpr());
    return true;
}

bool CheckUnusedParams::VisitBinaryOperator(BinaryOperator const * binaryOperator) {
    if (binaryOperator->getOpcode() != BO_Assign) {
        return true;
    }
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    checkForFunctionDecl(binaryOperator->getRHS());
    return true;
}

bool CheckUnusedParams::VisitCallExpr(CallExpr const * callExpr) {
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    for (auto arg : callExpr->arguments())
        checkForFunctionDecl(arg);
    return true;
}

bool CheckUnusedParams::VisitCXXConstructExpr(CXXConstructExpr const * constructExpr) {
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    for (auto arg : constructExpr->arguments())
        checkForFunctionDecl(arg);
    return true;
}

bool CheckUnusedParams::VisitInitListExpr(InitListExpr const * initListExpr) {
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    for (auto subStmt : *initListExpr)
        checkForFunctionDecl(dyn_cast<Expr>(subStmt));
    return true;
}

void CheckUnusedParams::checkForFunctionDecl(Expr const * expr, bool bCheckOnly) {
    auto e1 = expr->IgnoreParenCasts();
    auto declRef = dyn_cast<DeclRefExpr>(e1);
    if (!declRef)
        return;
    auto functionDecl = dyn_cast<FunctionDecl>(declRef->getDecl());
    if (!functionDecl)
        return;
    if (bCheckOnly)
        getParentStmt(expr)->dump();
    else
        m_addressOfSet.insert(functionDecl->getCanonicalDecl());
}

static int noFieldsInRecord(RecordType const * recordType) {
    auto recordDecl = recordType->getDecl();
    // if it's complicated, lets just assume it has fields
    if (isa<ClassTemplateSpecializationDecl>(recordDecl))
        return 1;
    return std::distance(recordDecl->field_begin(), recordDecl->field_end());
}
static bool startswith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}
static bool endswith(const std::string& rStr, const char* pSubStr) {
    auto len = strlen(pSubStr);
    if (len > rStr.size())
        return false;
    return rStr.compare(rStr.size() - len, rStr.size(), pSubStr) == 0;
}

bool CheckUnusedParams::VisitFunctionDecl(FunctionDecl const * decl) {
    if (m_phase != PluginPhase::Warning)
        return true;
    if (m_addressOfSet.find(decl->getCanonicalDecl()) != m_addressOfSet.end())
        return true;
    if (ignoreLocation(decl))
        return true;
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(decl->getLocation())))
        return true;

    auto cxxMethodDecl = dyn_cast<CXXMethodDecl>(decl);
    if (cxxMethodDecl) {
        if (cxxMethodDecl->isVirtual())
            return true;
        auto cxxConstructorDecl = dyn_cast<CXXConstructorDecl>(cxxMethodDecl);
        if (cxxConstructorDecl && cxxConstructorDecl->isCopyOrMoveConstructor())
            return true;
    }
    if (!decl->isThisDeclarationADefinition())
        return true;
    if (decl->isFunctionTemplateSpecialization())
        return true;
    if (decl->isDeleted())
        return true;
    if (decl->getTemplatedKind() != clang::FunctionDecl::TK_NonTemplate)
        return true;
    if (decl->isOverloadedOperator())
        return true;
    if (decl->isExternC())
        return true;

    //TODO, filtering out any functions relating to class templates for now:
    CXXRecordDecl const * r = dyn_cast<CXXRecordDecl>(decl->getDeclContext());
    if (r != nullptr
        && (r->getTemplateSpecializationKind() != TSK_Undeclared
            || r->isDependentContext()))
    {
        return true;
    }
    FunctionDecl const * canon = decl->getCanonicalDecl();
    std::string fqn = canon->getQualifiedNameAsString(); // because sometimes clang returns nonsense for the filename of canon
    if (ignoreLocation(canon))
        return true;
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(canon->getLocation())))
        return true;
    StringRef fn = getFilenameOfLocation(compiler.getSourceManager().getSpellingLoc(compat::getBeginLoc(canon)));
    // Some backwards compat magic.
    // TODO Can probably be removed, but need to do some checking
    if (loplugin::isSamePathname(fn, SRCDIR "/include/sax/fshelper.hxx"))
         return true;
    // Platform-specific code
    if (loplugin::isSamePathname(fn, SRCDIR "/include/svl/svdde.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/include/vcl/svmain.hxx"))
         return true;
    // passing pointer to function
    if (loplugin::isSamePathname(fn, SRCDIR "/include/vcl/BitmapReadAccess.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/inc/unx/gtk/gtkobject.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/inc/unx/gtk/gtksalframe.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/inc/unx/gtk/gtkframe.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/unx/gtk/fpicker/SalGtkFilePicker.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/extensions/source/propctrlr/propertyeditor.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/forms/source/solar/inc/navtoolbar.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/hwpfilter/source/grammar.cxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/hwpfilter/source/lexer.cxx"))
         return true;
    // marked with a TODO/FIXME
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/inc/sallayout.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/accessibility/inc/standard/vclxaccessiblelist.hxx"))
         return true;
    // these are "extern C" but clang doesn't seem to report that accurately
    if (loplugin::isSamePathname(fn, SRCDIR "/sax/source/fastparser/fastparser.cxx"))
         return true;
    // these all follow the same pattern, seems a pity to break that
    if (loplugin::isSamePathname(fn, SRCDIR "/include/vcl/graphicfilter.hxx"))
         return true;
    // looks like work in progress
    if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/filter/ipdf/pdfdocument.cxx"))
         return true;
    // macro magic
    if (loplugin::isSamePathname(fn, SRCDIR "/basctl/source/inc/basidesh.hxx"))
         return true;
    // template magic
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/canvas/"))
         return true;
    if (loplugin::hasPathnamePrefix(fn, SRCDIR "/include/canvas/"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/include/comphelper/unwrapargs.hxx"))
         return true;
    // this looks like vaguely useful code (ParseError) that I'm loathe to remove
    if (loplugin::isSamePathname(fn, SRCDIR "/connectivity/source/inc/RowFunctionParser.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/include/svx/EnhancedCustomShapeFunctionParser.hxx"))
         return true;
    // TODO marker parameter in constructor, should probably be using an enum
    if (loplugin::isSamePathname(fn, SRCDIR "/framework/inc/uielement/uicommanddescription.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/inc/SlideTransitionPane.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/animations/CustomAnimationPane.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/table/TableDesignPane.hxx"))
         return true;
    // debug stuff
    if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/core/data/column2.cxx"))
         return true;
    // weird stuff
    if (loplugin::isSamePathname(fn, SRCDIR "/scaddins/source/analysis/analysishelper.hxx"))
         return true;
    // SFX_DECL_CHILDWINDOWCONTEXT macro stuff
    if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/inc/NavigatorChildWindow.hxx"))
         return true;
    // TODO, need to remove this from the .sdi file too
    if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/inc/SlideSorterViewShell.hxx"))
         return true;
    if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/inc/OutlineViewShell.hxx"))
         return true;
    // SFX_DECL_INTERFACE macro stuff
    if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/inc/ViewShellBase.hxx"))
         return true;
    // debug stuff
    if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/filter/ppt/pptinanimations.hxx"))
         return true;
    // takes pointer to fn
    if (loplugin::isSamePathname(fn, SRCDIR "/include/sfx2/shell.hxx"))
         return true;
    // TODO, need to remove this from the .sdi file too
    if (fqn == "SfxObjectShell::StateView_Impl")
         return true;
    // SFX_DECL_CHILDWINDOW_WITHID macro
    if (loplugin::isSamePathname(fn, SRCDIR "/include/sfx2/infobar.hxx"))
         return true;
    // this looks like vaguely useful code (ParseError) that I'm loathe to remove
    if (loplugin::isSamePathname(fn, SRCDIR "/slideshow/source/inc/slideshowexceptions.hxx"))
         return true;
    // SFX_DECL_VIEWFACTORY macro
    if (loplugin::isSamePathname(fn, SRCDIR "/starmath/inc/view.hxx"))
         return true;
    // debugging
    if (fqn == "BrowseBox::DoShowCursor" || fqn == "BrowseBox::DoHideCursor")
         return true;
    // if I change this one, it then overrides a superclass virtual method
    if (fqn == "GalleryBrowser2::KeyInput")
         return true;
    // takes pointer to function
    if (fqn == "cmis::AuthProvider::onedriveAuthCodeFallback" || fqn == "cmis::AuthProvider::gdriveAuthCodeFallback")
         return true;
    if (fqn == "ooo_mount_operation_ask_password")
         return true;
    // TODO tricky to remove because of default params
    if (fqn == "xmloff::OAttribute2Property::addBooleanProperty")
         return true;
    // taking pointer to function
    if (fqn == "sw::DocumentContentOperationsManager::DeleteAndJoinWithRedlineImpl"
        || fqn == "sw::DocumentContentOperationsManager::DeleteRangeImpl"
        || fqn == "SwTableFormula::GetFormulaBoxes"
        || fqn == "SwFEShell::Drag"
        || fqn == "GetASCWriter" || fqn == "GetHTMLWriter" || fqn == "GetXMLWriter"
        || fqn == "SwWrtShell::UpdateLayoutFrame" || fqn == "SwWrtShell::DefaultDrag"
        || fqn == "SwWrtShell::DefaultEndDrag"
        || startswith(fqn, "SwWW8ImplReader::Read_"))
         return true;
    // WIN32 only
    if (fqn == "SwFntObj::GuessLeading")
         return true;
    // SFX_DECL_CHILDWINDOW_WITHID macro
    if (fqn == "SwSpellDialogChildWindow::SwSpellDialogChildWindow"
        || fqn == "SwFieldDlgWrapper::SwFieldDlgWrapper"
        || fqn == "SwInputChild::SwInputChild")
         return true;
    // SFX_DECL_VIEWFACTORY macro
    if (fqn == "SwSrcView::SwSrcView")
         return true;
    // Serves to disambiguate two very similar methods
    if (fqn == "MSWordStyles::BuildGetSlot")
         return true;
    // TODO there are just too many default params to make this worth fixing right now
    if (fqn == "ScDocument::CopyMultiRangeFromClip")
        return true;
    // TODO looks like this needs fixing?
    if (fqn == "ScTable::ExtendPrintArea")
        return true;
    // there is a FIXME in the code
    if (fqn == "ScRangeUtil::IsAbsTabArea")
        return true;
    // SFX_DECL_CHILDWINDOW_WITHID
    if (fqn == "ScInputWindowWrapper::ScInputWindowWrapper"
        || fqn == "sc::SearchResultsDlgWrapper::SearchResultsDlgWrapper")
        return true;
    // ExecMethod in .sdi file
    if (fqn == "ScChartShell::ExecuteExportAsGraphic")
        return true;
    // bool marker parameter
    if (fqn == "SvxIconReplacementDialog::SvxIconReplacementDialog")
        return true;
    // used as pointer to fn
    if (endswith(fqn, "_createInstance"))
        return true;
    // callback
    if (startswith(fqn, "SbRtl_"))
        return true;
    // takes pointer to fn
    if (fqn == "migration::BasicMigration_create" || fqn == "migration::WordbookMigration_create"
        || fqn == "comp_CBlankNode::_create" || fqn == "comp_CURI::_create"
        || fqn == "comp_CLiteral::_create" || fqn == "CDocumentBuilder::_getInstance"
        || fqn == "DOM::CDocumentBuilder::_getInstance"
        || fqn == "xml_security::serial_number_adapter::create"
        || fqn == "desktop::splash::create" || fqn == "ScannerManager_CreateInstance"
        || fqn == "formula::FormulaOpCodeMapperObj::create"
        || fqn == "(anonymous namespace)::createInstance"
        || fqn == "x_error_handler"
        || fqn == "warning_func"
        || fqn == "error_func"
        || fqn == "ScaDateAddIn_CreateInstance"
        || fqn == "ScaPricingAddIn_CreateInstance"
        || fqn == "(anonymous namespace)::PDFSigningPKCS7PasswordCallback"
        || fqn == "ContextMenuEventLink"
        || fqn == "DelayedCloseEventLink"
        || fqn == "GDIMetaFile::ImplColMonoFnc"
        || fqn == "vcl::getGlyph0"
        || fqn == "vcl::getGlyph6"
        || fqn == "vcl::getGlyph12"
        || fqn == "setPasswordCallback"
        || fqn == "VCLExceptionSignal_impl"
        || fqn == "getFontTable"
        || fqn == "textconversiondlgs::ChineseTranslation_UnoDialog::create"
        || fqn == "pcr::DefaultHelpProvider::Create"
        || fqn == "pcr::DefaultFormComponentInspectorModel::Create"
        || fqn == "pcr::ObjectInspectorModel::Create"
        || fqn == "GraphicExportFilter::GraphicExportFilter"
        || fqn == "CertificateContainer::CertificateContainer"
        || startswith(fqn, "ParseCSS1_")
        )
         return true;
    // TODO
    if (fqn == "FontSubsetInfo::CreateFontSubsetFromType1")
         return true;
    // used in template magic
    if (fqn == "MtfRenderer::MtfRenderer" || fqn == "shell::sessioninstall::SyncDbusSessionHelper::SyncDbusSessionHelper"
        || fqn == "dp_gui::LicenseDialog::LicenseDialog"
        || fqn == "(anonymous namespace)::OGLTransitionFactoryImpl::OGLTransitionFactoryImpl")
         return true;
    // FIXME
    if (fqn == "GtkSalDisplay::filterGdkEvent" || fqn == "SvXMLEmbeddedObjectHelper::ImplReadObject"
        || fqn == "chart::CachedDataSequence::CachedDataSequence")
         return true;
    // used via macro
    if (fqn == "framework::MediaTypeDetectionHelper::MediaTypeDetectionHelper"
        || fqn == "framework::UriAbbreviation::UriAbbreviation"
        || fqn == "framework::DispatchDisabler::DispatchDisabler"
        || fqn == "framework::DispatchRecorderSupplier::DispatchRecorderSupplier")
         return true;
    // TODO Armin Le Grand is still working on this
    if (fqn == "svx::frame::CreateDiagFrameBorderPrimitives"
        || fqn == "svx::frame::CreateBorderPrimitives")
         return true;
    // marked with a TODO
    if (fqn == "pcr::FormLinkDialog::getExistingRelation"
        || fqn == "ooo::vba::DebugHelper::basicexception"
        || fqn == "ScPrintFunc::DrawToDev")
         return true;
    // macros at work
    if (fqn == "msfilter::lcl_PrintDigest")
         return true;
    // TODO something wrong here, the method that calls this (Normal::GenSlidingWindowFunction) cannot be correct
    if (fqn == "sc::opencl::OpBase::Gen")
         return true;
    // Can't change this without conflicting with another constructor with the same signature
    if (fqn == "XclExpSupbook::XclExpSupbook")
         return true;
    // ignore the LINK macros from include/tools/link.hxx
    if (decl->getLocation().isMacroID())
        return true;
    // debug code in sw/
    if (fqn == "lcl_dbg_out")
         return true;

    for( auto it = decl->param_begin(); it != decl->param_end(); ++it) {
        auto param = *it;
        if (param->hasAttr<UnusedAttr>())
            continue;
        if (!param->getName().empty())
            continue;
        // ignore params which are enum types with only a single enumerator, these are marker/tag types
        auto paramType = param->getType();
        if (paramType->isEnumeralType()) {
            auto enumType = paramType->getAs<EnumType>();
            int cnt = std::distance(enumType->getDecl()->enumerator_begin(), enumType->getDecl()->enumerator_end());
            if (cnt == 1)
                continue;
        }
        // ignore params which are a reference to a struct which has no fields.
        // These are either
        //  (a) marker/tag types
        //  (b) selective "friend" access
        if (paramType->isReferenceType()) {
            auto referenceType = paramType->getAs<ReferenceType>();
            if (referenceType->getPointeeType()->isRecordType()) {
                auto recordType = referenceType->getPointeeType()->getAs<RecordType>();
                if (noFieldsInRecord(recordType) == 0)
                    continue;
            }
        }
        else if (paramType->isRecordType()) {
            if (noFieldsInRecord(paramType->getAs<RecordType>()) == 0)
                continue;
        }
        report( DiagnosticsEngine::Warning,
                "unused param %0 in %1", compat::getBeginLoc(param))
                << param->getSourceRange()
                << param->getName()
                << fqn;
        if (canon != decl)
        {
            unsigned idx = param->getFunctionScopeIndex();
            const ParmVarDecl* pOther = canon->getParamDecl(idx);
            report( DiagnosticsEngine::Note, "declaration is here",
                    compat::getBeginLoc(pOther))
                    << pOther->getSourceRange();
        }
    }
    return true;
}

loplugin::Plugin::Registration<CheckUnusedParams> X("checkunusedparams", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
