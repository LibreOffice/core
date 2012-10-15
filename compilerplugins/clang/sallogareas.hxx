/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef SALLOGAREAS_H
#define SALLOGAREAS_H

#include <set>

#include "compileplugin.hxx"

namespace loplugin
{

class SalLogAreas
    : public RecursiveASTVisitor< SalLogAreas >
    , public Plugin
    {
    public:
        explicit SalLogAreas( ASTContext& context );
        void run();
        bool VisitFunctionDecl( FunctionDecl* function );
        bool VisitCallExpr( CallExpr* call );
    private:
        void checkArea( StringRef area, SourceLocation location );
        void readLogAreas();
        const FunctionDecl* inFunction;
        set< string > logAreas;
    };

} // namespace

#endif // SALLOGAREAS_H
