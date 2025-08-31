/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cstdlib>
#include <iostream>
#include <vector>

#include <codemaker/generatedtypeset.hxx>
#include <codemaker/typemanager.hxx>
#include <codemaker/global.hxx>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/main.h>
#include <unoidl/unoidl.hxx>
#include <o3tl/string_view.hxx>

#include "pythonoptions.hxx"
#include "pythontype.hxx"

// coverity[tainted_data] - this is a build time tool
SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    // Object to hold parsed command-line options.
    PythonOptions options;
    try
    {
        // Initialize the options object from command-line arguments.
        // If it fails (e.g., --help or invalid options), it returns false.
        if (!options.initOptions(argc, argv))
        {
            return EXIT_FAILURE;
        }

        rtl::Reference<TypeManager> typeMgr(new TypeManager);
        for (const OString& f : options.getExtraInputFiles())
        {
            typeMgr->loadProvider(convertToFileUrl(f), false);
        }

        // Load the primary RDB files. Types from these files are the main candidates for stub generation.
        for (const OString& f : options.getInputFiles())
        {
            typeMgr->loadProvider(convertToFileUrl(f), true);
        }

        // `GeneratedTypeSet` keeps track of which types have already been processed
        // to avoid redundant work and infinite loops in case of circular dependencies.
        codemaker::GeneratedTypeSet generated;
        if (options.isValid("-T"_ostr))
        {
            OUString names(b2u(options.getOption("-T"_ostr)));
            for (sal_Int32 i = 0; i != -1;)
            {
                std::u16string_view name(o3tl::getToken(names, 0, ';', i));
                if (!name.empty())
                {
                    codemaker::pythonmaker::produce(
                        OUString(name == u"*"
                                     ? u""
                                     : o3tl::ends_with(name, u".*")
                                           ? name.substr(0, name.size() - std::strlen(".*"))
                                           : name),
                        typeMgr, generated, options);
                }
            }
        }
        else
        {
            // If -T is not specified, start the generation process from the root (global) namespace.
            codemaker::pythonmaker::produce(u""_ustr, typeMgr, generated, options);
        }
    }
    // Catch specific, known exceptions to provide clear error messages.
    catch (CannotDumpException const& e)
    {
        std::cerr << "ERROR: " << u2b(e.getMessage()) << '\n';
        return EXIT_FAILURE;
    }
    catch (unoidl::NoSuchFileException const& e)
    {
        std::cerr << "ERROR: No such file <" << u2b(e.getUri()) << ">\n";
        return EXIT_FAILURE;
    }
    catch (unoidl::FileFormatException const& e)
    {
        std::cerr << "ERROR: Bad format of <" << u2b(e.getUri()) << ">, \"" << u2b(e.getDetail())
                  << "\"\n";
        return EXIT_FAILURE;
    }
    catch (IllegalArgument const& e)
    {
        std::cerr << "Illegal option: " << e.m_message << '\n';
        return EXIT_FAILURE;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Standard exception: " << e.what() << '\n';
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "Unknown C++ exception occurred.\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
