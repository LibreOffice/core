/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

/*
This is a compile check.

Warns about functions with static keyword in an unnamed namespace.
*/

namespace loplugin
{

class RedundantStatic
    : public loplugin::FilteringPlugin<RedundantStatic>
    {
    public:
        explicit RedundantStatic( const InstantiationData& data );

        bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

        virtual void run() override;
        bool VisitFunctionDecl( const FunctionDecl* func );

        bool VisitVarDecl(VarDecl const * decl) {
            if (ignoreLocation(decl)) {
                return true;
            }
            if (!decl->isFileVarDecl() || decl->isStaticDataMember()) {
                return true;
            }
            if (decl->getStorageClass() != SC_Static) {
                return true;
            }
            if (decl->isInAnonymousNamespace()) {
                auto loc = compat::getBeginLoc(decl);
                while (compiler.getSourceManager().isMacroArgExpansion(loc)) {
                    loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
                }
                if (compiler.getSourceManager().isMacroBodyExpansion(loc)) {
                    auto const name = Lexer::getImmediateMacroName(
                        loc, compiler.getSourceManager(), compiler.getLangOpts());
                    if (name == "CPPUNIT_TEST_SUITE_REGISTRATION"
                        || name == "CPPUNIT_TEST_SUITE_NAMED_REGISTRATION")
                    {
                        // Those macros contain a `static`, but are often used in an unnamed
                        // namespace, so filter them out:
                        return true;
                    }
                }
                report(
                    DiagnosticsEngine::Warning, "redundant 'static' keyword in unnamed namespace",
                    decl->getLocation())
                    << decl->getSourceRange();
                return true;
            }
            if (decl->isInline()) {
                return true;
            }
            if (!loplugin::TypeCheck(decl->getType()).ConstNonVolatile()) {
                return true;
            }
            report(
                DiagnosticsEngine::Warning,
                "non-inline variable of non-volatile const-qualified type is redundantly marked as"
                    " 'static'",
                decl->getLocation())
                << decl->getSourceRange();
            return true;
        }
    };

RedundantStatic::RedundantStatic( const InstantiationData& data )
    : FilteringPlugin( data )
    {
    }

void RedundantStatic::run()
    {
    if (preRun()) {
        TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }
    }


bool RedundantStatic::VisitFunctionDecl( const FunctionDecl* func )

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
static Plugin::Registration< RedundantStatic > redundantstatic("redundantstatic");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
