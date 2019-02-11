/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if !defined _WIN32 //TODO, #include <sys/file.h>

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include <algorithm>
#include <sys/file.h>
#include <unistd.h>
#include "plugin.hxx"
#include "compat.hxx"
#include "check.hxx"

/**
  Finds variables that are effectively write-only.

  Largely the same as the unusedfields.cxx loplugin.
*/

namespace
{
struct MyVarInfo
{
    const VarDecl* varDecl;
    std::string parent;
    std::string varName;
    std::string varType;
    std::string sourceLocation;
};
bool operator<(const MyVarInfo& lhs, const MyVarInfo& rhs)
{
    return std::tie(lhs.parent, lhs.varName) < std::tie(rhs.parent, rhs.varName);
}

// try to limit the voluminous output a little
static std::set<MyVarInfo> readFromSet;
static std::set<MyVarInfo> writeToSet;
static std::set<MyVarInfo> definitionSet;

/**
 * Wrap the different kinds of callable and callee objects in the clang AST so I can define methods that handle everything.
 */
class CallerWrapper
{
    const CallExpr* m_callExpr;
    const CXXConstructExpr* m_cxxConstructExpr;

public:
    CallerWrapper(const CallExpr* callExpr)
        : m_callExpr(callExpr)
        , m_cxxConstructExpr(nullptr)
    {
    }
    CallerWrapper(const CXXConstructExpr* cxxConstructExpr)
        : m_callExpr(nullptr)
        , m_cxxConstructExpr(cxxConstructExpr)
    {
    }
    unsigned getNumArgs() const
    {
        return m_callExpr ? m_callExpr->getNumArgs() : m_cxxConstructExpr->getNumArgs();
    }
    const Expr* getArg(unsigned i) const
    {
        return m_callExpr ? m_callExpr->getArg(i) : m_cxxConstructExpr->getArg(i);
    }
};
class CalleeWrapper
{
    const FunctionDecl* m_calleeFunctionDecl = nullptr;
    const CXXConstructorDecl* m_cxxConstructorDecl = nullptr;
    const FunctionProtoType* m_functionPrototype = nullptr;

public:
    explicit CalleeWrapper(const FunctionDecl* calleeFunctionDecl)
        : m_calleeFunctionDecl(calleeFunctionDecl)
    {
    }
    explicit CalleeWrapper(const CXXConstructExpr* cxxConstructExpr)
        : m_cxxConstructorDecl(cxxConstructExpr->getConstructor())
    {
    }
    explicit CalleeWrapper(const FunctionProtoType* functionPrototype)
        : m_functionPrototype(functionPrototype)
    {
    }
    unsigned getNumParams() const
    {
        if (m_calleeFunctionDecl)
            return m_calleeFunctionDecl->getNumParams();
        else if (m_cxxConstructorDecl)
            return m_cxxConstructorDecl->getNumParams();
        else if (m_functionPrototype->param_type_begin() == m_functionPrototype->param_type_end())
            // FunctionProtoType will assert if we call getParamTypes() and it has no params
            return 0;
        else
            return m_functionPrototype->getParamTypes().size();
    }
    const QualType getParamType(unsigned i) const
    {
        if (m_calleeFunctionDecl)
            return m_calleeFunctionDecl->getParamDecl(i)->getType();
        else if (m_cxxConstructorDecl)
            return m_cxxConstructorDecl->getParamDecl(i)->getType();
        else
            return m_functionPrototype->getParamTypes()[i];
    }
    std::string getNameAsString() const
    {
        if (m_calleeFunctionDecl)
            return m_calleeFunctionDecl->getNameAsString();
        else if (m_cxxConstructorDecl)
            return m_cxxConstructorDecl->getNameAsString();
        else
            return "";
    }
    CXXMethodDecl const* getAsCXXMethodDecl() const
    {
        if (m_calleeFunctionDecl)
            return dyn_cast<CXXMethodDecl>(m_calleeFunctionDecl);
        return nullptr;
    }
};

class WriteOnlyVars : public loplugin::FilteringPlugin<WriteOnlyVars>
{
public:
    explicit WriteOnlyVars(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override;

    bool shouldVisitTemplateInstantiations() const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitVarDecl(const VarDecl*);
    bool VisitDeclRefExpr(const DeclRefExpr*);
    bool TraverseIfStmt(IfStmt*);

private:
    MyVarInfo niceName(const VarDecl*);
    void checkIfReadFrom(const VarDecl* varDecl, const Expr* memberExpr);
    void checkIfWrittenTo(const VarDecl* varDecl, const Expr* memberExpr);
    bool checkForWriteWhenUsingCollectionType(const CXXMethodDecl* calleeMethodDecl);
    bool IsPassedByNonConst(const VarDecl* varDecl, const Stmt* child, CallerWrapper callExpr,
                            CalleeWrapper calleeFunctionDecl);
    llvm::Optional<CalleeWrapper> getCallee(CallExpr const*);

    // For reasons I do not understand, parentFunctionDecl() is not reliable, so
    // we store the parent function on the way down the AST.
    FunctionDecl* insideFunctionDecl = nullptr;
    std::vector<VarDecl const*> insideConditionalCheckOfMemberSet;
};

void WriteOnlyVars::run()
{
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    if (!isUnitTestMode())
    {
        StringRef fn(handler.getMainFileName());
        // playing paging-in games with volatile
        if (loplugin::isSamePathname(fn, SRCDIR "/sal/osl/unx/file.cxx"))
            return;
        // playing paging-in games with volatile
        if (loplugin::isSamePathname(fn, SRCDIR "/desktop/unx/source/file_image_unx.c"))
            return;
        // false+
        if (loplugin::isSamePathname(fn, SRCDIR "/store/source/storpage.cxx"))
            return;
        // yydebug?
        if (loplugin::isSamePathname(fn, SRCDIR "/idlc/source/idlccompile.cxx"))
            return;
        if (fn.contains("/qa/"))
            return;
        if (fn.contains("/vcl/workben/"))
            return;
        // preload
        if (loplugin::isSamePathname(fn, SRCDIR "/cppuhelper/source/servicemanager.cxx"))
            return;
        // doing a "free items outside lock" thing
        if (loplugin::isSamePathname(fn, SRCDIR "/unotools/source/config/itemholder1.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/config/itemholder2.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svtools/source/config/itemholder2.cxx"))
            return;
        // doing a "keep objects alive" thing
        if (loplugin::isSamePathname(fn, SRCDIR "/jvmfwk/source/framework.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/jvmfwk/plugins/sunmajor/pluginlib/util.cxx"))
            return;
        // debug code
        if (loplugin::isSamePathname(fn, SRCDIR "/svl/source/items/style.cxx"))
            return;
        // ok
        if (loplugin::isSamePathname(fn, SRCDIR "/stoc/source/inspect/introspection.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/package/source/zippackage/ZipPackage.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/hwpfilter/source/hwpreader.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/treelist/transfer.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/app/brand.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/filter/igif/gifread.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/gdi/metaact.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/fontsubset/sft.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/filter/ipdf/pdfdocument.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/filter/ipdf/pdfdocument2.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/unx/generic/app/sm.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/filter/jpeg/JpegWriter.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/unx/generic/dtrans/X11_selection.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/filter/jpeg/jpegc.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/unx/generic/window/FWS.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/awt/vclxspinbutton.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/toolkit/source/controls/formattedcontrol.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svtools/source/config/helpopt.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svtools/source/filter/SvFilterOptionsDialog.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svtools/source/uno/generictoolboxcontroller.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svtools/source/java/javainteractionhandler.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/basic/source/classes/sbunoobj.cxx"))
            return;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/accessibility/source/standard/vclxaccessiblebox.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/cppcanvas/source/mtfrenderer/implrenderer.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/doc/guisaveas.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/appl/newhelp.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/control/thumbnailview.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/control/recentdocsview.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sfx2/source/view/viewfrm.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/framework/source/services/desktop.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/framework/source/uielement/generictoolbarcontroller.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/framework/source/uielement/complextoolbarcontroller.cxx"))
            return;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/framework/source/interaction/quietinteraction.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/editeng/source/editeng/editdoc.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/editeng/source/editeng/impedit4.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/editeng/source/editeng/editobj.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/editeng/source/items/frmitems.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/binaryurp/source/bridge.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/tbxctrls/fontworkgallery.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/basctl/source/basicide/moduldl2.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/canvas/source/cairo/cairo_spritecanvas.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/chart2/source/tools/DiagramHelper.cxx"))
            return;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/chart2/source/tools/ExplicitCategoriesProvider.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/chart2/source/tools/LegendHelper.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/chart2/source/tools/OPropertySet.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/chart2/source/tools/CommonConverters.cxx"))
            return;
        if (loplugin::isSamePathname(
                fn,
                SRCDIR "/chart2/source/controller/chartapiwrapper/WrappedNumberFormatProperty.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/chart2/source/tools/DataSourceHelper.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/oox/source/export/shapes.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/oox/source/export/chartexport.cxx"))
            return;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/filter/source/storagefilterdetect/filterdetect.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/filter/source/pdf/pdfexport.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/filter/source/svg/svgexport.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/filter/source/msfilter/svdfppt.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/dbaccess/source/core/recovery/subcomponentrecovery.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/dbaccess/source/core/dataaccess/documentcontainer.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/dbaccess/source/core/dataaccess/databasedocument.cxx"))
            return;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/dbaccess/source/ui/browser/genericcontroller.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/ucb/source/core/ucbcmds.cxx"))
            return;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/desktop/source/deployment/manager/dp_manager.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/desktop/source/deployment/registry/package/dp_package.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/desktop/source/lib/init.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/extensions/source/propctrlr/formcomponenthandler.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/embeddedobj/source/general/docholder.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/extensions/source/propctrlr/stringrepresentation.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpcontent.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpdivinfo.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpdoc.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/filter/source/pdf/impdialog.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwplayout.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpoleobject.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwprowlayout.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpfoundry.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpparastyle.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpnotes.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpfont.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwptblcell.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpusrdicts.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwpverdocument.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/lotuswordpro/source/filter/lwptblformula.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vbahelper/source/vbahelper/vbafontbase.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/vbahelper/source/vbahelper/vbadocumentbase.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/docshell/docsh8.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/docshell/docsh6.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/core/data/table3.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/unoobj/cellsuno.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/excel/xelink.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/lotus/lotus.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/vba/vbaworkbooks.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/vba/vbaworksheets.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/vba/vbarange.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/view/drviews2.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/filter/ppt/pptin.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/app/sdxfer.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/ui/view/drviewsf.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/filter/xml/sdxmlwrp.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/source/filter/html/pubdlg.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/txtnode/thints.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/doc/docbm.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/crsr/crsrsh.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/xml/swxml.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/doc/docredln.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/ww8/ww8par2.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/uibase/shells/drformsh.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/ww8/ww8par6.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/ui/dbui/dbinsdlg.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sdext/source/minimizer/impoptimizer.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sdext/source/presenter/PresenterTheme.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/sdext/source/pdfimport/wrapper/wrapper.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/slideshow/source/engine/animationnodes/generateevent.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/starmath/source/mathmlimport.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/starmath/source/eqnolefilehdr.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/svgio/source/svgreader/svgmarkernode.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/uui/source/iahndl-locking.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/shell/source/sessioninstall/SyncDbusSessionHelper.cxx"))
            return;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/slideshow/source/engine/opengl/TransitionerImpl.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/forms/source/component/FormattedField.cxx"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/forms/source/component/DatabaseForm.cxx"))
            return;
        if (loplugin::isSamePathname(fn,
                                     SRCDIR "/reportdesign/source/ui/report/ReportController.cxx"))
            return;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/test/"))
            return;
        if (loplugin::isSamePathname(fn, SRCDIR "/i18npool/source/localedata/LocaleNode.cxx"))
            return;

