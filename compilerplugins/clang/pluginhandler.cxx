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

#include "compat.hxx"
#include "pluginhandler.hxx"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Lex/PPCallbacks.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

/*
This source file manages all plugin actions. It is not necessary to modify this
file when adding new actions.
*/

namespace
{

bool isPrefix( const string& prefix, const string& full)
    {
    return full.compare(0, prefix.size(), prefix) == 0;
    }

}

namespace loplugin
{

struct PluginData
    {
    Plugin* (*create)( const Plugin::InstantiationData& );
    Plugin* object;
    const char* optionName;
    bool isPPCallback;
    bool byDefault;
    };

const int MAX_PLUGINS = 100;
static PluginData plugins[ MAX_PLUGINS ];
static int pluginCount = 0;
static bool pluginObjectsCreated = false;

PluginHandler::PluginHandler( CompilerInstance& compiler, const vector< string >& args )
    : compiler( compiler )
    , rewriter( compiler.getSourceManager(), compiler.getLangOpts())
    , scope( "mainfile" )
    {
    set< string > rewriters;
    for( vector< string >::const_iterator it = args.begin();
         it != args.end();
         ++it )
        {
        if( it->size() >= 2 && (*it)[ 0 ] == '-' && (*it)[ 1 ] == '-' )
            handleOption( it->substr( 2 ));
        else
            rewriters.insert( *it );
        }
    createPlugins( rewriters );
    pluginObjectsCreated = true;
    }

PluginHandler::~PluginHandler()
    {
    for( int i = 0;
         i < pluginCount;
         ++i )
        if( plugins[ i ].object != NULL )
            {
            // PPCallbacks is owned by preprocessor object, don't delete those
            if( !plugins[ i ].isPPCallback )
                delete plugins[ i ].object;
            }
    }

void PluginHandler::handleOption( const string& option )
    {
    if( option.substr( 0, 6 ) == "scope=" )
        {
        scope = option.substr( 6 );
        if( scope == "mainfile" || scope == "all" )
            ; // ok
        else
            {
            struct stat st;
            if( stat(( SRCDIR "/" + scope ).c_str(), &st ) != 0 || !S_ISDIR( st.st_mode ))
                report( DiagnosticsEngine::Fatal, "unknown scope %0 (no such module directory)" ) << scope;
            }
        }
    else if( option.substr( 0, 14 ) == "warnings-only=" )
        {
        warningsOnly = option.substr(14);
        }
    else
        report( DiagnosticsEngine::Fatal, "unknown option %0" ) << option;
    }

void PluginHandler::createPlugins( set< string > rewriters )
    {
    for( int i = 0;
         i < pluginCount;
         ++i )
        {
        if( rewriters.erase( plugins[i].optionName ) != 0 )
            plugins[ i ].object = plugins[ i ].create( Plugin::InstantiationData { plugins[ i ].optionName, *this, compiler, &rewriter } );
        else if( plugins[ i ].byDefault )
            plugins[ i ].object = plugins[ i ].create( Plugin::InstantiationData { plugins[ i ].optionName, *this, compiler, NULL } );
        }
    for( auto r: rewriters )
        report( DiagnosticsEngine::Fatal, "unknown plugin tool %0" ) << r;
    }

void PluginHandler::registerPlugin( Plugin* (*create)( const Plugin::InstantiationData& ), const char* optionName, bool isPPCallback, bool byDefault )
    {
    assert( !pluginObjectsCreated );
    assert( pluginCount < MAX_PLUGINS );
    plugins[ pluginCount ].create = create;
    plugins[ pluginCount ].object = NULL;
    plugins[ pluginCount ].optionName = optionName;
    plugins[ pluginCount ].isPPCallback = isPPCallback;
    plugins[ pluginCount ].byDefault = byDefault;
    ++pluginCount;
    }

DiagnosticBuilder PluginHandler::report( DiagnosticsEngine::Level level, const char* plugin, StringRef message, CompilerInstance& compiler,
    SourceLocation loc )
    {
    DiagnosticsEngine& diag = compiler.getDiagnostics();
    // Do some mappings (e.g. for -Werror) that clang does not do for custom messages for some reason.
    if( level == DiagnosticsEngine::Warning && diag.getWarningsAsErrors() && (plugin == nullptr || plugin != warningsOnly))
        level = DiagnosticsEngine::Error;
    if( level == DiagnosticsEngine::Error && diag.getErrorsAsFatal())
        level = DiagnosticsEngine::Fatal;
    string fullMessage = ( message + " [loplugin" ).str();
    if( plugin )
        {
        fullMessage += ":";
        fullMessage += plugin;
        }
    fullMessage += "]";
    if( loc.isValid())
        return diag.Report( loc, compat::getCustomDiagID(diag, level, fullMessage) );
    else
        return diag.Report( compat::getCustomDiagID(diag, level, fullMessage) );
    }

DiagnosticBuilder PluginHandler::report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc )
    {
    return report( level, nullptr, message, compiler, loc );
    }

