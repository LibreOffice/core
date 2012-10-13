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

#include "compileplugin.hxx"

namespace loplugin
{

class UnusedVariableCheck
    : public RecursiveASTVisitor< UnusedVariableCheck >
    , public Plugin
    {
    public:
        explicit UnusedVariableCheck( ASTContext& context );
        void run();
        bool VisitVarDecl( VarDecl* var );
    };

} // namespace

#endif // UNUSEDVARIABLECHECK_H
