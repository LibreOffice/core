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

#ifndef INCLUDED_SC_SOURCE_UI_VBA_VBAPALETTE_HXX
#define INCLUDED_SC_SOURCE_UI_VBA_VBAPALETTE_HXX

#include <vbahelper/vbahelper.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XIndexAccess; }
    namespace frame { class XModel; }
} } }

class SfxObjectShell;

class ScVbaPalette
{
private:
    SfxObjectShell* m_pShell;
public:
    explicit ScVbaPalette( SfxObjectShell* pShell = nullptr ) : m_pShell( pShell ) {}
    explicit ScVbaPalette( const css::uno::Reference< css::frame::XModel >& rxModel );
    // if no palette available e.g. because the document doesn't have a
    // palette defined then a default palette will be returned.
    css::uno::Reference< css::container::XIndexAccess > getPalette() const;
    static css::uno::Reference< css::container::XIndexAccess > getDefaultPalette();
};

#endif // INCLUDED_SC_SOURCE_UI_VBA_VBAPALETTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