        // yynerrs?
        if (loplugin::isSamePathname(fn, SRCDIR "/hwpfilter/source/grammar.cxx"))
            return;

        for (MyVarInfo const& v : definitionSet)
        {
            bool read = readFromSet.find(v) != readFromSet.end();
            bool write = writeToSet.find(v) != writeToSet.end();
            if (!read && write)
                report(DiagnosticsEngine::Warning, "write-only %0", compat::getBeginLoc(v.varDecl))
                    << v.varName;
        }
    }
    else
    {
        for (const MyVarInfo& s : readFromSet)
            report(DiagnosticsEngine::Warning, "read %0", compat::getBeginLoc(s.varDecl))
                << s.varName;
        for (const MyVarInfo& s : writeToSet)
            report(DiagnosticsEngine::Warning, "write %0", compat::getBeginLoc(s.varDecl))
                << s.varName;
    }
}

MyVarInfo WriteOnlyVars::niceName(const VarDecl* varDecl)
{
    MyVarInfo aInfo;

    aInfo.varDecl = varDecl->getCanonicalDecl();
    aInfo.varName = varDecl->getNameAsString();
    // sometimes the name (if it's an anonymous thing) contains the full path of the build folder, which we don't need
    size_t idx = aInfo.varName.find(SRCDIR);
    if (idx != std::string::npos)
    {
        aInfo.varName = aInfo.varName.replace(idx, strlen(SRCDIR), "");
    }
    aInfo.varType = varDecl->getType().getAsString();

    SourceLocation expansionLoc
        = compiler.getSourceManager().getExpansionLoc(varDecl->getLocation());
    StringRef filename = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation
        = std::string(filename.substr(strlen(SRCDIR) + 1)) + ":"
          + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    loplugin::normalizeDotDotInFilePath(aInfo.sourceLocation);
    aInfo.parent = filename;

    return aInfo;
}

