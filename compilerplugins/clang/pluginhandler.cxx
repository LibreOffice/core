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

#include <clang/AST/ASTContext.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <stdio.h>
#include <unistd.h>

/*
This source file manages all plugin actions. It is not necessary to modify this
file when adding new actions.
*/
namespace loplugin
{

struct PluginData
    {
    Plugin* (*create)( ASTContext&, Rewriter& );
    Plugin* object;
    const char* optionName;
    bool isRewriter;
    };

const int MAX_PLUGINS = 100;
static PluginData plugins[ MAX_PLUGINS ];
static int pluginCount = 0;
static bool pluginObjectsCreated = false;

PluginHandler::PluginHandler( ASTContext& context, const vector< string >& args )
    : context( context )
    , rewriter( context.getSourceManager(), context.getLangOpts())
    {
    bool wasCreated = false;
    for( int i = 0;
         i < pluginCount;
         ++i )
        {
        bool create = false;
        if( args.empty()) // no args -> create non-writer plugins
            create = !plugins[ i ].isRewriter;
        else // create only the given plugin(s)
            {
            if( find( args.begin(), args.end(), plugins[ i ].optionName ) != args.end())
                create = true;
            }
        if( create )
            {
            plugins[ i ].object = plugins[ i ].create( context, rewriter );
            wasCreated = true;
            }
        }
    pluginObjectsCreated = true;
    if( !args.empty() && !wasCreated )
        report( DiagnosticsEngine::Fatal, "unknown plugin tool %0" ) << args.front();
    }

PluginHandler::~PluginHandler()
    {
    for( int i = 0;
         i < pluginCount;
         ++i )
        if( plugins[ i ].object != NULL )
            delete plugins[ i ].object;
    }

void PluginHandler::registerPlugin( Plugin* (*create)( ASTContext&, Rewriter& ), const char* optionName, bool isRewriter )
    {
    assert( !pluginObjectsCreated );
    assert( pluginCount < MAX_PLUGINS );
    plugins[ pluginCount ].create = create;
    plugins[ pluginCount ].object = NULL;
    plugins[ pluginCount ].optionName = optionName;
    plugins[ pluginCount ].isRewriter = isRewriter;
    ++pluginCount;
    }

DiagnosticBuilder PluginHandler::report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc )
    {
    return Plugin::report( level, message, context, loc );
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
        /* Check where the file actually is, and warn about cases where modification
           most probably doesn't matter (generated files in workdir).
           The order here is important, as OUTDIR and WORKDIR are often in SRCDIR/BUILDDIR,
           and BUILDDIR is sometimes in SRCDIR. */
        string modifyFile;
        if( strncmp( e->getName(), OUTDIR, strlen( OUTDIR )) == 0 )
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
                report( DiagnosticsEngine::Warning, "modified source in solver/ : %0" ) << e->getName();
            }
        else if( strncmp( e->getName(), WORKDIR, strlen( WORKDIR )) == 0 )
            report( DiagnosticsEngine::Warning, "modified source in workdir/ : %0" ) << e->getName();
        else if( strcmp( SRCDIR, BUILDDIR ) != 0 && strncmp( e->getName(), BUILDDIR, strlen( BUILDDIR )) == 0 )
            report( DiagnosticsEngine::Warning, "modified source in build dir : %0" ) << e->getName();
        else if( strncmp( e->getName(), SRCDIR, strlen( SRCDIR )) == 0 )
            ; // ok
        else
            {
            report( DiagnosticsEngine::Warning, "modified source in unknown location, not modifying : %0" )
                 << e->getName();
            continue; // --->
            }
        if( modifyFile.empty())
            modifyFile = e->getName();
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

ASTConsumer* LibreOfficeAction::CreateASTConsumer( CompilerInstance& Compiler, StringRef InFile )
    {
    return new PluginHandler( Compiler.getASTContext(), _args );
    }

bool LibreOfficeAction::ParseArgs( const CompilerInstance& CI, const vector< string >& args )
    {
    _args = args;
    return true;
    }


static FrontendPluginRegistry::Add< loplugin::LibreOfficeAction > X( "loplugin", "LibreOffice compile check plugin" );

} // namespace
