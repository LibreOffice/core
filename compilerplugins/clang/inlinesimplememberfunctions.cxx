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

// Methods that purely return a local field should be declared in the header and be declared inline.
// So that the compiler can elide the function call and turn it into a simple fixed-offset-load instruction.

namespace {

class InlineSimpleMemberFunctions:
    public RecursiveASTVisitor<InlineSimpleMemberFunctions>, public loplugin::RewritePlugin
{
public:
    explicit InlineSimpleMemberFunctions(InstantiationData const & data): RewritePlugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXMethodDecl(const CXXMethodDecl * decl);
private:
    bool isInUnoIncludeFile(SourceLocation spellingLocation) const;
    bool isInMainFile(SourceLocation spellingLocation) const;
    bool rewrite(const CXXMethodDecl * functionDecl);
};

static bool oneAndOnlyOne(clang::Stmt::const_child_range range) {
    if (range.empty()) {
        return false;
    }
    if ((++range.first) != range.second) {
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
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              functionDecl->getCanonicalDecl()->getNameInfo().getLoc()))) {
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
                && childStmt2->children().empty())
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
            && childStmt2->children().empty())
        {
            return true;
        }
    }

    /* look for a chain like:
       CompoundStmt
         ReturnStmt
           ImplicitCastExpr
             MemberExpr
               CXXThisExpr
    */
    if (dyn_cast<ImplicitCastExpr>( childStmt ) != nullptr
        && oneAndOnlyOne( childStmt->children() ))
    {
        const Stmt* childStmt2 = *childStmt->child_begin();
        if (dyn_cast<MemberExpr>( childStmt2 ) != nullptr
            && oneAndOnlyOne(childStmt2->children()))
        {
            childStmt2 = *childStmt2->child_begin();
            if (dyn_cast<CXXThisExpr>( childStmt2 ) != nullptr
                && childStmt2->children().empty())
            {
                if (!rewrite(functionDecl))
                {
                    report(
                        DiagnosticsEngine::Warning,
                        "inlinesimpleaccessmethods",
                        functionDecl->getSourceRange().getBegin())
                      << functionDecl->getSourceRange();
                }
                return true;
            }
        }
    }

    return true;
}

bool InlineSimpleMemberFunctions::isInUnoIncludeFile(SourceLocation spellingLocation) const {
    StringRef name {
        compiler.getSourceManager().getFilename(spellingLocation) };
    return isInMainFile(spellingLocation)
        ? (name == SRCDIR "/cppu/source/cppu/compat.cxx"
           || name == SRCDIR "/cppuhelper/source/compat.cxx"
           || name == SRCDIR "/sal/osl/all/compat.cxx")
        : (name.startswith(SRCDIR "/include/com/")
           || name.startswith(SRCDIR "/include/cppu/")
           || name.startswith(SRCDIR "/include/cppuhelper/")
           || name.startswith(SRCDIR "/include/osl/")
           || name.startswith(SRCDIR "/include/rtl/")
           || name.startswith(SRCDIR "/include/sal/")
           || name.startswith(SRCDIR "/include/salhelper/")
           || name.startswith(SRCDIR "/include/systools/")
           || name.startswith(SRCDIR "/include/typelib/")
           || name.startswith(SRCDIR "/include/uno/")
           || name == SRCDIR "/include/comphelper/implbase_var.hxx");
}

bool InlineSimpleMemberFunctions::isInMainFile(SourceLocation spellingLocation) const {
#if (__clang_major__ == 3 && __clang_minor__ >= 4) || __clang_major__ > 3
    return compiler.getSourceManager().isInMainFile(spellingLocation);
#else
    return compiler.getSourceManager().isFromMainFile(spellingLocation);
#endif
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
    if (!isInMainFile(
               compiler.getSourceManager().getSpellingLoc(
                   functionDecl->getNameInfo().getLoc()))) {
        return false;
    }

    const char *p1, *p2;

    // get the function body contents
    p1 = compiler.getSourceManager().getCharacterData( functionDecl->getBody()->getLocStart() );
    p2 = compiler.getSourceManager().getCharacterData( functionDecl->getBody()->getLocEnd() );
    std::string s1( p1, p2 - p1 + 1);
    // strip linefeeds and any double-spaces, so we have a max of one space between tokens
    s1 = ReplaceString(s1, "\r", "");
    s1 = ReplaceString(s1, "\n", "");
    s1 = ReplaceString(s1, "\t", " ");
    s1 = ReplaceString(s1, "  ", " ");
    s1 = ReplaceString(s1, "  ", " ");
    s1 = ReplaceString(s1, "  ", " ");
    s1 = " " + s1;

    // scan from the end of the functions body through the whitespace, so we can do a nice clean remove
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
