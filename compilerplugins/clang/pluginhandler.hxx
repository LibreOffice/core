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
        static void registerPlugin( Plugin* (*create)( CompilerInstance&, Rewriter& ), const char* optionName, bool isRewriter, bool isPPCallback );
    private:
        void handleOption( const string& option );
        void createPlugin( const string& name );
        DiagnosticBuilder report( DiagnosticsEngine::Level level, StringRef message, SourceLocation loc = SourceLocation());
        CompilerInstance& compiler;
        Rewriter rewriter;
        string scope;
    };

/**
 The Clang plugin class, just forwards to PluginHandler.
*/
class LibreOfficeAction
    : public PluginASTAction
    {
    public:
        virtual ASTConsumer* CreateASTConsumer( CompilerInstance& Compiler, StringRef InFile );
        virtual bool ParseArgs( const CompilerInstance& CI, const vector< string >& args );
    private:
        vector< string > _args;
    };

} // namespace

#endif // COMPILEPLUGIN_H