static bool contains(std::string const& s, std::string const& needle)
{
    return s.find(needle) != std::string::npos;
}

bool WriteOnlyVars::VisitVarDecl(const VarDecl* varDecl)
{
    if (varDecl->isImplicit() || varDecl->isExternC() || isa<ParmVarDecl>(varDecl))
        return true;
    auto tc = loplugin::TypeCheck(varDecl->getType());
    if (tc.Pointer() || tc.LvalueReference() || tc.Class("shared_ptr").StdNamespace()
        || tc.Class("unique_ptr").StdNamespace())
        return true;
    if (tc.Typedef("BitmapScopedWriteAccess"))
        return true;
    std::string typeName = varDecl->getType().getAsString();
    if (contains(typeName, "Guard") || contains(typeName, "Reader") || contains(typeName, "Stream")
        || contains(typeName, "Parser") || contains(typeName, "Codec")
        || contains(typeName, "Exception"))
        return true;
    varDecl = varDecl->getCanonicalDecl();
    if (!varDecl->getLocation().isValid() || ignoreLocation(varDecl))
        return true;
    if (!compiler.getSourceManager().isInMainFile(varDecl->getLocation()))
        return true;
    if (compiler.getSourceManager().isMacroBodyExpansion(compat::getBeginLoc(varDecl)))
        return true;
    if (compiler.getSourceManager().isMacroArgExpansion(compat::getBeginLoc(varDecl)))
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(varDecl->getLocation())))
        return true;

    definitionSet.insert(niceName(varDecl));
    return true;
}

