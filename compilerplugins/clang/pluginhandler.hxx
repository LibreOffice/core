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

#ifndef PLUGINHANDLER_H
#define PLUGINHANDLER_H

#include "plugin.hxx"

#include <set>

#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/FrontendAction.h>

namespace loplugin
{

/**
 Class that manages all LO modules.
*/
class PluginHandler
    : public ASTConsumer
    {
    public:
        PluginHandler( CompilerInstance& compiler, const vector< string >& args );
        virtual ~PluginHandler();
        virtual void HandleTranslationUnit( ASTContext& context ) override;
        static void registerPlugin( Plugin* (*create)( const Plugin::InstantiationData& ), const char* optionName, bool isPPCallback, bool byDefault );
        DiagnosticBuilder report( DiagnosticsEngine::Level level, const char * plugin, StringRef message,
            CompilerInstance& compiler, SourceLocation loc = SourceLocation());
        bool addRemoval( SourceLocation loc );
    private:
        void handleOption( const string& option );
        void createPlugins( set< string > rewriters );
        DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc = SourceLocation());
        CompilerInstance& compiler;
        Rewriter rewriter;
        set< SourceLocation > removals;
        string scope;
        string warningsOnly;
        bool warningsAsErrors;
    };

/**
 The Clang plugin class, just forwards to PluginHandler.
*/
class LibreOfficeAction
    : public PluginASTAction
    {
    public:
#if CLANG_VERSION >= 30600
        virtual std::unique_ptr<ASTConsumer> CreateASTConsumer( CompilerInstance& Compiler, StringRef InFile );
#else
        virtual ASTConsumer* CreateASTConsumer( CompilerInstance& Compiler, StringRef InFile );
#endif

        virtual bool ParseArgs( const CompilerInstance& CI, const vector< string >& args );
    private:
        vector< string > _args;
    };

} // namespace

#endif // COMPILEPLUGIN_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
