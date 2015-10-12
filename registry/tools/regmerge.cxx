/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "registry/registry.hxx"
#include "fileurl.hxx"
#include "options.hxx"

#include "rtl/ustring.hxx"
#include "osl/diagnose.h"

#include <stdio.h>
#include <string.h>

using namespace registry::tools;

class Options_Impl : public Options
{
    bool m_bVerbose;

public:
    explicit Options_Impl (char const * program)
        : Options(program), m_bVerbose(false)
    {}
    bool isVerbose() const { return m_bVerbose; }

protected:
    virtual void printUsage_Impl() const override;
    virtual bool initOptions_Impl(std::vector< std::string > & rArgs) override;
};

void Options_Impl::printUsage_Impl() const
{
    fprintf(stderr, "using: regmerge [-v|--verbose] mergefile mergeKeyName regfile_1 ... regfile_n\n");
    fprintf(stderr, "       regmerge @regcmds\nOptions:\n");
    fprintf(stderr, "  -v, --verbose : verbose output on stdout.\n");
    fprintf(stderr, "  mergefile     : specifies the merged registry file. If this file doesn't exists,\n");
    fprintf(stderr, "                  it is created.\n");
    fprintf(stderr, "  mergeKeyName  : specifies the merge key, everything is merged under this key.\n");
    fprintf(stderr, "                  If this key doesn't exists, it is created.\n");
    fprintf(stderr, "  regfile_1..n  : specifies one or more registry files which are merged.\n");
}

bool Options_Impl::initOptions_Impl (std::vector< std::string > & rArgs)
{
    std::vector< std::string >::iterator first = rArgs.begin(), last = rArgs.end();
    if ((first != last) && ((*first)[0] == '-'))
    {
        std::string option(*first);
        if ((option.compare("-v") == 0) || (option.compare("--verbose") == 0))
        {
            m_bVerbose = true;
        }
        else if ((option.compare("-h") == 0) || (option.compare("-?") == 0))
        {
            return printUsage();
        }
        else
        {
            return badOption("unknown", option.c_str());
        }
        (void) rArgs.erase(first);
    }
    return true;
}

#if (defined UNX)
int main( int argc, char * argv[] )
#else
int __cdecl main( int argc, char * argv[] )
#endif
{
    Options_Impl options(argv[0]);

    std::vector< std::string > args;
    for (int i = 1; i < argc; i++)
    {
        if (!Options::checkArgument(args, argv[i], strlen(argv[i])))
        {
            options.printUsage();
            return 1;
        }
    }
    if (!options.initOptions(args))
    {
        return 1;
    }
    if (args.size() < 3)
    {
        options.printUsage();
        return 1;
    }

    Registry reg;
    OUString regName( convertToFileUrl(args[0].c_str(), args[0].size()) );
    if (reg.open(regName, RegAccessMode::READWRITE) != RegError::NO_ERROR)
    {
        if (reg.create(regName) != RegError::NO_ERROR)
        {
            if (options.isVerbose())
                fprintf(stderr, "open registry \"%s\" failed\n", args[0].c_str());
            return -1;
        }
    }

    RegistryKey rootKey;
    if (reg.openRootKey(rootKey) != RegError::NO_ERROR)
    {
        if (options.isVerbose())
            fprintf(stderr, "open root key of registry \"%s\" failed\n", args[0].c_str());
        return -4;
    }

    OUString mergeKeyName( OUString::createFromAscii(args[1].c_str()) );
    for (size_t i = 2; i < args.size(); i++)
    {
        OUString targetRegName( convertToFileUrl(args[i].c_str(), args[i].size()) );
        RegError _ret = reg.mergeKey(rootKey, mergeKeyName, targetRegName, false, options.isVerbose());
        if (_ret != RegError::NO_ERROR)
        {
            if (_ret == RegError::MERGE_CONFLICT)
            {
                if (options.isVerbose())
                    fprintf(stderr, "merging registry \"%s\" under key \"%s\" in registry \"%s\".\n",
                            args[i].c_str(), args[1].c_str(), args[0].c_str());
            }
            else
            {
                if (options.isVerbose())
                    fprintf(stderr, "ERROR: merging registry \"%s\" under key \"%s\" in registry \"%s\" failed.\n",
                            args[i].c_str(), args[1].c_str(), args[0].c_str());
                return -2;
            }
        }
        else
        {
            if (options.isVerbose())
                fprintf(stderr, "merging registry \"%s\" under key \"%s\" in registry \"%s\".\n",
                        args[i].c_str(), args[1].c_str(), args[0].c_str());
        }
    }

    rootKey.releaseKey();
    if (reg.close() != RegError::NO_ERROR)
    {
        if (options.isVerbose())
            fprintf(stderr, "closing registry \"%s\" failed\n", args[0].c_str());
        return -5;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
