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
#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_UTIL_RANDOMACCESSSTREAM_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_UTIL_RANDOMACCESSSTREAM_HXX

#include <osl/file.hxx>

namespace xmlsearch {

    namespace util {


        class RandomAccessStream
        {
        public:

            virtual ~RandomAccessStream() { };

            // The calle is responsible for allocating the buffer
            virtual void seek( sal_Int32 ) = 0;
            virtual sal_Int32 readBytes( sal_Int8*,sal_Int32 ) = 0;
            virtual void writeBytes( sal_Int8*, sal_Int32 ) = 0;
            virtual sal_Int32 length() = 0;
            virtual void close() = 0;


        protected:

            enum OPENFLAG { Read = osl_File_OpenFlag_Read,
                            Write = osl_File_OpenFlag_Write,
                            Create = osl_File_OpenFlag_Create };

        };


    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
