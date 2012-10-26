/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include "sallogareas.hxx"

#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>

#include <fstream>

namespace loplugin
{

/*
This is a compile check.

Check that areas used in SAL_LOG/SAL_WARN are listed in sal/inc/sal/log-areas.dox .
*/

SalLogAreas::SalLogAreas( ASTContext& context )
    : Plugin( context )
    {
    }

void SalLogAreas::run()
    {
    inFunction = NULL;
    lastSalDetailLogStreamMacro = SourceLocation();
    TraverseDecl( context.getTranslationUnitDecl());
    }

bool SalLogAreas::VisitFunctionDecl( FunctionDecl* function )
    {
    inFunction = function;
    return true;
    }

bool SalLogAreas::VisitCallExpr( CallExpr* call )
    {
    if( ignoreLocation( call ))
        return true;
    if( FunctionDecl* func = call->getDirectCallee())
        {
        // Optimize, getQualifiedNameAsString() is reportedly expensive.
        if( func->getNumParams() == 4 && func->getIdentifier() != NULL
            && ( func->getName() == "sal_detail_log" || func->getName() == "log" ))
            {
            string qualifiedName = func->getQualifiedNameAsString();
            if( qualifiedName == "sal_detail_log" || qualifiedName == "sal::detail::log" )
                {
                // The SAL_DETAIL_LOG_STREAM macro expands to two calls to sal::detail::log(),
                // so do not warn repeatedly about the same macro (the area->getLocStart() of all the calls
                // from the same macro should be the same).
                SourceLocation expansionLocation = context.getSourceManager().getExpansionLoc( call->getLocStart());
                if( expansionLocation == lastSalDetailLogStreamMacro )
                    return true;
                lastSalDetailLogStreamMacro = expansionLocation;
                if( const StringLiteral* area = dyn_cast< StringLiteral >( call->getArg( 1 )->IgnoreParenImpCasts()))
                    {
                    if( area->getKind() == StringLiteral::Ascii )
                        checkArea( area->getBytes(), area->getExprLoc());
                    else
                        report( DiagnosticsEngine::Warning, "unsupported string literal kind (plugin needs fixing?) [loplugin]",
                            area->getLocStart());
                    return true;
                    }
                if( inFunction->getQualifiedNameAsString() == "sal::detail::log" )
                    return true; // This function only forwards to sal_detail_log, so ok.
                if( call->getArg( 1 )->isNullPointerConstant( context, Expr::NPC_ValueDependentIsNotNull ) != Expr::NPCK_NotNull )
                    { // If the area argument is a null pointer, that is allowed only for SAL_DEBUG.
                    const SourceManager& source = context.getSourceManager();
                    for( SourceLocation loc = call->getLocStart();
                         loc.isMacroID();
                         loc = source.getImmediateExpansionRange( loc ).first )
                        {
                        StringRef inMacro = Lexer::getImmediateMacroName( loc, source, context.getLangOpts());
                        if( inMacro == "SAL_DEBUG" )
                            return true; // ok
                        }
                    report( DiagnosticsEngine::Warning, "missing log area [loplugin]",
                        call->getArg( 1 )->IgnoreParenImpCasts()->getLocStart());
                    return true;
                    }
                report( DiagnosticsEngine::Warning, "cannot analyse log area argument (plugin needs fixing?) [loplugin]",
                    call->getLocStart());
                }
            }
        }
    return true;
    }

void SalLogAreas::checkArea( StringRef area, SourceLocation location )
    {
    if( logAreas.empty())
        readLogAreas();
    if( !logAreas.count( area ))
        {
        report( DiagnosticsEngine::Warning, "unknown log area '%0' (check or extend sal/inc/sal/log-areas.dox) [loplugin]",
            location ) << area;
        }
    }

void SalLogAreas::readLogAreas()
    {
#define STRINGIFY2( s ) #s
#define STRINGIFY( s ) STRINGIFY2( s )
    ifstream is( STRINGIFY( SRCDIR ) "/sal/inc/sal/log-areas.dox" );
#undef STRINGIFY
#undef STRINGIFY2
    while( is.good())
        {
        string line;
        getline( is, line );
        size_t pos = line.find( "@li @c " );
        if( pos != string::npos )
            {
            pos += strlen( "@li @c " );
            size_t end = line.find( ' ', pos );
            if( end == string::npos )
                logAreas.insert( line.substr( pos ));
            else if( pos != end )
                logAreas.insert( line.substr( pos, end - pos ));
            }
        }
    // If you get this error message, you possibly have too old icecream (ICECC_EXTRAFILES is needed).
    if( logAreas.empty())
        report( DiagnosticsEngine::Warning, "error reading log areas [loplugin]" );
    }

} // namespace