static char easytolower(char in)
{
    if (in <= 'Z' && in >= 'A')
        return in - ('Z' - 'z');
    return in;
}

bool startswith(const std::string& rStr, const char* pSubStr)
{
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

bool WriteOnlyVars::TraverseIfStmt(IfStmt* ifStmt)
{
    VarDecl const* varDecl = nullptr;
    Expr const* cond = ifStmt->getCond()->IgnoreParenImpCasts();
    if (auto declRefExpr = dyn_cast<DeclRefExpr>(cond))
    {
        if ((varDecl = dyn_cast<VarDecl>(declRefExpr->getDecl())))
            insideConditionalCheckOfMemberSet.push_back(varDecl);
    }
    bool ret = RecursiveASTVisitor::TraverseIfStmt(ifStmt);
    if (varDecl)
        insideConditionalCheckOfMemberSet.pop_back();
    return ret;
}

void WriteOnlyVars::checkIfReadFrom(const VarDecl* varDecl, const Expr* memberExpr)
{
    auto parentsRange = compiler.getASTContext().getParents(*memberExpr);
    const Stmt* child = memberExpr;
    const Stmt* parent
        = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    // walk up the tree until we find something interesting
    bool bPotentiallyReadFrom = false;
    bool bDump = false;
    auto walkupUp = [&]() {
        child = parent;
        auto parentsRange = compiler.getASTContext().getParents(*parent);
        parent = parentsRange.begin() == parentsRange.end() ? nullptr
                                                            : parentsRange.begin()->get<Stmt>();
    };
    do
    {
        if (!parent)
        {
            // check if we're inside a CXXCtorInitializer or a VarDecl
            auto parentsRange = compiler.getASTContext().getParents(*child);
            if (parentsRange.begin() != parentsRange.end())
            {
                const Decl* decl = parentsRange.begin()->get<Decl>();
                if (decl && (isa<CXXConstructorDecl>(decl) || isa<VarDecl>(decl)))
                    bPotentiallyReadFrom = true;
            }
            if (!bPotentiallyReadFrom)
                return;
            break;
        }
        if (isa<CXXReinterpretCastExpr>(parent))
        {
            // once we see one of these, there is not much useful we can know
            bPotentiallyReadFrom = true;
            break;
        }
        else if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent)
                 || isa<ParenListExpr>(parent) || isa<ArrayInitLoopExpr>(parent)
                 || isa<ExprWithCleanups>(parent))
        {
            walkupUp();
        }
        else if (auto unaryOperator = dyn_cast<UnaryOperator>(parent))
        {
            UnaryOperator::Opcode op = unaryOperator->getOpcode();
            if (memberExpr->getType()->isArrayType() && op == UO_Deref)
            {
                // ignore, deref'ing an array does not count as a read
            }
            else if (op == UO_AddrOf || op == UO_Deref || op == UO_Plus || op == UO_Minus
                     || op == UO_Not || op == UO_LNot)
            {
                bPotentiallyReadFrom = true;
                break;
            }
            /* The following are technically reads, but from a code-sense they're more of a write/modify, so
                ignore them to find interesting fields that only modified, not usefully read:
                UO_PreInc / UO_PostInc / UO_PreDec / UO_PostDec
                But we still walk up in case the result of the expression is used in a read sense.
            */
            walkupUp();
        }
        else if (auto caseStmt = dyn_cast<CaseStmt>(parent))
        {
            bPotentiallyReadFrom = caseStmt->getLHS() == child || caseStmt->getRHS() == child;
            break;
        }
        else if (auto ifStmt = dyn_cast<IfStmt>(parent))
        {
            bPotentiallyReadFrom = ifStmt->getCond() == child;
            break;
        }
        else if (auto doStmt = dyn_cast<DoStmt>(parent))
        {
            bPotentiallyReadFrom = doStmt->getCond() == child;
            break;
        }
        else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(parent))
        {
            if (arraySubscriptExpr->getIdx() == child)
            {
                bPotentiallyReadFrom = true;
                break;
            }
            walkupUp();
        }
        else if (auto callExpr = dyn_cast<CXXMemberCallExpr>(parent))
        {
            // check for calls to ReadXXX() type methods and the operator>>= methods on Any.
            auto callee = getCallee(callExpr);
            if (callee && *callExpr->child_begin() == child)
            {
                // FIXME perhaps a better solution here would be some kind of SAL_PARAM_WRITEONLY attribute
                // which we could scatter around.
                std::string name = callee->getNameAsString();
                std::transform(name.begin(), name.end(), name.begin(), easytolower);
                if (startswith(name, "read"))
                    // this is a write-only call
                    ;
                else if (startswith(name, "emplace") || name == "insert" || name == "erase"
                         || name == "remove" || name == "remove_if" || name == "sort"
                         || name == "push_back" || name == "pop_back" || name == "push_front"
                         || name == "pop_front" || name == "reserve" || name == "resize"
                         || name == "clear" || name == "fill")
                    // write-only modifications to collections
                    ;
                else if (name.find(">>=") != std::string::npos && callExpr->getArg(1) == child)
                    // this is a write-only call
                    ;
                else if (name == "dispose" || name == "disposeAndClear" || name == "swap")
                    // we're abusing the write-only analysis here to look for vars which don't have anything useful
                    // being done to them, so we're ignoring things like std::vector::clear, std::vector::swap,
                    // and VclPtr::disposeAndClear
                    ;
                else
                    bPotentiallyReadFrom = true;
            }
            else
                bPotentiallyReadFrom = true;
            break;
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            // check for calls to ReadXXX() type methods and the operator>>= methods on Any.
            auto callee = getCallee(callExpr);
            if (callee)
            {
                // FIXME perhaps a better solution here would be some kind of SAL_PARAM_WRITEONLY attribute
                // which we could scatter around.
                std::string name = callee->getNameAsString();
                std::transform(name.begin(), name.end(), name.begin(), easytolower);
                if (startswith(name, "read"))
                    // this is a write-only call
                    ;
                else if (name.find(">>=") != std::string::npos && callExpr->getArg(1) == child)
                    // this is a write-only call
                    ;
                else
                    bPotentiallyReadFrom = true;
            }
            else
                bPotentiallyReadFrom = true;
            break;
        }
        else if (auto binaryOp = dyn_cast<BinaryOperator>(parent))
        {
            BinaryOperator::Opcode op = binaryOp->getOpcode();
            // If the child is on the LHS and it is an assignment op, we are obviously not reading from it
            const bool assignmentOp = op == BO_Assign || op == BO_MulAssign || op == BO_DivAssign
                                      || op == BO_RemAssign || op == BO_AddAssign
                                      || op == BO_SubAssign || op == BO_ShlAssign
                                      || op == BO_ShrAssign || op == BO_AndAssign
                                      || op == BO_XorAssign || op == BO_OrAssign;
            if (!(binaryOp->getLHS() == child && assignmentOp))
            {
                bPotentiallyReadFrom = true;
            }
            break;
        }
        else if (isa<ReturnStmt>(parent) || isa<CXXConstructExpr>(parent)
                 || isa<ConditionalOperator>(parent) || isa<SwitchStmt>(parent)
                 || isa<DeclStmt>(parent) || isa<WhileStmt>(parent) || isa<CXXNewExpr>(parent)
                 || isa<ForStmt>(parent) || isa<InitListExpr>(parent)
                 || isa<CXXDependentScopeMemberExpr>(parent) || isa<UnresolvedMemberExpr>(parent)
                 || isa<MaterializeTemporaryExpr>(parent))
        {
            bPotentiallyReadFrom = true;
            break;
        }
        else if (isa<CXXDeleteExpr>(parent) || isa<UnaryExprOrTypeTraitExpr>(parent)
                 || isa<CXXUnresolvedConstructExpr>(parent) || isa<CompoundStmt>(parent)
                 || isa<LabelStmt>(parent) || isa<CXXForRangeStmt>(parent)
                 || isa<CXXTypeidExpr>(parent) || isa<DefaultStmt>(parent)
                 || isa<GCCAsmStmt>(parent) || isa<VAArgExpr>(parent)
#if CLANG_VERSION >= 80000
                 || isa<ConstantExpr>(parent)
#endif
                 || isa<CXXDefaultArgExpr>(parent) || isa<LambdaExpr>(parent))
        {
            break;
        }
        else
        {
            bPotentiallyReadFrom = true;
            bDump = true;
            break;
        }
    } while (true);

    if (bDump)
    {
        report(DiagnosticsEngine::Warning, "oh dear, what can the matter be?",
               compat::getBeginLoc(memberExpr))
            << memberExpr->getSourceRange();
        report(DiagnosticsEngine::Note, "parent over here", compat::getBeginLoc(parent))
            << parent->getSourceRange();
        parent->dump();
        memberExpr->dump();
    }

    MyVarInfo varInfo = niceName(varDecl);
    if (bPotentiallyReadFrom)
    {
        readFromSet.insert(varInfo);
    }
}

