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
        if( func->getNumParams() == 1 && func->getIdentifier() != NULL
            && ( func->getName() == "bar" ))
            {
            string qualifiedName = func->getQualifiedNameAsString();
            if( qualifiedName == "bar" )
                {
                // Further checks about arguments. Check mainly ParmVarDecl, VarDecl,
                // ValueDecl and QualType for Clang API details.
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

} // namespace
