/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>

using namespace clang;

namespace
{

/// Reports a diagnostic at the specified location.
clang::DiagnosticBuilder report(clang::ASTContext* context, const std::string& string,
                                clang::SourceLocation location, const std::string& category)
{
    clang::DiagnosticsEngine& engine = context->getDiagnostics();
    clang::DiagnosticIDs::Level level = clang::DiagnosticIDs::Level::Warning;
    if (engine.getWarningsAsErrors())
        level = clang::DiagnosticIDs::Level::Error;
    std::string formatString = string + " [coplugin:" + category + "]";
    return engine.Report(location, engine.getDiagnosticIDs()->getCustomDiagID(level, formatString));
}

/// Finds uses of lambda captures where the variable is captured by reference, but the variable is a
/// parameter of a function, so the capture should be by value, assuming the lambda will be
/// long-living.
class CaptureCheck : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    bool ignoreCapture(const clang::LambdaCapture& capture,
                       const std::set<const clang::ParmVarDecl*>& functionParams)
    {
        if (!capture.capturesVariable())
        {
            return true;
        }

        if (capture.getCaptureKind() != clang::LCK_ByRef)
        {
            return true;
        }

        auto lambdaParm = llvm::dyn_cast<clang::ParmVarDecl>(capture.getCapturedVar());
        if (!lambdaParm)
        {
            return true;
        }

        auto it = functionParams.find(lambdaParm);
        if (it == functionParams.end())
        {
            return true;
        }

        return false;
    }

    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        const clang::FunctionDecl* functionDecl =
            result.Nodes.getNodeAs<clang::CXXConstructorDecl>("constructorDecl");
        if (!functionDecl)
        {
            functionDecl = result.Nodes.getNodeAs<clang::CXXMethodDecl>("methodDecl");
        }

        std::set<const clang::ParmVarDecl*> functionParams;
        for (const clang::ParmVarDecl* functionParm : functionDecl->parameters())
        {
            functionParams.insert(functionParm);
        }

        const auto lambdaExpr = result.Nodes.getNodeAs<clang::LambdaExpr>("lambdaExpr");
        if (!lambdaExpr)
        {
            return;
        }

        for (auto captureIt = lambdaExpr->capture_begin(); captureIt != lambdaExpr->capture_end();
             ++captureIt)
        {
            const clang::LambdaCapture& capture = *captureIt;
            if (ignoreCapture(capture, functionParams))
            {
                continue;
            }

            clang::SourceManager& sourceManager = result.Context->getSourceManager();
            if (sourceManager.isInSystemHeader(capture.getLocation()))
            {
                continue;
            }

            clang::SourceRange range(capture.getLocation());
            clang::SourceLocation location(range.getBegin());
            report(result.Context,
                   "function parameter captured by reference, capture by value instead", location,
                   "capture")
                << range;
        }
    }

    static clang::ast_matchers::StatementMatcher makeMatcher()
    {
        using namespace clang::ast_matchers;
        return lambdaExpr(
                   anyOf(hasAncestor(cxxConstructorDecl(isDefinition()).bind("constructorDecl")),
                         hasAncestor(cxxMethodDecl(hasAncestor(lambdaExpr())).bind("methodDecl"))))
            .bind("lambdaExpr");
    }
};

/// Finds locations where CheckFileInfo is allocated on the stack.
class RefcountingCheck : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        const clang::VarDecl* varDecl =
            result.Nodes.getNodeAs<clang::VarDecl>("varDecl");

        clang::SourceManager& sourceManager = result.Context->getSourceManager();
        if (sourceManager.isInSystemHeader(varDecl->getLocation()))
        {
            return;
        }

        clang::SourceRange range(varDecl->getLocation());
        clang::SourceLocation location(range.getBegin());
        report(result.Context, "instance allocated on the stack, create it with std::shared_ptr",
               location, "refcounting")
            << range;
    }

    static clang::ast_matchers::DeclarationMatcher makeMatcher()
    {
        using namespace clang::ast_matchers;
        return varDecl(hasType(cxxRecordDecl(anyOf(
                           hasName("CheckFileInfo"), hasName("SocketPoll"),
                           isDerivedFrom(anyOf(hasName("CheckFileInfo"), hasName("SocketPoll")))))))
            .bind("varDecl");
    }
};

/// only "SocketPoll" should have std::shared_ptr<Socket> as a member.
/// Everything else should have std::weak_ptr<Socket> of those.
class SocketPollCheck : public clang::ast_matchers::MatchFinder::MatchCallback
{
public:
    void run(const clang::ast_matchers::MatchFinder::MatchResult& result) override
    {
        const clang::FieldDecl* fieldDecl =
            result.Nodes.getNodeAs<clang::FieldDecl>("fieldDecl");

        clang::SourceManager& sourceManager = result.Context->getSourceManager();
        if (sourceManager.isInSystemHeader(fieldDecl->getLocation()))
        {
            return;
        }
        auto recordDecl = fieldDecl->getParent();
        if (recordDecl && recordDecl->getIdentifier()
            && (recordDecl->getName() == "SocketPoll" || recordDecl->getName() == "SocketDisposition"))
        {
            return;
        }

        clang::SourceRange range(fieldDecl->getLocation());
        clang::SourceLocation location(range.getBegin());

        // we trust the code in here
#if __clang_major__ > 15
        if (sourceManager.getFilename(location).ends_with("Socket.cpp"))
#else
        // At least 15.0.7 needs this
        if (sourceManager.getFilename(location).endswith("Socket.cpp"))
#endif
        {
            return;
        }

        report(result.Context, "Only SocketPoll can have std::shared_ptr<Socket> as a member. Everything else should have std::weak_ptr<Socket>",
               location, "socketPoll")
            << range;
    }

    static clang::ast_matchers::DeclarationMatcher makeMatcher()
    {
        using namespace clang::ast_matchers;
        return fieldDecl(
                    hasType(
                        classTemplateSpecializationDecl(
                            hasName("std::shared_ptr"),
                            hasAnyTemplateArgument(
                                refersToType(
                                    recordType(
                                        hasDeclaration(
                                            cxxRecordDecl(isSameOrDerivedFrom("Socket")))))))))
            .bind("fieldDecl");
    }
};

/// Builds a list of checks to be executed.
class CheckRegistry
{
public:
    CheckRegistry()
    {
        _finder.addMatcher(CaptureCheck::makeMatcher(), &_captureCheck);
        _finder.addMatcher(RefcountingCheck::makeMatcher(), &_refcountingCheck);
        _finder.addMatcher(SocketPollCheck::makeMatcher(), &_socketPollCheck);
    }

    std::unique_ptr<ASTConsumer> makeASTConsumer() { return _finder.newASTConsumer(); }

private:
    clang::ast_matchers::MatchFinder _finder;
    CaptureCheck _captureCheck;
    RefcountingCheck _refcountingCheck;
    SocketPollCheck _socketPollCheck;
};

/// Connects CheckRegistry to an already existing compiler instance.
class COPluginAction : public PluginASTAction
{
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& ci, llvm::StringRef) override
    {
        // Use placement new to keep this alive till the AST context is alive.
        void* buf = ci.getASTContext().Allocate<CheckRegistry>();
        auto registry = new (buf) CheckRegistry();
        return registry->makeASTConsumer();
    }

    bool ParseArgs(const CompilerInstance&, const std::vector<std::string>&) override
    {
        return true;
    }
};

} // namespace

static FrontendPluginRegistry::Add<COPluginAction> X("coplugin", "COOL plugin");

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