void WriteOnlyVars::checkIfWrittenTo(const VarDecl* varDecl, const Expr* memberExpr)
{
    // if we're inside a block that looks like
    //   if (varDecl)
    //       ....
    // then writes to this var don't matter, because unless we find another write to this var, this var is dead
    if (std::find(insideConditionalCheckOfMemberSet.begin(),
                  insideConditionalCheckOfMemberSet.end(), varDecl)
        != insideConditionalCheckOfMemberSet.end())
        return;

    auto parentsRange = compiler.getASTContext().getParents(*memberExpr);
    const Stmt* child = memberExpr;
    const Stmt* parent
        = parentsRange.begin() == parentsRange.end() ? nullptr : parentsRange.begin()->get<Stmt>();
    // walk up the tree until we find something interesting
    bool bPotentiallyWrittenTo = false;
    bool bDump = false;
    auto walkupUp = [&]() {
        child = parent;
        auto parentsRange = compiler.getASTContext().getParents(*parent);
        parent = parentsRange.begin() == parentsRange.end() ? nullptr
                                                            : parentsRange.begin()->get<Stmt>();
    };
    do
    {
        if (!parent)
        {
            // check if we have an expression like
            //    int& r = var;
            auto parentsRange = compiler.getASTContext().getParents(*child);
            if (parentsRange.begin() != parentsRange.end())
            {
                auto varDecl = dyn_cast_or_null<VarDecl>(parentsRange.begin()->get<Decl>());
                // The isImplicit() call is to avoid triggering when we see the vardecl which is part of a for-range statement,
                // which is of type 'T&&' and also an l-value-ref ?
                if (varDecl && !varDecl->isImplicit()
                    && loplugin::TypeCheck(varDecl->getType()).LvalueReference().NonConst())
                {
                    bPotentiallyWrittenTo = true;
                }
            }
            break;
        }
        if (isa<CXXReinterpretCastExpr>(parent))
        {
            // once we see one of these, there is not much useful we can know
            bPotentiallyWrittenTo = true;
            break;
        }
        else if (isa<CastExpr>(parent) || isa<MemberExpr>(parent) || isa<ParenExpr>(parent)
                 || isa<ParenListExpr>(parent) || isa<ArrayInitLoopExpr>(parent)
                 || isa<ExprWithCleanups>(parent))
        {
            walkupUp();
        }
        else if (auto unaryOperator = dyn_cast<UnaryOperator>(parent))
        {
            UnaryOperator::Opcode op = unaryOperator->getOpcode();
            if (op == UO_AddrOf || op == UO_PostInc || op == UO_PostDec || op == UO_PreInc
                || op == UO_PreDec)
            {
                bPotentiallyWrittenTo = true;
            }
            break;
        }
        else if (auto arraySubscriptExpr = dyn_cast<ArraySubscriptExpr>(parent))
        {
            if (arraySubscriptExpr->getIdx() == child)
                break;
            walkupUp();
        }
        else if (auto operatorCallExpr = dyn_cast<CXXOperatorCallExpr>(parent))
        {
            auto callee = getCallee(operatorCallExpr);
            if (callee)
            {
                // if calling a non-const operator on the var
                auto calleeMethodDecl = callee->getAsCXXMethodDecl();
                if (calleeMethodDecl && operatorCallExpr->getArg(0) == child)
                {
                    if (!calleeMethodDecl->isConst())
                        bPotentiallyWrittenTo
                            = checkForWriteWhenUsingCollectionType(calleeMethodDecl);
                }
                else if (IsPassedByNonConst(varDecl, child, operatorCallExpr, *callee))
                {
                    bPotentiallyWrittenTo = true;
                }
            }
            else
                bPotentiallyWrittenTo = true; // conservative, could improve
            break;
        }
        else if (auto cxxMemberCallExpr = dyn_cast<CXXMemberCallExpr>(parent))
        {
            const CXXMethodDecl* calleeMethodDecl = cxxMemberCallExpr->getMethodDecl();
            if (calleeMethodDecl)
            {
                // if calling a non-const method on the var
                const Expr* tmp = dyn_cast<Expr>(child);
                if (tmp->isBoundMemberFunction(compiler.getASTContext()))
                {
                    tmp = dyn_cast<MemberExpr>(tmp)->getBase();
                }
                if (cxxMemberCallExpr->getImplicitObjectArgument() == tmp)
                {
                    if (!calleeMethodDecl->isConst())
                        bPotentiallyWrittenTo
                            = checkForWriteWhenUsingCollectionType(calleeMethodDecl);
                    break;
                }
                else if (IsPassedByNonConst(varDecl, child, cxxMemberCallExpr,
                                            CalleeWrapper(calleeMethodDecl)))
                    bPotentiallyWrittenTo = true;
            }
            else
                bPotentiallyWrittenTo = true; // can happen in templates
            break;
        }
        else if (auto cxxConstructExpr = dyn_cast<CXXConstructExpr>(parent))
        {
            if (IsPassedByNonConst(varDecl, child, cxxConstructExpr,
                                   CalleeWrapper(cxxConstructExpr)))
                bPotentiallyWrittenTo = true;
            break;
        }
        else if (auto callExpr = dyn_cast<CallExpr>(parent))
        {
            auto callee = getCallee(callExpr);
            if (callee)
            {
                if (IsPassedByNonConst(varDecl, child, callExpr, *callee))
                    bPotentiallyWrittenTo = true;
            }
            else
                bPotentiallyWrittenTo = true; // conservative, could improve
            break;
        }
        else if (auto binaryOp = dyn_cast<BinaryOperator>(parent))
        {
            BinaryOperator::Opcode op = binaryOp->getOpcode();
            const bool assignmentOp = op == BO_Assign || op == BO_MulAssign || op == BO_DivAssign
                                      || op == BO_RemAssign || op == BO_AddAssign
                                      || op == BO_SubAssign || op == BO_ShlAssign
                                      || op == BO_ShrAssign || op == BO_AndAssign
                                      || op == BO_XorAssign || op == BO_OrAssign;
            if (assignmentOp)
            {
                if (binaryOp->getLHS() == child)
                    bPotentiallyWrittenTo = true;
                else if (loplugin::TypeCheck(binaryOp->getLHS()->getType())
                             .LvalueReference()
                             .NonConst())
                    // if the LHS is a non-const reference, we could write to the var later on
                    bPotentiallyWrittenTo = true;
            }
            break;
        }
        else if (isa<ReturnStmt>(parent))
        {
            if (insideFunctionDecl)
            {
                auto tc = loplugin::TypeCheck(insideFunctionDecl->getReturnType());
                if (tc.LvalueReference().NonConst())
                    bPotentiallyWrittenTo = true;
            }
            break;
        }
        else if (isa<ConditionalOperator>(parent) || isa<SwitchStmt>(parent)
                 || isa<DeclStmt>(parent) || isa<WhileStmt>(parent) || isa<CXXNewExpr>(parent)
                 || isa<ForStmt>(parent) || isa<InitListExpr>(parent)
                 || isa<CXXDependentScopeMemberExpr>(parent) || isa<UnresolvedMemberExpr>(parent)
                 || isa<MaterializeTemporaryExpr>(parent) || isa<IfStmt>(parent)
                 || isa<DoStmt>(parent) || isa<CXXDeleteExpr>(parent)
                 || isa<UnaryExprOrTypeTraitExpr>(parent) || isa<CXXUnresolvedConstructExpr>(parent)
                 || isa<CompoundStmt>(parent) || isa<LabelStmt>(parent)
                 || isa<CXXForRangeStmt>(parent) || isa<CXXTypeidExpr>(parent)
                 || isa<DefaultStmt>(parent)
#if CLANG_VERSION >= 80000
                 || isa<ConstantExpr>(parent)
#endif
                 || isa<GCCAsmStmt>(parent) || isa<VAArgExpr>(parent)
                 || isa<CXXDefaultArgExpr>(parent) || isa<LambdaExpr>(parent))
        {
            break;
        }
        else
        {
            bPotentiallyWrittenTo = true;
            bDump = true;
            break;
        }
    } while (true);

    if (bDump)
    {
        report(DiagnosticsEngine::Warning, "oh dear2, what can the matter be? writtenTo=%0",
               compat::getBeginLoc(memberExpr))
            << bPotentiallyWrittenTo << memberExpr->getSourceRange();
        if (parent)
        {
            report(DiagnosticsEngine::Note, "parent over here", compat::getBeginLoc(parent))
                << parent->getSourceRange();
            parent->dump();
        }
        memberExpr->dump();
        varDecl->getType()->dump();
    }

    MyVarInfo varInfo = niceName(varDecl);
    if (bPotentiallyWrittenTo)
    {
        writeToSet.insert(varInfo);
    }
}

