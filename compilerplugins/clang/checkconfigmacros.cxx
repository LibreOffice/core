/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#include "plugin.hxx"

#include <clang/Lex/Preprocessor.h>

namespace loplugin
{

/*
This is a compile check.

Feature macros from config_XXX.h headers are always #defined (to 1 or 0 in case of yes/no
settings). It is a mistake to use #ifdef/#ifndef/defined to check them.

Using 1/0 instead of defined/undefined avoids undetected problems when e.g. the necessary
#include of the config_XXX.h file is missing.
*/

class CheckConfigMacros
    : public PPCallbacks
    , public Plugin
    {
    public:
        explicit CheckConfigMacros( CompilerInstance& compiler );
        virtual void run();
        virtual void MacroDefined( const Token& macroToken, const MacroInfo* info );
        virtual void MacroUndefined( const Token& macroToken , const MacroInfo* info );
        virtual void Ifdef( SourceLocation location, const Token& macroToken );
        virtual void Ifndef( SourceLocation location, const Token& macroToken );
        virtual void Defined( const Token& macroToken );
    private:
        void checkMacro( const Token& macroToken, SourceLocation location );
        std::set< string > configMacros;
    };

CheckConfigMacros::CheckConfigMacros( CompilerInstance& compiler )
    : Plugin( compiler )
    {
    compiler.getPreprocessor().addPPCallbacks( this );
    }

void CheckConfigMacros::run()
    {
    // nothing, only check preprocessor usage
    }

void CheckConfigMacros::MacroDefined( const Token& macroToken, const MacroInfo* info )
    {
    const char* filename = compiler.getSourceManager().getPresumedLoc( info->getDefinitionLoc()).getFilename();
    if( filename != NULL
        && ( strncmp( filename, BUILDDIR "/config_host/", strlen( BUILDDIR "/config_host/" )) == 0
            || strncmp( filename, BUILDDIR "/config_build/", strlen( BUILDDIR "/config_build/" )) == 0 ))
        {
//        fprintf(stderr,"DEF: %s %s\n", macroToken.getIdentifierInfo()->getName().data(), filename );
        configMacros.insert( macroToken.getIdentifierInfo()->getName());
        }
    }

void CheckConfigMacros::MacroUndefined( const Token& macroToken, const MacroInfo* )
    {
    configMacros.erase( macroToken.getIdentifierInfo()->getName());
    }

void CheckConfigMacros::Ifdef( SourceLocation location, const Token& macroToken )
    {
    checkMacro( macroToken, location );
    }

void CheckConfigMacros::Ifndef( SourceLocation location, const Token& macroToken )
    {
    checkMacro( macroToken, location );
    }

void CheckConfigMacros::Defined( const Token& macroToken )
    {
    checkMacro( macroToken, macroToken.getLocation());
    }

void CheckConfigMacros::checkMacro( const Token& macroToken, SourceLocation location )
    {
    if( configMacros.find( macroToken.getIdentifierInfo()->getName()) != configMacros.end())
        {
        report( DiagnosticsEngine::Error, "checking whether a config macro %0 is defined",
            location ) << macroToken.getIdentifierInfo()->getName();
        report( DiagnosticsEngine::Note, "use #if instead of #ifdef/#ifndef/defined", location );
        }
    }

static Plugin::Registration< CheckConfigMacros > X( "bodynotinblock" );

} // namespace
