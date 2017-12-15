/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include "plugin.hxx"
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <cstring>

/**
  This is intended to be run as the second stage of the "unnecessaryvirtuals" clang plugin.
*/

namespace {

class RemoveVirtuals:
    public RecursiveASTVisitor<RemoveVirtuals>, public loplugin::RewritePlugin
{
public:
    explicit RemoveVirtuals(InstantiationData const & data);
    ~RemoveVirtuals();

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

RemoveVirtuals::RemoveVirtuals(InstantiationData const & data): RewritePlugin(data)
{
    static const char sInputFile[] = SRCDIR "/result.txt";
    mmapFilesize = getFilesize(sInputFile);
    //Open file
    mmapFD = open(sInputFile, O_RDONLY, 0);
    assert(mmapFD != -1);
    //Execute mmap
    mmappedData = static_cast<char*>(mmap(NULL, mmapFilesize, PROT_READ, MAP_PRIVATE | MAP_POPULATE, mmapFD, 0));
    assert(mmappedData != NULL);
}

RemoveVirtuals::~RemoveVirtuals()
{
    //Cleanup
    int rc = munmap(mmappedData, mmapFilesize);
    assert(rc == 0);
    close(mmapFD);
}

std::string niceName(const CXXMethodDecl* functionDecl)
{
    std::string s =
           functionDecl->getParent()->getQualifiedNameAsString() + "::"
           + functionDecl->getReturnType().getAsString() + "-"
           + functionDecl->getNameAsString() + "(";
    for (const ParmVarDecl *pParmVarDecl : functionDecl->params()) {
        s += pParmVarDecl->getType().getAsString();
        s += ",";
    }
    s += ")";
    if (functionDecl->isConst()) {
        s += "const";
    }
    return s;
}

bool RemoveVirtuals::VisitCXXMethodDecl( const CXXMethodDecl* functionDecl )
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

    if (!functionDecl->isVirtualAsWritten()) {
        return true;
    }
    std::string aNiceName = "\n" + niceName(functionDecl) + "\n";
    const char *aNiceNameStr = aNiceName.c_str();
    char* found = std::search(mmappedData, mmappedData + mmapFilesize, aNiceNameStr, aNiceNameStr + strlen(aNiceNameStr));
    if(!(found < mmappedData + mmapFilesize)) {
        return true;
    }
    if (functionDecl->isPure()) {
        if (!removeText(functionDecl->getSourceRange())) {
            report(
                DiagnosticsEngine::Warning,
                "Could not remove unused pure virtual method",
                functionDecl->getLocStart())
              << functionDecl->getSourceRange();
        }
    } else {
        std::string aOrigText = rewriter->getRewrittenText(functionDecl->getSourceRange());
        size_t iVirtualTokenIndex = aOrigText.find_first_of("virtual ");
        if (iVirtualTokenIndex == std::string::npos) {
            return true;
        }
        if (!replaceText(functionDecl->getSourceRange(), aOrigText.replace(iVirtualTokenIndex, strlen("virtual "), ""))) {
            report(
                DiagnosticsEngine::Warning,
                "Could not remove virtual qualifier from method",
                functionDecl->getLocStart())
              << functionDecl->getSourceRange();
        }
    }
    return true;
}


loplugin::Plugin::Registration< RemoveVirtuals > X("removevirtuals", false);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
