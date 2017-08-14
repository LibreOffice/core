/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include "plugin.hxx"
#include "check.hxx"

/**
The types 'long' and 'unsigned long' are different sizes on different platforms, making them wholy unsuitable
for portable code.
And when I mean different sizes, I mean 64bit Linux and 64bit Windows have different sizes.
 */
namespace {

static bool startswith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

class DropLong:
    public RecursiveASTVisitor<DropLong>, public loplugin::Plugin
{
public:
    explicit DropLong(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        std::string fn( compiler.getSourceManager().getFileEntryForID(
                        compiler.getSourceManager().getMainFileID())->getName() );
        normalizeDotDotInFilePath(fn);
        if (startswith(fn, SRCDIR "/sal/"))
            return;
        if (startswith(fn, SRCDIR "/desktop/unx/"))
            return;
        if (startswith(fn, SRCDIR "/bridges/"))
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool shouldVisitTemplateInstantiations () const { return true; }

    bool VisitBinAssign(BinaryOperator const *);
    bool VisitVarDecl(VarDecl const *);
private:
    bool check(QualType lhs, QualType rhs);
};

bool DropLong::VisitBinAssign(BinaryOperator const * expr)
{
    if (ignoreLocation(expr))
        return true;
    auto rhsType = expr->getRHS()->IgnoreCasts()->getType();
    if (check(expr->getLHS()->getType(), rhsType))
    {
        report(
            DiagnosticsEngine::Warning,
            "rather replace long with %0",
            expr->getExprLoc())
          << rhsType
          << expr->getSourceRange();
//        expr->getLHS()->getType()->dump();
    }
    return true;
}

bool DropLong::VisitVarDecl(VarDecl const * varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    if (!varDecl->hasInit())
        return true;
    auto rhsType = varDecl->getInit()->IgnoreCasts()->getType();
    if (check(varDecl->getType(), rhsType))
    {
        report(
            DiagnosticsEngine::Warning,
            "rather replace long with %0",
            varDecl->getLocation())
          << rhsType
          << varDecl->getSourceRange();
//        varDecl->getType()->dump();
    }
    return true;
}

bool DropLong::check(QualType lhs, QualType rhs)
{
    if (!lhs->isSpecificBuiltinType(BuiltinType::Kind::Long)
        && !lhs->isSpecificBuiltinType(BuiltinType::Kind::ULong))
        return false;

    if (rhs->isSpecificBuiltinType(BuiltinType::Kind::Long)
        || rhs->isSpecificBuiltinType(BuiltinType::Kind::ULong))
        return false;

    // Lots of stuff in the standard library and in sal/types.h is
    // 'long' on Linux, so just ignore all typedefs.
    if (loplugin::TypeCheck(lhs).Typedef())
        return false;

    if (isa<SubstTemplateTypeParmType>(lhs))
        return false;
    if (isa<AutoType>(lhs))
        return false;

    return true;
}

loplugin::Plugin::Registration< DropLong > X("droplong", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
