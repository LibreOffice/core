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

#ifndef INCLUDED_VCL_METRIC_HXX
#define INCLUDED_VCL_METRIC_HXX

#include <vcl/dllapi.h>
#include <vcl/font.hxx>
#include <vcl/outdev.hxx>

#include <boost/intrusive_ptr.hpp>

class ImplFontMetric;
class ImplFontCharMap;
class CmapResult;

typedef sal_uInt32 sal_UCS4;
typedef boost::intrusive_ptr< ImplFontCharMap > ImplFontCharMapPtr;
typedef boost::intrusive_ptr< FontCharMap > FontCharMapPtr;

class VCL_DLLPUBLIC FontMetric : public vcl::Font
{
public:
    explicit            FontMetric();
                        FontMetric( const FontMetric& );  // TODO make this explicit
    virtual             ~FontMetric();

    FontType            GetType() const;

    long                GetAscent() const;
    long                GetDescent() const;
    long                GetInternalLeading() const;
    long                GetExternalLeading() const;
    long                GetLineHeight() const;
    long                GetSlant() const;
    long                GetBulletOffset() const;

    void                SetAscent(long);
    void                SetDescent(long);
    void                SetExternalLeading(long);
    void                SetInternalLeading(long);
    void                SetLineHeight(long);
    void                SetSlant(long);
    void                SetBulletOffset(long);

    bool                IsScalable() const;
    bool                IsFullstopCentered() const;
    bool                IsBuiltInFont() const;

    void                SetScalableFlag(bool);
    void                SetFullstopCenteredFlag(bool);
    void                SetBuiltInFontFlag(bool);

    FontMetric&         operator=( const FontMetric& rMetric );
    bool                operator==( const FontMetric& rMetric ) const;
    bool                operator!=( const FontMetric& rMetric ) const
                            { return !operator==( rMetric ); }
protected:
    boost::intrusive_ptr<ImplFontMetric> mpImplMetric;    // Implementation
};

template< typename charT, typename traits >
inline std::basic_ostream<charT, traits> & operator <<(
    std::basic_ostream<charT, traits> & stream, const FontMetric& rMetric )
{
    stream << "{"
           << "name=" << "\"" << rMetric.GetFamilyName() << "\""
           << ",size=(" << rMetric.GetFontSize().Width() << "," << rMetric.GetFontSize().Height() << ")"
           << ",ascent=" << rMetric.GetAscent()
           << ",descent=" << rMetric.GetDescent()
           << ",intLeading=" << rMetric.GetInternalLeading()
           << ",extLeading=" << rMetric.GetExternalLeading()
           << ",lineHeight=" << rMetric.GetLineHeight()
           << ",slant=" << rMetric.GetSlant()
           << "}";
    return stream;
}

#endif // INCLUDED_VCL_METRIC_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
