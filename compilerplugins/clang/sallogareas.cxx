/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"
#include "check.hxx"
#include "compat.hxx"

#include <clang/Lex/Lexer.h>

#include <fstream>
#include <set>

namespace
{

class SalLogAreas
    : public loplugin::FilteringPlugin< SalLogAreas >
    {
    public:
        explicit SalLogAreas( const loplugin::InstantiationData& data )
            : FilteringPlugin(data), inFunction(nullptr) {}

        bool preRun() override {
            return true;
        }

        void run() override {
            if (preRun())
                {
                lastSalDetailLogStreamMacro = SourceLocation();
                TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
                }
        }

        bool VisitFunctionDecl( const FunctionDecl* function );
        bool VisitCallExpr( const CallExpr* call );
    private:
        void checkArea( StringRef area, SourceLocation location );
        void checkAreaSyntax(StringRef area, SourceLocation location);
        void readLogAreas();
        const FunctionDecl* inFunction;
        SourceLocation lastSalDetailLogStreamMacro;
        std::set< std::string > logAreas;
#if 0
        std::string firstSeenLogArea;
        SourceLocation firstSeenLocation;
#endif
    };

/*
This is a compile check.

Check area used in SAL_INFO/SAL_WARN macros against the list in include/sal/log-areas.dox and
report if the area is not listed there. The fix is either use a proper area or add it to the list
if appropriate.
*/

bool SalLogAreas::VisitFunctionDecl( const FunctionDecl* function )
    {
    inFunction = function;
    return true;
    }

bool SalLogAreas::VisitCallExpr( const CallExpr* call )
    {
    if( ignoreLocation( call ))
        return true;
    const FunctionDecl* func = call->getDirectCallee();
    if( !func || !func->getIdentifier())
        return true;

    auto name = func->getName();
    if( !( name == "sal_detail_log" || name == "log" || name == "DbgUnhandledException" || name == "XMLOFF_WARN_UNKNOWN") )
        return true;

    auto tc = loplugin::DeclCheck(func);
    enum class LogCallKind { Sal, DbgUnhandledException};
    LogCallKind kind;
    int areaArgIndex;
    if( tc.Function("XMLOFF_WARN_UNKNOWN") )
        {
        kind = LogCallKind::Sal; // fine
        areaArgIndex = 0;
        }
    else if( tc.Function("sal_detail_log") || tc.Function("log").Namespace("detail").Namespace("sal").GlobalNamespace() )
        {
        kind = LogCallKind::Sal; // fine
        areaArgIndex = 1;
        }
    else if( tc.Function("DbgUnhandledException").GlobalNamespace() )
        {
        kind = LogCallKind::DbgUnhandledException; // ok
        areaArgIndex = 3;
        }
    else
        return true;

    // The SAL_DETAIL_LOG_STREAM macro expands to two calls to sal::detail::log(),
    // so do not warn repeatedly about the same macro (the area->getLocStart() of all the calls
    // from the same macro should be the same).
    if( kind == LogCallKind::Sal )
        {
        SourceLocation expansionLocation = compiler.getSourceManager().getExpansionLoc( compat::getBeginLoc(call));
        if( expansionLocation == lastSalDetailLogStreamMacro )
            return true;
        lastSalDetailLogStreamMacro = expansionLocation;
        };
    if( const clang::StringLiteral* area = dyn_cast< clang::StringLiteral >( call->getArg( areaArgIndex )->IgnoreParenImpCasts()))
        {
        if( area->getKind() == clang::StringLiteral::Ascii )
            checkArea( area->getBytes(), area->getExprLoc());
        else
            report( DiagnosticsEngine::Warning, "unsupported string literal kind (plugin needs fixing?)",
                compat::getBeginLoc(area));
        return true;
        }
    if( loplugin::DeclCheck(inFunction).Function("log").Namespace("detail").Namespace("sal").GlobalNamespace()
        || loplugin::DeclCheck(inFunction).Function("sal_detail_logFormat").GlobalNamespace() )
        return true; // These functions only forward to sal_detail_log, so ok.
    if( loplugin::DeclCheck(inFunction).Function("XMLOFF_WARN_UNKNOWN").GlobalNamespace())
        return true;
    if( call->getArg( areaArgIndex )->isNullPointerConstant( compiler.getASTContext(),
        Expr::NPC_ValueDependentIsNotNull ) != Expr::NPCK_NotNull )
        { // If the area argument is a null pointer, that is allowed only for SAL_DEBUG.
        const SourceManager& source = compiler.getSourceManager();
        for( SourceLocation loc = compat::getBeginLoc(call);
             loc.isMacroID();
             loc = compat::getImmediateExpansionRange(source, loc ).first )
            {
            StringRef inMacro = Lexer::getImmediateMacroName( loc, source, compiler.getLangOpts());
            if( inMacro == "SAL_DEBUG" || inMacro == "SAL_DEBUG_BACKTRACE" )
                return true; // ok
            }
        report( DiagnosticsEngine::Warning, "missing log area",
            compat::getBeginLoc(call->getArg( 1 )->IgnoreParenImpCasts()));
        return true;
        }
    report( DiagnosticsEngine::Warning, "cannot analyse log area argument (plugin needs fixing?)",
        compat::getBeginLoc(call));
    return true;
    }

void SalLogAreas::checkArea( StringRef area, SourceLocation location )
    {
    if( logAreas.empty())
        readLogAreas();
    if( !logAreas.count( area.str() ))
        {
        report( DiagnosticsEngine::Warning, "unknown log area '%0' (check or extend include/sal/log-areas.dox)",
            location ) << area;
        checkAreaSyntax(area, location);
        return;
        }
// don't leave this alive by default, generates too many false+
#if 0
    if (compiler.getSourceManager().isInMainFile(location))
        {
        auto matchpair = [this,area](StringRef p1, StringRef p2) {
            return (area == p1 && firstSeenLogArea == p2) || (area == p2 && firstSeenLogArea == p1);
        };
        // these are "cross-module" log areas
        if (area == "i18n" || area == "lok" || area == "lok.tiledrendering")
            ;
        // these appear to be cross-file log areas
        else if (  area == "chart2"
                || area == "oox.cscode" || area == "oox.csdata"
                || area == "slideshow.verbose"
                || area == "sc.opencl"
                || area == "sc.core.formulagroup"
                || area == "sw.pageframe" || area == "sw.idle" || area == "sw.level2"
                || area == "sw.docappend" || area == "sw.mailmerge"
                || area == "sw.uno"
                || area == "vcl.layout" || area == "vcl.a11y"
                || area == "vcl.gdi.fontmetric" || area == "vcl.opengl"
                || area == "vcl.harfbuzz" || area == "vcl.eventtesting"
                || area == "vcl.schedule" || area == "vcl.unity"
                || area == "xmlsecurity.comp"
                )
            ;
        else if (firstSeenLogArea == "")
            {
                firstSeenLogArea = area;
                firstSeenLocation = location;
            }
        // some modules do this deliberately
        else if (firstSeenLogArea.compare(0, 3, "jfw") == 0
                || firstSeenLogArea.compare(0, 6, "opencl") == 0)
            ;
        // mixing these in the same file seems legitimate
        else if (
                   matchpair("chart2.pie.label.bestfit", "chart2.pie.label.bestfit.inside")
                || matchpair("editeng", "editeng.chaining")
                || matchpair("oox.drawingml", "oox.cscode")
                || matchpair("oox.drawingml", "oox.drawingml.gradient")
                || matchpair("sc.core", "sc.core.grouparealistener")
                || matchpair("sc.orcus", "sc.orcus.condformat")
                || matchpair("sc.orcus", "sc.orcus.style")
                || matchpair("sc.orcus", "sc.orcus.autofilter")
                || matchpair("svx", "svx.chaining")
                || matchpair("sw.ww8", "sw.ww8.level2")
                || matchpair("writerfilter", "writerfilter.profile")
                )
            ;
        else if (firstSeenLogArea != area)
            {
            report( DiagnosticsEngine::Warning, "two different log areas '%0' and '%1' in the same file?",
                location ) << firstSeenLogArea << area;
            report( DiagnosticsEngine::Note, "first area was seen here",
                firstSeenLocation );
            }
        }
#endif
    }

void SalLogAreas::checkAreaSyntax(StringRef area, SourceLocation location) {
    for (std::size_t i = 0;;) {
        std::size_t j = area.find('.', i);
        if (j == StringRef::npos) {
            j = area.size();
        }
        if (j == i) {
            goto bad;
        }
        for (; i != j; ++i) {
            auto c = area[i];
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z'))) {
                goto bad;
            }
        }
        if (j == area.size()) {
            return;
        }
        i = j + 1;
    }
bad:
    report(
        DiagnosticsEngine::Warning,
        "invalid log area syntax '%0'%1 (see include/sal/log.hxx for details)",
        location)
        << area << (location.isValid() ? "" : " in include/sal/log-areas.dox");
}

void SalLogAreas::readLogAreas()
    {
    std::ifstream is( SRCDIR "/include/sal/log-areas.dox" );
    while( is.good())
        {
        std::string line;
        getline( is, line );
        size_t pos = line.find( "@li @c " );
        if( pos != std::string::npos )
            {
            pos += strlen( "@li @c " );
            size_t end = line.find( ' ', pos );
            std::string area;
            if( end == std::string::npos )
                area = line.substr( pos );
            else if( pos != end )
                area = line.substr( pos, end - pos );
            checkAreaSyntax(area, SourceLocation());
            logAreas.insert(area);
            }
        }
    // If you get this error message, you possibly have too old icecream (ICECC_EXTRAFILES is needed).
    if( logAreas.empty())
        report( DiagnosticsEngine::Warning, "error reading log areas" );
    }

static loplugin::Plugin::Registration< SalLogAreas > sallogareas( "sallogareas" );

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
