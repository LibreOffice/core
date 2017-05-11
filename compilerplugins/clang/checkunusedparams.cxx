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

#include "plugin.hxx"

/**
Find parameters that have no name, i.e. they are unused and we're worked around the "unused parameter" warning.

Most of these can be removed.

TODO look for places where we are working around the warning by doing
    (void) param1;
 */
namespace {

class CheckUnusedParams: public RecursiveASTVisitor<CheckUnusedParams>, public loplugin::Plugin {
public:
    explicit CheckUnusedParams(InstantiationData const & data): Plugin(data) {}
    void run() override;
    bool VisitFunctionDecl(FunctionDecl const * decl);
    bool VisitDeclRefExpr(DeclRefExpr const *);
private:
    std::set<FunctionDecl const *> m_addressOfSet;
    enum class PluginPhase { FindAddressOf, Warning };
    PluginPhase m_phase;
};

void CheckUnusedParams::run()
{
    StringRef fn( compiler.getSourceManager().getFileEntryForID(
                      compiler.getSourceManager().getMainFileID())->getName() );
    if (fn.startswith(SRCDIR "/sal/"))
         return;
    // Taking pointer to function
    if (fn == SRCDIR "/l10ntools/source/xmlparse.cxx")
         return;
    // macro magic which declares something needed by an external library
    if (fn == SRCDIR "/svl/source/misc/gridprinter.cxx")
         return;

    // valid test/qa code
    if (fn.startswith(SRCDIR "/compilerplugins/clang/test/"))
         return;
    if (fn == SRCDIR "/cppu/qa/test_reference.cxx")
         return;

    // leave this alone for now
    if (fn.startswith(SRCDIR "/libreofficekit/"))
         return;

    m_phase = PluginPhase::FindAddressOf;
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    m_phase = PluginPhase::Warning;
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
}

bool CheckUnusedParams::VisitDeclRefExpr(DeclRefExpr const * declRef) {
    if (m_phase != PluginPhase::FindAddressOf)
        return true;
    if (ignoreLocation(declRef))
        return true;
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(declRef->getLocStart())))
        return true;
    auto functionDecl = dyn_cast<FunctionDecl>(declRef->getDecl());
    if (!functionDecl)
        return true;
    m_addressOfSet.insert(functionDecl);
    return true;
}


