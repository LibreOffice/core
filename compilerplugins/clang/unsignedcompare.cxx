/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

// Find explicit casts from signed to unsigned integer in comparison against unsigned integer, where
// the cast is presumably used to avoid warnings about signed vs. unsigned comparisons, and could
// thus be replaced with o3tl::make_unsigned for clarity.

#include <cassert>

#include "config_clang.h"

#include "compat.hxx"
#include "plugin.hxx"

namespace
{
// clang::Type::isSignedIntegerType returns true for more types than what C++ defines as signed
// integer types:
bool isSignedIntegerType(QualType type)
{
    if (auto const t = type->getAs<BuiltinType>())
    {
        // Assumes that the only extended signed integer type supported by Clang is Int128:
        switch (t->getKind())
        {
            case BuiltinType::SChar:
            case BuiltinType::Short:
            case BuiltinType::Int:
            case BuiltinType::Long:
            case BuiltinType::LongLong:
            case BuiltinType::Int128:
                return true;
            default:
                break;
        }
    }
    return false;
}

// clang::Type::isUnsignedIntegerType returns true for more types than what C++ defines as signed
// integer types:
bool isUnsignedIntegerType(QualType type)
{
    if (auto const t = type->getAs<BuiltinType>())
    {
        // Assumes that the only extended unsigned integer type supported by Clang is UInt128:
        switch (t->getKind())
        {
            case BuiltinType::UChar:
            case BuiltinType::UShort:
            case BuiltinType::UInt:
            case BuiltinType::ULong:
            case BuiltinType::ULongLong:
            case BuiltinType::UInt128:
                return true;
            default:
                break;
        }
    }
    return false;
}

int getRank(QualType type)
{
    auto const t = type->getAs<BuiltinType>();
    assert(t != nullptr);
    // Assumes that the only extended signed/unsigned integer types supported by Clang are Int128
    // and UInt128:
    switch (t->getKind())
    {
        case BuiltinType::SChar:
        case BuiltinType::UChar:
            return 0;
        case BuiltinType::Short:
        case BuiltinType::UShort:
            return 1;
        case BuiltinType::Int:
        case BuiltinType::UInt:
            return 2;
        case BuiltinType::Long:
        case BuiltinType::ULong:
            return 3;
        case BuiltinType::LongLong:
        case BuiltinType::ULongLong:
            return 4;
        case BuiltinType::Int128:
        case BuiltinType::UInt128:
            return 5;
        default:
            llvm_unreachable("bad integer type");
    }
}

int orderTypes(QualType type1, QualType type2)
{
    auto const r1 = getRank(type1);
    auto const r2 = getRank(type2);
    return r1 < r2 ? -1 : r1 == r2 ? 0 : 1;
}

class UnsignedCompare : public loplugin::FilteringPlugin<UnsignedCompare>
{
public:
    explicit UnsignedCompare(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitBinaryOperator(BinaryOperator const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        // o3tl::make_unsigned requires its argument to be non-negative, but this plugin doesn't
        // check that when it reports its finding, so will produce false positives when the cast is
        // actually meant to e.g. clamp from a large signed type to a small unsigned type.  The
        // assumption is that this will only be likely the case for BO_EQ (==) and BO_NE (!=)
        // comparisons, so filter these out here (not sure what case BO_Cmp (<=>) will turn out to
        // be, so lets keep it here at least for now):
        switch (expr->getOpcode())
        {
#if CLANG_VERSION >= 60000
            case BO_Cmp:
#endif
            case BO_LT:
            case BO_GT:
            case BO_LE:
            case BO_GE:
                break;
            default:
                return true;
        }
        auto const castL = isCastToUnsigned(expr->getLHS());
        auto const castR = isCastToUnsigned(expr->getRHS());
        //TODO(?): Also report somewhat suspicious cases where both sides are cast to unsigned:
        if ((castL == nullptr) == (castR == nullptr))
        {
            return true;
        }
        auto const cast = castL != nullptr ? castL : castR;
        auto const other = castL != nullptr ? expr->getRHS() : expr->getLHS();
        auto const otherT = other->IgnoreImpCasts()->getType();
        if (!isUnsignedIntegerType(otherT))
        {
            return true;
        }
        auto const castFromT = cast->getSubExprAsWritten()->getType();
        auto const castToT = cast->getTypeAsWritten();
        report(DiagnosticsEngine::Warning,
               "explicit cast from %0 to %1 (of %select{smaller|equal|larger}2 rank) in comparison "
               "against %3: if the cast value is known to be non-negative, use o3tl::make_unsigned "
               "instead of the cast",
               cast->getExprLoc())
            << castFromT << castToT << (orderTypes(castToT, castFromT) + 1) << otherT
            << expr->getSourceRange();
        return true;
    }

    bool preRun() override
    {
        return compiler.getLangOpts().CPlusPlus
               && compiler.getPreprocessor()
                      .getIdentifierInfo("LIBO_INTERNAL_ONLY")
                      ->hasMacroDefinition();
    }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

private:
    ExplicitCastExpr const* isCastToUnsigned(Expr const* expr)
    {
        auto const e = dyn_cast<ExplicitCastExpr>(expr->IgnoreParenImpCasts());
        if (e == nullptr)
        {
            return nullptr;
        }
        auto const t1 = e->getTypeAsWritten();
        if (!isUnsignedIntegerType(t1))
        {
            return nullptr;
        }
        auto const e2 = e->getSubExprAsWritten();
        auto const t2 = e2->getType();
        if (!isSignedIntegerType(t2))
        {
            return nullptr;
        }
        // Filter out e.g. `size_t(-1)`:
        if (!e2->isValueDependent())
        {
            if (auto const val = compat::getIntegerConstantExpr(e2, compiler.getASTContext()))
            {
                if (val->isNegative())
                {
                    return nullptr;
                }
            }
        }
        auto loc = compat::getBeginLoc(e);
        while (compiler.getSourceManager().isMacroArgExpansion(loc))
        {
            loc = compiler.getSourceManager().getImmediateMacroCallerLoc(loc);
        }
        // This covers both "plain" code in such include files, as well as expansion of (object-like) macros like
        //
        //   #define SAL_MAX_INT8          ((sal_Int8)   0x7F)
        //
        // defined in such include files:
        if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(loc)))
        { //TODO: '#ifdef LIBO_INTERNAL_ONLY' within UNO include files
            return nullptr;
        }
        return e;
    }
};

loplugin::Plugin::Registration<UnsignedCompare> unsignedcompare("unsignedcompare");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
