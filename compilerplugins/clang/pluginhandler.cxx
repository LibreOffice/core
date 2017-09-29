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

#include <memory>
#include "compat.hxx"
#include "pluginhandler.hxx"

#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Lex/PPCallbacks.h>
#include <stdio.h>

#if defined _WIN32
#include <process.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

/**
This source file manages all plugin actions. It is not necessary to modify this
file when adding new actions.
*/

static bool isPrefix( const std::string& prefix, const std::string& full)
{
    return full.compare(0, prefix.size(), prefix) == 0;
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
static bool bPluginObjectsCreated = false;
static bool unitTestMode = false;

PluginHandler::PluginHandler( CompilerInstance& compiler, const std::vector< std::string >& args )
    : compiler( compiler )
    , rewriter( compiler.getSourceManager(), compiler.getLangOpts())
    , scope( "mainfile" )
    , warningsAsErrors( false )
{
    std::set< std::string > rewriters;
    for( std::string const & arg : args )
        {
        if( arg.size() >= 2 && arg[ 0 ] == '-' && arg[ 1 ] == '-' )
            handleOption( arg.substr( 2 ));
        else
            rewriters.insert( arg );
        }
    createPlugins( rewriters );
    bPluginObjectsCreated = true;
}

PluginHandler::~PluginHandler()
{
    for( int i = 0; i < pluginCount; ++i )
        if( plugins[ i ].object != NULL )
        {
            // PPCallbacks is owned by preprocessor object, don't delete those
            if( !plugins[ i ].isPPCallback )
                delete plugins[ i ].object;
        }
}

bool PluginHandler::isUnitTestMode()
{
    return unitTestMode;
}

void PluginHandler::handleOption( const std::string& option )
{
    if( option.substr( 0, 6 ) == "scope=" )
    {
        scope = option.substr( 6 );
        if( scope == "mainfile" || scope == "all" )
            ; // ok
        else
        {
#if !defined _WIN32 //TODO, S_ISDIR
            struct stat st;
            if( stat(( SRCDIR "/" + scope ).c_str(), &st ) != 0 || !S_ISDIR( st.st_mode ))
                report( DiagnosticsEngine::Fatal, "unknown scope %0 (no such module directory)" ) << scope;
#endif
        }
    }
    else if( option.substr( 0, 14 ) == "warnings-only=" )
    {
        warningsOnly = option.substr(14);
    }
    else if( option == "warnings-as-errors" )
        warningsAsErrors = true;
    else if( option == "unit-test-mode" )
        unitTestMode = true;
    else
        report( DiagnosticsEngine::Fatal, "unknown option %0" ) << option;
}

void PluginHandler::createPlugins( std::set< std::string > rewriters )
{
    for( int i = 0; i < pluginCount; ++i )
    {
        const char* name = plugins[i].optionName;
        if( rewriters.erase( name ) != 0 )
            plugins[ i ].object = plugins[ i ].create( Plugin::InstantiationData { name, *this, compiler, &rewriter } );
        else if( plugins[ i ].byDefault )
            plugins[ i ].object = plugins[ i ].create( Plugin::InstantiationData { name, *this, compiler, NULL } );
        else if( unitTestMode && strcmp(name, "unusedmethodsremove") != 0 && strcmp(name, "unusedfieldsremove") != 0)
            plugins[ i ].object = plugins[ i ].create( Plugin::InstantiationData { name, *this, compiler, NULL } );
    }
    for( auto r: rewriters )
        report( DiagnosticsEngine::Fatal, "unknown plugin tool %0" ) << r;
}

void PluginHandler::registerPlugin( Plugin* (*create)( const Plugin::InstantiationData& ), const char* optionName, bool isPPCallback, bool byDefault )
{
    assert( !bPluginObjectsCreated );
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
    if( level == DiagnosticsEngine::Warning && ((diag.getWarningsAsErrors() && (plugin == nullptr || plugin != warningsOnly)) || warningsAsErrors))
        level = DiagnosticsEngine::Error;
    if( level == DiagnosticsEngine::Error && diag.getErrorsAsFatal())
        level = DiagnosticsEngine::Fatal;
    std::string fullMessage = ( message + " [loplugin" ).str();
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
    StringRef const mainFileName = context.getSourceManager().getFileEntryForID(context.getSourceManager().getMainFileID())->getName();
    if (mainFileName.endswith(".ii"))
    {
        report(DiagnosticsEngine::Fatal,
            "input file has suffix .ii: \"%0\"\nhighly suspicious, probably ccache generated, this will break warning suppressions; export CCACHE_CPP2=1 to prevent this") << mainFileName;
        return;
    }

    for( int i = 0; i < pluginCount; ++i )
    {
        if( plugins[ i ].object != NULL )
        {
            // When in unit-test mode, ignore plugins whose names don't match the filename of the test,
            // so that we only generate warnings for the plugin that we want to test.
            if (!unitTestMode || mainFileName.find(plugins[ i ].optionName) != StringRef::npos)
                plugins[ i ].object->run();
        }
    }
#if defined _WIN32
    //TODO: make the call to 'rename' work on Windows (where the renamed-to
    // original file is probably still held open somehow):
    rewriter.overwriteChangedFiles();
#else
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
        std::string modifyFile;
        const char* pathWarning = NULL;
        bool bSkip = false;
        StringRef const name = e->getName();
        if( name.startswith(WORKDIR "/") )
            pathWarning = "modified source in workdir/ : %0";
        else if( strcmp( SRCDIR, BUILDDIR ) != 0 && name.startswith(BUILDDIR "/") )
            pathWarning = "modified source in build dir : %0";
        else if( name.startswith(SRCDIR "/") )
            ; // ok
        else
        {
            pathWarning = "modified source in unknown location, not modifying : %0";
            bSkip = true;
        }
        if( modifyFile.empty())
            modifyFile = name;
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
            report( DiagnosticsEngine::Warning, pathWarning ) << name;
        if( bSkip )
            continue;
        char* filename = new char[ modifyFile.length() + 100 ];
        sprintf( filename, "%s.new.%d", modifyFile.c_str(), getpid());
        std::string error;
        bool bOk = false;
        std::unique_ptr<raw_fd_ostream> ostream(
            compat::create_raw_fd_ostream(filename, error) );
        if( error.empty())
        {
            it->second.write( *ostream );
            ostream->close();
            if( !ostream->has_error() && rename( filename, modifyFile.c_str()) == 0 )
                bOk = true;
        }
        ostream->clear_error();
        unlink( filename );
        if( !bOk )
            report( DiagnosticsEngine::Error, "cannot write modified source to %0 (%1)" ) << modifyFile << error;
        delete[] filename;
    }
#endif
 }

#if CLANG_VERSION >= 30600
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

bool LibreOfficeAction::ParseArgs( const CompilerInstance&, const std::vector< std::string >& args )
{
    _args = args;
    return true;
}

static FrontendPluginRegistry::Add< loplugin::LibreOfficeAction > X( "loplugin", "LibreOffice compile check plugin" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
