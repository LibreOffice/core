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
#ifndef INCLUDED_SVX_SVXCOLORVALUESET_HXX
#define INCLUDED_SVX_SVXCOLORVALUESET_HXX

#include <svtools/valueset.hxx>
#include <svx/svxdllapi.h>
#include <set>

class XColorList;

class SVXCORE_DLLPUBLIC SvxColorValueSet : public ValueSet
{
public:
    SvxColorValueSet(std::unique_ptr<weld::ScrolledWindow> pWindow);

    virtual void Resize() override;

    /* Maximum row/line count for the ColorValueSet control. If more lines would be needed, a scrollbar will
       be used. */
    static constexpr sal_uInt32 getMaxRowCount() { return 10; }
    static sal_uInt32 getEntryEdgeLength();
    static sal_uInt32 getColumnCount();

    void addEntriesForXColorList(const XColorList& rXColorList, sal_uInt32 nStartIndex = 1);
    void addEntriesForColorSet(const std::set<Color>& rColorSet, const OUString& rNamePrefix);
    Size layoutAllVisible(sal_uInt32 nEntryCount);
    Size layoutToGivenHeight(sal_uInt32 nHeight, sal_uInt32 nEntryCount);

    virtual FactoryFunction GetUITestFactory() const override;
};

#endif // INCLUDED_SVX_SVXCOLORVALUESET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
