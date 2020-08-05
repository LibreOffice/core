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
#pragma once

#include <svl/itemset.hxx>
#include <tools/link.hxx>

namespace com::sun::star::frame { class XModel; }
namespace com::sun::star::uno { class XComponentContext; }
namespace weld { class Builder; }
namespace weld { class CheckButton; }
namespace weld { class RadioButton; }
namespace weld { class ToggleButton; }

namespace chart
{

class LegendPositionResources final
{

public:
    //constructor without Display checkbox
    LegendPositionResources(weld::Builder& rBuilder);
    //constructor inclusive Display checkbox
    LegendPositionResources(weld::Builder& rBuilder, const css::uno::Reference<
                       css::uno::XComponentContext>& xCC );
    ~LegendPositionResources();

    void writeToResources( const css::uno::Reference< css::frame::XModel >& xChartModel );
    void writeToModel( const css::uno::Reference< css::frame::XModel >& xChartModel ) const;

    void initFromItemSet( const SfxItemSet& rInAttrs );
    void writeToItemSet( SfxItemSet& rOutAttrs ) const;

    void SetChangeHdl( const Link<LinkParamNone*,void>& rLink );

    DECL_LINK( PositionEnableHdl, weld::ToggleButton&, void );
    DECL_LINK( PositionChangeHdl, weld::ToggleButton&, void );

private:
    void impl_setRadioButtonToggleHdl();

private:
    css::uno::Reference< css::uno::XComponentContext>    m_xCC;
    Link<LinkParamNone*,void> m_aChangeLink;

    std::unique_ptr<weld::CheckButton> m_xCbxShow;
    std::unique_ptr<weld::RadioButton> m_xRbtLeft;
    std::unique_ptr<weld::RadioButton> m_xRbtRight;
    std::unique_ptr<weld::RadioButton> m_xRbtTop;
    std::unique_ptr<weld::RadioButton> m_xRbtBottom;
};

} //namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
