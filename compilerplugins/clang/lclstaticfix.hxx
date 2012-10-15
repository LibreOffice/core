/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef LCLSTATICFIX_H
#define LCLSTATICFIX_H

#include "plugin.hxx"

namespace loplugin
{

class LclStaticFix
    : public RecursiveASTVisitor< LclStaticFix >
    , public RewritePlugin
    {
    public:
        explicit LclStaticFix( ASTContext& context, Rewriter& rewriter );
        void run();
        bool VisitFunctionDecl( FunctionDecl* declaration );
    };

} // namespace

#endif // POSTFIXINCREMENTFIX_H

