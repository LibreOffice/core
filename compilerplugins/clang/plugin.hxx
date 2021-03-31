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

#pragma once

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/Basic/FileManager.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <unordered_map>
#include <vector>

#include <clang/Rewrite/Core/Rewriter.h>

#include "compat.hxx"
#include "pluginhandler.hxx"

using namespace clang;
using namespace llvm;

namespace loplugin
{

struct InstantiationData
{
    const char* name;
    PluginHandler& handler;
    CompilerInstance& compiler;
    Rewriter* rewriter;
};

/**
    Base class for plugins.

    If you want to create a non-rewriter action, inherit from this class. Remember to also
    use Plugin::Registration.
*/
class Plugin
{
public:
    explicit Plugin( const InstantiationData& data );
    virtual ~Plugin() {}
    // The main function of the plugin.
    // Note that for shared plugins, its functionality must be split into preRun() and postRun(),
    // see sharedvisitor/generator.cxx .
    virtual void run() = 0;
    // Should be called from run() before TraverseDecl().
    // If returns false, run() should not do anything.
    virtual bool preRun() { return true; }
    virtual void postRun() {}
    template< typename T > class Registration;
    // Returns location right after the end of the token that starts at the given location.
    SourceLocation locationAfterToken( SourceLocation location );
    virtual bool setSharedPlugin( Plugin* /*plugin*/, const char* /*name*/ ) { return false; }
    enum { isPPCallback = false };
    enum { isSharedPlugin = false };
protected:
    DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc = SourceLocation()) const;
    bool ignoreLocation( SourceLocation loc ) const
    { return handler.ignoreLocation(loc); }
    bool ignoreLocation( const Decl* decl ) const;
    bool ignoreLocation( const Stmt* stmt ) const;
    bool ignoreLocation(TypeLoc tloc) const;
    CompilerInstance& compiler;
    PluginHandler& handler;
    /**
     Returns the parent of the given AST node. Clang's internal AST representation doesn't provide this information,
     it can only provide children, but getting the parent is often useful for inspecting a part of the AST.
    */
    const Stmt* getParentStmt( const Stmt* stmt );
    Stmt* getParentStmt( Stmt* stmt );
    const FunctionDecl* getParentFunctionDecl( const Stmt* stmt );

    /**
     Get filename of the given location. Use this instead of SourceManager::getFilename(), as that one
     does not handle source with expanded #inline directives (used by Icecream for remote compilation).
    */
    StringRef getFilenameOfLocation(SourceLocation spellingLocation) const;
    /**
     Checks if the location is inside a UNO file, more specifically, if it forms part of the URE stable interface,
     which is not allowed to be changed.
    */
    bool isInUnoIncludeFile(SourceLocation spellingLocation) const;
    bool isInUnoIncludeFile(const FunctionDecl*) const;

    bool isDebugMode() const { return handler.isDebugMode(); }

    static bool isUnitTestMode();

    bool containsPreprocessingConditionalInclusion(SourceRange range);

    enum class IdenticalDefaultArgumentsResult { No, Yes, Maybe };
    IdenticalDefaultArgumentsResult checkIdenticalDefaultArguments(
        Expr const * argument1, Expr const * argument2);

private:
    static void registerPlugin( Plugin* (*create)( const InstantiationData& ), const char* optionName,
        bool isPPCallback, bool isSharedPlugin, bool byDefault );
    template< typename T > static Plugin* createHelper( const InstantiationData& data );
    bool evaluate(const Expr* expr, APSInt& x);

    enum { isRewriter = false };
    const char* name;
};

template<typename Derived>
class FilteringPlugin : public RecursiveASTVisitor<Derived>, public Plugin
{
public:
    explicit FilteringPlugin( const InstantiationData& data ) : Plugin(data) {}

