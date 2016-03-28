/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 */
#include "stinnspace.hxx"
/*
This is a compile check.

Warns about functions with static keyword in an unnamed namespace.
*/



namespace loplugin
{


StInNspace::StInNspace( const InstantiationData& data )
    : Plugin( data )
    {
    }

void StInNspace::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }


bool StInNspace::VisitFunctionDecl( FunctionDecl* func )

    {
    if( ignoreLocation( func ) )
        return true;
    if( func -> isInAnonymousNamespace () )
    {
      if ( !isa<CXXMethodDecl>(func) )
         {
            if(func-> getStorageClass() == SC_Static)
               {
                    report( DiagnosticsEngine::Warning,
                        "redundant 'static' keyword in unnamed namespace",
                        func->getLocStart());
               }
         }
    }

    return true;
    }

// Register the plugin action with the LO plugin handling.
static Plugin::Registration< StInNspace > X( "stinnspace",true);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
