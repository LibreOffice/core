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

// Description:
// Parse a string of features specified as ; separated pairs.
// e.g.
// 1001=1&2002=2&fav1=0

#ifndef INCLUDED_VCL_INC_GRAPHITE_FEATURES_HXX
#define INCLUDED_VCL_INC_GRAPHITE_FEATURES_HXX

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <graphite_static.hxx>
#include <graphite2/Font.h>

namespace grutils
{
    union FeatId
    {
        gr_uint32 num;
        unsigned char label[5];
    };

    class GrFeatureParser
    {
    public:
        enum { MAX_FEATURES = 64 };
        static const char FEAT_PREFIX;
        static const char FEAT_SEPARATOR;
        static const char FEAT_ID_VALUE_SEPARATOR;
        GrFeatureParser(const gr_face * face, const OString& features, const OString& lang);
        GrFeatureParser(const gr_face * face, const OString& lang);
        ~GrFeatureParser();
        gr_feature_val * values() const { return mpSettings; };
    private:
        GrFeatureParser(const GrFeatureParser & copy) = delete;
        void setLang(const gr_face * face, const OString & lang);
        static bool isCharId(const OString & id, size_t offset, size_t length);
        static gr_uint32 getCharId(const OString & id, size_t offset, size_t length);
        short getIntValue(const OString & id, size_t offset, size_t length);
        size_t mnNumSettings;
        FeatId maLang;
        bool mbErrors;
        sal_uInt32 mnHash;
        gr_feature_val * mpSettings;
    };

}

#endif // INCLUDED_VCL_INC_GRAPHITE_FEATURES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
