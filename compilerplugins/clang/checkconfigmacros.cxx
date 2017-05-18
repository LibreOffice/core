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

#include <set>

#include "compat.hxx"
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
        explicit CheckConfigMacros( const InstantiationData& data );
        virtual void run() override;
        virtual void MacroDefined( const Token& macroToken, const MacroDirective* info ) override;
        virtual void MacroUndefined( const Token& macroToken, compat::MacroDefinitionParam
#if CLANG_VERSION >= 50000
            , MacroDirective const *
#endif
            ) override;
        virtual void Ifdef( SourceLocation location, const Token& macroToken, compat::MacroDefinitionParam ) override;
        virtual void Ifndef( SourceLocation location, const Token& macroToken, compat::MacroDefinitionParam ) override;
        virtual void Defined( const Token& macroToken, compat::MacroDefinitionParam, SourceRange Range ) override;
        enum { isPPCallback = true };
    private:
        void checkMacro( const Token& macroToken, SourceLocation location );
        std::set< string > configMacros;
    };

CheckConfigMacros::CheckConfigMacros( const InstantiationData& data )
    : Plugin( data )
    {
    compat::addPPCallbacks(compiler.getPreprocessor(), this);
    }

void CheckConfigMacros::run()
    {
    // nothing, only check preprocessor usage
    }

void CheckConfigMacros::MacroDefined( const Token& macroToken, const MacroDirective* info )
    {
    SourceLocation location = info->getLocation();
    const char* filename = compiler.getSourceManager().getPresumedLoc( location ).getFilename();
    if( filename != NULL
        && ( hasPathnamePrefix(filename, BUILDDIR "/config_host/")
            || hasPathnamePrefix(filename, BUILDDIR "/config_build/") ))
        {
//        fprintf(stderr,"DEF: %s %s\n", macroToken.getIdentifierInfo()->getName().data(), filename );
        configMacros.insert( macroToken.getIdentifierInfo()->getName());
        }
    }

void CheckConfigMacros::MacroUndefined( const Token& macroToken, compat::MacroDefinitionParam
#if CLANG_VERSION >= 50000
                                        , MacroDirective const *
#endif
                                        )
    {
    configMacros.erase( macroToken.getIdentifierInfo()->getName());
    }

void CheckConfigMacros::Ifdef( SourceLocation location, const Token& macroToken, compat::MacroDefinitionParam )
    {
    checkMacro( macroToken, location );
    }

void CheckConfigMacros::Ifndef( SourceLocation location, const Token& macroToken, compat::MacroDefinitionParam )
    {
    checkMacro( macroToken, location );
    }

void CheckConfigMacros::Defined( const Token& macroToken, compat::MacroDefinitionParam , SourceRange )
    {
    checkMacro( macroToken, macroToken.getLocation());
    }

void CheckConfigMacros::checkMacro( const Token& macroToken, SourceLocation location )
    {
    if( configMacros.find( macroToken.getIdentifierInfo()->getName()) != configMacros.end())
        {
        const char* filename = compiler.getSourceManager().getPresumedLoc( location ).getFilename();
        if( filename == NULL
            || !hasPathnamePrefix(filename, SRCDIR "/include/LibreOfficeKit/") )
            {
            report( DiagnosticsEngine::Error, "checking whether a config macro %0 is defined",
                location ) << macroToken.getIdentifierInfo()->getName();
            report( DiagnosticsEngine::Note, "use #if instead of #ifdef/#ifndef/defined", location );
            }
        }
    }

static Plugin::Registration< CheckConfigMacros > X( "checkconfigmacros" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
