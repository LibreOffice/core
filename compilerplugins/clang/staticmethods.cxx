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

/*
  Look for member functions that can be static
*/
namespace {

class StaticMethods:
    public RecursiveASTVisitor<StaticMethods>, public loplugin::Plugin
{
private:
    bool bVisitedThis;
public:
    explicit StaticMethods(InstantiationData const & data): Plugin(data) {}

    void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool TraverseCXXMethodDecl(const CXXMethodDecl * decl);

    bool VisitCXXThisExpr(const CXXThisExpr *) { bVisitedThis = true; return true; }
    // these two indicate that we hit something that makes our analysis unreliable
    bool VisitUnresolvedMemberExpr(const UnresolvedMemberExpr *) { bVisitedThis = true; return true; }
    bool VisitCXXDependentScopeMemberExpr(const CXXDependentScopeMemberExpr *) { bVisitedThis = true; return true; }
private:
    std::string getFilename(SourceLocation loc);
};

bool BaseCheckNotTestFixtureSubclass(
    const CXXRecordDecl *BaseDefinition
#if __clang_major__ == 3 && __clang_minor__ <= 7
    , void *
#endif
    )
{
    if (BaseDefinition->getQualifiedNameAsString().compare("CppUnit::TestFixture") == 0) {
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
        !compat::forallBases(*decl, BaseCheckNotTestFixtureSubclass, nullptr, true)) {
        return true;
    }
    return false;
}

std::string StaticMethods::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    return compiler.getSourceManager().getFilename(spellingLocation);
}

static bool startsWith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool StaticMethods::TraverseCXXMethodDecl(const CXXMethodDecl * pCXXMethodDecl) {
    if (ignoreLocation(pCXXMethodDecl)) {
        return true;
    }
    if (!pCXXMethodDecl->isInstance() || pCXXMethodDecl->isVirtual() || !pCXXMethodDecl->hasBody()) {
        return true;
    }
    if (pCXXMethodDecl->getOverloadedOperator() != OverloadedOperatorKind::OO_None || pCXXMethodDecl->hasAttr<OverrideAttr>()) {
        return true;
    }
    if (isa<CXXConstructorDecl>(pCXXMethodDecl) || isa<CXXDestructorDecl>(pCXXMethodDecl) || isa<CXXConversionDecl>(pCXXMethodDecl)) {
        return true;
    }
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(pCXXMethodDecl->getCanonicalDecl()->getLocStart()))) {
        return true;
    }
    if ( pCXXMethodDecl != pCXXMethodDecl->getCanonicalDecl() ) {
        return true;
    }

    // the CppUnit stuff uses macros and methods that can't be changed
    if (isDerivedFromTestFixture(pCXXMethodDecl->getParent())) {
        return true;
    }
    // don't mess with the backwards compatibility stuff
    if (getFilename(pCXXMethodDecl->getLocStart()) == SRCDIR "/cppuhelper/source/compat.cxx") {
        return true;
    }
    // the DDE has a dummy implementation on Linux and a real one on Windows
    std::string aFilename = getFilename(pCXXMethodDecl->getCanonicalDecl()->getLocStart());
    if (aFilename == SRCDIR "/include/svl/svdde.hxx") {
        return true;
    }
    std::string aParentName = pCXXMethodDecl->getParent()->getQualifiedNameAsString();
    // special case having something to do with static initialisation
    // sal/osl/all/utility.cxx
    if (aParentName == "osl::OGlobalTimer") {
        return true;
    }
    // leave the TopLeft() method alone for consistency with the other "corner" methods
    if (aParentName == "BitmapInfoAccess") {
        return true;
    }
    // can't change it because in debug mode it can't be static
    // sal/cpprt/operators_new_delete.cxx
    if (aParentName == "(anonymous namespace)::AllocatorTraits") {
        return true;
    }
    // in this case, the code is taking the address of the member function
    // shell/source/unix/sysshell/recently_used_file_handler.cxx
    if (aParentName == "(anonymous namespace)::recently_used_item") {
        return true;
    }
    // the unotools and svl config code stuff is doing weird stuff with a reference-counted statically allocated pImpl class
    if (startsWith(aFilename, SRCDIR "/include/unotools")) {
        return true;
    }
    if (startsWith(aFilename, SRCDIR "/include/svl")) {
        return true;
    }
    if (startsWith(aFilename, SRCDIR "/include/framework") || startsWith(aFilename, SRCDIR "/framework")) {
        return true;
    }
    // there is some odd stuff happening here I don't fully understand, leave it for now
    if (startsWith(aFilename, SRCDIR "/include/canvas") || startsWith(aFilename, SRCDIR "/canvas")) {
        return true;
    }
    // classes that have static data and some kind of weird reference-counting trick in it's constructor
    if (aParentName == "LinguOptions" || aParentName == "svtools::EditableExtendedColorConfig"
        || aParentName == "svtools::ExtendedColorConfig" || aParentName == "SvtMiscOptions"
        || aParentName == "SvtAccessibilityOptions" || aParentName == "svtools::ColorConfig"
        || aParentName == "SvtOptionsDrawinglayer" || aParentName == "SvtMenuOptions"
        || aParentName == "SvtToolPanelOptions" || aParentName == "SvtSlideSorterBarOptions"
        || aParentName == "connectivity::SharedResources"
        || aParentName == "svxform::OParseContextClient"
        || aParentName == "frm::OLimitedFormats" )
    {
        return true;
    }
    std::string fqn = aParentName + "::" + pCXXMethodDecl->getNameAsString();
    // only empty on Linux, not on windows
    if (fqn == "OleEmbeddedObject::GetVisualRepresentationInNativeFormat_Impl"
        || fqn == "OleEmbeddedObject::GetRidOfComponent"
        || fqn == "connectivity::mozab::ProfileAccess::isProfileLocked"
        || startsWith(fqn, "SbxDecimal::")
        || fqn == "SbiDllMgr::Call" || fqn == "SbiDllMgr::FreeDll"
        || fqn == "SfxApplication::InitializeDde" || fqn == "SfxApplication::RemoveDdeTopic"
        || fqn == "ScannerManager::ReleaseData") {
        return true;
    }
    // debugging stuff
    if (fqn == "chart::InternalData::dump") {
        return true;
    }
    // used in a function-pointer-table
    if (startsWith(fqn, "SbiRuntime::Step") || startsWith(fqn, "oox::xls::OoxFormulaParserImpl::")
        || startsWith(fqn, "SwTableFormula::")
        || startsWith(fqn, "oox::xls::BiffFormulaParserImpl::")
        || fqn == "SwWW8ImplReader::Read_F_Shape"
        || fqn == "SwWW8ImplReader::Read_Majority") {
        return true;
    }
    // have no idea why this can't be static, but 'make check' fails with it so...
    if (fqn == "oox::drawingml::Shape::resolveRelationshipsOfTypeFromOfficeDoc") {
        return true;
    }
    // template magic
    if (fqn == "ColumnBatch::getValue" || startsWith(fqn, "TitleImpl::")
        || fqn == "ooo::vba::DefaultReturnHelper::getDefaultPropertyName") {
        return true;
    }
    // depends on config options
    if (fqn == "psp::PrintFontManager::autoInstallFontLangSupport"
        || fqn == "GtkSalFrame::AllocateFrame"
        || fqn == "GtkSalFrame::TriggerPaintEvent") {
        return true;
    }

    bVisitedThis = false;
    TraverseStmt(pCXXMethodDecl->getBody());
    if (bVisitedThis) {
        return true;
    }

    report(
        DiagnosticsEngine::Warning,
        "this method can be declared static " + fqn,
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
