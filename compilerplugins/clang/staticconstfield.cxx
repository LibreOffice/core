/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"
#include "check.hxx"
#include "compat.hxx"
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

/**
 Look for fields which are const, which can be made static const
*/
namespace
{
class StaticConstField : public loplugin::FilteringPlugin<StaticConstField>
{
public:
    explicit StaticConstField(loplugin::InstantiationData const& data)
        : loplugin::FilteringPlugin<StaticConstField>(data)
    {
    }

    void run() override;

    bool TraverseConstructorInitializer(CXXCtorInitializer* init);
    bool TraverseCXXConstructorDecl(CXXConstructorDecl* decl);

private:
    struct Data
    {
        std::vector<CXXCtorInitializer const*> inits;
        std::string value;
    };
    std::unordered_map<FieldDecl const*, Data> m_potentials;
    std::unordered_set<FieldDecl const*> m_excluded;
    CXXConstructorDecl* m_currentConstructor = nullptr;
};

void StaticConstField::run()
{
    std::string fn = handler.getMainFileName();
    loplugin::normalizeDotDotInFilePath(fn);

    // unusual case where a user constructor sets a field to one value, and a copy constructor sets it to a different value
    if (fn == SRCDIR "/sw/source/core/attr/hints.cxx")
        return;
    if (fn == SRCDIR "/oox/source/core/contexthandler2.cxx")
        return;

    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

    for (auto const& pair : m_potentials)
    {
        report(DiagnosticsEngine::Error, "const field can be static", pair.first->getLocation())
            << pair.first->getSourceRange();
        for (CXXCtorInitializer const* init : pair.second.inits)
            if (pair.first->getLocation() != init->getSourceLocation())
                report(DiagnosticsEngine::Note, "init here", init->getSourceLocation())
                    << init->getSourceRange();
    }
}

bool StaticConstField::TraverseCXXConstructorDecl(CXXConstructorDecl* decl)
{
    auto prev = m_currentConstructor;
    m_currentConstructor = decl;
    bool ret = FilteringPlugin::TraverseCXXConstructorDecl(decl);
    m_currentConstructor = prev;
    return ret;
}

bool StaticConstField::TraverseConstructorInitializer(CXXCtorInitializer* init)
{
    if (!init->getSourceLocation().isValid() || ignoreLocation(init->getSourceLocation()))
        return true;
    if (!init->getMember())
        return true;
    if (!init->getInit())
        return true;
    if (!m_currentConstructor || m_currentConstructor->isCopyOrMoveConstructor())
        return true;
    if (!m_currentConstructor->getParent()->isCompleteDefinition())
        return true;
    if (m_excluded.find(init->getMember()) != m_excluded.end())
        return true;
    auto type = init->getMember()->getType();
    auto tc = loplugin::TypeCheck(type);
    if (!tc.Const())
        return true;

    bool found = false;
    std::string value;
    auto const initexpr = compat::IgnoreImplicit(init->getInit());
    if (tc.Const().Class("OUString").Namespace("rtl").GlobalNamespace()
        || tc.Const().Class("OString").Namespace("rtl").GlobalNamespace())
    {
        if (auto constructExpr = dyn_cast<CXXConstructExpr>(initexpr))
        {
            if (constructExpr->getNumArgs() >= 1
                && isa<clang::StringLiteral>(constructExpr->getArg(0)))
            {
                value = dyn_cast<clang::StringLiteral>(constructExpr->getArg(0))->getString();
                found = true;
            }
        }
    }
    else if (type->isFloatingType())
    {
        APFloat x1(0.0f);
        if (initexpr->EvaluateAsFloat(x1, compiler.getASTContext()))
        {
            std::string s;
            llvm::raw_string_ostream os(s);
            x1.print(os);
            value = os.str();
            found = true;
        }
    }
    // ignore this, it seems to trigger an infinite recursion
    else if (isa<UnaryExprOrTypeTraitExpr>(initexpr))
        ;
    // ignore this, calling EvaluateAsInt on it will crash clang
    else if (initexpr->isValueDependent())
        ;
    else
    {
        APSInt x1;
        if (compat::EvaluateAsInt(initexpr, x1, compiler.getASTContext()))
        {
            value = x1.toString(10);
            found = true;
        }
    }

    if (!found)
    {
        m_potentials.erase(init->getMember());
        m_excluded.insert(init->getMember());
        return true;
    }

    auto findIt = m_potentials.find(init->getMember());
    if (findIt != m_potentials.end())
    {
        if (findIt->second.value != value)
        {
            m_potentials.erase(findIt);
            m_excluded.insert(init->getMember());
        }
        else
            findIt->second.inits.push_back(init);
    }
    else
    {
        Data& data = m_potentials[init->getMember()];
        data.inits.push_back(init);
        data.value = value;
    }

    return true;
}

loplugin::Plugin::Registration<StaticConstField> X("staticconstfield", true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
