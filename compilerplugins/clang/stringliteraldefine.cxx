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

    bool VisitFunctionDecl(FunctionDecl const* functionDecl)
    {
        if (ignoreLocation(functionDecl))
            return true;
        //        if (functionDecl->getIdentifier() && functionDecl->getName() == "f5")
        //            functionDecl->dump();
        return true;
    }

    bool VisitCXXConstructExpr(CXXConstructExpr const* expr)
    {
        if (ignoreLocation(expr))
            return true;
        loplugin::TypeCheck const tc(expr->getType());
        if (tc.Class("OString").Namespace("rtl").GlobalNamespace()
            || tc.Class("OUString").Namespace("rtl").GlobalNamespace())
        {
            processOUStringConstruct(expr);
            return true;
        }
        // now check for the std::forward magic e.g. std::map<OUString,OUString>::emplace
        auto const ctor = expr->getConstructor();
        unsigned const n = std::min(ctor->getNumParams(), expr->getNumArgs());
        for (size_t i = 0; i < n; ++i)
        {
            if (!checkForPossibleCharArrayType(ctor->getParamDecl(i)->getType().getTypePtr()))
                continue;
            const Expr* arg = expr->getArg(i)->IgnoreParenImpCasts();
            auto const stringLiteral = dyn_cast<clang::StringLiteral>(arg);
            if (!stringLiteral)
                continue;
            processDefault(expr, stringLiteral, true);
        }
        return true;
    }

    // now for the std::forward magic e.g. std::map<OUString,OUString>::emplace
    bool VisitCallExpr(CallExpr const* expr)
    {
        if (ignoreLocation(expr))
            return true;
        FunctionDecl const* callee = expr->getDirectCallee();
        if (!callee)
            return true;
        unsigned firstArg = 0;
        if (isa<CXXOperatorCallExpr>(expr))
            if (auto const cmd = dyn_cast_or_null<CXXMethodDecl>(callee))
                if (!cmd->isStatic())
                    firstArg = 1;
        unsigned const n = std::min(callee->getNumParams(), expr->getNumArgs());
        for (unsigned i = firstArg; i < n; ++i)
        {
            if (!checkForPossibleCharArrayType(callee->getParamDecl(i)->getType().getTypePtr()))
                continue;
            const Expr* arg = expr->getArg(i)->IgnoreParenImpCasts();
            auto const stringLiteral = dyn_cast<clang::StringLiteral>(arg);
            if (!stringLiteral)
                continue;
            processDefault(expr, stringLiteral, true);
        }
        return true;
    }

    // now check for OUString::operator=
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr const* operatorCall)
    {
        if (ignoreLocation(operatorCall))
            return true;
        if (operatorCall->getOperator() != OO_Equal)
            return true;
        auto tc = loplugin::TypeCheck(operatorCall->getType()->getUnqualifiedDesugaredType());
        bool isOUString;
        if (tc.Struct("OUStringConcat").Namespace("rtl").GlobalNamespace()
            || tc.Class("OUString").Namespace("rtl").GlobalNamespace())
            isOUString = true;
        else if (tc.Struct("OStringConcat").Namespace("rtl").GlobalNamespace()
                 || tc.Class("OString").Namespace("rtl").GlobalNamespace())
            isOUString = false;
        else
            return true;

        if (operatorCall->getNumArgs() < 2)
            return true;
        const Expr* arg = operatorCall->getArg(1)->IgnoreParenImpCasts();
        auto const stringLiteral = dyn_cast<clang::StringLiteral>(arg);
        if (!stringLiteral)
            return true;
        processDefault(operatorCall, stringLiteral, isOUString);
        return true;
    }

    void processOUStringConstruct(CXXConstructExpr const* expr)
    {
        auto const ctor = expr->getConstructor();
        if (ctor->getNumParams() != 2)
            return;

        const Expr* arg0 = expr->getArg(0)->IgnoreParenImpCasts();
        auto const stringLiteral = dyn_cast<clang::StringLiteral>(arg0);
        if (!stringLiteral)
            return;
        loplugin::TypeCheck const tc(expr->getType());
        processDefault(expr, stringLiteral,
                       bool(tc.Class("OUString").Namespace("rtl").GlobalNamespace()));
    }

    bool checkForPossibleCharArrayType(const clang::Type* type)
    {
        auto lvrt = dyn_cast<LValueReferenceType>(type);
        if (!lvrt)
            return false;
        auto cat = dyn_cast<ConstantArrayType>(lvrt->getPointeeType());
        if (!cat)
            return false;
        auto bit = dyn_cast<BuiltinType>(cat->getElementType());
        if (!bit)
            return false;
        return true;
    }

    void processDefault(Expr const* expr, const clang::StringLiteral* stringLiteral,
                        bool isOUString)
    {
        auto macroLoc = stringLiteral->getStrTokenLoc(0);
        // check if the arg is a macro
        if (macroLoc.isMacroID())
            ; // definitely a macro
        else if (stringLiteral->getNumConcatenated() == 2)
        {
            // probably not a macro, but check for the form
            //   OUString(u"" FOO)
            //            std::string argString = getSourceAsString(SourceRange(argLoc, argLoc.getLocWithOffset(3)));
            //            std::cout << argString << std::endl;
            //            if (argString != "u\"\"")
            //                return true;
            macroLoc = stringLiteral->getStrTokenLoc(1);
            if (!macroLoc.isMacroID())
                return;
        }
        else
            return;

        // check if it is the right kind of macro (not particularly reliable checks)
        auto macroBodyLoc = compiler.getSourceManager().getSpellingLoc(macroLoc);
        if (!macroBodyLoc.isValid() || compiler.getSourceManager().isInSystemHeader(macroBodyLoc)
// not sure when these became available
#if CLANG_VERSION >= 130000
            || compiler.getSourceManager().isWrittenInBuiltinFile(macroBodyLoc)
            || compiler.getSourceManager().isWrittenInScratchSpace(macroBodyLoc)
            || compiler.getSourceManager().isWrittenInCommandLineFile(macroBodyLoc)
#endif
            || isInUnoIncludeFile(macroBodyLoc))
            return;

        StringRef fileName = getFilenameOfLocation(macroBodyLoc);
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/config_host/"))
            return;

        StringRef name = Lexer::getImmediateMacroName(macroLoc, compiler.getSourceManager(),
                                                      compiler.getLangOpts());

        // used in both OUString and OString context
        if (name == "FM_COL_LISTBOX" || name == "HID_RELATIONDIALOG_LEFTFIELDCELL"
            || name == "OOO_HELP_INDEX" || name == "IMP_PNG" || name.startswith("MNI_ACTION_"))
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/svx/source/stbctrls/pszctrl.cxx"))
            return;
        // used as a prefix and/or concatenated with other strings
        if (name.startswith("UNO_JAVA_JFW") || name == "SETNODE_BINDINGS" || name == "PATHDELIMITER"
            || name == "SETNODE_ALLFILEFORMATS" || name == "SETNODE_DISABLED"
            || name == "XMLNS_DIALOGS_PREFIX" || name == "XMLNS_LIBRARY_PREFIX"
            || name == "XMLNS_SCRIPT_PREFIX" || name == "XMLNS_TOOLBAR" || name == "XMLNS_XLINK"
            || name == "XMLNS_XLINK_PREFIX")
            return;
        if (loplugin::hasPathnamePrefix(fileName,
                                        SRCDIR "/stoc/source/security/access_controller.cxx")
            && (name == "SERVICE_NAME" || name == "USER_CREDS"))
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/stoc/source/security/file_policy.cxx")
            && name == "IMPL_NAME")
            return;
        if (loplugin::hasPathnamePrefix(fileName,
                                        SRCDIR "/desktop/source/migration/services/jvmfwk.cxx")
            && name == "IMPL_NAME")
            return;
        if (loplugin::hasPathnamePrefix(
                fileName, SRCDIR "/xmlsecurity/source/xmlsec/xmldocumentwrapper_xmlsecimpl.cxx")
            && name == "STRXMLNS")
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/sw/source/ui/fldui/fldvar.cxx")
            && name == "USER_DATA_VERSION_1")
            return;
        if (loplugin::hasPathnamePrefix(fileName,
                                        SRCDIR "/desktop/source/deployment/inc/lockfile.hxx")
            && name.startswith("LOCKFILE_"))
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/svx/source/inc/gridcols.hxx")
            && name.startswith("FM_COL_"))
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/include/oox/core/relations.hxx")
            && name.startswith("CREATE_"))
            return;
        if (loplugin::hasPathnamePrefix(fileName,
                                        SRCDIR "/include/svtools/htmlkywd.hxx")
            && name.startswith("OOO_STRING_")) // TODO could maybe convert these
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/reportdesign/inc/helpids.h")
            && (name == "HID_RPT_FIELD_SEL_WIN" || name == "UID_RPT_RPT_APP_VIEW"))
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/sd/inc/strings.hxx")
            && name == "STR_HTMLEXP_DEFAULT_EXTENSION")
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/sdext/inc/bitmaps.hlst")
            && name == "BMP_PRESENTATION_MINIMIZER")
            return;
        if (loplugin::hasPathnamePrefix(fileName,
                                        SRCDIR "/sdext/source/pdfimport/tree/pdfiprocessor.cxx")
            && name == "OASIS_STR")
            return;
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
            || name == "MC_E" || name == "FILL_PROPERTIES_BMP" || name == "FILL_PROPERTIES_DEFAULTS"
            || name == "FILL_PROPERTIES" || name == "LINE_PROPERTIES_DEFAULTS"
            || name == "LINE_PROPERTIES" || name == "LINE_PROPERTIES_START_END"
            || name == "SHAPE_DESCRIPTOR_PROPERTIES" || name == "MISC_OBJ_PROPERTIES_NO_SHEAR"
            || name == "GLOW_PROPERTIES" || name == "SOFTEDGE_PROPERTIES"
            || name == "TEXT_PROPERTIES_DEFAULTS" || name == "FONTWORK_PROPERTIES"
            || name == "SPECIAL_CONNECTOR_PROPERTIES"
            || name == "SPECIAL_DIMENSIONING_PROPERTIES_DEFAULTS"
            || name == "SPECIAL_DIMENSIONING_PROPERTIES" || name == "MAP"
            || name == "MAP_CONST_C_ASCII" || name == "MAP_CONST_S")
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR
                                        "/dbaccess/source/ui/querydesign/SelectionBrowseBox.cxx")
            && name == "DEFAULT_SIZE")
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/filter/source/t602/t602filter.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/hwpfilter/source/formula.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/hwpfilter/source/hwpreader.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/filter/source/svg/svgexport.cxx")
            && name == "NSPREFIX")
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/vcl/source/filter/ieps/ieps.cxx")
            && name == "EXESUFFIX")
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/connectivity/inc/bitmaps.hlst")
            && name == "LINKED_TEXT_TABLE_IMAGE_RESOURCE")
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/jvmfwk/")
            && (name == "JFW_ENSURE" || name == "JFW_TRACE2"))
            return;

        // false+ from the check for 'char (&x)[]' parameters
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/oox/source/dump/pptxdumper.cxx")
            && name == "DUMP_PPTX_CONFIG_ENVVAR")
            return;
        if (loplugin::hasPathnamePrefix(fileName,
                                        SRCDIR "/extensions/source/propctrlr/eventhandler.cxx")
            && name == "DESCRIBE_EVENT")
            return;

        // false+ from the OUString::operator= check
        if (name == "SAL_CONFIGFILE")
            return;
        if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/svx/source/form/fmservs.cxx")
            && name == "REGISTER_SERVICE")
            return;

        if (!reported_.insert(macroBodyLoc).second)
            return;

        if (rewrite1(macroBodyLoc, isOUString))
            return;

        report(DiagnosticsEngine::Warning,
               "change macro '%0' to 'constexpr "
               "%select{OStringLiteral|OUStringLiteral}1'",
               macroBodyLoc)
            << name << (!isOUString ? 0 : 1);
        report(DiagnosticsEngine::Note, "macro used here", compat::getBeginLoc(expr))
            << expr->getSourceRange();
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
        static const std::regex defineRegex("^(\\s*)#define\\s+(\\w+)\\s+u?(\".*\")\\s*$");
        static const std::string replace1("$1constexpr OUStringLiteral $2 = u$3;");
        static const std::string replace2("$1constexpr OStringLiteral $2 = $3;");
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

    std::string getSourceAsString(SourceRange range)
    {
        SourceManager& SM = compiler.getSourceManager();
        SourceLocation startLoc = range.getBegin();
        SourceLocation endLoc = range.getEnd();
        char const* p1 = SM.getCharacterData(startLoc);
        char const* p2 = SM.getCharacterData(endLoc);
        if (p2 < p1)
        {
            // workaround clang weirdness, but don't return empty string
            // in case it happens during code replacement
            return "clang returned bad pointers";
        }
        if (p2 - p1 > 64 * 1024)
        {
            // workaround clang weirdness, but don't return empty string
            // in case it happens during code replacement
            return "clang returned overly large source range";
        }
        return std::string(p1, p2 - p1);
    }

    std::set<SourceLocation> reported_;
};

// Off by default because it needs some hand-holding
static loplugin::Plugin::Registration<StringLiteralDefine> reg("stringliteraldefine", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
