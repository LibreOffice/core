/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "clang/AST/Attr.h"

#include "config_clang.h"

#include "check.hxx"
#include "plugin.hxx"

/*
  Look for member functions that can be static
*/
namespace {

class StaticMethods:
    public loplugin::FilteringPlugin<StaticMethods>
{
private:
    bool bVisitedThis;
public:
    explicit StaticMethods(loplugin::InstantiationData const & data): FilteringPlugin(data), bVisitedThis(false) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool TraverseCXXMethodDecl(const CXXMethodDecl * decl);

    bool VisitCXXThisExpr(const CXXThisExpr *) { bVisitedThis = true; return true; }
    // these two indicate that we hit something that makes our analysis unreliable
    bool VisitUnresolvedMemberExpr(const UnresolvedMemberExpr *) { bVisitedThis = true; return true; }
    bool VisitCXXDependentScopeMemberExpr(const CXXDependentScopeMemberExpr *) { bVisitedThis = true; return true; }
private:
    StringRef getFilename(SourceLocation loc);
};

bool BaseCheckNotTestFixtureSubclass(const CXXRecordDecl *BaseDefinition) {
    if (loplugin::TypeCheck(BaseDefinition).Class("TestFixture").Namespace("CppUnit").GlobalNamespace()) {
        return false;
    }
    return true;
}

bool isDerivedFromTestFixture(const CXXRecordDecl *decl) {
    if (!decl->hasDefinition())
        return false;
    if (// not sure what hasAnyDependentBases() does,
        // but it avoids classes we don't want, e.g. WeakAggComponentImplHelper1
        !decl->hasAnyDependentBases() &&
        !decl->forallBases(BaseCheckNotTestFixtureSubclass)) {
        return true;
    }
    return false;
}

StringRef StaticMethods::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    return getFilenameOfLocation(spellingLocation);
}

