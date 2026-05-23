/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// ScPatternAttr ownership rules (canonical source).
//
// ScPatternAttr is the cell-pattern value object owned by a
// CellAttributeHelper. Each ScDocument creates its helper lazily via
// getCellAttributeHelper(); clipboard/undo documents share the helper
// through ScDocument::SharePooledResources. The helper holds a registry
// keyed by style name; each registered ScPatternAttr has a refcount
// tracked by the helper. CellAttributeHolder is the smart-pointer
// wrapper around an ScPatternAttr that bumps the refcount on assign and
// drops it on destruction; doUnregister destroys the pattern when its
// refcount reaches zero.
//
// The rule: any long-lived reference to an ScPatternAttr is held by
// CellAttributeHolder, not by raw pointer. A short-lived raw const
// ScPatternAttr* in a single function scope is fine if no operation
// that can apply a new pattern to a cell (formula interpretation,
// conditional-format evaluation, style change, SetNumberFormat,
// ApplyAttr, broadcasts on cells, ...) happens between the GetPattern()
// call and the use. A raw pointer stored in a class member, or held in
// a loop variable across such an operation, is a use after free waiting
// to happen; for the chain of prior per-call-site workarounds see
//   git log --grep='ScPatternAttr.*use-after-free' -- engine/sc/
//
// This plugin enforces the rule on class member fields: any class field
// of type (const) ScPatternAttr* is flagged. Allow-listed exceptions,
// named individually below:
//   - CellAttributeHelper and CellAttributeHolder themselves
//   - default-pattern cache fields that point at the helper's default
//     ScPatternAttr (lives for the helper's full lifetime)
//   - paint-pipeline structs whose lifetime is bounded to one paint
//     call: DrawEditParam and the file-local ScDrawStringsVars in
//     output2.cxx
// Additions to the allow list need an explicit entry.
//
// Not flagged: function-scope locals and function parameters. The
// plugin is declaration-AST only; flow-sensitive analysis is out of
// scope. Don't reintroduce raw class fields and don't hold raw locals
// across the operations listed above.

#ifndef LO_CLANG_SHARED_PLUGINS

#include <set>
#include <string>

#include "config_clang.h"

#include "check.hxx"
#include "compat.hxx"
#include "plugin.hxx"

namespace
{
class ScPatternOwnership final : public loplugin::FilteringPlugin<ScPatternOwnership>
{
public:
    explicit ScPatternOwnership(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override { return compiler.getLangOpts().CPlusPlus; }

    void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitFieldDecl(FieldDecl const* decl)
    {
        if (ignoreLocation(decl))
            return true;

        // The enclosing class
        auto const* parent = dyn_cast<CXXRecordDecl>(decl->getParent());
        if (!parent)
            return true;

        // Skip our two owner-ish classes; they implement the model.
        std::string const parentName = parent->getNameAsString();
        if (parentName == "CellAttributeHelper" || parentName == "CellAttributeHolder")
            return true;

        std::string const fieldName = decl->getNameAsString();

        // Default-pattern cache fields whose target is the helper's default
        // ScPatternAttr, which lives for the helper's full lifetime.
        if (parentName == "ScAttrIterator" && fieldName == "pDefPattern")
            return true;
        if (parentName == "AttrList" && fieldName == "mpDefPattern") // oox::xls
            return true;

        // Paint-pipeline structs whose lifetime is bounded to the call that
        // builds them; the pattern is set once, used immediately, and the
        // struct goes out of scope long before the document does. The same
        // applies to the file-local ScDrawStringsVars in output2.cxx.
        if (parentName == "DrawEditParam"
            && (fieldName == "mpPattern" || fieldName == "mpOldPattern"))
            return true;
        if (parentName == "ScDrawStringsVars" && fieldName == "pPattern")
            return true;

        // Is it ScPatternAttr* or const ScPatternAttr*?
        QualType qt = decl->getType();
        if (!qt->isPointerType())
            return true;
        QualType pt = qt->getPointeeType().getUnqualifiedType().getCanonicalType();
        auto* rt = pt->getAs<RecordType>();
        if (!rt)
            return true;
        std::string const pointeeName = rt->getDecl()->getNameAsString();
        if (pointeeName != "ScPatternAttr")
            return true;

        report(DiagnosticsEngine::Warning,
               "class member of type %0 in %1; long-lived references to "
               "ScPatternAttr must be held by CellAttributeHolder",
               decl->getLocation())
            << qt << parentName;
        return true;
    }
};

loplugin::Plugin::Registration<ScPatternOwnership> scpatternownership("scpatternownership");
}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