static int noFieldsInRecord(RecordType const * recordType) {
    return std::distance(recordType->getDecl()->field_begin(), recordType->getDecl()->field_end());
}
static bool startswith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool CheckUnusedParams::VisitFunctionDecl(FunctionDecl const * decl) {
    if (m_phase != PluginPhase::Warning)
        return true;
    if (m_addressOfSet.find(decl) != m_addressOfSet.end())
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
    StringRef fn = compiler.getSourceManager().getFilename(compiler.getSourceManager().getSpellingLoc(canon->getLocStart()));
    // Some backwards compat magic.
    // TODO Can probably be removed, but need to do some checking
    if (fn == SRCDIR "/include/sax/fshelper.hxx")
         return true;
    // Platform-specific code
    if (fn == SRCDIR "/include/svl/svdde.hxx")
         return true;
    if (fn == SRCDIR "/include/vcl/svmain.hxx")
         return true;
    // passing pointer to function
    if (fn == SRCDIR "/include/vcl/bitmapaccess.hxx")
         return true;
    if (fn == SRCDIR "/vcl/inc/unx/gtk/gtkobject.hxx")
         return true;
    if (fn == SRCDIR "/vcl/inc/unx/gtk/gtksalframe.hxx")
         return true;
    if (fn == SRCDIR "/vcl/inc/unx/gtk/gtkframe.hxx")
         return true;
    if (fn == SRCDIR "/vcl/unx/gtk/fpicker/SalGtkFilePicker.hxx")
         return true;
    if (fn == SRCDIR "/extensions/source/propctrlr/propertyeditor.hxx")
         return true;
    if (fn == SRCDIR "/forms/source/solar/inc/navtoolbar.hxx")
         return true;
    if (fn == SRCDIR "/hwpfilter/source/grammar.cxx")
         return true;
    if (fn == SRCDIR "/hwpfilter/source/lexer.cxx")
         return true;
    // marked with a TODO/FIXME
    if (fn == SRCDIR "/vcl/inc/sallayout.hxx")
         return true;
    if (fn == SRCDIR "/accessibility/inc/standard/vclxaccessiblelist.hxx")
         return true;
    // these are "extern C" but clang doesn't seem to report that accurately
    if (fn == SRCDIR "/sax/source/fastparser/fastparser.cxx")
         return true;
    // these all follow the same pattern, seems a pity to break that
    if (fn == SRCDIR "/include/vcl/graphicfilter.hxx")
         return true;
    // looks like work in progress
    if (fn == SRCDIR "/vcl/source/filter/ipdf/pdfdocument.cxx")
         return true;
    // macro magic
    if (fn == SRCDIR "/basctl/source/inc/basidesh.hxx")
         return true;
    // template magic
    if (fn.startswith(SRCDIR "/canvas/"))
         return true;
    if (fn.startswith(SRCDIR "/include/canvas/"))
         return true;
    if (fn == SRCDIR "/include/comphelper/unwrapargs.hxx")
         return true;
    // this looks like vaguely useful code (ParseError) that I'm loathe to remove
    if (fn == SRCDIR "/connectivity/source/inc/RowFunctionParser.hxx")
         return true;
    if (fn == SRCDIR "/include/svx/EnhancedCustomShapeFunctionParser.hxx")
         return true;
    // TODO marker parameter in constructor, should probably be using an enum
    if (fn == SRCDIR "/framework/inc/uielement/uicommanddescription.hxx")
         return true;
    if (fn == SRCDIR "/sd/source/ui/inc/SlideTransitionPane.hxx")
         return true;
    if (fn == SRCDIR "/sd/source/ui/animations/CustomAnimationPane.hxx")
         return true;
    if (fn == SRCDIR "/sd/source/ui/table/TableDesignPane.hxx")
         return true;
    // debug stuff
    if (fn == SRCDIR "/sc/source/core/data/column2.cxx")
         return true;
    // weird stuff
    if (fn == SRCDIR "/scaddins/source/analysis/analysishelper.hxx")
         return true;
    // SFX_DECL_CHILDWINDOWCONTEXT macro stuff
    if (fn == SRCDIR "/sd/source/ui/inc/NavigatorChildWindow.hxx")
         return true;
    // TODO, need to remove this from the .sdi file too
    if (fn == SRCDIR "/sd/source/ui/inc/SlideSorterViewShell.hxx")
         return true;
    if (fn == SRCDIR "/sd/source/ui/inc/OutlineViewShell.hxx")
         return true;
    // SFX_DECL_INTERFACE macro stuff
    if (fn == SRCDIR "/sd/source/ui/inc/ViewShellBase.hxx")
         return true;
    // debug stuff
    if (fn == SRCDIR "/sd/source/filter/ppt/pptinanimations.hxx")
         return true;
    // takes pointer to fn
    if (fn == SRCDIR "/include/sfx2/shell.hxx")
         return true;
    // TODO, need to remove this from the .sdi file too
    if (fqn == "SfxObjectShell::StateView_Impl")
         return true;
    // SFX_DECL_CHILDWINDOW_WITHID macro
    if (fn == SRCDIR "/include/sfx2/infobar.hxx")
         return true;
    // this looks like vaguely useful code (ParseError) that I'm loathe to remove
    if (fn == SRCDIR "/slideshow/source/inc/slideshowexceptions.hxx")
         return true;
    // SFX_DECL_VIEWFACTORY macro
    if (fn == SRCDIR "/starmath/inc/view.hxx")
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

    // ignore the LINK macros from include/tools/link.hxx
    if (decl->getLocation().isMacroID())
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
                "unused param %0 in %1", decl->getLocation())
                << decl->getSourceRange()
                << param->getName()
                << fqn;
        if (canon != decl)
            report( DiagnosticsEngine::Note, "declaration is here",
                    canon->getLocation())
                    << canon->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<CheckUnusedParams> X("checkunusedparams", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
