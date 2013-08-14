/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

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
    Plugin* (*create)( CompilerInstance&, Rewriter& );
    Plugin* object;
    const char* optionName;
    bool isRewriter;
    bool isPPCallback;
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
    bool wasPlugin = false;
    for( vector< string >::const_iterator it = args.begin();
         it != args.end();
         ++it )
        {
        if( it->size() >= 2 && (*it)[ 0 ] == '-' && (*it)[ 1 ] == '-' )
            handleOption( it->substr( 2 ));
        else
            {
            createPlugin( *it );
            wasPlugin = true;
            }
        }
    if( !wasPlugin )
        createPlugin( "" ); // = all non-rewriters
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
    else
        report( DiagnosticsEngine::Fatal, "unknown option %0" ) << option;
    }

void PluginHandler::createPlugin( const string& name )
    {
    for( int i = 0;
         i < pluginCount;
         ++i )
        {
        if( name.empty())  // no plugin given -> create non-writer plugins
            {
            if( !plugins[ i ].isRewriter )
                plugins[ i ].object = plugins[ i ].create( compiler, rewriter );
            }
        else if( plugins[ i ].optionName == name )
            {
            plugins[ i ].object = plugins[ i ].create( compiler, rewriter );
            return;
            }
        }
    if( !name.empty())
        report( DiagnosticsEngine::Fatal, "unknown plugin tool %0" ) << name;
    }

void PluginHandler::registerPlugin( Plugin* (*create)( CompilerInstance&, Rewriter& ), const char* optionName, bool isRewriter, bool isPPCallback )
    {
    assert( !pluginObjectsCreated );
    assert( pluginCount < MAX_PLUGINS );
    plugins[ pluginCount ].create = create;
    plugins[ pluginCount ].object = NULL;
    plugins[ pluginCount ].optionName = optionName;
    plugins[ pluginCount ].isRewriter = isRewriter;
    plugins[ pluginCount ].isPPCallback = isPPCallback;
    ++pluginCount;
    }

DiagnosticBuilder PluginHandler::report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc )
    {
    return Plugin::report( level, message, compiler, loc );
    }

void PluginHandler::HandleTranslationUnit( ASTContext& context )
    {
    if( context.getDiagnostics().hasErrorOccurred())
        return;
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
           The order here is important, as OUTDIR and WORKDIR are often in SRCDIR/BUILDDIR,
           and BUILDDIR is sometimes in SRCDIR. */
        string modifyFile;
        const char* pathWarning = NULL;
        bool skip = false;
        if( strncmp( e->getName(), OUTDIR "/", strlen( OUTDIR "/" )) == 0 )
            {
            /* Try to find a matching file for a file in solver/ (include files
               are usually included from there rather than from the source dir) if possible. */
            if( strncmp( e->getName(), OUTDIR "/inc/", strlen( OUTDIR ) + strlen( "/inc/" )) == 0 )
                {
                string filename( e->getName());
                int modulePos = strlen( OUTDIR ) + strlen( "/inc/" );
                size_t moduleEnd = filename.find( '/', modulePos );
                if( moduleEnd != string::npos )
                    {
                    modifyFile = SRCDIR "/" + filename.substr( modulePos, moduleEnd - modulePos )
                        + "/inc/" + filename.substr( modulePos );
                    }
                }
            if( modifyFile.empty())
                pathWarning = "modified source in solver/ : %0";
            }
        else if( strncmp( e->getName(), WORKDIR "/", strlen( WORKDIR "/" )) == 0 )
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
        // Check whether the modified file is in the wanted scope (done after path checking above), so
        // that files mapped from OUTDIR to SRCDIR are included.
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
        raw_fd_ostream ostream( filename, error );
        if( error.empty())
            {
            it->second.write( ostream );
            ostream.close();
            if( !ostream.has_error() && rename( filename, modifyFile.c_str()) == 0 )
                ok = true;
            }
        ostream.clear_error();
        unlink( filename );
        if( !ok )
            report( DiagnosticsEngine::Error, "cannot write modified source to %0 (%1)" ) << modifyFile << error;
        delete[] filename;
        }
    }

ASTConsumer* LibreOfficeAction::CreateASTConsumer( CompilerInstance& Compiler, StringRef )
    {
    return new PluginHandler( Compiler, _args );
    }

bool LibreOfficeAction::ParseArgs( const CompilerInstance&, const vector< string >& args )
    {
    _args = args;
    return true;
    }


static FrontendPluginRegistry::Add< loplugin::LibreOfficeAction > X( "loplugin", "LibreOffice compile check plugin" );

} // namespace
