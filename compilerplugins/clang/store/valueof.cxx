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
    // Using getDirectCallee() here means that we find only calls
    // that call the function directly (i.e. not using a pointer, for example).
    // Use getCallee() to include also those :
    //    if( const FunctionDecl* func = dyn_cast_or_null< FunctionDecl >( call->getCalleeDecl()))
    if( const FunctionDecl* func = call->getDirectCallee())
        {
        // Optimize, getQualifiedNameAsString() is reportedly expensive,
        // so first check fast details like number of arguments or the (unqualified)
        // name before checking the fully qualified name.
        // See FunctionDecl for all the API about the function.
        if( func->getIdentifier() != NULL
            && ( func->getName() == "valueOf" ))
            {
            string qualifiedName = func->getQualifiedNameAsString();
            if( qualifiedName == "rtl::OString::valueOf" )
                {
                // Further checks about arguments. Check mainly ParmVarDecl, VarDecl,
                // ValueDecl and QualType for Clang API details.
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
                // Further checks about arguments. Check mainly ParmVarDecl, VarDecl,
                // ValueDecl and QualType for Clang API details.
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
// Explicit casts don't seem to actually always change the type (integer promotion
// takes place first?), so remove also preceding implicit casts:
//     void f( int );
//     char a;
//     f( int( a ));
//    |-CallExpr 0x1a84f20 <line:6:5, col:16> 'void'
//    | |-ImplicitCastExpr 0x1a84f08 <col:5> 'void (*)(int)' <FunctionToPointerDecay>
//    | | `-DeclRefExpr 0x1a84eb8 <col:5> 'void (int)' lvalue Function 0x1a58900 'f' 'void (int)'
//    | `-CXXFunctionalCastExpr 0x1a84e90 <col:8, col:15> 'int' functional cast to int <NoOp>
//    |   `-ImplicitCastExpr 0x1a84e78 <col:13> 'int' <IntegralCast>
//    |     `-ImplicitCastExpr 0x1a84e60 <col:13> 'char' <LValueToRValue>
//    |       `-DeclRefExpr 0x1a58b88 <col:13> 'char' lvalue Var 0x1a58ab0 'a' 'char'
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

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
