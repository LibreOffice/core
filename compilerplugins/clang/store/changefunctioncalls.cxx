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

Changes all calls to a specific function (after it's been renamed or its
arguments have changed).

This specific example checks for calls to function 'void bar(unsigned int)'
and adds '+ 10' to the argument (as plain text, so if the argument is a more
complex expression, operator precedence may mean the result is actually different).

This can be easily adjusted for different modifications to a function:
- replace CallExpr with CXXOperatorCallExpr or CXXMemberCallExpr
- check different names or arguments
- change getDirectCallee() to getCallee()
- etc.
*/

#include "plugin.hxx"

namespace loplugin
{

class ChangeFunctionCalls
    : public RecursiveASTVisitor< ChangeFunctionCalls >
    , public RewritePlugin
    {
    public:
        explicit ChangeFunctionCalls( CompilerInstance& compiler, Rewriter& rewriter );
        virtual void run() override;
        bool VisitCallExpr( const CallExpr* call );
    };

ChangeFunctionCalls::ChangeFunctionCalls( CompilerInstance& compiler, Rewriter& rewriter )
    : RewritePlugin( compiler, rewriter )
    {
    }

void ChangeFunctionCalls::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool ChangeFunctionCalls::VisitCallExpr( const CallExpr* call )
    {
    if( ignoreLocation( call ))
        return true;
    
    
    
    
    if( const FunctionDecl* func = call->getDirectCallee())
        {
        
        
        
        
        if( func->getNumParams() == 1 && func->getIdentifier() != NULL
            && ( func->getName() == "bar" ))
            {
            string qualifiedName = func->getQualifiedNameAsString();
            if( qualifiedName == "bar" )
                {
                
                
                string arg0 = func->getParamDecl( 0 )->getType().getAsString();
                if( arg0 == "unsigned int" )
                    {
                    insertTextAfterToken( call->getArg( 0 )->getLocEnd(), " + 10" );
                    report( DiagnosticsEngine::Warning, "found", call->getLocStart());
                    }
                }
            }
        }
    return true;
    }

static Plugin::Registration< ChangeFunctionCalls > X( "changefunctioncalls" );

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
