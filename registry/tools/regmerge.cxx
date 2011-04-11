/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_registry.hxx"

#include "registry/registry.hxx"
#include "fileurl.hxx"
#include "options.hxx"

#include "rtl/ustring.hxx"
#include "osl/diagnose.h"

#include <stdio.h>
#include <string.h>

using namespace rtl;
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
    virtual void printUsage_Impl() const;
    virtual bool initOptions_Impl(std::vector< std::string > & rArgs);
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

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    Options_Impl options(argv[0]);

    std::vector< std::string > args;
    for (int i = 1; i < argc; i++)
    {
        if (!Options::checkArgument(args, argv[i], strlen(argv[i])))
        {
            options.printUsage();
            return (1);
        }
    }
    if (!options.initOptions(args))
    {
        return (1);
    }
    if (args.size() < 3)
    {
        options.printUsage();
        return (1);
    }

    Registry reg;
    OUString regName( convertToFileUrl(args[0].c_str(), args[0].size()) );
    if (reg.open(regName, REG_READWRITE) != REG_NO_ERROR)
    {
        if (reg.create(regName) != REG_NO_ERROR)
        {
            if (options.isVerbose())
                fprintf(stderr, "open registry \"%s\" failed\n", args[0].c_str());
            return (-1);
        }
    }

    RegistryKey rootKey;
    if (reg.openRootKey(rootKey) != REG_NO_ERROR)
    {
        if (options.isVerbose())
            fprintf(stderr, "open root key of registry \"%s\" failed\n", args[0].c_str());
        return (-4);
    }

    OUString mergeKeyName( OUString::createFromAscii(args[1].c_str()) );
    for (size_t i = 2; i < args.size(); i++)
    {
        OUString targetRegName( convertToFileUrl(args[i].c_str(), args[i].size()) );
        RegError _ret = reg.mergeKey(rootKey, mergeKeyName, targetRegName, sal_False, options.isVerbose());
        if (_ret != REG_NO_ERROR)
        {
            if (_ret == REG_MERGE_CONFLICT)
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
                return (-2);
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
    if (reg.close() != REG_NO_ERROR)
    {
        if (options.isVerbose())
            fprintf(stderr, "closing registry \"%s\" failed\n", args[0].c_str());
        return (-5);
    }

    return(0);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */