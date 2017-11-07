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
The types 'long' and 'unsigned long' are different sizes on different platforms, making them wholly unsuitable
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
    explicit DropLong(loplugin::InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        std::string fn( compiler.getSourceManager().getFileEntryForID(
                        compiler.getSourceManager().getMainFileID())->getName() );
        loplugin::normalizeDotDotInFilePath(fn);
        if (startswith(fn, SRCDIR "/sal/"))
            return;
        if (startswith(fn, SRCDIR "/desktop/unx/"))
            return;
        if (startswith(fn, SRCDIR "/bridges/"))
            return;
        if (startswith(fn, SRCDIR "/registry/"))
            return;
        if (startswith(fn, SRCDIR "/tools/source/generic/fract.cxx"))
            return;
        if (startswith(fn, SRCDIR "/tools/source/generic/bigint.cxx"))
            return;
        // TODO figure out how to cope with iterators
        if (startswith(fn, SRCDIR "/cppu/source/threadpool/jobqueue.cxx"))
            return;

        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitBinAssign(BinaryOperator const *);
    bool VisitVarDecl(VarDecl const *);
    bool VisitCastExpr(CastExpr const *);
private:
    bool isOK(QualType lhs, QualType rhs);
};

bool DropLong::VisitBinAssign(BinaryOperator const * expr)
{
    if (ignoreLocation(expr))
        return true;
    StringRef fileName { compiler.getSourceManager().getFilename(expr->getExprLoc()) };
    if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/include/tools/bigint.hxx"))
        return true;

    auto lhsType = expr->getLHS()->getType();
    auto rhsType = expr->getRHS()->IgnoreCasts()->getType();
    if (!isOK(lhsType, rhsType))
    {
        report(
            DiagnosticsEngine::Warning,
            "rather replace %0 with %1",
            expr->getExprLoc())
          << lhsType
          << rhsType
          << expr->getSourceRange();
//        lhsType->dump();
    }
    return true;
}

bool DropLong::VisitVarDecl(VarDecl const * varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    StringRef fileName { compiler.getSourceManager().getFilename(varDecl->getLocation()) };
    if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/include/tools/bigint.hxx"))
        return true;
    if (!varDecl->hasInit())
        return true;
    auto lhsType = varDecl->getType();
    auto rhsType = varDecl->getInit()->IgnoreCasts()->getType();
    if (!isOK(lhsType, rhsType))
    {
        report(
            DiagnosticsEngine::Warning,
            "rather replace %0 with %1",
            varDecl->getLocation())
          << lhsType
          << rhsType
          << varDecl->getSourceRange();
//        lhsType->dump();
    }
    return true;
}

bool DropLong::VisitCastExpr(CastExpr const * castExpr)
{
    if (ignoreLocation(castExpr))
        return true;
    StringRef fileName { compiler.getSourceManager().getFilename(castExpr->getExprLoc()) };
    if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/include/tools/bigint.hxx"))
        return true;
    if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/include/sal/types.h"))
        return true;
    if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/include/rtl/math.hxx"))
        return true;
    // TODO
    if (loplugin::hasPathnamePrefix(fileName, SRCDIR "/include/tools/helpers.hxx"))
        return true;
    if (isa<ImplicitCastExpr>(castExpr))
        return true;

    auto type = castExpr->getType();
    if (loplugin::TypeCheck(type).Typedef())
    {
        TypedefType const * typedefType = type->getAs<TypedefType>();
        if (typedefType->getDecl()->getName() == "sal_uLong")
            report(
                DiagnosticsEngine::Warning,
                "sal_uLong cast from %0",
                castExpr->getExprLoc())
              << castExpr->getSubExpr()->getType()
              << castExpr->getSourceRange();
    }
    else if (type->isSpecificBuiltinType(BuiltinType::Kind::Long)
         || type->isSpecificBuiltinType(BuiltinType::Kind::ULong))
    {
        report(
            DiagnosticsEngine::Warning,
            "long cast from %0",
            castExpr->getExprLoc())
          << castExpr->getSubExpr()->getType()
          << castExpr->getSourceRange();
    }
    return true;
}

bool DropLong::isOK(QualType lhs, QualType rhs)
{
    if (loplugin::TypeCheck(lhs).Typedef())
    {
        TypedefType const * typedefType = lhs->getAs<TypedefType>();
        // Lots of stuff in the standard library and in sal/types.h is
        // 'long' on Linux, so just ignore all typedefs.
        if (typedefType->getDecl()->getName() != "sal_uLong")
            return true;
    }
    else if (lhs->isSpecificBuiltinType(BuiltinType::Kind::Long)
         || lhs->isSpecificBuiltinType(BuiltinType::Kind::ULong))
    {
        if (rhs->isSpecificBuiltinType(BuiltinType::Kind::Long)
            || rhs->isSpecificBuiltinType(BuiltinType::Kind::ULong))
            return true;
    }
    else
        return true;

    if (isa<SubstTemplateTypeParmType>(lhs))
        return true;
    if (isa<AutoType>(lhs))
        return true;

    return false;
}

loplugin::Plugin::Registration< DropLong > X("droplong", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
