/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include "plugin.hxx"
#include "compat.hxx"

/**

Comments from Bjoern Michaelsen:

Killing the 1-argument vector fill constructor:

 std::vector< basebmp::Color > aDevPal(2);

in general is probably a Good Thing(tm). It can just be too misleading.
Requiring at least the explicit two-value fill constructor for the rare cases where
someone wants a filled vector isn't too much to ask and less prone to
misunderstandings:

 std::vector< basebmp::Color > aDevPal(2, basebmp::Color(0,0,0));

Although that _still_ might be misleading[1], so turning all those into the
somewhat longer, but more explicit:

 std::vector< basebmp::Color > aDevPal;
 aDevPal.reserve(2);
 aDevPal.push_back(...);
 ...

> So I suppose the check would be for a size reservation on a vector
> followed by push_back - rather than some array indexing - does that make
> sense ? or did I go crazy ;-)

Yes, in general you want neither of the above forms. Preferably instead of
e.g.:

 std::vector< basebmp::Color > aDevPal(2);
 aDevPal[0] = basebmp::Color( 0, 0, 0 );
 aDevPal[1] = basebmp::Color( 0xff, 0xff, 0xff );

you would -- if possible -- simply:

 std::vector< basebmp::Color > aDevPal{
    basebmp::Color( 0, 0, 0 ),
    basebmp::Color( 0xff, 0xff, 0xff ) };

and only for complex cases, where you do not have the elements statically
available, something like:

 std::vector< foo > vFoos;
 vFoos.reserve(vInput.size());
 std::transform(std::back_inserter(vFoos),
     vInput.begin(),
     vInput.end(),
     [] (decltype(vInput)::value_type aInputValue) { return do_something(aInputValue); });

see also:
https://skyfromme.wordpress.com/2015/03/02/50-ways-to-fill-your-vector/
https://skyfromme.wordpress.com/2015/03/12/following-the-white-rabbit/
(tl;dr: Use initializer lists to fill vectors when possible).

Best,

Bjoern

[1] Well, except that:
     std::vector<int>(3, 0)
    is doing something different from:
     std::vector<int>{3, 0}
    just to make things more interesting. But hey, that's C++ for you.
    But that wart exists for the 1-arg ctor too -- yet another reason to kill that.
*/

namespace {


class BadVectorInit:
    public RecursiveASTVisitor<BadVectorInit>, public loplugin::Plugin
{
public:
    explicit BadVectorInit(InstantiationData const & data): Plugin(data), mbInsideFunction(false) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXConstructExpr(const CXXConstructExpr* );
    bool TraverseFunctionDecl(FunctionDecl* );
    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr* );
private:
    StringRef getFilename(SourceLocation loc);
    bool mbInsideFunction;
    std::set<const VarDecl*> suspectSet;
};

bool BadVectorInit::TraverseFunctionDecl(FunctionDecl* decl)
{
    mbInsideFunction = true;
    bool ret = RecursiveASTVisitor::TraverseFunctionDecl(decl);
    mbInsideFunction = false;
    suspectSet.clear();
    return ret;
}

StringRef BadVectorInit::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name;
}

bool BadVectorInit::VisitCXXMemberCallExpr(const CXXMemberCallExpr* expr)
{
    if (suspectSet.empty() || ignoreLocation( expr ))
        return true;

    // need to exclude some false positives
    StringRef aFileName = getFilename(expr->getLocStart());
    if (aFileName == SRCDIR "/framework/source/services/autorecovery.cxx"
        || aFileName == SRCDIR "/vcl/source/opengl/OpenGLHelper.cxx"
        || aFileName == SRCDIR "/vcl/source/gdi/gdimtf.cxx"
        )
    {
        return true;
    }

    const FunctionDecl* functionDecl = expr->getDirectCallee();
    if (!functionDecl)
        return true;
    if (functionDecl->getNameAsString().find("push_back") == string::npos)
        return true;
    const DeclRefExpr* declExpr = dyn_cast<DeclRefExpr>(expr->getImplicitObjectArgument());
    if (!declExpr)
        return true;
    const VarDecl* varDecl = dyn_cast<VarDecl>(declExpr->getDecl());
    if (!varDecl)
        return true;
    varDecl = varDecl->getCanonicalDecl();
    if (suspectSet.find(varDecl) == suspectSet.end())
        return true;
    report(
        DiagnosticsEngine::Warning,
        "calling push_back after using sized constructor",
        expr->getLocStart())
        << expr->getSourceRange();
    report(
        DiagnosticsEngine::Note,
        "on this var",
        varDecl->getLocStart())
        << varDecl->getSourceRange();

    return true;
}

bool BadVectorInit::VisitCXXConstructExpr(const CXXConstructExpr* expr)
{
    if (ignoreLocation( expr ))
        return true;

    const CXXConstructorDecl *consDecl = expr->getConstructor();
    consDecl = consDecl->getCanonicalDecl();

    // The default constructor can potentially have a parameter, e.g.
    // in glibcxx-debug the default constructor is:
    // explicit vector(const _Allocator& __a = _Allocator())
    if (consDecl->param_size() == 0 || consDecl->isDefaultConstructor())
        return true;

    std::string aParentName = consDecl->getParent()->getQualifiedNameAsString();
    if (aParentName.find("vector") == string::npos && aParentName.find("deque") == string::npos)
        return true;

    // ignore the copy constructor
    const ParmVarDecl* pParam = consDecl->getParamDecl(0);
    std::string aParam1 = pParam->getOriginalType().getAsString();
    if (aParam1.find("vector") != string::npos
        || aParam1.find("deque") != string::npos
        || aParam1.find("initializer_list") != string::npos
        || aParam1.find("iterator") != string::npos)
        return true;

     // found a call to the 1-arg vector constructor, now look for the VarDecl it belongs to

     const Stmt* parent = expr;
     do {
         parent = parentStmt(parent);
         if (!parent) break;
         if (isa<DeclStmt>(parent))
         {
             const DeclStmt* declStmt = dyn_cast<DeclStmt>(parent);
             const Decl* decl = declStmt->getSingleDecl();
             if (decl && isa<VarDecl>(decl))
                 suspectSet.insert(dyn_cast<VarDecl>(decl)->getCanonicalDecl());
             break;
         }
     } while (true);

    return true;
}

loplugin::Plugin::Registration< BadVectorInit > X("badvectorinit", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
