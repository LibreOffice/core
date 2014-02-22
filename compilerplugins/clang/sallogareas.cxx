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

#include "sallogareas.hxx"

#include <clang/Lex/Lexer.h>

#include <fstream>

namespace loplugin
{

/*
This is a compile check.

Check area used in SAL_INFO/SAL_WARN macros against the list in include/sal/log-areas.dox and
report if the area is not listed there. The fix is either use a proper area or add it to the list
if appropriate.
*/

SalLogAreas::SalLogAreas( const InstantiationData& data )
    : Plugin( data )
    {
    }

void SalLogAreas::run()
    {
    inFunction = NULL;
    lastSalDetailLogStreamMacro = SourceLocation();
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool SalLogAreas::VisitFunctionDecl( const FunctionDecl* function )
    {
    inFunction = function;
    return true;
    }

bool SalLogAreas::VisitCallExpr( const CallExpr* call )
    {
    if( ignoreLocation( call ))
        return true;
    if( const FunctionDecl* func = call->getDirectCallee())
        {
        
        if( func->getNumParams() == 4 && func->getIdentifier() != NULL
            && ( func->getName() == "sal_detail_log" || func->getName() == "log" ))
            {
            string qualifiedName = func->getQualifiedNameAsString();
            if( qualifiedName == "sal_detail_log" || qualifiedName == "sal::detail::log" )
                {
                
                
                
                SourceLocation expansionLocation = compiler.getSourceManager().getExpansionLoc( call->getLocStart());
                if( expansionLocation == lastSalDetailLogStreamMacro )
                    return true;
                lastSalDetailLogStreamMacro = expansionLocation;
                if( const StringLiteral* area = dyn_cast< StringLiteral >( call->getArg( 1 )->IgnoreParenImpCasts()))
                    {
                    if( area->getKind() == StringLiteral::Ascii )
                        checkArea( area->getBytes(), area->getExprLoc());
                    else
                        report( DiagnosticsEngine::Warning, "unsupported string literal kind (plugin needs fixing?)",
                            area->getLocStart());
                    return true;
                    }
                if( inFunction->getQualifiedNameAsString() == "sal::detail::log" )
                    return true; 
                if( call->getArg( 1 )->isNullPointerConstant( compiler.getASTContext(),
                    Expr::NPC_ValueDependentIsNotNull ) != Expr::NPCK_NotNull )
                    { 
                    const SourceManager& source = compiler.getSourceManager();
                    for( SourceLocation loc = call->getLocStart();
                         loc.isMacroID();
                         loc = source.getImmediateExpansionRange( loc ).first )
                        {
                        StringRef inMacro = Lexer::getImmediateMacroName( loc, source, compiler.getLangOpts());
                        if( inMacro == "SAL_DEBUG" )
                            return true; 
                        }
                    report( DiagnosticsEngine::Warning, "missing log area",
                        call->getArg( 1 )->IgnoreParenImpCasts()->getLocStart());
                    return true;
                    }
                report( DiagnosticsEngine::Warning, "cannot analyse log area argument (plugin needs fixing?)",
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
        report( DiagnosticsEngine::Warning, "unknown log area '%0' (check or extend include/sal/log-areas.dox)",
            location ) << area;
        }
    }

void SalLogAreas::readLogAreas()
    {
    ifstream is( SRCDIR "/include/sal/log-areas.dox" );
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
    
    if( logAreas.empty())
        report( DiagnosticsEngine::Warning, "error reading log areas" );
    }

static Plugin::Registration< SalLogAreas > X( "sallogareas" );

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
