/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// A more aggressive check for unused C struct/C++ class members than what plain Clang offers.  On
// the one hand, unlike -Wunused-private-field, it warns about all members regardless of access
// specifiers, if all code that can use a class has been seen.  On the other hand, it warns about
// all kinds of members.  But it uses some heuristics (the type showing up in sizeof, alignof,
// offsetof, certain casts) to determine that seemingly unused data members are probably used after
// all; the used heuristics were enough to not require any explicit [[maybe_unused]] decorations
// across the existing code base.

#include <cassert>
#include <set>

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

namespace
{
// Whether the CXXRecordDecl itself or one of its enclosing classes is a template:
bool isTemplated(CXXRecordDecl const* decl)
{
    if (decl->getDescribedClassTemplate() != nullptr)
    {
        return true;
    }
    if (auto const d = dyn_cast<CXXRecordDecl>(decl->getParent()))
    {
        return isTemplated(d);
    }
    return false;
}

bool isWarnUnusedType(QualType type)
{
    if (auto const t = type->getAs<RecordType>())
    {
        if (t->getDecl()->hasAttr<WarnUnusedAttr>())
        {
            return true;
        }
    }
    return loplugin::isExtraWarnUnusedType(type);
}

class UnusedMember final : public loplugin::FilteringPlugin<UnusedMember>
{
public:
    explicit UnusedMember(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool VisitDeclaratorDecl(DeclaratorDecl const* decl)
    {
        // For declarations like
        //
        //   enum E { ... } e;
        //
        // it may be that the declaration of E is not marked as referenced even though the
        // declaration of e clearly references it:
        if (auto const t = decl->getType()->getAs<EnumType>())
        {
            deferred_.erase(t->getDecl());
        }
        return true;
    }

    bool VisitCXXRecordDecl(CXXRecordDecl const* decl) //TODO: non-CXX RecordDecl?
    {
        if (ignoreLocation(decl))
        {
            return true;
        }
        if (!decl->isThisDeclarationADefinition())
        {
            return true;
        }
        if (!handler.isAllRelevantCodeDefined(decl))
        {
            return true;
        }
        if (!compiler.getSourceManager().isInMainFile(decl->getLocation()))
        {
            // include/rtl/instance.hxx declares entities in an unnamed namespace
            return true;
        }
        if (isTemplated(decl) || isa<ClassTemplatePartialSpecializationDecl>(decl))
        {
            return true;
        }
        if (decl->isUnion() && decl->getIdentifier() == nullptr)
        {
            return true; //TODO
        }
        for (auto i = decl->decls_begin(); i != decl->decls_end(); ++i)
        {
            auto const d = *i;
            if (d->isImplicit() || isa<AccessSpecDecl>(d) || isa<UsingDecl>(d))
            {
                //TODO: only filter out UsingDecls that are actually used (if only to silence
                // -Woverloaded-virtual)
                continue;
            }
            if (isa<ClassTemplateDecl>(d) || isa<FunctionTemplateDecl>(d))
            {
                //TODO: only filter out ones that are not instantiated at all
                continue;
            }
            if (auto const d1 = dyn_cast<FriendDecl>(d))
            {
                //TODO: determine whether the friendship is actually required
                auto const d2 = d1->getFriendDecl();
                if (d2 == nullptr)
                { // happens for "friend class C;"
                    continue;
                }
                if (auto const d3 = dyn_cast<FunctionDecl>(d2))
                {
#if 0 //TODO: friend function definitions are not marked as referenced even if used?
                    if (!d3->isThisDeclarationADefinition()) //TODO: do this check for all kinds?
#else
                    (void)d3;
#endif
                    {
                        continue;
                    }
                }
            }
            if (d->isReferenced())
            {
                continue;
            }
            if (d->hasAttr<UnusedAttr>())
            {
                continue;
            }
            // Check individual members instead of the whole CXXRecordDecl for coming from a macro,
            // as CppUnit's CPPUNIT_TEST_SUITE_END (cppunit/extensions/HelperMacros.h) contains a
            // partial member list ending in
            //
            //    private: /* dummy typedef so that the macro can still end with ';'*/
            //      typedef int CppUnitDummyTypedefForSemiColonEnding__
            //
            if (compiler.getSourceManager().isMacroBodyExpansion(d->getLocation()))
            {
                return true;
            }
            if (auto const d1 = dyn_cast<FieldDecl>(d))
            {
                if (d1->isUnnamedBitfield())
                {
                    continue;
                }
                if (!isWarnWhenUnusedType(d1->getType()))
                {
                    continue;
                }
                deferred_.insert(d1);
                continue;
            }
            if (auto const d1 = dyn_cast<FunctionDecl>(d))
            {
                if (d1->isDeletedAsWritten()) // TODO: just isDeleted?
                {
                    continue;
                }
                if (d1->isExplicitlyDefaulted())
                {
                    continue;
                }
            }
            else if (auto const d2 = dyn_cast<TagDecl>(d))
            {
                if (d2->getIdentifier() == nullptr)
                {
                    continue;
                }
                if (isa<EnumDecl>(d2))
                {
                    deferred_.insert(d2);
                    continue;
                }
            }
            else if (auto const d3 = dyn_cast<TypedefNameDecl>(d))
            {
                // Some types, like (specializations of) std::iterator_traits, have specific
                // requirements on their members; only covers std::iterator_traits for now (TODO:
                // check that at least some member is actually used)
                // (isa<ClassTemplatePartialSpecializationDecl>(decl) is already filtered out
                // above):
                if (isa<ClassTemplateSpecializationDecl>(decl)
                    && loplugin::DeclCheck(decl).Struct("iterator_traits").StdNamespace())
                {
                    auto const id = d3->getIdentifier();
                    assert(id != nullptr);
                    auto const n = id->getName();
                    if (n == "difference_type" || n == "iterator_category" || n == "pointer"
                        || n == "reference" || n == "value_type")
                    {
                        continue;
                    }
                }
            }
            report(DiagnosticsEngine::Warning, "unused class member", d->getLocation())
                << d->getSourceRange();
        }
        return true;
    }

    bool VisitOffsetOfExpr(OffsetOfExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const t1 = expr->getTypeSourceInfo()->getType();
        if (t1->isTemplateTypeParmType())
        {
            return true;
        }
        RecordDecl const* d;
        if (auto const t2 = t1->getAs<InjectedClassNameType>())
        {
            d = t2->getDecl();
        }
        else
        {
            d = t1->castAs<RecordType>()->getDecl();
        }
        recordRecordDeclAndBases(d);
        return true;
    }

    bool VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        switch (expr->getKind())
        {
            case UETT_SizeOf:
            case UETT_AlignOf:
            case UETT_PreferredAlignOf:
                break;
            default:
                return true;
        }
        if (!expr->isArgumentType())
        {
            return true;
        }
        auto t = expr->getArgumentType();
        if (auto const t1 = t->getAs<ReferenceType>())
        {
            t = t1->getPointeeType();
        }
        if (auto const t1 = t->getAsArrayTypeUnsafe())
        {
            t = compiler.getASTContext().getBaseElementType(t1);
        }
        if (auto const t1 = t->getAs<RecordType>())
        {
            recordRecordDeclAndBases(t1->getDecl());
        }
        return true;
    }

