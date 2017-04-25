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

#ifndef INCLUDED_SC_SOURCE_FILTER_INC_THEMEBUFFER_HXX
#define INCLUDED_SC_SOURCE_FILTER_INC_THEMEBUFFER_HXX

#include <memory>
#include <oox/drawingml/theme.hxx>
#include "workbookhelper.hxx"

namespace oox {
namespace xls {

struct FontModel;

class ThemeBuffer : public ::oox::drawingml::Theme, public WorkbookHelper
{
public:
    explicit            ThemeBuffer( const WorkbookHelper& rHelper );
    virtual             ~ThemeBuffer() override;

    /** Returns the theme color with the specified token identifier. */
    sal_Int32           getColorByToken( sal_Int32 nToken ) const;

    sal_Int32           getColorByIndex(size_t nIndex) const;

    /** Returns the default font data for the current file type. */
    const FontModel& getDefaultFontModel() const { return *mxDefFontModel; }

private:
    typedef ::std::unique_ptr< FontModel > FontModelPtr;
    FontModelPtr        mxDefFontModel;
};

} // namespace xls
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
