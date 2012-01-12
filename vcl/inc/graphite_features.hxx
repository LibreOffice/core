/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// Description:
// Parse a string of features specified as ; separated pairs.
// e.g.
// 1001=1&2002=2&fav1=0
#include <preextstl.h>
#include <graphite/GrClient.h>
#include <graphite/Font.h>
#include <graphite/GrFeature.h>
#include <postextstl.h>

namespace grutils
{

    class GrFeatureParser
    {
    public:
        enum { MAX_FEATURES = 64 };
        static const char FEAT_PREFIX;
        static const char FEAT_SEPARATOR;
        static const char FEAT_ID_VALUE_SEPARATOR;
        static const std::string ISO_LANG;
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
