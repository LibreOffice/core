/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#if !defined _WIN32 //TODO, #include <sys/mman.h>

#include <cassert>
#include <string>
#include <iostream>
#include "plugin.hxx"
#include "check.hxx"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <cstring>

/**
  This is intended to be run as the second stage of the "constfields" clang plugin.
*/

namespace
{
class ConstFieldsRewrite : public RecursiveASTVisitor<ConstFieldsRewrite>,
                           public loplugin::RewritePlugin
{
public:
    explicit ConstFieldsRewrite(loplugin::InstantiationData const& data);
    ~ConstFieldsRewrite();

    virtual void run() override
    {
        if (rewriter)
        {
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    bool VisitFieldDecl(const FieldDecl* var);

private:
    // I use a brute-force approach - mmap the results file and do a linear search on it
    // It works surprisingly well, because the file is small enough to fit into L2 cache on modern CPU's
    size_t mmapFilesize;
    int mmapFD;
    char* mmappedData;
};

size_t getFilesize(const char* filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}

ConstFieldsRewrite::ConstFieldsRewrite(loplugin::InstantiationData const& data)
    : RewritePlugin(data)
{
    static const char sInputFile[] = SRCDIR "/compilerplugins/clang/constfields.results";
    mmapFilesize = getFilesize(sInputFile);
    //Open file
    mmapFD = open(sInputFile, O_RDONLY, 0);
    assert(mmapFD != -1);
    //Execute mmap
    mmappedData = static_cast<char*>(mmap(NULL, mmapFilesize, PROT_READ, MAP_PRIVATE, mmapFD, 0));
    assert(mmappedData != NULL);
}

ConstFieldsRewrite::~ConstFieldsRewrite()
{
    //Cleanup
    int rc = munmap(mmappedData, mmapFilesize);
    assert(rc == 0);
    close(mmapFD);
}

bool ConstFieldsRewrite::VisitFieldDecl(const FieldDecl* fieldDecl)
{
    if (ignoreLocation(fieldDecl))
        return true;
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
            fieldDecl->getCanonicalDecl()->getLocation())))
        return true;
    // in case we've already processed this field
    if (fieldDecl->getType().isConstQualified())
        return true;
    // in case we've already processed this field
    if (fieldDecl->getType().isConstQualified())
        return true;
    // TODO rewriting T& is a bit trickier
    if (loplugin::TypeCheck(fieldDecl->getType()).LvalueReference())
        return true;

    const RecordDecl* recordDecl = fieldDecl->getParent();
    std::string parentClassName;
    if (const CXXRecordDecl* cxxRecordDecl = dyn_cast<CXXRecordDecl>(recordDecl))
    {
        if (cxxRecordDecl->getTemplateInstantiationPattern())
            cxxRecordDecl = cxxRecordDecl->getTemplateInstantiationPattern();
        parentClassName = cxxRecordDecl->getQualifiedNameAsString();
    }
    else
    {
        parentClassName = recordDecl->getQualifiedNameAsString();
    }
    // the extra spaces match the formatting in the results file, and help avoid false+
    std::string aNiceName = "    " + parentClassName + " " + fieldDecl->getNameAsString() + " "
                            + fieldDecl->getType().getAsString() + "\n";

    // search mmap'ed file for field
    const char* aNiceNameStr = aNiceName.c_str();
    char* found = std::search(mmappedData, mmappedData + mmapFilesize, aNiceNameStr,
                              aNiceNameStr + aNiceName.size());
    if (!(found < mmappedData + mmapFilesize))
        return true;

    auto endLoc = fieldDecl->getTypeSourceInfo()->getTypeLoc().getEndLoc();
    endLoc = endLoc.getLocWithOffset(
        Lexer::MeasureTokenLength(endLoc, compiler.getSourceManager(), compiler.getLangOpts()));

    if (!insertText(endLoc, " const"))
    {
        report(DiagnosticsEngine::Warning, "Could not mark field as const",
               compat::getBeginLoc(fieldDecl))
            << fieldDecl->getSourceRange();
    }
    return true;
}

loplugin::Plugin::Registration<ConstFieldsRewrite> X("constfieldsrewrite", false);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
