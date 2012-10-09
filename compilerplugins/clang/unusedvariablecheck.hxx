/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef UNUSEDVARIABLECHECK_H
#define UNUSEDVARIABLECHECK_H

#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;

namespace loplugin
{

class UnusedVariableCheck
    : public RecursiveASTVisitor< UnusedVariableCheck >
    {
    public:
        explicit UnusedVariableCheck( ASTContext& context );
        void run();
        bool VisitNamedDecl( NamedDecl* declaration );
    private:
        DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc );
        ASTContext& context;
    };

} // namespace

#endif // UNUSEDVARIABLECHECK_H
