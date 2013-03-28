/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef TUTORIAL2_H
#define TUTORIAL2_H

#include "plugin.hxx"

namespace loplugin
{

// The same like for Tutorial1.
class Tutorial2
    : public RecursiveASTVisitor< Tutorial2 >
    , public Plugin
    {
    public:
        Tutorial2( CompilerInstance& compiler );
        virtual void run();
        // Will be called for every if statement.
        bool VisitIfStmt( IfStmt* ifstmt );
    private:
        // Helper function to check if the statement is 'return false;'.
        bool isReturnFalse( const Stmt* stmt );
    };

} // namespace

#endif // POSTFIXINCREMENTFIX_H

