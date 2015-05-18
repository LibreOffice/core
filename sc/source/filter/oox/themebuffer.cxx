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

#include "themebuffer.hxx"

#include "stylesbuffer.hxx"

namespace oox {
namespace xls {

using ::oox::drawingml::ClrScheme;

ThemeBuffer::ThemeBuffer( const WorkbookHelper& rHelper ) :
    WorkbookHelper( rHelper ),
    mxDefFontModel( new FontModel )
{
    switch( getFilterType() )
    {
        case FILTER_OOXML:
            //! TODO: locale dependent font name
            mxDefFontModel->maName = "Cambria";
            mxDefFontModel->mfHeight = 11.0;
        break;
        case FILTER_BIFF:
            //! TODO: BIFF dependent font name
            mxDefFontModel->maName = "Arial";
            mxDefFontModel->mfHeight = 10.0;
        break;
        case FILTER_UNKNOWN: break;
    }
}

ThemeBuffer::~ThemeBuffer()
{
}

sal_Int32 ThemeBuffer::getColorByToken( sal_Int32 nToken ) const
{
    sal_Int32 nColor = 0;
    return getClrScheme().getColor( nToken, nColor ) ? nColor : API_RGB_TRANSPARENT;
}

sal_Int32 ThemeBuffer::getColorByIndex(size_t nIndex) const
{
    sal_Int32 nColor = 0;
    return getClrScheme().getColorByIndex(nIndex, nColor) ? nColor : API_RGB_TRANSPARENT;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
