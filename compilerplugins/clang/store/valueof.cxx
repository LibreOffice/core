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

/*
This is a rewriter.

Replaces all calls to the deprecated O(U)String::valueOf() .

*/

#include "plugin.hxx"

namespace loplugin
{

class ConvertValueOf
    : public RecursiveASTVisitor< ConvertValueOf >
    , public RewritePlugin
    {
    public:
        explicit ConvertValueOf( CompilerInstance& compiler, Rewriter& rewriter );
        virtual void run() override;
        bool VisitCallExpr( const CallExpr* call );
    private:
        void removeCast( const Expr* arg );
    };

ConvertValueOf::ConvertValueOf( CompilerInstance& compiler, Rewriter& rewriter )
    : RewritePlugin( compiler, rewriter )
    {
    }

void ConvertValueOf::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool ConvertValueOf::VisitCallExpr( const CallExpr* call )
    {
    if( ignoreLocation( call ))
        return true;
    
    
    
    
    if( const FunctionDecl* func = call->getDirectCallee())
        {
        
        
        
        
        if( func->getIdentifier() != NULL
            && ( func->getName() == "valueOf" ))
            {
            string qualifiedName = func->getQualifiedNameAsString();
            if( qualifiedName == "rtl::OString::valueOf" )
                {
                
                
                string arg0 = func->getParamDecl( 0 )->getType().getAsString();
                if( arg0 == "sal_Bool" )
                    replaceText( call->getCallee()->getSourceRange(), "OString::boolean" );
                else if( arg0 == "sal_Char" )
                    replaceText( call->getCallee()->getSourceRange(), "OString" );
                else
                    {
                    replaceText( call->getCallee()->getSourceRange(), "OString::number" );
                    removeCast( call->getArg( 0 ));
                    }
                }
            if( qualifiedName == "rtl::OUString::valueOf" )
                {
                
                
                string arg0 = func->getParamDecl( 0 )->getType().getAsString();
                if( arg0 == "sal_Bool" )
                    replaceText( call->getCallee()->getSourceRange(), "OUString::boolean" );
                else if( arg0 == "sal_Unicode" )
                    replaceText( call->getCallee()->getSourceRange(), "OUString" );
                else
                    {
                    replaceText( call->getCallee()->getSourceRange(), "OUString::number" );
                    removeCast( call->getArg( 0 ));
                    }
                }
            }
        }
    return true;
    }

void ConvertValueOf::removeCast( const Expr* arg )
    {
    arg = arg->IgnoreImpCasts();
    if( const ExplicitCastExpr* cast = dyn_cast< ExplicitCastExpr >( arg ))
        {












        const Expr* castFrom = cast->getSubExpr()->IgnoreImpCasts();
        if( cast->getType()->isIntegerType() && castFrom->getType()->isIntegerType())
            {
            string fromType = castFrom->getType().getAsString();
            if( fromType != "sal_Bool" && fromType != "bool" && fromType != "sal_Char" && fromType != "sal_Unicode" )
                {
                if( const CXXFunctionalCastExpr* funcCast = dyn_cast< CXXFunctionalCastExpr >( cast ))
                    {
                    removeText( CharSourceRange::getCharRange( funcCast->getLocStart(),
                        compiler.getSourceManager().getExpansionLoc( funcCast->getSubExpr()->getLocStart())));
                    removeText( CharSourceRange::getCharRange( locationAfterToken(
                        compiler.getSourceManager().getExpansionLoc( funcCast->getSubExpr()->getLocEnd())),
                        locationAfterToken( funcCast->getLocEnd())));
                    }
                else if( const CXXNamedCastExpr* namedCast = dyn_cast< CXXNamedCastExpr >( cast ))
                    {
                    removeText( CharSourceRange::getCharRange( namedCast->getLocStart(),
                        compiler.getSourceManager().getExpansionLoc( namedCast->getSubExpr()->getLocStart())));
                    removeText( CharSourceRange::getCharRange( locationAfterToken(
                        compiler.getSourceManager().getExpansionLoc( namedCast->getSubExpr()->getLocEnd())),
                    locationAfterToken( namedCast->getLocEnd())));
                    }
                else if( const CStyleCastExpr* cCast = dyn_cast< CStyleCastExpr >( cast ))
                    removeText( SourceRange( cCast->getLocStart(), cCast->getRParenLoc()));
                else
                    abort();
                }
            }
        }
    }

static Plugin::Registration< ConvertValueOf > X( "convertvalueof" );

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
