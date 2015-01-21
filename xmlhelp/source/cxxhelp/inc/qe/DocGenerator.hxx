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
#ifndef INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_QE_DOCGENERATOR_HXX
#define INCLUDED_XMLHELP_SOURCE_CXXHELP_INC_QE_DOCGENERATOR_HXX

#include <rtl/ref.hxx>
#include <rtl/ustring.hxx>
#include <excep/XmlSearchExceptions.hxx>
#include <util/Decompressor.hxx>


namespace xmlsearch {

    namespace qe {


        class RoleFiller
        {
        public:

            static RoleFiller* STOP() { return &roleFiller_; }

            RoleFiller();

            ~RoleFiller();

            void acquire() { ++m_nRefcount; }
            void release() { if( ! --m_nRefcount ) delete this; }

        private:

            static RoleFiller roleFiller_;

            sal_uInt32     m_nRefcount;

            std::vector< RoleFiller* > fillers_;
        };
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
