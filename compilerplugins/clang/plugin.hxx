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

#ifndef PLUGIN_H
#define PLUGIN_H

#include <config_clang.h>

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <unordered_map>

#include <clang/Rewrite/Core/Rewriter.h>

using namespace clang;
using namespace llvm;
using namespace std;

namespace loplugin
{

class PluginHandler;

/**
    Base class for plugins.

    If you want to create a non-rewriter action, inherit from this class. Remember to also
    use Plugin::Registration.
*/
class Plugin
    {
    public:
        struct InstantiationData
            {
            const char* name;
            PluginHandler& handler;
            CompilerInstance& compiler;
            Rewriter* rewriter;
            };
        explicit Plugin( const InstantiationData& data );
        virtual ~Plugin();
        virtual void run() = 0;
        template< typename T > class Registration;
        enum { isPPCallback = false };
        // Returns location right after the end of the token that starts at the given location.
        SourceLocation locationAfterToken( SourceLocation location );
    protected:
        DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc = SourceLocation()) const;
        bool ignoreLocation( SourceLocation loc );
        bool ignoreLocation( const Decl* decl );
        bool ignoreLocation( const Stmt* stmt );
        CompilerInstance& compiler;
        PluginHandler& handler;
        /**
         Returns the parent of the given AST node. Clang's internal AST representation doesn't provide this information,
         it can only provide children, but getting the parent is often useful for inspecting a part of the AST.
        */
        const Stmt* parentStmt( const Stmt* stmt );
        Stmt* parentStmt( Stmt* stmt );
        /**
         Checks if the location is inside an UNO file, more specifically, if it forms part of the URE stable interface,
         which is not allowed to be changed.
        */
        bool isInUnoIncludeFile(SourceLocation spellingLocation) const;
    private:
        static void registerPlugin( Plugin* (*create)( const InstantiationData& ), const char* optionName, bool isPPCallback, bool byDefault );
        template< typename T > static Plugin* createHelper( const InstantiationData& data );
        enum { isRewriter = false };
        const char* name;
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
        explicit RewritePlugin( const InstantiationData& data );
    protected:
        enum RewriteOption
            {
            // This enum allows passing just 'RemoveLineIfEmpty' to functions below.
            // If the resulting line would be completely empty, it'll be removed.
            RemoveLineIfEmpty = 1 << 0,
            // Use this to remove the declaration/statement as a whole, i.e. all whitespace before the statement
            // and the trailing semicolor (is not part of the AST element range itself).
            // The trailing semicolon must be present.
            RemoveWholeStatement = 1 << 1,
            // Removes also all whitespace preceding and following the expression (completely, so that
            // the preceding and following tokens would be right next to each other, follow with insertText( " " )
            // if this is not wanted). Despite the name, indentation whitespace is not removed.
            RemoveAllWhitespace = 1 << 2
            };
        struct RewriteOptions
            : public Rewriter::RewriteOptions
            {
            RewriteOptions();
            explicit RewriteOptions( RewriteOption option );
            const int flags;
            };
        // syntactic sugar to be able to write 'RemoveLineIfEmpty | RemoveWholeStatement'
        friend RewriteOption operator|( RewriteOption option1, RewriteOption option2 );
        // These following insert/remove/replaceText functions map to functions
        // in clang::Rewriter, with these differences:
        // - they (more intuitively) return false on failure rather than true
        // - they report a warning when the change cannot be done
        // - There are more options for easier removal of surroundings of a statement/expression.
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
        Rewriter* rewriter;
    private:
        template< typename T > friend class Plugin::Registration;
        enum { isRewriter = true };
        bool reportEditFailure( SourceLocation loc );
        bool adjustRangeForOptions( CharSourceRange* range, RewriteOptions options );
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
        Registration( const char* optionName, bool byDefault = !T::isRewriter );
    };

class RegistrationCreate
    {
    public:
        template< typename T, bool > static T* create( const Plugin::InstantiationData& data );
    };

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
    // Invalid location can happen at least for ImplicitCastExpr of
    // ImplicitParam 'self' in Objective C method declarations:
    return stmt->getLocStart().isValid() && ignoreLocation( stmt->getLocStart());
    }

template< typename T >
Plugin* Plugin::createHelper( const InstantiationData& data )
    {
    return new T( data );
    }

template< typename T >
inline
Plugin::Registration< T >::Registration( const char* optionName, bool byDefault )
    {
    registerPlugin( &T::template createHelper< T >, optionName, T::isPPCallback, byDefault );
    }

inline
RewritePlugin::RewriteOptions::RewriteOptions()
    : flags( 0 )
    {
    }

inline
RewritePlugin::RewriteOptions::RewriteOptions( RewriteOption option )
    : flags( option )
    {
    // Note that 'flags' stores also RemoveLineIfEmpty, it must be kept in sync with the base class.
    if( flags & RewritePlugin::RemoveLineIfEmpty )
        this->RemoveLineIfEmpty = true;
    }

inline
RewritePlugin::RewriteOption operator|( RewritePlugin::RewriteOption option1, RewritePlugin::RewriteOption option2 )
    {
    return static_cast< RewritePlugin::RewriteOption >( int( option1 ) | int( option2 ));
    }

} // namespace

#endif // COMPILEPLUGIN_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
