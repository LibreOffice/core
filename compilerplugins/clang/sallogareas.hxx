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

#ifndef SALLOGAREAS_H
#define SALLOGAREAS_H

#include <set>

#include "plugin.hxx"

namespace loplugin
{

class SalLogAreas
    : public RecursiveASTVisitor< SalLogAreas >
    , public Plugin
    {
    public:
        explicit SalLogAreas( CompilerInstance& compiler );
        virtual void run() override;
        bool VisitFunctionDecl( const FunctionDecl* function );
        bool VisitCallExpr( const CallExpr* call );
    private:
        void checkArea( StringRef area, SourceLocation location );
        void readLogAreas();
        const FunctionDecl* inFunction;
        SourceLocation lastSalDetailLogStreamMacro;
        set< string > logAreas;
    };

} // namespace

#endif // SALLOGAREAS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
