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

#include <sal/config.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include <codemaker/generatedtypeset.hxx>
#include <codemaker/typemanager.hxx>
#include <rtl/ref.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>
#include <sal/main.h>
#include <sal/types.h>
#include <unoidl/unoidl.hxx>

#include "javaoptions.hxx"
#include "javatype.hxx"

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv) {
    JavaOptions options;
    try {
        if (!options.initOptions(argc, argv)) {
            return EXIT_FAILURE;
        }

        rtl::Reference typeMgr(new TypeManager);
        for (const OString& i : options.getExtraInputFiles())
        {
            typeMgr->loadProvider(convertToFileUrl(i), false);
        }
        for (const OString& i : options.getInputFiles())
        {
            typeMgr->loadProvider(convertToFileUrl(i), true);
        }
        codemaker::GeneratedTypeSet generated;
        if (options.isValid("-T")) {
            OUString names(b2u(options.getOption("-T")));
            for (sal_Int32 i = 0; i != -1;) {
                OUString name(names.getToken(0, ';', i));
                if (!name.isEmpty()) {
                    produce(
                        (name == "*"
                         ? ""
                         : name.endsWith(".*")
                         ? name.copy(0, name.getLength() - std::strlen(".*"))
                         : name),
                        typeMgr, generated, options);
                }
            }
        } else {
            produce("", typeMgr, generated, options);
        }
    }
    catch (CannotDumpException & e) {
        std::cerr << "ERROR: " << e.getMessage() << '\n';
        return EXIT_FAILURE;
    } catch (unoidl::NoSuchFileException & e) {
        std::cerr << "ERROR: No such file <" << e.getUri() << ">\n";
        return EXIT_FAILURE;
    } catch (unoidl::FileFormatException & e) {
        std::cerr
            << "ERROR: Bad format of <" << e.getUri() << ">, \""
            << e.getDetail() << "\"\n";
        return EXIT_FAILURE;
    } catch (IllegalArgument & e) {
        std::cerr << "Illegal option " << e.m_message << '\n';
        return EXIT_FAILURE;
    } catch (std::exception & e) {
        std::cerr << "Failure " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