    bool TraverseNamespaceDecl(NamespaceDecl * decl) {
        if (ignoreLocation(compat::getBeginLoc(decl)))
            return true;
        return RecursiveASTVisitor<Derived>::TraverseNamespaceDecl(decl);
    }
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
        RemoveLineIfEmpty    = 1 << 0,
        // Use this to remove the declaration/statement as a whole, i.e. all whitespace before the statement
        // and the trailing semicolon (is not part of the AST element range itself).
        // The trailing semicolon must be present.
        RemoveWholeStatement = 1 << 1,
        // Removes also all whitespace preceding and following the expression (completely, so that
        // the preceding and following tokens would be right next to each other, follow with insertText( " " )
        // if this is not wanted). Despite the name, indentation whitespace is not removed.
        RemoveAllWhitespace  = 1 << 2
    };
    struct RewriteOptions
        : public Rewriter::RewriteOptions
    {
        RewriteOptions() : flags( 0 ) {}
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
    bool wouldRewriteWorkdir(SourceLocation loc);
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
    template< typename T, bool > static T* create( const InstantiationData& data );
};

inline
bool Plugin::ignoreLocation( const Decl* decl ) const
{
    return ignoreLocation( decl->getLocation());
}

inline
bool Plugin::ignoreLocation( const Stmt* stmt ) const
{
    // Invalid location can happen at least for ImplicitCastExpr of
    // ImplicitParam 'self' in Objective C method declarations:
    return compat::getBeginLoc(stmt).isValid() && ignoreLocation( compat::getBeginLoc(stmt));
}

inline bool Plugin::ignoreLocation(TypeLoc tloc) const
{
    // Invalid locations appear to happen at least with Clang 5.0.2 (but no longer with at least
    // recent Clang 10 trunk):
    auto const loc = tloc.getBeginLoc();
    return loc.isValid() && ignoreLocation(loc);
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
    registerPlugin( &T::template createHelper< T >, optionName, T::isPPCallback, T::isSharedPlugin, byDefault );
}

inline
RewritePlugin::RewriteOptions::RewriteOptions( RewriteOption option )
    : flags( option )
{
    // Note that 'flags' stores also RemoveLineIfEmpty, it must be kept in sync with the base class.
    if( flags & RewritePlugin::RemoveLineIfEmpty )
        RemoveLineIfEmpty = true;
}

inline
RewritePlugin::RewriteOption operator|( RewritePlugin::RewriteOption option1, RewritePlugin::RewriteOption option2 )
{
    return static_cast< RewritePlugin::RewriteOption >( int( option1 ) | int( option2 ));
}

template<typename Derived>
class FilteringRewritePlugin : public RecursiveASTVisitor<Derived>, public RewritePlugin
{
public:
    explicit FilteringRewritePlugin( const InstantiationData& data ) : RewritePlugin(data) {}

    bool TraverseNamespaceDecl(NamespaceDecl * decl) {
        if (ignoreLocation(compat::getBeginLoc(decl)))
            return true;
        return RecursiveASTVisitor<Derived>::TraverseNamespaceDecl(decl);
    }
};

void normalizeDotDotInFilePath(std::string&);

// Same as pathname.startswith(prefix), except on Windows, where pathname and
// prefix may also contain backslashes:
bool hasPathnamePrefix(StringRef pathname, StringRef prefix);

// Same as pathname == other, except on Windows, where pathname and other may
// also contain backslashes:
bool isSamePathname(StringRef pathname, StringRef other);

// It appears that, given a function declaration, there is no way to determine
// the language linkage of the function's type, only of the function's name
// (via FunctionDecl::isExternC); however, in a case like
//
//   extern "C" { static void f(); }
//
// the function's name does not have C language linkage while the function's
// type does (as clarified in C++11 [decl.link]); cf. <http://clang-developers.
// 42468.n3.nabble.com/Language-linkage-of-function-type-tt4037248.html>
// "Language linkage of function type":
bool hasCLanguageLinkageType(FunctionDecl const * decl);

// Count the number of times the base class is present in the subclass hierarchy
//
int derivedFromCount(clang::QualType subclassType, clang::QualType baseclassType);
int derivedFromCount(const CXXRecordDecl* subtypeRecord, const CXXRecordDecl* baseRecord);

// It looks like Clang wrongly implements DR 4
// (<http://www.open-std.org/jtc1/sc22/wg21/docs/cwg_defects.html#4>) and treats
// a variable declared in an 'extern "..." {...}'-style linkage-specification as
// if it contained the 'extern' specifier:
bool hasExternalLinkage(VarDecl const * decl);

bool isSmartPointerType(const Expr*);

const Decl* getFunctionDeclContext(ASTContext& context, const Stmt* stmt);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
