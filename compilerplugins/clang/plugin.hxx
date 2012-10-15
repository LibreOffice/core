/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef PLUGIN_H
#define PLUGIN_H

#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Rewrite/Rewriter.h>

using namespace clang;
using namespace llvm;
using namespace std;

namespace loplugin
{

class Plugin
    {
    public:
        explicit Plugin( ASTContext& context );
    protected:
        DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc = SourceLocation());
        bool ignoreLocation( SourceLocation loc );
        bool ignoreLocation( const Decl* decl );
        bool ignoreLocation( const Stmt* stmt );
        ASTContext& context;
    };

class RewritePlugin
    : public Plugin
    {
    public:
        explicit RewritePlugin( ASTContext& context, Rewriter& rewriter );
    protected:
        typedef Rewriter::RewriteOptions RewriteOptions;
        // These following insert/remove/replaceText functions map to functions
        // in clang::Rewriter, with two differences:
        // - they (more intuitively) return false on failure rather than true
        // - they report a warning when the change cannot be done
        bool insertText( SourceLocation Loc, StringRef Str,
            bool InsertAfter = true, bool indentNewLines = false );
        bool insertTextAfter( SourceLocation Loc, StringRef Str );
        bool insertTextAfterToken( SourceLocation Loc, StringRef Str );
        bool insertTextBefore( SourceLocation Loc, StringRef Str );
        bool removeText( SourceLocation Start, unsigned Length, RewriteOptions opts = RewriteOptions());
        bool removeText( CharSourceRange range, RewriteOptions opts = RewriteOptions());
        bool removeText( SourceRange range, RewriteOptions opts = RewriteOptions());
        bool replaceText( SourceLocation Start, unsigned OrigLength, StringRef NewStr );
        bool replaceText( SourceRange range, StringRef NewStr );
        bool replaceText( SourceRange range, SourceRange replacementRange );
        Rewriter& rewriter;
    private:
        bool reportEditFailure( SourceLocation loc );
    };

inline
bool Plugin::ignoreLocation( const Decl* decl )
    {
    return ignoreLocation( decl->getLocation());
    }

inline
bool Plugin::ignoreLocation( const Stmt* stmt )
    {
    return ignoreLocation( stmt->getLocStart());
    }

} // namespace

#endif // COMPILEPLUGIN_H
