/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>

#include "plugin.hxx"
#include "compat.hxx"

// Methods that purely return a local field should be declared in the header and be declared inline.
// So that the compiler can elide the function call and turn it into a simple fixed-offset-load instruction.

namespace {

class InlineSimpleMemberFunctions:
    public RecursiveASTVisitor<InlineSimpleMemberFunctions>, public loplugin::RewritePlugin
{
public:
    explicit InlineSimpleMemberFunctions(loplugin::InstantiationData const & data): RewritePlugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXMethodDecl(const CXXMethodDecl * decl);
private:
    bool rewrite(const CXXMethodDecl * functionDecl);
};

static bool oneAndOnlyOne(clang::Stmt::const_child_range range) {
    if (range.begin() == range.end()) {
        return false;
    }
    if (++range.begin() != range.end()) {
        return false;
    }
    return true;
}


bool InlineSimpleMemberFunctions::VisitCXXMethodDecl(const CXXMethodDecl * functionDecl) {
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    // no point in doing virtual methods, the compiler always has to generate a vtable entry and a method
    if (functionDecl->isVirtual()) {
        return true;
    }
    if (functionDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate) {
        return true;
    }
    if (!functionDecl->isInstance()) {
        return true;
    }
    if (!functionDecl->isOutOfLine()) {
        return true;
    }
    if( !functionDecl->hasBody()) {
        return true;
    }
    if( functionDecl->isInlineSpecified()) {
        return true;
    }
    if( functionDecl->getCanonicalDecl()->isInlineSpecified()) {
        return true;
    }
    if( functionDecl->getNameAsString().find("Impl") != std::string::npos) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl)) {
        return true;
    }
    // ignore stuff like:
    //   template<class E> E * Sequence<E>::begin() { return getArray(); }
    if( functionDecl->getParent()->getDescribedClassTemplate() != nullptr ) {
        return true;
    }

    /*
      The chain here looks like
        CompoundStmt
          ReturnStmt
            other stuff
              CXXThisExpr
    */

    const CompoundStmt* compoundStmt = dyn_cast< CompoundStmt >( functionDecl->getBody() );
    if (compoundStmt == nullptr) {
        return true;
    }
    if (compoundStmt->body_begin() == compoundStmt->body_end()) {
        return true;
    }


    const Stmt* childStmt = *compoundStmt->child_begin();

    if (dyn_cast<ReturnStmt>( childStmt ) == nullptr) {
        return true;
    }
    if (!oneAndOnlyOne(childStmt->children())) {
        return true;
    }


    /* Don't warn if we see a method definition like
         X X::a() {
             return *this;
         }
      which translates to:
        CompoundStmt
          ReturnStmt
            ImplicitCastExpr
              UnaryOperator
                CXXThisExpr
      or:
        CompoundStmt
          ReturnStmt
            UnaryOperator
              CXXThisExpr
    */
    childStmt = *childStmt->child_begin();
    if (dyn_cast<ImplicitCastExpr>( childStmt ) != nullptr
        && oneAndOnlyOne( childStmt->children() ))
    {
        const Stmt* childStmt2 = *childStmt->child_begin();
        if (dyn_cast<UnaryOperator>( childStmt2 ) != nullptr
            && oneAndOnlyOne(childStmt2->children()))
        {
            childStmt2 = *childStmt2->child_begin();
            if (dyn_cast<CXXThisExpr>( childStmt2 ) != nullptr
                && childStmt2->children().begin() == childStmt2->children().end())
            {
                return true;
            }
        }
    }
    if (dyn_cast<UnaryOperator>( childStmt ) != nullptr
        && oneAndOnlyOne( childStmt->children() ))
    {
        const Stmt* childStmt2 = *childStmt->child_begin();
        if (dyn_cast<CXXThisExpr>( childStmt2 ) != nullptr
            && childStmt2->children().begin() == childStmt2->children().end())
        {
            return true;
        }
    }

    /* look for a chains like:
       CompoundStmt
         ReturnStmt
           stuff
             CXXThisExpr
    */
    childStmt = *(*compoundStmt->child_begin())->child_begin();
    while (1) {
        if (dyn_cast<CallExpr>( childStmt ) != nullptr)
            return true;
        if (dyn_cast<CXXNewExpr>( childStmt ) != nullptr)
            return true;
        if (dyn_cast<CXXConstructExpr>( childStmt ) != nullptr)
            return true;
        if (dyn_cast<ConditionalOperator>( childStmt ) != nullptr)
            return true;
        if (dyn_cast<BinaryOperator>( childStmt ) != nullptr)
            return true;
        // exclude methods that return fields on incomplete types .e.g the pImpl pattern
        const MemberExpr* memberExpr = dyn_cast<MemberExpr>( childStmt );
        if (memberExpr != nullptr && memberExpr->getMemberDecl()) {
            const FieldDecl* fieldDecl = dyn_cast<FieldDecl>(memberExpr->getMemberDecl());
            if (fieldDecl != nullptr)
            {
                // yes, a little bit of a hack. However, it is quite hard to determine if the method
                // in question is accessing a field via a pImpl pattern.
                if (fieldDecl->getType()->isIncompleteType())
                    return true;
                if (fieldDecl->getNameAsString().find("Impl") != std::string::npos)
                    return true;
                if (fieldDecl->getNameAsString().find("pImp") != std::string::npos)
                    return true;
                // somewhere in VCL
                if (fieldDecl->getNameAsString().find("mpGlobalSyncData") != std::string::npos)
                    return true;
                std::string s = fieldDecl->getType().getAsString();
                if (s.find("Impl") != std::string::npos || s.find("pImp") != std::string::npos || s.find("Internal") != std::string::npos)
                    return true;
            }
        }
        if (dyn_cast<CXXThisExpr>( childStmt ) != nullptr) {
            if (!rewrite(functionDecl))
            {
                report(
                    DiagnosticsEngine::Warning,
                    "inlinesimpleaccessmethods",
                    functionDecl->getSourceRange().getBegin())
                  << functionDecl->getSourceRange();
                // display the location of the class member declaration so I don't have to search for it by hand
                report(
                    DiagnosticsEngine::Note,
                    "inlinesimpleaccessmethods",
                    functionDecl->getCanonicalDecl()->getSourceRange().getBegin())
                  << functionDecl->getCanonicalDecl()->getSourceRange();
                }
             return true;
        }
        if ( childStmt->children().begin() == childStmt->children().end() )
            return true;
        childStmt = *childStmt->child_begin();
    }
    return true;
}

