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
