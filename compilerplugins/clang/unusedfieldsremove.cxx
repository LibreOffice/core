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
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <cstring>

/**
  This is intended to be run as the second stage of the "unusedfields" clang plugin.
*/

namespace {

class UnusedFieldsRemove:
    public RecursiveASTVisitor<UnusedFieldsRemove>, public loplugin::RewritePlugin
{
public:
    explicit UnusedFieldsRemove(InstantiationData const & data);
    ~UnusedFieldsRemove();

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitFieldDecl( const FieldDecl* var );
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

UnusedFieldsRemove::UnusedFieldsRemove(InstantiationData const & data): RewritePlugin(data)
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

UnusedFieldsRemove::~UnusedFieldsRemove()
{
    //Cleanup
    int rc = munmap(mmappedData, mmapFilesize);
    assert(rc == 0);
    close(mmapFD);
}

std::string niceName(const FieldDecl* fieldDecl)
{
    return fieldDecl->getParent()->getQualifiedNameAsString() + " " +
        fieldDecl->getNameAsString();
}

bool UnusedFieldsRemove::VisitFieldDecl( const FieldDecl* fieldDecl )
{
    if (rewriter == nullptr) {
        return true;
    }
    if (ignoreLocation(fieldDecl)) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(
                              fieldDecl->getCanonicalDecl()->getLocation()))) {
        return true;
    }

    // don't mess with templates
/*    if (isa<CXXRecordDecl>(fieldDecl->getParent())) {
        if (dyn_cast<CXXRecordDecl>(fieldDecl->getParent())->getDescribedClassTemplate() != nullptr) {
            return true;
        }
    }
*/
    std::string aNiceName = " " + niceName(fieldDecl) + "\n";
    const char *aNiceNameStr = aNiceName.c_str();
    char* found = std::search(mmappedData, mmappedData + mmapFilesize, aNiceNameStr, aNiceNameStr + strlen(aNiceNameStr));
    if(!(found < mmappedData + mmapFilesize)) {
        return true;
    }
    SourceRange replaceRange(fieldDecl->getSourceRange());
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
            "Could not remove unused field (" + niceName(fieldDecl) + ")",
            fieldDecl->getLocStart())
          << fieldDecl->getSourceRange();
    }
    return true;
}


loplugin::Plugin::Registration< UnusedFieldsRemove > X("unusedfieldsremove", false);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