    // Handling implicit, C-style, static and reinterpret casts between void* and record types
    // (though reinterpret_cast would be ruled out by loplugin:redundantcast):
    bool VisitCastExpr(CastExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        auto const t1 = expr->getType();
        auto const t2 = compat::getSubExprAsWritten(expr)->getType();
        if (loplugin::TypeCheck(t1).Pointer().Void())
        {
            recordCastedRecordDecl(t2);
        }
        else if (loplugin::TypeCheck(t2).Pointer().Void())
        {
            recordCastedRecordDecl(t1);
        }
        return true;
    }

    bool VisitCXXReinterpretCastExpr(CXXReinterpretCastExpr const* expr)
    {
        if (ignoreLocation(expr))
        {
            return true;
        }
        recordCastedRecordDecl(expr->getTypeAsWritten());
        recordCastedRecordDecl(expr->getSubExprAsWritten()->getType());
        return true;
    }

    bool VisitElaboratedTypeLoc(ElaboratedTypeLoc tloc)
    {
        if (ignoreLocation(tloc))
        {
            return true;
        }
        auto const tl = tloc.getNamedTypeLoc().getAs<TagTypeLoc>();
        if (tl.isNull())
        {
            return true;
        }
        if (tl.isDefinition())
        {
            return true;
        }
        if (auto const d = dyn_cast<EnumDecl>(tl.getDecl()))
        {
            // For some reason, using an elaborated type specifier in (at least) a FieldDecl, as in
            //
            //   enum E { ... };
            //   enum E e;
            //
            // doesn't cause the EnumDecl to be marked as referenced.  (This should fix it, but note
            // the warning at <https://github.com/llvm/llvm-project/commit/
            // b96ec568715450106b4f1dd4a20c1c14e9bca6c4#diff-019094457f96a6ed0ee072731d447073R396>:
            // "[...] a type written 'struct foo' should be represented as an ElaboratedTypeLoc.  We
            // currently only do that when C++ is enabled [...]"
            deferred_.erase(d->getCanonicalDecl());
        }
        return true;
    }

