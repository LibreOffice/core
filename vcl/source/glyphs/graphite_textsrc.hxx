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



#ifndef _SV_GRAPHITETEXTSRC_HXX
#define _SV_GRAPHITETEXTSRC_HXX
// Description: Implements the Graphite interfaces IGrTextSource and
//              IGrGraphics which provide Graphite with access to the
//              app's text storage system and the platform's font and
//              graphics systems.

// We need this to enable namespace support in libgrengine headers.
#define GR_NAMESPACE

// Standard Library
#include <stdexcept>
// Platform

#ifndef _SVWIN_H
#include <tools/svwin.h>
#endif

#include <svsys.h>
#include <salgdi.hxx>
#include <sallayout.hxx>

// Module
#include "vcl/dllapi.h"

// Libraries
#include <preextstl.h>
#include <graphite/GrClient.h>
#include <graphite/Font.h>
#include <graphite/ITextSource.h>
#include <postextstl.h>

// Module type definitions and forward declarations.
//
namespace grutils
{
    class GrFeatureParser;
}
// Implements the Adaptor pattern to adapt the LayoutArgs and the ServerFont interfaces to the
// gr::IGrTextSource interface.
// @author tse
//
class TextSourceAdaptor : public gr::ITextSource
{
public:
        TextSourceAdaptor(ImplLayoutArgs &layout_args, const int nContextLen) throw();
        ~TextSourceAdaptor();
        virtual gr::UtfType     utfEncodingForm();
        virtual size_t          getLength();
        virtual size_t          fetch(gr::toffset ichMin, size_t cch, gr::utf32 * prgchBuffer);
        virtual size_t          fetch(gr::toffset ichMin, size_t cch, gr::utf16 * prgchwBuffer);
        virtual size_t          fetch(gr::toffset ichMin, size_t cch, gr::utf8  * prgchsBuffer);
        virtual bool            getRightToLeft(gr::toffset ich);
        virtual unsigned int    getDirectionDepth(gr::toffset ich);
        virtual float           getVerticalOffset(gr::toffset ich);
        virtual gr::isocode     getLanguage(gr::toffset ich);

        virtual ext_std::pair<gr::toffset, gr::toffset> propertyRange(gr::toffset ich);
        virtual size_t  getFontFeatures(gr::toffset ich, gr::FeatureSetting * prgfset);
        virtual bool    sameSegment(gr::toffset ich1, gr::toffset ich2);
        virtual bool featureVariations() { return false; }

        operator ImplLayoutArgs & () throw();
        void setFeatures(const grutils::GrFeatureParser * pFeatures);
        const ImplLayoutArgs & getLayoutArgs() const { return maLayoutArgs; }
        size_t          getContextLength() const { return mnEnd; };
        inline void switchLayoutArgs(ImplLayoutArgs & newArgs);
private:
        // Prevent the generation of a default assignment operator.
        TextSourceAdaptor & operator=(const TextSourceAdaptor &);

        void getCharProperties(const int, int &, int &, size_t &);

        ImplLayoutArgs  maLayoutArgs;
        size_t    mnEnd;
        const grutils::GrFeatureParser * mpFeatures;
};

inline TextSourceAdaptor::TextSourceAdaptor(ImplLayoutArgs &la, const int nContextLen) throw()
  : maLayoutArgs(la),
    mnEnd(std::min(la.mnLength, nContextLen)),
    mpFeatures(NULL)
{
}

inline  TextSourceAdaptor::operator ImplLayoutArgs & () throw() {
        return maLayoutArgs;
}

inline void TextSourceAdaptor::switchLayoutArgs(ImplLayoutArgs & aNewArgs)
{
    mnEnd += aNewArgs.mnMinCharPos - maLayoutArgs.mnMinCharPos;
    maLayoutArgs = aNewArgs;
}

#endif
