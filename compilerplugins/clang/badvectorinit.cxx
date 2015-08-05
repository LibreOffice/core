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
someone wants a filled vector isnt too much to ask and less prone to
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
    explicit BadVectorInit(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitCXXConstructExpr(const CXXConstructExpr* );
};

bool BadVectorInit::VisitCXXConstructExpr(const CXXConstructExpr* expr)
{
    if (ignoreLocation( expr ))
        return true;

    const CXXConstructorDecl *consDecl = expr->getConstructor();
    consDecl = consDecl->getCanonicalDecl();

    if (consDecl->param_size() == 0)
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

    report(
        DiagnosticsEngine::Warning,
        "calling 1-arg vector constructor, rather use resize(x) or C++ brace initialisation syntax",
        expr->getLocStart())
        << expr->getSourceRange();

    return true;
}

loplugin::Plugin::Registration< BadVectorInit > X("badvectorinit", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
