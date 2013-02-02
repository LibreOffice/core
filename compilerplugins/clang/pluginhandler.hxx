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

#include "bodynotinblock.hxx"
#include "lclstaticfix.hxx"
#include "postfixincrementfix.hxx"
#include "removeforwardstringdecl.hxx"
#include "sallogareas.hxx"
#include "unusedvariablecheck.hxx"

namespace loplugin
{

/**
 Class that manages all LO modules.
*/
class PluginHandler
    : public ASTConsumer
    {
    public:
        PluginHandler( ASTContext& context, const vector< string >& args );
        virtual void HandleTranslationUnit( ASTContext& context );
    private:
        bool isArg( const char* arg ) const;
        Rewriter rewriter;
        vector< string > args;
        BodyNotInBlock bodyNotInBlock;
        LclStaticFix lclStaticFix;
        PostfixIncrementFix postfixIncrementFix;
        RemoveForwardStringDecl removeForwardStringDecl;
        SalLogAreas salLogAreas;
        UnusedVariableCheck unusedVariableCheck;
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

/////

inline
bool PluginHandler::isArg( const char* arg ) const
    {
    return find( args.begin(), args.end(), arg ) != args.end();
    }

} // namespace

#endif // COMPILEPLUGIN_H