// return true if this not a collection type, or if it is a collection type, and we might be writing to it
bool WriteOnlyVars::checkForWriteWhenUsingCollectionType(const CXXMethodDecl* calleeMethodDecl)
{
    auto const tc = loplugin::TypeCheck(calleeMethodDecl->getParent());
    bool listLike = false, setLike = false, mapLike = false, cssSequence = false;
    if (tc.Class("deque").StdNamespace() || tc.Class("list").StdNamespace()
        || tc.Class("queue").StdNamespace() || tc.Class("vector").StdNamespace())
    {
        listLike = true;
    }
    else if (tc.Class("set").StdNamespace() || tc.Class("unordered_set").StdNamespace())
    {
        setLike = true;
    }
    else if (tc.Class("map").StdNamespace() || tc.Class("unordered_map").StdNamespace())
    {
        mapLike = true;
    }
    else if (tc.Class("Sequence")
                 .Namespace("uno")
                 .Namespace("star")
                 .Namespace("sun")
                 .Namespace("com")
                 .GlobalNamespace())
    {
        cssSequence = true;
    }
    else
        return true;

    if (calleeMethodDecl->isOverloadedOperator())
    {
        auto oo = calleeMethodDecl->getOverloadedOperator();
        if (oo == OO_Equal)
            return true;
        // This is operator[]. We only care about things that add elements to the collection.
        // if nothing modifies the size of the collection, then nothing useful
        // is stored in it.
        if (listLike)
            return false;
        return true;
    }

    auto name = calleeMethodDecl->getName();
    if (listLike || setLike || mapLike)
    {
        if (name == "reserve" || name == "shrink_to_fit" || name == "clear" || name == "erase"
            || name == "pop_back" || name == "pop_front" || name == "front" || name == "back"
            || name == "data" || name == "remove" || name == "remove_if" || name == "unique"
            || name == "sort" || name == "begin" || name == "end" || name == "rbegin"
            || name == "rend" || name == "at" || name == "find" || name == "equal_range"
            || name == "lower_bound" || name == "upper_bound")
            return false;
    }
    if (cssSequence)
    {
        if (name == "getArray" || name == "begin" || name == "end")
            return false;
    }

    return true;
}

