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
#include "registry/reflread.hxx"
#include "fileurl.hxx"
#include "options.hxx"

#include "rtl/ustring.hxx"
#include "osl/diagnose.h"

#include <stdio.h>
#include <string.h>

#include <vector>
#include <string>

using namespace rtl;
using namespace registry::tools;

#define U2S( s ) \
    OUStringToOString(s, RTL_TEXTENCODING_UTF8).getStr()
#define S2U( s ) \
    OStringToOUString(s, RTL_TEXTENCODING_UTF8)

class Options_Impl : public Options
{
public:
    explicit Options_Impl(char const * program)
        : Options (program), m_bForceOutput(false)
        {}

    std::string const & getIndexReg() const
        { return m_indexRegName; }
    std::string const & getTypeReg() const
        { return m_typeRegName; }
    bool hasBase() const
        { return (m_base.getLength() > 0); }
    const OString & getBase() const
        { return m_base; }
    bool forceOutput() const
        { return m_bForceOutput; }

protected:
    virtual void printUsage_Impl() const;
    virtual bool initOptions_Impl (std::vector< std::string > & rArgs);

    std::string m_indexRegName;
    std::string m_typeRegName;
    OString     m_base;
    bool m_bForceOutput;
};

// virtual
void Options_Impl::printUsage_Impl() const
{
    std::string const & rProgName = getProgramName();
    fprintf(stderr,
            "Usage: %s -r<filename> -o<filename> [-options] | @<filename>\n", rProgName.c_str()
            );
    fprintf(stderr,
            "    -o<filename>  = filename specifies the name of the new singleton index registry.\n"
            "    -r<filename>  = filename specifies the name of the type registry.\n"
            "    @<filename>   = filename specifies a command file.\n"
            "Options:\n"
            "    -b<name>  = name specifies the name of a start key. The types will be searched\n"
            "                under this key in the type registry.\n"
            "    -f        = force the output of all found singletons.\n"
            "    -h|-?     = print this help message and exit.\n"
            );
    fprintf(stderr,
            "\nSun Microsystems (R) %s Version 1.0\n\n", rProgName.c_str()
            );
}

// virtual
bool Options_Impl::initOptions_Impl(std::vector< std::string > & rArgs)
{
    std::vector< std::string >::const_iterator first = rArgs.begin(), last = rArgs.end();
    for (; first != last; ++first)
    {
        std::string option (*first);
        if ((*first)[0] != '-')
        {
            return badOption("invalid", option.c_str());
        }
        switch ((*first)[1])
        {
        case 'r':
        case 'R':
            {
                if (!((++first != last) && ((*first)[0] != '-')))
                {
                    return badOption("invalid", option.c_str());
                }
                m_typeRegName = OString((*first).c_str(), (*first).size());
                break;
            }
        case 'o':
        case 'O':
            {
                if (!((++first != last) && ((*first)[0] != '-')))
                {
                    return badOption("invalid", option.c_str());
                }
                m_indexRegName = (*first);
                break;
            }
        case 'b':
        case 'B':
            {
                if (!((++first != last) && ((*first)[0] != '-')))
                {
                    return badOption("invalid", option.c_str());
                }
                m_base = OString((*first).c_str(), (*first).size());
                break;
            }
        case 'f':
        case 'F':
            {
                if ((*first).size() > 2)
                {
                    return badOption("invalid", option.c_str());
                }
                m_bForceOutput = sal_True;
                break;
            }
        case 'h':
        case '?':
            {
                if ((*first).size() > 2)
                {
                    return badOption("invalid", option.c_str());
                }
                return printUsage();
                // break; // unreachable
            }
        default:
            return badOption("unknown", option.c_str());
            // break; // unreachable
        }
    }
    return true;
}

