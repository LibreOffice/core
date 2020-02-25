/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */
#ifndef LO_CLANG_SHARED_PLUGINS

#include "compat.hxx"
#include "plugin.hxx"
#include "check.hxx"
#include <iostream>
#include "clang/AST/CXXInheritance.h"

/*
 * This is a compile-time checker.
 *
 * Check that when we override SvXmlImportContext, and we override createFastChildContext,
 * we have also overridden startFastElement, or the fast-parser stuff will not work
 * correctly.
*/

namespace
{
class XmlImport : public loplugin::FilteringPlugin<XmlImport>
{
public:
    explicit XmlImport(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    bool preRun() override
    {
        //        std::string fn(handler.getMainFileName());
        //        loplugin::normalizeDotDotInFilePath(fn);
        return true;
    }

    void run() override
    {
        if (preRun())
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitCXXMethodDecl(const CXXMethodDecl*);
};

static bool containsStartFastElementMethod(const CXXRecordDecl* cxxRecordDecl,
                                           bool& rbFoundImportContext)
{
    auto dc = loplugin::DeclCheck(cxxRecordDecl);
    if (dc.Class("SvXMLImportContext"))
    {
        rbFoundImportContext = true;
        return false;
    }
    if (dc.Class("XFastContextHandler"))
        return false;
    for (auto it = cxxRecordDecl->method_begin(); it != cxxRecordDecl->method_end(); ++it)
    {
        auto i = *it;
        if (i->getIdentifier() && i->getName() == "startFastElement")
        {
            //            i->dump();
            return true;
        }
    }
    return false;
}
bool XmlImport::VisitCXXMethodDecl(const CXXMethodDecl* methodDecl)
{
    auto beginLoc = compat::getBeginLoc(methodDecl);
    if (!beginLoc.isValid() || ignoreLocation(beginLoc))
        return true;

    auto cxxRecordDecl = methodDecl->getParent();
    if (!cxxRecordDecl || !cxxRecordDecl->getIdentifier())
        return true;
    auto className = cxxRecordDecl->getName();
    if (className == "OOXMLFactory") // writerfilter
        return true;
    if (className == "SvXMLLegacyToFastDocHandler" || className == "ImportDocumentHandler"
        || className == "ExportDocumentHandler") // reportdesign
        return true;
    if (className == "XMLEmbeddedObjectExportFilter" || className == "XMLBasicExportFilter"
        || className == "XMLTransformerBase" || className == "SvXMLMetaExport") // xmloff
        return true;
    if (!methodDecl->getIdentifier())
        return true;
    if (!(methodDecl->getName() == "createFastChildContext" || methodDecl->getName() == "characters"
          || methodDecl->getName() == "endFastElement"))
        return true;
    if (loplugin::DeclCheck(cxxRecordDecl).Class("SvXMLImportContext"))
        return true;

    bool foundImportContext = false;
    if (containsStartFastElementMethod(cxxRecordDecl, foundImportContext))
        return true;

    bool foundStartFastElement = false;
    CXXBasePaths aPaths;
    cxxRecordDecl->lookupInBases(
        [&](const CXXBaseSpecifier* Specifier, CXXBasePath & /*Path*/) -> bool {
            if (!Specifier->getType().getTypePtr())
                return false;
            const CXXRecordDecl* baseCXXRecordDecl = Specifier->getType()->getAsCXXRecordDecl();
            if (!baseCXXRecordDecl)
                return false;
            if (baseCXXRecordDecl->isInvalidDecl())
                return false;
            foundStartFastElement
                |= containsStartFastElementMethod(baseCXXRecordDecl, foundImportContext);
            return false;
        },
        aPaths);

    if (foundStartFastElement || !foundImportContext)
        return true;

    report(DiagnosticsEngine::Warning, "must override startFastElement too",
           compat::getBeginLoc(methodDecl))
        << methodDecl->getSourceRange();
    return true;
}

loplugin::Plugin::Registration<XmlImport> xmlimport("xmlimport");

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
