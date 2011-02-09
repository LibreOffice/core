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

// Description:
// Parse a string of features specified as ; separated pairs.
// e.g.
// 1001=1&2002=2&fav1=0
#include <graphite/GrClient.h>
#include <graphite/Font.h>
#include <graphite/GrFeature.h>

namespace grutils
{

    class GrFeatureParser
    {
    public:
        enum { MAX_FEATURES = 64 };
        static const char FEAT_PREFIX;
        static const char FEAT_SEPARATOR;
        static const char FEAT_ID_VALUE_SEPARATOR;
        GrFeatureParser(gr::Font & font, const std::string features, const std::string lang);
        GrFeatureParser(gr::Font & font, const std::string lang);
        GrFeatureParser(const GrFeatureParser & copy);
        ~GrFeatureParser();
        size_t getFontFeatures(gr::FeatureSetting settings[MAX_FEATURES]) const;
        bool parseErrors() { return mbErrors; };
        static bool isValid(gr::Font & font, gr::FeatureSetting & setting);
        gr::isocode getLanguage() const { return maLang; };
        bool hasLanguage() const { return (maLang.rgch[0] != '\0'); }
        sal_Int32 hashCode() const;
    private:
        void setLang(gr::Font & font, const std::string & lang);
        bool isCharId(const std::string & id, size_t offset, size_t length);
        int getCharId(const std::string & id, size_t offset, size_t length);
        int getIntValue(const std::string & id, size_t offset, size_t length);
        size_t mnNumSettings;
        gr::isocode maLang;
        bool mbErrors;
        gr::FeatureSetting maSettings[64];
    };

    union FeatId
    {
        gr::featid num;
        unsigned char label[5];
    };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
