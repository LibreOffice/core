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
#ifndef INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEWIDTHVALUESET_HXX
#define INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEWIDTHVALUESET_HXX

#include <svtools/valueset.hxx>
#include <vcl/image.hxx>
#include <array>

namespace svx { namespace sidebar {

class LineWidthValueSet final : public ValueSet
{
public:
    explicit LineWidthValueSet(vcl::Window* pParent);
    virtual ~LineWidthValueSet() override;

    void SetUnit(std::array<OUString,9> const & strUnits);
    void SetSelItem(sal_uInt16 nSel);
    sal_uInt16 GetSelItem() const { return nSelItem;}
    void SetImage(const Image& img);
    void SetCusEnable(bool bEnable);

    virtual void    UserDraw( const UserDrawEvent& rUDEvt ) override;
    virtual void    Resize() override;
    virtual Size    GetOptimalSize() const override;

private:
    sal_uInt16          nSelItem;
    std::array<OUString,9> maStrUnits;
    Image           imgCus;
    bool            bCusEnable;
};

} } // end of namespace svx::sidebar

#endif // INCLUDED_SVX_SOURCE_SIDEBAR_LINE_LINEWIDTHVALUESET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
