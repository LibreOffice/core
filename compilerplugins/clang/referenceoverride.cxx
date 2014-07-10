/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <vector>
#include <sstream>

#include "clang/AST/Attr.h"
#include "clang/AST/CXXInheritance.h"

#include "compat.hxx"
#include "plugin.hxx"

/**
 Try to enforce some basic rules around the usage of our reference-counted classes.

 e.g.
  - only inherit from one reference-counting type of base-class, so we don't have
    two different reference-counting mechanisms at work in the same class.
*/
namespace {

class ReferenceOverride:
    public RecursiveASTVisitor<ReferenceOverride>, public loplugin::Plugin
{
public:
    explicit ReferenceOverride(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitClassTemplateDecl(ClassTemplateDecl const * decl);
    bool VisitCXXRecordDecl(CXXRecordDecl const * decl);

    std::string getInheritanceChain(const CXXBasePath& path);
    std::string dumpPaths(const std::vector<std::string>& vec);

    std::vector<std::string> pathsToSimpleReferenceObject, // salhelper::SimpleReferenceObject
                             pathsToSimpleReferenceObjectVirtual,
                             pathsToOWeakObject, //cppu::OWeakObject
                             pathsToReferenceObject, // salhelper::ReferenceObject
                             pathsToSvRefBase; // tools::SvRefBase

};

bool ReferenceOverride::VisitClassTemplateDecl(ClassTemplateDecl const *) {
// TODO validate templates that extend reference classes
    return true;
}

bool forallBasesCallback(const CXXBaseSpecifier *Specifier, CXXBasePath &path, void *UserData) {
    ReferenceOverride* ro = (ReferenceOverride*) UserData;
    QualType qt = Specifier->getType();
    std::string name = qt.getUnqualifiedType().getCanonicalType().getAsString();
    if (name == "class cppu::OWeakObject") {
          ro->pathsToOWeakObject.push_back( ro->getInheritanceChain(path) );
    }
    else if (name == "class salhelper::SimpleReferenceObject") {
        if (Specifier->isVirtual()) {
            ro->pathsToSimpleReferenceObjectVirtual.push_back( ro->getInheritanceChain(path) );
        } else {
            ro->pathsToSimpleReferenceObject.push_back( ro->getInheritanceChain(path) );
        }
    }
    else if (name == "class salhelper::ReferenceObject") {
        ro->pathsToReferenceObject.push_back( ro->getInheritanceChain(path) );
    }
    else if (name == "class tools::SvRefBase") {
        ro->pathsToSvRefBase.push_back( ro->getInheritanceChain(path) );
    }
    return false;
}

std::string ReferenceOverride::getInheritanceChain(const CXXBasePath& path) {
    std::stringstream tmp;
    int i = 0;
    for (CXXBasePath::const_iterator it = path.begin(); it != path.end();
         ++it) {
      std::string s1 = it->Base->getType().getAsString();
      s1.resize(40, ' ');
      tmp << "\t\t" << i << " "
          << s1 << "  "
          << it->Base->getLocStart().printToString(compiler.getSourceManager())
          << "\n";
      i++;
    }
    return tmp.str();
}

bool ReferenceOverride::VisitCXXRecordDecl(CXXRecordDecl const * decl) {
    if (ignoreLocation(decl)) {
        return true;
    }
    if (!decl->hasDefinition()) {
        return true;
    }
    if (decl->getNumBases() == 0) {
        return true;
    }

    std::string declQualifiedname = decl->getQualifiedNameAsString();
    /* exclude weird cppu::OWeakAggObject case in svtools/source/graphic/graphic.hxx
       <sberg> noelgrandin, you're probably missing nothing, just discovered a case of poor (albeit "working") design
       ---Graphic::acquire/release forward to the GraphicDescriptor implementation while the GraphicTransformer one is unused
    */
    if (declQualifiedname == "unographic::Graphic") {
        return true;
    }
    /* exclude another cppu::OWeakAggObject case in include/svx/unoshape.hxx */
    if (declQualifiedname == "SvxCustomShape") {
        return true;
    }
    /* exclude another cppu::OWeakAggObject case in sc/inc/fielduno.hxx */
    if (declQualifiedname == "ScEditFieldObj") {
        return true;
    }

    /* this one is very dodgy (it's not even using OWeakAggObject) case in dbaccess/source/ui/uno/textconnectionsettings_uno.cxx */
    if (declQualifiedname == "dbaui::OTextConnectionSettingsDialog") {
        return true;
    }
    /* another dodgy one in sw/inc/unoframe.hxx */
    if (declQualifiedname == "SwXTextFrame") {
        return true;
    }
    /* another dodgy one in sw/inc/unoframe.hxx */
    if (declQualifiedname == "SwXTextGraphicObject") {
        return true;
    }
    /* another dodgy one in sw/inc/unoframe.hxx */
    if (declQualifiedname == "SwXTextEmbeddedObject") {
        return true;
    }
    /* another dodgy one in sw/inc/unotxdoc.hxx */
    if (declQualifiedname == "SwXTextDocument") {
        return true;
    }

    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              decl->getCanonicalDecl()->getLocStart()))) {
        return true;
    }

    pathsToOWeakObject.clear();
    pathsToSimpleReferenceObject.clear();
    pathsToSimpleReferenceObjectVirtual.clear();
    pathsToReferenceObject.clear();
    pathsToSvRefBase.clear();
    CXXBasePaths paths;
    decl->lookupInBases(forallBasesCallback, this, paths);
    if ( pathsToOWeakObject.size() +
         !pathsToSimpleReferenceObjectVirtual.empty() +
         pathsToSimpleReferenceObject.size() +
         pathsToReferenceObject.size() +
         pathsToSvRefBase.size() <= 1 ) {
        return true;
    }

     std::stringstream tmp;
     if ( !pathsToOWeakObject.empty() )
         tmp << "\tcppu::OWeakObject via:\n" << dumpPaths(pathsToOWeakObject);
     if ( !pathsToSimpleReferenceObjectVirtual.empty() )
         tmp << "\tvirtual salhelper::SimpleReferenceObject via:\n" << dumpPaths(pathsToSimpleReferenceObjectVirtual);
     if ( !pathsToSimpleReferenceObject.empty() )
         tmp << "\tsalhelper::SimpleReferenceObject via:\n" << dumpPaths(pathsToSimpleReferenceObject);
     if ( !pathsToReferenceObject.empty() )
         tmp << "\tsalhelper::ReferenceObject via:\n" << dumpPaths(pathsToReferenceObject);
     if ( !pathsToSvRefBase.empty() )
         tmp << "\ttools::SvRefBase via:\n" << dumpPaths(pathsToSvRefBase);
    report(
        DiagnosticsEngine::Warning,
        declQualifiedname + " extends\n" + tmp.str() + ", more than one reference-counted base not good",
        decl->getLocation())
        << decl->getSourceRange();

    return true;
}

std::string ReferenceOverride::dumpPaths(const std::vector<std::string>& vec) {
    std::stringstream tmp;
    bool first = true;
    for (std::vector<std::string>::const_iterator it = vec.begin(); it != vec.end();
         ++it) {
      if (!first)
          tmp << "\t  and\n";
      tmp << *it;
      first = false;
    }
    return tmp.str();
}

loplugin::Plugin::Registration<ReferenceOverride> X("referenceoverride", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
