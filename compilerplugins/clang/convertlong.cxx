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
  plugin to help to when converting code from

     sal_uIntPtr/sal_uLong/sal_Long/long/unsigned long

  to something more precise.
 */
namespace
{
class ConvertLong : public loplugin::FilteringPlugin<ConvertLong>
{
public:
    explicit ConvertLong(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual void run() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        // using sal_uIntPtr as in-between type when converting void* to rtl_TextEncoding
        if (fn == SRCDIR "/sal/osl/unx/thread.cxx")
            return;
        // too much magic
        if (fn == SRCDIR "/sal/rtl/alloc_arena.cxx")
            return;
        if (fn == SRCDIR "/sal/rtl/alloc_cache.cxx")
            return;
        // TODO not sure what is going on here
        if (fn == SRCDIR "/tools/source/generic/bigint.cxx")
            return;
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitVarDecl(VarDecl const*);
    bool TraverseFunctionDecl(FunctionDecl*);

private:
    bool isInterestingType(QualType qt);
};

bool ConvertLong::TraverseFunctionDecl(FunctionDecl* functionDecl)
{
    // ignore template stuff
    if (functionDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate)
    {
        return true;
    }
    return RecursiveASTVisitor::TraverseFunctionDecl(functionDecl);
}

bool ConvertLong::VisitVarDecl(VarDecl const* varDecl)
{
    if (ignoreLocation(varDecl))
        return true;
    StringRef fileName{ getFileNameOfSpellingLoc(varDecl->getLocation()) };
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/tools/bigint.hxx"))
        return true;
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/tools/solar.h"))
        return true;
    if (!varDecl->hasInit())
        return true;
    if (isa<IntegerLiteral>(varDecl->getInit()->IgnoreParenImpCasts()))
        return true;
    // ignore int x = -1;
    if (isa<UnaryOperator>(varDecl->getInit()->IgnoreParenImpCasts()))
        return true;
    auto lhsType = varDecl->getType();
    auto rhsType = varDecl->getInit()->IgnoreParenImpCasts()->getType();
    if (lhsType.getLocalUnqualifiedType() == rhsType)
        return true;
    if (!rhsType.getTypePtrOrNull())
        return true;
    if (isInterestingType(rhsType))
        return true;
    if (!isInterestingType(lhsType))
        return true;
    if (rhsType->isFloatingType()) // TODO
        return true;
    report(DiagnosticsEngine::Warning, "rather replace type of decl %0 with %1",
           varDecl->getLocation())
        << lhsType << rhsType << varDecl->getSourceRange();
    //lhsType->dump();
    //varDecl->dump();
    return true;
}

bool ConvertLong::isInterestingType(QualType qt)
{
    auto tc = loplugin::TypeCheck(qt);
    if (tc.Typedef())
    {
        TypedefType const* typedefType = qt->getAs<TypedefType>();
        auto name = typedefType->getDecl()->getName();
        if (name == "sal_uLong")
            return true;
        // because this is a typedef to long on 64-bit Linux
        if (name == "sal_Int64" || name == "sal_uInt64" || name.find("size_t") != StringRef::npos)
            return false;
    }
    if (isa<AutoType>(qt.getTypePtr()))
        return false;
    auto unqual = qt.getUnqualifiedType();
    if (unqual->isSpecificBuiltinType(BuiltinType::Kind::Long)
        || unqual->isSpecificBuiltinType(BuiltinType::Kind::ULong))
    {
        return true;
    }
    if (!tc.Typedef())
        return false;
    TypedefType const* typedefType = qt->getAs<TypedefType>();
    auto name = typedefType->getDecl()->getName();
    return name == "sal_uIntPtr" || name == "sal_IntPtr";
}

loplugin::Plugin::Registration<ConvertLong> X("convertlong", false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