static std::string ReplaceString(std::string subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

bool InlineSimpleMemberFunctions::rewrite(const CXXMethodDecl * functionDecl) {
    if (rewriter == nullptr) {
        return false;
    }
    // Only rewrite declarations in include files if a
    // definition is also seen, to avoid compilation of a
    // definition (in a main file only processed later) to fail
    // with a "mismatch" error before the rewriter had a chance
    // to act upon the definition.
    if (!compiler.getSourceManager().isInMainFile(
            compiler.getSourceManager().getSpellingLoc(
                functionDecl->getNameInfo().getLoc())))
    {
        return false;
    }

    const char *p1, *p2;

    // get the function body contents
    p1 = compiler.getSourceManager().getCharacterData( functionDecl->getBody()->getLocStart() );
    p2 = compiler.getSourceManager().getCharacterData( functionDecl->getBody()->getLocEnd() );
    std::string s1( p1, p2 - p1 + 1);

    /* we can't safely move around stuff containing comments, we mess up the resulting code */
    if ( s1.find("/*") != std::string::npos || s1.find("//") != std::string::npos ) {
        return false;
    }

    // strip linefeeds and any double-spaces, so we have a max of one space between tokens
    s1 = ReplaceString(s1, "\r", "");
    s1 = ReplaceString(s1, "\n", "");
    s1 = ReplaceString(s1, "\t", " ");
    s1 = ReplaceString(s1, "  ", " ");
    s1 = ReplaceString(s1, "  ", " ");
    s1 = ReplaceString(s1, "  ", " ");
    s1 = " " + s1;

    // scan from the end of the function's body through the trailing whitespace, so we can do a nice clean remove
// commented out because for some reason it will sometimes chomp an extra token
//    SourceLocation endOfRemoveLoc = functionDecl->getBody()->getLocEnd();
//    for (;;) {
//        endOfRemoveLoc = endOfRemoveLoc.getLocWithOffset(1);
//        p1 = compiler.getSourceManager().getCharacterData( endOfRemoveLoc );
//        if (*p1 != ' ' && *p1 != '\r' && *p1 != '\n' && *p1 != '\t')
//            break;
//    }

    // remove the function's out of line body and declaration
    RewriteOptions opts;
    opts.RemoveLineIfEmpty = true;
    if (!removeText(SourceRange(functionDecl->getLocStart(), functionDecl->getBody()->getLocEnd()), opts)) {
        return false;
    }

    // scan forward until we find the semicolon
    const FunctionDecl * canonicalDecl = functionDecl->getCanonicalDecl();
    p1 = compiler.getSourceManager().getCharacterData( canonicalDecl->getLocEnd() );
    p2 = ++p1;
    while (*p2 != 0 && *p2 != ';') p2++;

    // insert the function body into the inline function definition (i.e. the one inside the class definition)
    return replaceText(canonicalDecl->getLocEnd().getLocWithOffset(p2 - p1 + 1), 1, s1);
}

loplugin::Plugin::Registration< InlineSimpleMemberFunctions > X("inlinesimplememberfunctions");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
