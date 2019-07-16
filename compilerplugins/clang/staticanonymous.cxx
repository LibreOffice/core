/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"

/*
This is a compile check.

Warns about functions with static keyword in an unnamed namespace.
*/

namespace loplugin
{

class StaticAnonymous
    : public loplugin::FilteringPlugin<StaticAnonymous>
    {
    public:
        explicit StaticAnonymous( const InstantiationData& data );
        virtual void run() override;
        bool VisitFunctionDecl( const FunctionDecl* func );

    };

StaticAnonymous::StaticAnonymous( const InstantiationData& data )
    : FilteringPlugin( data )
    {
    }

void StaticAnonymous::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }


bool StaticAnonymous::VisitFunctionDecl( const FunctionDecl* func )

    {
    if( ignoreLocation( func ) )
        return true;
    if( func -> isInAnonymousNamespace () )
    {
      if ( !isa<CXXMethodDecl>(func) && !func->isInExternCContext() )
         {
            if(func-> getStorageClass() == SC_Static)
               {
                    report( DiagnosticsEngine::Warning,
                        "redundant 'static' keyword in unnamed namespace",
                        compat::getBeginLoc(func));
               }
         }
    }

    return true;
    }

// Register the plugin action with the LO plugin handling.
static Plugin::Registration< StaticAnonymous > staticanonymous("staticanonymous");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