    void postRun() override
    {
        for (auto const d : deferred_)
        {
            if (auto const d1 = dyn_cast<FieldDecl>(d))
            {
                bool layout = false;
                for (auto d2 = d1->getParent();;)
                {
                    if (layout_.find(d2->getCanonicalDecl()) != layout_.end())
                    {
                        layout = true;
                        break;
                    }
                    // Heuristic to recursively check parent RecordDecl if given RecordDecl is
                    // unnamed and either an anonymous struct (or union, but which are already
                    // filtered out anyway), or defined in a non-static data member declaration
                    // (TODO: which is erroneously approximated here with getTypedefNameForAnonDecl
                    // for now, which fails to filter out RecordDecls in static data member
                    // declarations):
                    if (!(d2->getDeclName().isEmpty()
                          && (d2->isAnonymousStructOrUnion()
                              || d2->getTypedefNameForAnonDecl() == nullptr)))
                    {
                        break;
                    }
                    d2 = dyn_cast<RecordDecl>(d2->getParent());
                    if (d2 == nullptr)
                    {
                        break;
                    }
                }
                if (layout)
                {
                    continue;
                }
            }
            report(DiagnosticsEngine::Warning, "unused class member", d->getLocation())
                << d->getSourceRange();
        }
    }

private:
    void run() override
    {
        if (TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()))
        {
            postRun();
        }
    }

    bool isWarnWhenUnusedType(QualType type)
    {
        auto t = type;
        if (auto const t1 = t->getAs<ReferenceType>())
        {
            t = t1->getPointeeType();
        }
        return t.isTrivialType(compiler.getASTContext()) || isWarnUnusedType(t);
    }

    void recordRecordDeclAndBases(RecordDecl const* decl)
    {
        if (!layout_.insert(decl->getCanonicalDecl()).second)
        {
            return;
        }
        if (auto const d2 = dyn_cast_or_null<CXXRecordDecl>(decl->getDefinition()))
        {
            for (auto i = d2->bases_begin(); i != d2->bases_end(); ++i)
            {
                recordRecordDeclAndBases(i->getType()->castAs<RecordType>()->getDecl());
            }
            //TODO: doesn't iterate vbases, but presence of such would run counter to the layout
            // heuristic anyway
        }
    }

    void recordCastedRecordDecl(QualType type)
    {
        for (auto t = type;;)
        {
            if (auto const t1 = t->getAs<clang::PointerType>())
            {
                t = t1->getPointeeType();
                continue;
            }
            if (auto const t1 = t->getAs<RecordType>())
            {
                recordRecordDeclAndBases(t1->getDecl());
            }
            break;
        }
    }

    // RecordDecls whose layout (i.e., contained FieldDecls) must presumably not be changed:
    std::set<TagDecl const*> layout_;

    std::set<Decl const*> deferred_;
};

loplugin::Plugin::Registration<UnusedMember> unusedmember("unusedmember");
}

// Cannot be shared, uses TraverseStmt().

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
