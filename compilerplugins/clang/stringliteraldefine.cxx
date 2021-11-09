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

#include "check.hxx"
#include "plugin.hxx"
#include <cassert>
#include <regex>
#include <iostream>
#include <fstream>

namespace
{
class StringLiteralDefine final : public loplugin::FilteringRewritePlugin<StringLiteralDefine>
{
public:
    explicit StringLiteralDefine(loplugin::InstantiationData const& data)
        : FilteringRewritePlugin(data)
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
        if (!macroLoc.isValid() || compiler.getSourceManager().isInSystemHeader(macroLoc)
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
        if (loplugin::hasPathnamePrefix(fileName,
                                        SRCDIR "/desktop/source/deployment/inc/lockfile.hxx")
            && name.startswith("LOCKFILE_"))
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
            || name == "DBG_ASSERT" || name.startswith("CPPUNIT_ASSERT")
            || name == "SVX_UNOEDIT_CHAR_PROPERTIES" || name == "SVX_UNOEDIT_PARA_PROPERTIES"
            || name == "SVX_UNOEDIT_OUTLINER_PROPERTIES" || name == "SVX_UNOEDIT_NUMBERING_PROPERTY"
            || name == "SVX_UNOEDIT_OUTLINER_PROPERTIES" || name == "MAP_ASCII" || name == "GMAP"
            || name == "GMAP_D" || name == "GMAPV" || name == "PMAP" || name == "TMAP"
            || name == "DPMAP" || name == "MAP_" || name == "MAP_ENTRY" || name == "MAP_CONTEXT"
            || name == "MAP_SPECIAL" || name == "MAP_ENTRY_ODF12" || name == "MAP_ENTRY_ODF_EXT"
            || name == "MAP_ENTRY_ODF_EXT_IMPORT" || name == "MAP_FULL"
            || name == "MAP_SPECIAL_ODF13" || name == "MAP_ENTRY_ODF13"
            || name == "MAP_SPECIAL_ODF12" || name == "PLMAP" || name == "PLMAP_ODF13"
            || name == "PLMAP_12" || name == "PLMAP_EXT" || name == "HFMAP" || name == "CMAP"
            || name == "RMAP" || name == "CELLMAP" || name == "MP_E" || name == "MAP_ODF13"
            || name == "MT_E" || name == "MT_ED" || name == "MAP_EXT_I" || name == "MAP_EXT"
            || name == "MP_ED" || name == "MG_ED" || name == "MG_E" || name == "MG_EV"
            || name == "MS_E" || name == "MR_E" || name == "MR_EV" || name == "M_ED_"
            || name == "MC_E")
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

        if (rewrite1(macroLoc, bool(tc.Class("OString").Namespace("rtl").GlobalNamespace())))
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
    bool rewrite1(SourceLocation macroLoc, bool isOUstring)
    {
        static const std::regex defineRegex("^(\\s*)#define(\\s+\\w+\\s+)(\".*\")$");
        static const std::string replace1("$1constexpr OUStringLiteral$2 = u$3;");
        static const std::string replace2("$1constexpr OStringLiteral$2 = u$3;");
        if (!rewriter)
            return false;
        SourceManager& SM = compiler.getSourceManager();
        char const* p0 = SM.getCharacterData(macroLoc);
        // extend backwards and forwards until we have the whole line
        char const* pStart = p0;
        char const* pEnd = p0;
        while (*pStart != '\r' && *pStart != '\n')
            --pStart;
        while (*pEnd != '\r' && *pEnd != '\n' && *pEnd != 0)
            ++pEnd;
        int len = pEnd - pStart - 1;
        if (len < 10 || len > 512)
        {
            std::cout << "fail1 " << len << std::endl;
            return false;
        }
        std::string s(pStart + 1, len);
        auto s2 = std::regex_replace(s, defineRegex, isOUstring ? replace1 : replace2);
        if (s2 == s)
        {
            std::cout << "fail2 " << s << std::endl;
            return false;
        }
        auto startLoc = macroLoc.getLocWithOffset((pStart + 1) - p0);
        auto endLoc = macroLoc.getLocWithOffset(pEnd - p0 - 1);
        if (!replaceText(SourceRange(startLoc, endLoc), s2))
        {
            std::cout << "fail3 " << s << " " << s2 << std::endl;
            return false;
        }
        std::cout << "success!" << std::endl;
        return true;
    }

    std::set<SourceLocation> reported_;
};

// Off by default because it needs some hand-holding
static loplugin::Plugin::Registration<StringLiteralDefine> reg("stringliteraldefine", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
