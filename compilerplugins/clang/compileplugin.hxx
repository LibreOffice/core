/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef COMPILEPLUGIN_H
#define COMPILEPLUGIN_H

#include <clang/AST/RecursiveASTVisitor.h>

using namespace clang;

namespace loplugin
{

class Plugin
    {
    public:
        explicit Plugin( ASTContext& context );
    protected:
        DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc );
        ASTContext& context;
    };

} // namespace

#endif // COMPILEPLUGIN_H
