/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "compat.hxx"
#include <iostream>

/*
  Look for member functions that merely return a compile-time constant, or they are empty, and can thus
  be either removed, or converted into a constant.

  This mostly tends to happen as a side-effect of other cleanups.
*/
namespace {

class ConstantFunction:
    public RecursiveASTVisitor<ConstantFunction>, public loplugin::Plugin
{
    StringRef getFilename(SourceLocation loc);
public:
    explicit ConstantFunction(InstantiationData const & data): Plugin(data) {}

    void run() override
    {
        // these files crash clang-3.5 somewhere in the isEvaluatable/EvaluateAsXXX stuff
        FileID mainFileID = compiler.getSourceManager().getMainFileID();
        if (strstr(compiler.getSourceManager().getFileEntryForID(mainFileID)->getDir()->getName(), "sc/source/core/data") != 0) {
            return;
        }
        if (strstr(compiler.getSourceManager().getFileEntryForID(mainFileID)->getDir()->getName(), "sc/source/ui/app") != 0) {
            return;
        }
        if (strstr(compiler.getSourceManager().getFileEntryForID(mainFileID)->getDir()->getName(), "sc/qa/unit") != 0) {
            return;
        }
        if (strstr(compiler.getSourceManager().getFileEntryForID(mainFileID)->getName(), "docuno.cxx") != 0) {
            return;
        }
        if (strstr(compiler.getSourceManager().getFileEntryForID(mainFileID)->getName(), "viewdata.cxx") != 0) {
            return;
        }
        if (strstr(compiler.getSourceManager().getFileEntryForID(mainFileID)->getName(), "calcoptionsdlg.cxx") != 0) {
            return;
        }
        if (strstr(compiler.getSourceManager().getFileEntryForID(mainFileID)->getDir()->getName(), "sc/source/core/opencl") != 0) {
            return;
        }
        if (strstr(compiler.getSourceManager().getFileEntryForID(mainFileID)->getDir()->getName(), "sc/source/core/tool") != 0) {
            return;
        }

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFunctionDecl(const FunctionDecl *);
};

StringRef ConstantFunction::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name;
}

static bool startsWith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool ConstantFunction::VisitFunctionDecl(const FunctionDecl * pFunctionDecl) {
    if (ignoreLocation(pFunctionDecl)) {
        return true;
    }
    if (!pFunctionDecl->hasBody()) {
        return true;
    }
    // stuff declared extern-C is almost always used as a some kind of callback
    if (pFunctionDecl->isExternC()) {
        return true;
    }

    StringRef aFileName = getFilename(pFunctionDecl->getLocStart());

    // various tests in here are empty stubs under Linux
    if (aFileName.startswith(SRCDIR "/sal/qa/")) {
        return true;
    }
    // lots of empty stuff here where it looks like someone is still going to "fill in the blanks"
    if (aFileName.startswith(SRCDIR "/basegfx/test/")) {
        return true;
    }
    // some stuff is just stubs under Linux, although this appears to be a SOLARIS-specific hack, so it
    // should probably not even be compiling under Linux.
    if (aFileName == SRCDIR "/setup_native/scripts/source/getuid.c") {
        return true;
    }
    // bridges has some weird stuff in it....
    if (aFileName.startswith(SRCDIR "/bridges/")) {
        return true;
    }
    // dummy implementation of DDE, since it is only active on Windows
    if (aFileName == SRCDIR "/svl/unx/source/svdde/ddedummy.cxx"
        || aFileName == SRCDIR "/include/svl/svdde.hxx") {
        return true;
    }
    // fancy templates at work here
    if (aFileName == SRCDIR "/vcl/source/gdi/bmpfast.cxx") {
        return true;
    }
    // bunch of stuff used as callbacks here
    if (aFileName == SRCDIR "/vcl/generic/glyphs/gcach_layout.cxx") {
        return true;
    }
    // salplug runtime-loading mechanism at work
    if (getFilename(pFunctionDecl->getCanonicalDecl()->getLocStart()) == SRCDIR "/vcl/inc/salinst.hxx") {
        return true;
    }
    // lots of callbacks here
    if (aFileName == SRCDIR "/extensions/source/plugin/unx/npnapi.cxx") {
        return true;
    }
    // template magic
    if (aFileName == SRCDIR "/filter/source/svg/svgreader.cxx") {
        return true;
    }
    // vcl/unx/gtk3 re-using vcl/unx/gtk:
    if (aFileName.find("/../../gtk/") != std::string::npos) {
        return true;
    }
    // used by code generated by python
    if (getFilename(pFunctionDecl->getCanonicalDecl()->getLocStart()) == SRCDIR "/writerfilter/source/ooxml/OOXMLFastContextHandler.hxx") {
        return true;
    }


    const CXXMethodDecl *pCXXMethodDecl = dyn_cast<CXXMethodDecl>(pFunctionDecl);
    if (pCXXMethodDecl) {
        if (pCXXMethodDecl->isVirtual()) {
            return true;
        }
        // static with inline body will be optimised at compile-time to a constant anyway
        if (pCXXMethodDecl->isStatic() && (pCXXMethodDecl->hasInlineBody() || pCXXMethodDecl->isInlineSpecified())) {
            return true;
        }
        // this catches some stuff in templates
        if (pFunctionDecl->hasAttr<OverrideAttr>()) {
            return true;
        }
    }
    // a free function with an inline body will be optimised at compile-time to a constant anyway
    if (!pCXXMethodDecl && pFunctionDecl->isInlineSpecified()) {
        return true;
    }
    if (isa<CXXConstructorDecl>(pFunctionDecl) || isa<CXXDestructorDecl>(pFunctionDecl) || isa<CXXConversionDecl>(pFunctionDecl)) {
        return true;
    }
    SourceLocation canonicalLoc = pFunctionDecl->getCanonicalDecl()->getLocStart();
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(canonicalLoc))) {
        return true;
    }

    switch (pFunctionDecl->getOverloadedOperator()) {
    case OO_Delete:
    case OO_EqualEqual:
    case OO_Call:
        return true;
    default:
        break;
    }

    std::string aFunctionName = pFunctionDecl->getQualifiedNameAsString();

    // something to do with dynamic loading in sal/textenc/textenc.cxx
    if (aFunctionName == "thisModule") {
        return true;
    }
    // an empty stub under certain conditions, sal/osl/unx/thread.cxx
    if (aFunctionName == "osl_thread_priority_init_Impl") {
        return true;
    }
    // a pointer to this function is taken and passed to an underlying API, shell/source/unix/sysshell/recently_used_file_handler.cxx
    if (aFunctionName == "(anonymous namespace)::recently_used_item::set_nothing") {
        return true;
    }
    // a pointer to this function is taken and passed to an underlying API, cppu/source/uno/lbenv.cxx
    if (aFunctionName == "defenv_dispose") {
        return true;
    }
    // a pointer to this function is taken and passed to an underlying API, cppuhelper/source/exc_thrower.cxx
    if (aFunctionName == "ExceptionThrower_acquire_release_nop") {
        return true;
    }
    // differetnt hook function is called on different platforms, /vcl/source/app/svmainhook.cxx
    if (aFunctionName == "ImplSVMainHook") {
        return true;
    }
    // used as a callback, /vcl/source/filter/jpeg/JpegReader.cxx
    if (aFunctionName == "term_source") {
        return true;
    }
    // only valid for windows, extensions/source/update/check/updatecheck.cxx
    if (aFunctionName == "(anonymous namespace)::UpdateCheckThread::hasInternetConnection") {
        return true;
    }
    // used as callback, extensions/source/plugin/unx/npwrap.cxx
    if (aFunctionName == "plugin_x_error_handler" || aFunctionName == "noClosure") {
        return true;
    }
    // used as callback, sax/source/expatwrap/sax_expat.cxx
    if (aFunctionName == "(anonymous namespace)::SaxExpatParser_Impl::callbackUnknownEncoding") {
        return true;
    }
    // used as callback, i18npool/source/textconversion/textconversion.cxx
    if (aFunctionName == "com::sun::star::i18n::nullFunc") {
        return true;
    }
    // used as callback, xmloff/source/text/txtparae.cxx
    if (aFunctionName == "(anonymous namespace)::lcl_TextContentsUnfiltered") {
        return true;
    }
    // template magic, include/canvas/verifyinput.hxx
    if (aFunctionName == "canvas::tools::verifyInput") {
        return true;
    }
    // template magic, cppcanvas/source/mtfrenderer/implrenderer.cxx
    if (aFunctionName == "cppcanvas::internal::(anonymous namespace)::AreaQuery::result") {
        return true;
    }
    // callback, drawinglayer/source/dumper/XShapeDumper.
    if (aFunctionName == "(anonymous namespace)::closeCallback") {
        return true;
    }
    // callback, basic/source/runtime/runtime.cxx
    if (aFunctionName == "SbiRuntime::StepNOP") {
        return true;
    }
    // DLL stuff, only used on windows, basic/source/runtime/dllmgr.hxx
    if (aFunctionName == "SbiDllMgr::FreeDll") {
        return true;
    }
    // only used on Windows, basic/source/sbx/sbxdec.cxx
    if (aFunctionName == "SbxDecimal::neg" || aFunctionName == "SbxDecimal::isZero") {
        return true;
    }
    // used as a callback, include/sfx2/shell.hxx
    if (aFunctionName == "SfxShell::EmptyExecStub" || aFunctionName == "SfxShell::EmptyStateStub"
        || aFunctionName == "SfxShell::VerbState") {
        return true;
    }
    // SFX_IMPL_POS_CHILDWINDOW_WITHID macro
    if (aFunctionName.find("GetChildWindowId") != std::string::npos) {
        return true;
    }
    // SFX_IMPL_SUPERCLASS_INTERFACE macro
    if (aFunctionName.find("InitInterface_Impl") != std::string::npos) {
        return true;
    }
    // callback, vcl/unx/generic/app/sm.cxx
    if (aFunctionName == "IgnoreIceIOErrors" || aFunctionName == "IgnoreIceErrors") {
        return true;
    }
    // callback, vcl/unx/gtk/a11y/atkcomponent.cxx
    if (aFunctionName == "component_wrapper_get_mdi_zorder") {
        return true;
    }
    // callback, vcl/unx/gtk/a11y/atkaction.cxx
    if (aFunctionName == "action_wrapper_set_description") {
        return true;
    }
    // callback, vcl/unx/gtk/a11y/atkutil.cxx
    if (aFunctionName == "ooo_atk_util_get_toolkit_version" || aFunctionName == "ooo_atk_util_get_toolkit_name") {
        return true;
    }
    // callback, vcl/unx/gtk/a11y/atktextattributes.cxx
    if (aFunctionName == "InvalidValue") {
        return true;
    }
    // callback, vcl/unx/gtk/a11y/atktable.cxx
    if (aFunctionName == "table_wrapper_set_summary" || aFunctionName == "table_wrapper_set_row_header"
        || aFunctionName == "table_wrapper_set_row_description"
        || aFunctionName == "table_wrapper_set_column_header"
        || aFunctionName == "table_wrapper_set_column_description"
        || aFunctionName == "table_wrapper_set_caption") {
        return true;
    }
    // callbacks, vcl/unx/gtk/window/gtksalframe.cxx
    if (startsWith(aFunctionName, "GtkSalFrame::IMHandler::signal")) {
        return true;
    }
    // callbacks, vcl/unx/gtk/window/glomenu.cxx
    if (startsWith(aFunctionName, "g_lo_menu_is_mutable")) {
        return true;
    }
    // only contains code for certain versions of GTK, /vcl/unx/gtk/window/gtksalframe.cx
    if (aFunctionName == "GtkSalFrame::AllocateFrame") {
        return true;
    }
    // only valid for Windows, embeddedobj/source/msole/olemisc.cxx
    if (aFunctionName == "OleEmbeddedObject::GetRidOfComponent") {
        return true;
    }
    // callback, svx/source/accessibility/ShapeTypeHandler.cxx
    if (aFunctionName == "accessibility::CreateEmptyShapeReference") {
        return true;
    }
    //  chart2/source/view/main/AbstractShapeFactory.cxx
    if (aFunctionName == "chart::(anonymous namespace)::thisModule") {
        return true;
    }
    //  chart2/source/tools/InternalData.cxx
    if (aFunctionName == "chart::InternalData::dump") {
        return true;
    }
    //  hwpfilter/
    if (aFunctionName == "debug" || aFunctionName == "token_debug") {
        return true;
    }
    //  callback, sdext/source/presenter/PresenterFrameworkObserver.cxx
    if (aFunctionName == "sdext::presenter::PresenterFrameworkObserver::True") {
        return true;
    }
    //  hidden behind the ENABLE_PANE_RESIZING macro
    if (aFunctionName == "sdext::presenter::PresenterWindowManager::UpdateWindowList") {
        return true;
    }
    // callback, sw/source/core/doc/tblrwcl.cxx
    if (aFunctionName == "lcl_DelOtherBox") {
        return true;
    }
    // callback, sw/source/filter/ww8/ww8par.cxx
    if (aFunctionName == "SwWW8ImplReader::Read_Majority") {
        return true;
    }
    // callback, sw/source/filter/ww8/ww8par5.cxx
    if (aFunctionName == "SwWW8ImplReader::Read_F_Shape") {
        return true;
    }
    // called from SDI file, I don't know what that stuff is about, sd/source/ui/slidesorter/shell/SlideSorterViewShell.cx
    if (aFunctionName == "sd::slidesorter::SlideSorterViewShell::ExecStatusBar"
        || aFunctionName == "sd::OutlineViewShell::ExecStatusBar") {
        return true;
    }
    // only used in debug mode, sd/source/filter/ppt/pptinanimations.cxx
    if (startsWith(aFunctionName, "ppt::AnimationImporter::dump")) {
        return true;
    }
    // only used in ENABLE_SDREMOTE_BLUETOOTH mode, sd/source/ui/dlg/tpoption.cx
    if (aFunctionName == "SdTpOptionsMisc::SetImpressMode") {
        return true;
    }
    // template magic, sc/source/ui/docshell/datastream.cxx
    if (startsWith(aFunctionName, "sc::(anonymous namespace)::CSVHandler::")) {
        return true;
    }
    // called from SDI file, I don't know what that stuff is about, sc/source/ui/docshell/docsh7.cxx
    if (aFunctionName == "ScDocShell::GetDrawObjState") {
        return true;
    }
    // called from SDI file, I don't know what that stuff is about, sc/source/ui/view/cellsh4.cxx
    if (aFunctionName == "ScCellShell::GetStateCursor") {
        return true;
    }
    // called from SDI file, I don't know what that stuff is about, sc/source/ui/view/tabvwshh.cxx
    if (aFunctionName == "ScTabViewShell::ExecuteSbx" || aFunctionName == "ScTabViewShell::GetSbxState") {
        return true;
    }
    // template magic, sc/source/filter/excel/xepivot.cxx
    if (aFunctionName == "XclExpPivotCache::SaveXml") {
        return true;
    }
    // template magic, sc/source/filter/html/htmlpars.cxx
    if (startsWith(aFunctionName, "(anonymous namespace)::CSSHandler::")) {
        return true;
    }
    // callbacks, sc/source/filter/oox/formulaparser.cxx
    if (startsWith(aFunctionName, "oox::xls::BiffFormulaParserImpl::import")) {
        return true;
    }
    // template magic, sc/qa/unit/helper/csv_handler.hxx
    if (startsWith(aFunctionName, "csv_handler::") || startsWith(aFunctionName, "conditional_format_handler::")) {
        return true;
    }
    // template magic, slideshow/source/inc/listenercontainer.hxx
    if (startsWith(aFunctionName, "slideshow::internal::EmptyBase::EmptyClearableGuard::")) {
        return true;
    }
    // callback, scripting/source/vbaevents/eventhelper.cxx
    if (aFunctionName == "ApproveAll") {
        return true;
    }
    // only on WNT, basic/qa/cppunit/test_vba.cx
    if (aFunctionName == "(anonymous namespace)::VBATest::testMiscOLEStuff") {
        return true;
    }
    // GtkSalFrame::TriggerPaintEvent() is only compiled under certain versions of GTK
    if (aFunctionName == "GtkSalFrame::TriggerPaintEvent") {
        return true;
    }
    if (aFunctionName == "SwVectorModifyBase::dumpAsXml") {
        return true;
    }
    // vcl/unx/gtk3 re-using vcl/unx/gtk:
    if (aFunctionName == "DeInitAtkBridge"
        || aFunctionName == "GtkData::initNWF"
        || aFunctionName == "GtkSalFrame::EnsureAppMenuWatch"
        || aFunctionName == "InitAtkBridge")
    {
        return true;
    }
    // LINK callback which supplies a return value which means something
    if (aFunctionName == "SfxVirtualMenu::Highlight" || aFunctionName == "framework::MenuManager::Highlight"
        || aFunctionName == "framework::MenuBarManager::Highlight") {
        return true;
    }

    // can't mess with the TYPEINIT macros in include/tools/rtti.hxx or the LINK macros in include/tools/link.hxx
    std::string aImmediateMacro = "";
    if (compat::isMacroBodyExpansion(compiler, pFunctionDecl->getLocStart()) ) {
        StringRef name { Lexer::getImmediateMacroName(
                pFunctionDecl->getLocStart(), compiler.getSourceManager(), compiler.getLangOpts()) };
        aImmediateMacro = name;
        if (name == "TYPEINIT_FACTORY" || name == "TYPEINFO" || name == "TYPEINFO_OVERRIDE"
            || name.startswith("IMPL_LINK_") )
        {
            return true;
        }
    }

    const CompoundStmt *pCompoundStmt = dyn_cast<CompoundStmt>(pFunctionDecl->getBody());
    bool aEmptyBody = false;
    if (pCompoundStmt) {
        if (pCompoundStmt->size() > 1) {
            return true;
        }
        if (pCompoundStmt->size() > 0) {
            const ReturnStmt *pReturnStmt = dyn_cast<ReturnStmt>(*pCompoundStmt->body_begin());
            if (!pReturnStmt) {
                return true;
            }
            if (pReturnStmt->getRetValue() != nullptr) {
                // && !pReturnStmt->getRetValue()->isEvaluatable(compiler.getASTContext())) {
                bool aBoolResult;
                llvm::APSInt aIntResult;
                if (pReturnStmt->getRetValue()->isTypeDependent()
                    || (!pReturnStmt->getRetValue()->EvaluateAsBooleanCondition(aBoolResult, compiler.getASTContext())
                        && !pReturnStmt->getRetValue()->EvaluateAsInt(aIntResult, compiler.getASTContext())))
                {
                    return true;
                }
            }
        } else {
            aEmptyBody = true;
        }
    }

    std::string aMessage = "this ";
    aMessage += pCXXMethodDecl ? "method" : "function";
    if (aEmptyBody) {
        aMessage += " is empty and should be removed, " + aFunctionName;
    } else {
        aMessage += " returns a constant value and should be converted to a constant "
                    "or to static inline, " + aFunctionName + ", " + aImmediateMacro;
    }
    report(
        DiagnosticsEngine::Warning,
        aMessage,
        pFunctionDecl->getLocStart())
      << pFunctionDecl->getSourceRange()
      << pFunctionDecl->getCanonicalDecl()->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<ConstantFunction> X("constantfunction");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
