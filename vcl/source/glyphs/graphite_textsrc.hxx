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

#include <tools/svwin.h>

#include <svsys.h>

#include <vcl/salgdi.hxx>

#include <vcl/sallayout.hxx>

// Module
#include "vcl/dllapi.h"

// Libraries
#include <graphite/GrClient.h>
#include <graphite/Font.h>
#include <graphite/ITextSource.h>

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

        virtual std::pair<gr::toffset, gr::toffset> propertyRange(gr::toffset ich);
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