bool PluginHandler::addRemoval( SourceLocation loc )
    {
    return removals.insert( loc ).second;
    }

void PluginHandler::HandleTranslationUnit( ASTContext& context )
    {
    if( context.getDiagnostics().hasErrorOccurred())
        return;
    char const*const mainFileName = context.getSourceManager().getFileEntryForID(context.getSourceManager().getMainFileID())->getName();
    size_t const len = strlen(mainFileName);
    if (len > 3 && strncmp(mainFileName + len - 3, ".ii", 3) == 0)
    {
        report(DiagnosticsEngine::Fatal,
            "input file has suffix .ii: \"%0\"\nhighly suspicious, probably ccache generated, this will break warning suppressions; export CCACHE_CPP2=1 to prevent this") << mainFileName;
        return;
    }

    for( int i = 0;
         i < pluginCount;
         ++i )
        {
        if( plugins[ i ].object != NULL )
            plugins[ i ].object->run();
        }
    for( Rewriter::buffer_iterator it = rewriter.buffer_begin();
         it != rewriter.buffer_end();
         ++it )
        {
        const FileEntry* e = context.getSourceManager().getFileEntryForID( it->first );
        if( e == NULL )
            continue; // Failed modification because of a macro expansion?
        /* Check where the file actually is, and warn about cases where modification
           most probably doesn't matter (generated files in workdir).
           The order here is important, as INSTDIR and WORKDIR are often in SRCDIR/BUILDDIR,
           and BUILDDIR is sometimes in SRCDIR. */
        string modifyFile;
        const char* pathWarning = NULL;
        bool skip = false;
        if( strncmp( e->getName(), WORKDIR "/", strlen( WORKDIR "/" )) == 0 )
            pathWarning = "modified source in workdir/ : %0";
        else if( strcmp( SRCDIR, BUILDDIR ) != 0 && strncmp( e->getName(), BUILDDIR "/", strlen( BUILDDIR "/" )) == 0 )
            pathWarning = "modified source in build dir : %0";
        else if( strncmp( e->getName(), SRCDIR "/", strlen( SRCDIR "/" )) == 0 )
            ; // ok
        else
            {
            pathWarning = "modified source in unknown location, not modifying : %0";
            skip = true;
            }
        if( modifyFile.empty())
            modifyFile = e->getName();
        // Check whether the modified file is in the wanted scope
        if( scope == "mainfile" )
            {
            if( it->first != context.getSourceManager().getMainFileID())
                continue;
            }
        else if( scope == "all" )
            ; // ok
        else // scope is module
            {
            if( !( isPrefix( SRCDIR "/" + scope + "/", modifyFile ) || isPrefix( SRCDIR "/include/" + scope + "/", modifyFile ) ) )
                continue;
            }
        // Warn only now, so that files not in scope do not cause warnings.
        if( pathWarning != NULL )
            report( DiagnosticsEngine::Warning, pathWarning ) << e->getName();
        if( skip )
            continue;
        char* filename = new char[ modifyFile.length() + 100 ];
        sprintf( filename, "%s.new.%d", modifyFile.c_str(), getpid());
        string error;
        bool ok = false;
        std::unique_ptr<raw_fd_ostream> ostream(
            compat::create_raw_fd_ostream(filename, error) );
        if( error.empty())
            {
            it->second.write( *ostream );
            ostream->close();
            if( !ostream->has_error() && rename( filename, modifyFile.c_str()) == 0 )
                ok = true;
            }
        ostream->clear_error();
        unlink( filename );
        if( !ok )
            report( DiagnosticsEngine::Error, "cannot write modified source to %0 (%1)" ) << modifyFile << error;
        delete[] filename;
        }
    }

#if (__clang_major__ == 3 && __clang_minor__ >= 6) || __clang_major__ > 3
std::unique_ptr<ASTConsumer> LibreOfficeAction::CreateASTConsumer( CompilerInstance& Compiler, StringRef )
    {
    return llvm::make_unique<PluginHandler>( Compiler, _args );
    }
#else
ASTConsumer* LibreOfficeAction::CreateASTConsumer( CompilerInstance& Compiler, StringRef )
    {
    return new PluginHandler( Compiler, _args );
    }
#endif

bool LibreOfficeAction::ParseArgs( const CompilerInstance&, const vector< string >& args )
    {
    _args = args;
    return true;
    }

static FrontendPluginRegistry::Add< loplugin::LibreOfficeAction > X( "loplugin", "LibreOffice compile check plugin" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