bool startsWith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool StaticMethods::TraverseCXXMethodDecl(const CXXMethodDecl * pCXXMethodDecl) {
    if (ignoreLocation(pCXXMethodDecl)) {
        return true;
    }
    if (!pCXXMethodDecl->isInstance() || pCXXMethodDecl->isVirtual() || !pCXXMethodDecl->doesThisDeclarationHaveABody() || pCXXMethodDecl->isLateTemplateParsed()) {
        return true;
    }
    if (pCXXMethodDecl->getOverloadedOperator() != OverloadedOperatorKind::OO_None || pCXXMethodDecl->hasAttr<OverrideAttr>()) {
        return true;
    }
    if (isa<CXXConstructorDecl>(pCXXMethodDecl) || isa<CXXDestructorDecl>(pCXXMethodDecl) || isa<CXXConversionDecl>(pCXXMethodDecl)) {
        return true;
    }
    if (isInUnoIncludeFile(pCXXMethodDecl)) {
        return true;
    }
    if (pCXXMethodDecl->getTemplateSpecializationKind() == TSK_ExplicitSpecialization)
        return true;

    // the CppUnit stuff uses macros and methods that can't be changed
    if (isDerivedFromTestFixture(pCXXMethodDecl->getParent())) {
        return true;
    }
    // don't mess with the backwards compatibility stuff
    if (loplugin::isSamePathname(getFilename(pCXXMethodDecl->getBeginLoc()), SRCDIR "/cppuhelper/source/compat.cxx")) {
        return true;
    }
    // the DDE has a dummy implementation on Linux and a real one on Windows
    auto aFilename = getFilename(pCXXMethodDecl->getCanonicalDecl()->getBeginLoc());
    if (loplugin::isSamePathname(aFilename, SRCDIR "/include/svl/svdde.hxx")) {
        return true;
    }
    auto cdc = loplugin::DeclCheck(pCXXMethodDecl->getParent());
    // special case having something to do with static initialisation
    // sal/osl/all/utility.cxx
    if (cdc.Class("OGlobalTimer").Namespace("osl").GlobalNamespace()) {
        return true;
    }
    // leave the TopLeft() method alone for consistency with the other "corner" methods
    if (cdc.Class("BitmapInfoAccess").GlobalNamespace()) {
        return true;
    }
    // there is some odd stuff happening here I don't fully understand, leave it for now
    if (loplugin::hasPathnamePrefix(aFilename, SRCDIR "/include/canvas/") || loplugin::hasPathnamePrefix(aFilename, SRCDIR "/canvas/")) {
        return true;
    }
    // classes that have static data and some kind of weird reference-counting trick in its constructor
    if (cdc.Class("LinguOptions").GlobalNamespace()
        || (cdc.Class("EditableExtendedColorConfig").Namespace("svtools")
            .GlobalNamespace())
        || (cdc.Class("ExtendedColorConfig").Namespace("svtools")
            .GlobalNamespace())
        || cdc.Class("SvtMiscOptions").GlobalNamespace()
        || cdc.Class("SvtAccessibilityOptions").GlobalNamespace()
        || cdc.Class("ColorConfig").Namespace("svtools").GlobalNamespace()
        || cdc.Class("SvtOptionsDrawinglayer").GlobalNamespace()
        || cdc.Class("SvtMenuOptions").GlobalNamespace()
        || cdc.Class("SvtToolPanelOptions").GlobalNamespace()
        || (cdc.Class("SharedResources").Namespace("connectivity")
            .GlobalNamespace())
        || (cdc.Class("OParseContextClient").Namespace("svxform")
            .GlobalNamespace())
        || cdc.Class("OLimitedFormats").Namespace("frm").GlobalNamespace())
    {
        return true;
    }

    auto fdc = loplugin::DeclCheck(pCXXMethodDecl);

    // somebody has work-in-progress here
    if ((fdc.Function("getCurrZeroChar")
         .Class("LocaleDataWrapper").GlobalNamespace()))
        return true;

    // the unotools and svl config code stuff is doing weird stuff with a reference-counted statically allocated pImpl class
    if ((fdc.Function("getByName2")
         .Class("GlobalEventConfig").GlobalNamespace()))
        return true;
    if ((cdc.Class("SvtLinguConfig").GlobalNamespace()))
        return true;
    if ((cdc.Class("SvtModuleOptions").GlobalNamespace()))
        return true;

    // only empty on Linux, not on windows
    if ((fdc.Function("GetVisualRepresentationInNativeFormat_Impl")
         .Class("OleEmbeddedObject").GlobalNamespace())
        || (fdc.Function("GetRidOfComponent").Class("OleEmbeddedObject")
            .GlobalNamespace())
        || cdc.Class("SbxDecimal").GlobalNamespace()
        || fdc.Function("Call").Class("SbiDllMgr").GlobalNamespace()
        || fdc.Function("FreeDll").Class("SbiDllMgr").GlobalNamespace()
        || (fdc.Function("InitializeDde").Class("SfxApplication")
            .GlobalNamespace())
        || (fdc.Function("RemoveDdeTopic").Class("SfxApplication")
            .GlobalNamespace())
        || (fdc.Function("UpdateSkiaStatus").Class("OfaViewTabPage")
            .GlobalNamespace())
        || (fdc.Function("ReleaseData").Class("ScannerManager")
            .GlobalNamespace()))
    {
        return true;
    }
    // debugging stuff
    if (fdc.Function("dump").Class("InternalData").Namespace("chart")
        .GlobalNamespace())
    {
        return true;
    }
    // used in a function-pointer-table
    if ((cdc.Class("SbiRuntime").GlobalNamespace()
         && startsWith(pCXXMethodDecl->getNameAsString(), "Step"))
        || (cdc.Class("OoxFormulaParserImpl").AnonymousNamespace().Namespace("xls").Namespace("oox")
            .GlobalNamespace())
        || cdc.Class("SwTableFormula").GlobalNamespace()
        || (cdc.Class("BiffFormulaParserImpl").Namespace("xls").Namespace("oox")
            .GlobalNamespace())
        || (fdc.Function("Read_F_Shape").Class("SwWW8ImplReader")
            .GlobalNamespace())
        || (fdc.Function("Read_Majority").Class("SwWW8ImplReader")
            .GlobalNamespace())
        || fdc.Function("Ignore").Class("SwWrtShell").GlobalNamespace()
        || (cdc.Class("AttributesChecker").AnonymousNamespace().GlobalNamespace()
            && startsWith(pCXXMethodDecl->getNameAsString(), "check")))
    {
        return true;
    }
    // have no idea why this can't be static, but 'make check' fails with it so...
    if (fdc.Function("resolveRelationshipsOfTypeFromOfficeDoc").Class("Shape")
        .Namespace("drawingml").Namespace("oox").GlobalNamespace())
    {
        return true;
    }
    // template magic
    if (fdc.Function("getValue").Class("ColumnBatch").GlobalNamespace()
        || cdc.Class("TitleImpl").GlobalNamespace()
        || (fdc.Function("getDefaultPropertyName").Class("DefaultReturnHelper")
            .Namespace("vba").Namespace("ooo").GlobalNamespace()))
    {
        return true;
    }
    // depends on config options
    if ((fdc.Function("autoInstallFontLangSupport").Class("PrintFontManager")
         .Namespace("psp").GlobalNamespace())
        || fdc.Function("AllocateFrame").Class("GtkSalFrame").GlobalNamespace()
        || (fdc.Function("TriggerPaintEvent").Class("GtkSalFrame")
            .GlobalNamespace()))
    {
        return true;
    }

    bVisitedThis = false;
    TraverseStmt(pCXXMethodDecl->getBody());
    if (bVisitedThis) {
        return true;
    }

    if (containsPreprocessingConditionalInclusion((pCXXMethodDecl->getSourceRange()))) {
        return true;
    }

    report(
        DiagnosticsEngine::Warning,
        "this member function can be declared static",
        pCXXMethodDecl->getCanonicalDecl()->getLocation())
      << pCXXMethodDecl->getCanonicalDecl()->getSourceRange();
    FunctionDecl const * def;
    if (pCXXMethodDecl->isDefined(def)
        && def != pCXXMethodDecl->getCanonicalDecl())
    {
        report(DiagnosticsEngine::Note, "defined here:", def->getLocation())
            << def->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<StaticMethods> X("staticmethods");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
