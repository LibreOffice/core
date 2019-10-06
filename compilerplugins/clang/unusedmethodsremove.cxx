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
#include "compat.hxx"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <cstring>

/**
  This is intended to be run as the second stage of the "unusedmethods" clang plugin.
*/

namespace {

class UnusedMethodsRemove:
    public loplugin::FilteringRewritePlugin<UnusedMethodsRemove>
{
public:
    explicit UnusedMethodsRemove(loplugin::InstantiationData const & data);
    ~UnusedMethodsRemove();

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitCXXMethodDecl( const CXXMethodDecl* var );
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

UnusedMethodsRemove::UnusedMethodsRemove(loplugin::InstantiationData const & data): FilteringRewritePlugin(data)
{
    static const char sInputFile[] = SRCDIR "/result.txt";
    mmapFilesize = getFilesize(sInputFile);
    //Open file
    mmapFD = open(sInputFile, O_RDONLY, 0);
    assert(mmapFD != -1);
    //Execute mmap
    mmappedData = static_cast<char*>(mmap(NULL, mmapFilesize, PROT_READ, MAP_PRIVATE, mmapFD, 0));
    assert(mmappedData != NULL);
}

UnusedMethodsRemove::~UnusedMethodsRemove()
{
    //Cleanup
    int rc = munmap(mmappedData, mmapFilesize);
    assert(rc == 0);
    (void)rc;
    close(mmapFD);
}

std::string niceName(const CXXMethodDecl* functionDecl)
{
    std::string s =
        functionDecl->getReturnType().getCanonicalType().getAsString()
        + " " + functionDecl->getParent()->getQualifiedNameAsString()
        + "::" + functionDecl->getNameAsString()
        + "(";
    bool bFirst = true;
    for (const ParmVarDecl *pParmVarDecl : functionDecl->parameters()) {
        if (bFirst)
            bFirst = false;
        else
            s += ",";
        s += pParmVarDecl->getType().getCanonicalType().getAsString();
    }
    s += ")";
    if (functionDecl->isConst()) {
        s += " const";
    }
    return s;
}

bool UnusedMethodsRemove::VisitCXXMethodDecl( const CXXMethodDecl* functionDecl )
{
    if (rewriter == nullptr) {
        return true;
    }
    if (ignoreLocation(functionDecl)) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(functionDecl)) {
        return true;
    }

    // don't mess with templates
    if (functionDecl->getParent()->getDescribedClassTemplate() != nullptr) {
        return true;
    }
    if (functionDecl->getTemplatedKind() != FunctionDecl::TK_NonTemplate) {
        return true;
    }

    std::string aNiceName = "\n" + niceName(functionDecl) + "\n";
    const char *aNiceNameStr = aNiceName.c_str();
    char* found = std::search(mmappedData, mmappedData + mmapFilesize, aNiceNameStr, aNiceNameStr + strlen(aNiceNameStr));
    if(!(found < mmappedData + mmapFilesize)) {
        return true;
    }
    SourceRange replaceRange(functionDecl->getSourceRange());
    // sometimes the declaration has a semicolon just after it, and it's much neater to remove that too.
    if (rewriter->getRewrittenText(SourceRange(replaceRange.getEnd(), replaceRange.getEnd().getLocWithOffset(1))) == ";") {
        replaceRange.setEnd(replaceRange.getEnd().getLocWithOffset(1));
    }
    // remove leading spaces
    while (rewriter->getRewrittenText(SourceRange(replaceRange.getBegin().getLocWithOffset(-1), replaceRange.getBegin())) == " ")
    {
        replaceRange.setBegin(replaceRange.getBegin().getLocWithOffset(-1));
    }
    if (!replaceText(replaceRange, "")) {
        report(
            DiagnosticsEngine::Warning,
            "Could not remove unused method (" + niceName(functionDecl) + ")",
            compat::getBeginLoc(functionDecl))
          << functionDecl->getSourceRange();
    }
    return true;
}


loplugin::Plugin::Registration< UnusedMethodsRemove > X("unusedmethodsremove", false);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