static sal_Bool checkSingletons(Options_Impl const & options, RegistryKey& singletonKey, RegistryKey& typeKey)
{
    RegValueType valueType = RG_VALUETYPE_NOT_DEFINED;
    sal_uInt32 size = 0;
    OUString tmpName;
    sal_Bool bRet = sal_False;

    RegError e = typeKey.getValueInfo(tmpName, &valueType, &size);
    if ((e != REG_VALUE_NOT_EXISTS) && (e != REG_INVALID_VALUE) && (valueType == RG_VALUETYPE_BINARY))
    {
        std::vector< sal_uInt8 > value(size);
        typeKey.getValue(tmpName, &value[0]); // @@@ broken api: write to buffer w/o buffer size.

        RegistryTypeReader reader(&value[0], value.size(), sal_False);
        if ( reader.isValid() && reader.getTypeClass() == RT_TYPE_SINGLETON )
        {
            RegistryKey entryKey;
            OUString    singletonName = reader.getTypeName().replace('/', '.');
            if ( singletonKey.createKey(singletonName, entryKey) )
            {
                fprintf(stderr, "%s: could not create SINGLETONS entry for \"%s\"\n",
                    options.getProgramName().c_str(), U2S( singletonName ));
            }
            else
            {
                bRet = sal_True;
                OUString value2 = reader.getSuperTypeName();

                if ( entryKey.setValue(tmpName, RG_VALUETYPE_UNICODE,
                                       (RegValue)value2.getStr(), sizeof(sal_Unicode)* (value2.getLength()+1)) )
                {
                    fprintf(stderr, "%s: could not create data entry for singleton \"%s\"\n",
                            options.getProgramName().c_str(), U2S( singletonName ));
                }

                if ( options.forceOutput() )
                {
                    fprintf(stderr, "%s: create SINGLETON entry for \"%s\" -> \"%s\"\n",
                            options.getProgramName().c_str(), U2S( singletonName ), U2S(value2));
                }
            }
        }
    }

       RegistryKeyArray subKeys;
    typeKey.openSubKeys(tmpName, subKeys);

    sal_uInt32 length = subKeys.getLength();
    for (sal_uInt32 i = 0; i < length; i++)
    {
        RegistryKey elementKey = subKeys.getElement(i);
        if ( checkSingletons(options, singletonKey, elementKey) )
        {
            bRet = sal_True;
        }
    }
    return bRet;
}

#if (defined UNX) || (defined OS2) || (defined __MINGW32__)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    std::vector< std::string > args;
    for (int i = 1; i < argc; i++)
    {
        int result = Options::checkArgument(args, argv[i], strlen(argv[i]));
        if (result != 0)
        {
            // failure.
            return (result);
        }
    }

    Options_Impl options(argv[0]);
    if (!options.initOptions(args))
    {
        options.printUsage();
        return (1);
    }

    OUString indexRegName( convertToFileUrl(options.getIndexReg().c_str(), options.getIndexReg().size()) );
    Registry indexReg;
    if ( indexReg.open(indexRegName, REG_READWRITE) )
    {
        if ( indexReg.create(indexRegName) )
        {
            fprintf(stderr, "%s: open registry \"%s\" failed\n",
                    options.getProgramName().c_str(), options.getIndexReg().c_str());
            return (2);
        }
    }

    OUString typeRegName( convertToFileUrl(options.getTypeReg().c_str(), options.getTypeReg().size()) );
    Registry typeReg;
    if ( typeReg.open(typeRegName, REG_READONLY) )
    {
        fprintf(stderr, "%s: open registry \"%s\" failed\n",
                options.getProgramName().c_str(), options.getTypeReg().c_str());
        return (3);
    }

    RegistryKey indexRoot;
    if ( indexReg.openRootKey(indexRoot) )
    {
        fprintf(stderr, "%s: open root key of registry \"%s\" failed\n",
                options.getProgramName().c_str(), options.getIndexReg().c_str());
        return (4);
    }

    RegistryKey typeRoot;
    if ( typeReg.openRootKey(typeRoot) )
    {
        fprintf(stderr, "%s: open root key of registry \"%s\" failed\n",
                options.getProgramName().c_str(), options.getTypeReg().c_str());
        return (5);
    }

    RegistryKey typeKey;
    if ( options.hasBase() )
    {
        if ( typeRoot.openKey(S2U(options.getBase()), typeKey) )
        {
            fprintf(stderr, "%s: open base key of registry \"%s\" failed\n",
                    options.getProgramName().c_str(), options.getTypeReg().c_str());
            return (6);
        }
    }
    else
    {
        typeKey = typeRoot;
    }

    RegistryKey singletonKey;
    if ( indexRoot.createKey(OUString::createFromAscii("SINGLETONS"), singletonKey) )
    {
        fprintf(stderr, "%s: open/create SINGLETONS key of registry \"%s\" failed\n",
                options.getProgramName().c_str(), options.getIndexReg().c_str());
        return (7);
    }

    sal_Bool bSingletonsExist = checkSingletons(options, singletonKey, typeKey);

    indexRoot.releaseKey();
    typeRoot.releaseKey();
    typeKey.releaseKey();
    singletonKey.releaseKey();
    if ( indexReg.close() )
    {
        fprintf(stderr, "%s: closing registry \"%s\" failed\n",
                options.getProgramName().c_str(), options.getIndexReg().c_str());
        return (9);
    }
    if ( !bSingletonsExist )
    {
        if ( indexReg.destroy(OUString()) )
        {
            fprintf(stderr, "%s: destroy registry \"%s\" failed\n",
                    options.getProgramName().c_str(), options.getIndexReg().c_str());
            return (10);
        }
    }
    if ( typeReg.close() )
    {
        fprintf(stderr, "%s: closing registry \"%s\" failed\n",
                options.getProgramName().c_str(), options.getTypeReg().c_str());
        return (11);
    }
}
