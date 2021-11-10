/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Find constant character array variables that are either
//   (a) passed into O[U]String constructors
//   (b) assigned to O[U]String
// and are declared using macro names
// and should thus be turned into O[U]StringLiteral variables.
//

#include <cassert>

#include "check.hxx"
#include "plugin.hxx"

namespace
{
class StringLiteralDefine final : public loplugin::FilteringPlugin<StringLiteralDefine>
{
public:
    explicit StringLiteralDefine(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool TraverseInitListExpr(InitListExpr* expr, DataRecursionQueue* queue = nullptr)
    {
        return WalkUpFromInitListExpr(expr)
               && TraverseSynOrSemInitListExpr(
                      expr->isSemanticForm() ? expr : expr->getSemanticForm(), queue);
    }

    bool VisitCXXConstructExpr(CXXConstructExpr const* expr)
    {
        if (ignoreLocation(expr))
            return true;
        loplugin::TypeCheck const tc(expr->getType());
        if (!(tc.Class("OString").Namespace("rtl").GlobalNamespace()
              || tc.Class("OUString").Namespace("rtl").GlobalNamespace()))
        {
            return true;
        }
        auto const ctor = expr->getConstructor();
        if (ctor->getNumParams() != 2)
            return true;

        const Expr* arg0 = expr->getArg(0)->IgnoreParenImpCasts();
        auto const e1 = dyn_cast<clang::StringLiteral>(arg0);
        if (!e1)
            return true;
        auto argLoc = compat::getBeginLoc(arg0);
        // check if the arg is a macro
        auto macroLoc = compiler.getSourceManager().getSpellingLoc(argLoc);
        if (argLoc == macroLoc)
            return true;
        // check if it is the right kind of macro (not particularly reliable checks)
        if (!macroLoc.isValid() || !compiler.getSourceManager().isInMainFile(macroLoc)
            || compiler.getSourceManager().isInSystemHeader(macroLoc)
// not sure when these became available
#if CLANG_VERSION >= 130000
            || compiler.getSourceManager().isWrittenInBuiltinFile(macroLoc)
            || compiler.getSourceManager().isWrittenInScratchSpace(macroLoc)
            || compiler.getSourceManager().isWrittenInCommandLineFile(macroLoc)
#endif
            || isInUnoIncludeFile(macroLoc))
            return true;
        StringRef fileName = getFilenameOfLocation(macroLoc);
        StringRef name{ Lexer::getImmediateMacroName(
            compat::getBeginLoc(arg0), compiler.getSourceManager(), compiler.getLangOpts()) };
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/config_host/"))
            return true;
        // used in both OUString and OString context
        if (name == "FM_COL_LISTBOX" || name == "HID_RELATIONDIALOG_LEFTFIELDCELL"
            || name == "OOO_HELP_INDEX" || name == "IMP_PNG" || name.startswith("MNI_ACTION_"))
            return true;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/svx/source/stbctrls/pszctrl.cxx"))
            return true;
        // used as a prefix and/or concatenated with other strings
        if (name.startswith("UNO_JAVA_JFW") || name == "SETNODE_BINDINGS" || name == "PATHDELIMITER"
            || name == "SETNODE_ALLFILEFORMATS" || name == "SETNODE_DISABLED"
            || name == "XMLNS_DIALOGS_PREFIX" || name == "XMLNS_LIBRARY_PREFIX"
            || name == "XMLNS_SCRIPT_PREFIX" || name == "XMLNS_TOOLBAR" || name == "XMLNS_XLINK"
            || name == "XMLNS_XLINK_PREFIX")
            return true;
        if (loplugin::hasPathnamePrefix(fileName,
                                        SRCDIR "/stoc/source/security/access_controller.cxx")
            && (name == "SERVICE_NAME" || name == "USER_CREDS"))
            return true;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/stoc/source/security/file_policy.cxx")
            && name == "IMPL_NAME")
            return true;
        if (loplugin::hasPathnamePrefix(fileName,
                                        SRCDIR "/desktop/source/migration/services/jvmfwk.cxx")
            && name == "IMPL_NAME")
            return true;
        if (loplugin::hasPathnamePrefix(
                fileName, SRCDIR "/xmlsecurity/source/xmlsec/xmldocumentwrapper_xmlsecimpl.cxx")
            && name == "STRXMLNS")
            return true;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/sw/source/ui/fldui/fldvar.cxx")
            && name == "USER_DATA_VERSION_1")
            return true;
        // not sure how to exclude the case where the whole block is in a macro
        // (vs. what I am looking for - regular code with a macro name as the argument)
        if (name == "assert" || name == "SAL_INFO" || name == "DECLIMPL_SERVICEINFO_DERIVED"
            || name == "OSL_VERIFY" || name == "OSL_ENSURE" || name == "DECL_PROP_2"
            || name == "DECL_PROP_3" || name == "DECL_PROP_1" || name == "DECL_DEP_PROP_2"
            || name == "DECL_DEP_PROP_3" || name == "CALL_ELEMENT_HANDLER_AND_CARE_FOR_EXCEPTIONS"
            || name == "IMPLEMENT_SERVICE_INFO" || name == "SQL_GET_REFERENCES"
            || name == "SFX_IMPL_OBJECTFACTORY" || name == "IMPLEMENT_SERVICE_INFO1"
            || name == "IMPLEMENT_SERVICE_INFO2" || name == "IMPLEMENT_SERVICE_INFO3"
            || name == "IMPLEMENT_SERVICE_INFO_IMPLNAME" || name == "SC_SIMPLE_SERVICE_INFO"
            || name == "SC_SIMPLE_SERVICE_INFO_COMPAT" || name == "OUT_COMMENT"
            || name == "LOCALE_EN" || name == "LOCALE" || name == "VBAFONTBASE_PROPNAME"
            || name == "VBAHELPER_IMPL_XHELPERINTERFACE" || name == "IMPRESS_MAP_ENTRIES"
            || name == "DRAW_MAP_ENTRIES" || name == "DRAW_PAGE_NOTES_PROPERTIES"
            || name == "COMMON_FLDTYP_PROPERTIES" || name == "GRAPHIC_PAGE_PROPERTIES"
            || name == "makeDelay" || name == "makeEvent" || name == "OOO_IMPORTER"
            || name == "DBG_ASSERT" || name.startswith("CPPUNIT_ASSERT"))
            return true;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR
                                        "/dbaccess/source/ui/querydesign/SelectionBrowseBox.cxx")
            && name == "DEFAULT_SIZE")
            return true;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/filter/source/t602/t602filter.cxx"))
            return true;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/hwpfilter/source/formula.cxx"))
            return true;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/hwpfilter/source/hwpreader.cxx"))
            return true;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/filter/source/svg/svgexport.cxx")
            && name == "NSPREFIX")
            return true;

        if (!reported_.insert(macroLoc).second)
            return true;

        report(DiagnosticsEngine::Warning,
               "change macro '%0' to 'constexpr "
               "%select{OStringLiteral|OUStringLiteral}1'",
               macroLoc)
            << name << (tc.Class("OString").Namespace("rtl").GlobalNamespace() ? 0 : 1);
        report(DiagnosticsEngine::Note, "macro used here", compat::getBeginLoc(arg0))
            << arg0->getSourceRange();
        return true;
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

private:
    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    std::set<SourceLocation> reported_;
};

// Off by default because it needs some hand-holding
static loplugin::Plugin::Registration<StringLiteralDefine> reg("stringliteraldefine", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
