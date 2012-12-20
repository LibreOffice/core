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

#if __clang_major__ < 3 || __clang_major__ == 3 && __clang_minor__ < 2
#include <clang/Rewrite/Rewriter.h>
#else
#include <clang/Rewrite/Core/Rewriter.h>
#endif

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
        // This enum allows passing just 'RemoveLineIfEmpty' to functions below.
        enum RemoveLineIfEmpty_t { RemoveLineIfEmpty };
        // Use this to remove the declaration/statement as a whole, i.e. all whitespace before the statement
        // and the trailing semicolor (is not part of the AST element range itself).
        // The trailing semicolon must be present.
        enum RemoveWholeStatement_t { RemoveWholeStatement };
        enum RemoveLineIfEmptyAndWholeStatement_t { RemoveLineIfEmptyAndWholeStatement };
        // syntactic sugar to be able to write 'RemoveLineIfEmpty | RemoveWholeStatement'
        friend RemoveLineIfEmptyAndWholeStatement_t operator|( RemoveLineIfEmpty_t, RemoveWholeStatement_t )
            { return RemoveLineIfEmptyAndWholeStatement; }
        struct RewriteOptions
            : public Rewriter::RewriteOptions
            {
            RewriteOptions() : RemoveWholeStatement( false ) {} // default
            RewriteOptions( RemoveLineIfEmpty_t ) : RemoveWholeStatement( false ) { RemoveLineIfEmpty = true; }
            RewriteOptions( RemoveWholeStatement_t ) : RemoveWholeStatement( true ) {}
            RewriteOptions( RemoveLineIfEmptyAndWholeStatement_t ) : RemoveWholeStatement( true ) { RemoveLineIfEmpty = true; }
            bool RemoveWholeStatement;
            };
        // These following insert/remove/replaceText functions map to functions
        // in clang::Rewriter, with these differences:
        // - they (more intuitively) return false on failure rather than true
        // - they report a warning when the change cannot be done
        // - There is RemoveWholeStatement to also remove the trailing semicolon when removing (must be there)
        //   and al preceding whitespace.
        bool insertText( SourceLocation Loc, StringRef Str,
            bool InsertAfter = true, bool indentNewLines = false );
        bool insertTextAfter( SourceLocation Loc, StringRef Str );
        bool insertTextAfterToken( SourceLocation Loc, StringRef Str );
        bool insertTextBefore( SourceLocation Loc, StringRef Str );
        bool removeText( SourceLocation Start, unsigned Length, RewriteOptions opts = RewriteOptions());
        // CharSourceRange not supported, unless really needed, as it makes RemoveSemicolon more complicated
        //bool removeText( CharSourceRange range, RewriteOptions opts = RewriteOptions());
        bool removeText( SourceRange range, RewriteOptions opts = RewriteOptions());
        bool replaceText( SourceLocation Start, unsigned OrigLength, StringRef NewStr );
        bool replaceText( SourceRange range, StringRef NewStr );
        bool replaceText( SourceRange range, SourceRange replacementRange );
        Rewriter& rewriter;
    private:
        bool reportEditFailure( SourceLocation loc );
        bool adjustForWholeStatement( SourceRange* range );
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
