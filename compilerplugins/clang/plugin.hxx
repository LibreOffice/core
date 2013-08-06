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

#include <config_clang.h>

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <set>
#include <unordered_map>

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

/**
    Base class for plugins.

    If you want to create a non-rewriter action, inherit from this class. Remember to also
    use Plugin::Registration.
*/
class Plugin
    {
    public:
        explicit Plugin( CompilerInstance& compiler );
        virtual ~Plugin();
        virtual void run() = 0;
        template< typename T > class Registration;
        enum { isPPCallback = false };
        DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc = SourceLocation());
        static DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message,
            CompilerInstance& compiler, SourceLocation loc = SourceLocation());
    protected:
        bool ignoreLocation( SourceLocation loc );
        bool ignoreLocation( const Decl* decl );
        bool ignoreLocation( const Stmt* stmt );
        CompilerInstance& compiler;
        /**
         Returns the parent of the given AST node. Clang's internal AST representation doesn't provide this information,
         it can only provide children, but getting the parent is often useful for inspecting a part of the AST.
        */
        const Stmt* parentStmt( const Stmt* stmt );
        Stmt* parentStmt( Stmt* stmt );
    private:
        static void registerPlugin( Plugin* (*create)( CompilerInstance&, Rewriter& ), const char* optionName, bool isRewriter, bool isPPCallback );
        template< typename T > static Plugin* createHelper( CompilerInstance& compiler, Rewriter& rewriter );
        enum { isRewriter = false };
        static unordered_map< const Stmt*, const Stmt* > parents;
        static void buildParents( CompilerInstance& compiler );
    };

/**
    Base class for rewriter plugins.

    Remember to also use Plugin::Registration.
*/
class RewritePlugin
    : public Plugin
    {
    public:
        explicit RewritePlugin( CompilerInstance& compiler, Rewriter& rewriter );
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
        // CharSourceRange not supported, unless really needed, as it needs handling for RemoveWholeStatement.
        //bool removeText( CharSourceRange range, RewriteOptions opts = RewriteOptions());
        bool removeText( SourceRange range, RewriteOptions opts = RewriteOptions());
        bool replaceText( SourceLocation Start, unsigned OrigLength, StringRef NewStr );
        bool replaceText( SourceRange range, StringRef NewStr );
        bool replaceText( SourceRange range, SourceRange replacementRange );
        Rewriter& rewriter;
    private:
        template< typename T > friend class Plugin::Registration;
        template< typename T > static Plugin* createHelper( CompilerInstance& compiler, Rewriter& rewriter );
        enum { isRewriter = true };
        bool reportEditFailure( SourceLocation loc );
        bool adjustForWholeStatement( SourceRange* range );
        set< SourceLocation > removals;
    };

/**
    Plugin registration helper.

    If you create a new helper class, create also an instance of this class to automatically register it.
    The passed argument is name of the plugin, used for explicitly invoking rewriter plugins
    (it is ignored for non-rewriter plugins).

    @code
    static Plugin::Registration< NameOfClass > X( "nameofclass" );
    @endcode
*/
template< typename T >
class Plugin::Registration
    {
    public:
        Registration( const char* optionName );
    };

class RegistrationCreate
    {
    public:
        template< typename T, bool > static T* create( CompilerInstance& compiler, Rewriter& rewriter );
    };

/////

inline
Plugin::~Plugin()
    {
    }

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

template< typename T >
Plugin* Plugin::createHelper( CompilerInstance& compiler, Rewriter& )
    {
    return new T( compiler );
    }

template< typename T >
Plugin* RewritePlugin::createHelper( CompilerInstance& compiler, Rewriter& rewriter )
    {
    return new T( compiler, rewriter );
    }

template< typename T >
inline
Plugin::Registration< T >::Registration( const char* optionName )
    {
    registerPlugin( &T::template createHelper< T >, optionName, T::isRewriter, T::isPPCallback );
    }

} // namespace

#endif // COMPILEPLUGIN_H