bool WriteOnlyVars::IsPassedByNonConst(const VarDecl* varDecl, const Stmt* child,
                                       CallerWrapper callExpr, CalleeWrapper calleeFunctionDecl)
{
    unsigned len = std::min(callExpr.getNumArgs(), calleeFunctionDecl.getNumParams());
    // if it's an array, passing it by value to a method typically means the
    // callee takes a pointer and can modify the array
    if (varDecl->getType()->isConstantArrayType())
    {
        for (unsigned i = 0; i < len; ++i)
            if (callExpr.getArg(i) == child)
                if (loplugin::TypeCheck(calleeFunctionDecl.getParamType(i)).Pointer().NonConst())
                    return true;
    }
    else
    {
        for (unsigned i = 0; i < len; ++i)
            if (callExpr.getArg(i) == child)
                if (loplugin::TypeCheck(calleeFunctionDecl.getParamType(i))
                        .LvalueReference()
                        .NonConst())
                    return true;
    }
    return false;
}

bool WriteOnlyVars::VisitDeclRefExpr(const DeclRefExpr* declRefExpr)
{
    const Decl* decl = declRefExpr->getDecl();
    const VarDecl* varDecl = dyn_cast<VarDecl>(decl);
    if (!varDecl)
        return true;
    if (varDecl->isImplicit() || isa<ParmVarDecl>(varDecl))
        return true;
    varDecl = varDecl->getCanonicalDecl();
    if (ignoreLocation(varDecl))
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(varDecl->getLocation())))
        return true;

    checkIfReadFrom(varDecl, declRefExpr);

    checkIfWrittenTo(varDecl, declRefExpr);

    return true;
}

llvm::Optional<CalleeWrapper> WriteOnlyVars::getCallee(CallExpr const* callExpr)
{
    FunctionDecl const* functionDecl = callExpr->getDirectCallee();
    if (functionDecl)
        return CalleeWrapper(functionDecl);

    // Extract the functionprototype from a type
    clang::Type const* calleeType = callExpr->getCallee()->getType().getTypePtr();
    if (auto pointerType = calleeType->getUnqualifiedDesugaredType()->getAs<clang::PointerType>())
    {
        if (auto prototype = pointerType->getPointeeType()
                                 ->getUnqualifiedDesugaredType()
                                 ->getAs<FunctionProtoType>())
        {
            return CalleeWrapper(prototype);
        }
    }

    return llvm::Optional<CalleeWrapper>();
}

loplugin::Plugin::Registration<WriteOnlyVars> X("writeonlyvars", false);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
