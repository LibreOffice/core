/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <string>
#include <set>

#include "plugin.hxx"

// Look for places calling std::find on a standard container where it should be using the container find method, which
// is more efficient.
//
// This lives in /store because the implementation is a hack and is highly dependent on the inwards
// of the libc++ library on the machine it runs on.
//

namespace {

class FindOnContainer:
    public RecursiveASTVisitor<FindOnContainer>, public loplugin::Plugin
{
public:
    explicit FindOnContainer(InstantiationData const & data): Plugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCallExpr(const CallExpr * expr);
};

bool FindOnContainer::VisitCallExpr(const CallExpr * expr) {
    if (ignoreLocation(expr)) {
        return true;
    }
    FunctionDecl const * fdecl = expr->getDirectCallee();
    if (fdecl == nullptr) {
        return true;
    }
    std::string qname { fdecl->getQualifiedNameAsString() };
    if (qname == "std::find")
    {
        std::string tname = expr->getArg(0)->getType().getAsString();
        if (tname.find("std::_List_iterator") != std::string::npos
            || tname.find("std::_List_const_iterator") != std::string::npos
            || tname.find("std::vector") != std::string::npos
            || tname.find("std::_Deque_iterator") != std::string::npos
            || tname == "const int *"
            || tname == "struct std::_Bit_const_iterator"
            || tname == "const rtl::OUString *"
            || tname == "class rtl::OUString *"
            || tname == "const class rtl::OUString *"
            || tname == "const sal_Int8 *"
            || tname == "const sal_Int32 *"
            || tname == "sal_Int32 *"
            || tname == "sal_uInt16 *" )
        {
            return true;
        }
        expr->dump();
        report(
            DiagnosticsEngine::Warning,
            ("rather use the more specific find method " + tname),
            expr->getExprLoc());
        return true;
    }
    return true;
}

loplugin::Plugin::Registration< FindOnContainer > X("findoncontainer");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
