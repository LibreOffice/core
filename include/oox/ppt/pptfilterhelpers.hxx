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

#ifndef INCLUDED_OOX_PPT_PPTFILTERHELPERS_HXX
#define INCLUDED_OOX_PPT_PPTFILTERHELPERS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/animations/XAnimationNode.hpp>
#include <oox/dllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace oox { namespace ppt {

    // conversion of MS to OOo attributes.
    enum MS_AttributeNames
    {
        MS_PPT_X, MS_PPT_Y, MS_PPT_W, MS_PPT_H, MS_PPT_C, MS_R, MS_XSHEAR, MS_FILLCOLOR, MS_FILLTYPE,
        MS_STROKECOLOR, MS_STROKEON, MS_STYLECOLOR, MS_STYLEROTATION, MS_FONTWEIGHT,
        MS_STYLEUNDERLINE, MS_STYLEFONTFAMILY, MS_STYLEFONTSIZE, MS_STYLEFONTSTYLE,
        MS_STYLEVISIBILITY, MS_STYLEOPACITY, MS_UNKNOWN
    };

    struct ImplAttributeNameConversion
    {
        MS_AttributeNames meAttribute;
        const char* mpMSName;
        const char* mpAPIName;
    };

    OOX_DLLPUBLIC const ImplAttributeNameConversion *getAttributeConversionList();

    struct OOX_DLLPUBLIC transition
    {
        const sal_Char* mpName;
        sal_Int16 mnType;
        sal_Int16 mnSubType;
        bool mbDirection; // true: default geometric direction

        static const transition* getList();
        static const transition* find( const OUString& rName );
    };

    struct OOX_DLLPUBLIC convert_subtype
    {
        sal_Int32 mnID;
        const sal_Char* mpStrSubType;

        static const convert_subtype* getList();
    };

    struct OOX_DLLPUBLIC preset_maping
    {
        sal_Int32   mnPresetClass;
        sal_Int32   mnPresetId;
        const sal_Char* mpStrPresetId;

        static const preset_maping* getList();
    };

    OOX_DLLPUBLIC OUString getConvertedSubType( sal_Int16 nPresetClass, sal_Int32 nPresetId, sal_Int32 nPresetSubType );

    OOX_DLLPUBLIC void fixMainSequenceTiming( const css::uno::Reference< css::animations::XAnimationNode >& xNode );

    OOX_DLLPUBLIC void fixInteractiveSequenceTiming( const css::uno::Reference< css::animations::XAnimationNode >& xNode );
} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
