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

#ifndef INCLUDED_SD_SOURCE_UI_TABLE_TABLEDESIGNPANE_HXX
#define INCLUDED_SD_SOURCE_UI_TABLE_TABLEDESIGNPANE_HXX

#include <svtools/valueset.hxx>
#include <vcl/button.hxx>
#include <svx/sidebar/PanelLayout.hxx>

namespace com { namespace sun { namespace star { namespace beans { class XPropertySet; } } } }
namespace com { namespace sun { namespace star { namespace container { class XIndexAccess; } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XDrawView; } } } }

namespace sd
{

namespace tools {
class EventMultiplexerEvent;
}

class ViewShellBase;

enum TableCheckBox : sal_uInt16
{
    CB_HEADER_ROW       = 0,
    CB_TOTAL_ROW        = 1,
    CB_BANDED_ROWS      = 2,
    CB_FIRST_COLUMN     = 3,
    CB_LAST_COLUMN      = 4,
    CB_BANDED_COLUMNS   = 5,
    CB_COUNT            = CB_BANDED_COLUMNS + 1
};

class TableValueSet : public ValueSet
{
private:
    bool m_bModal;
public:
    TableValueSet(vcl::Window *pParent, WinBits nStyle);
    virtual void Resize() override;
    virtual void DataChanged( const DataChangedEvent& rDCEvt ) override;
    void updateSettings();
    void setModal(bool bModal) { m_bModal = bModal; }
};

class TableDesignWidget final
{
public:
    TableDesignWidget( VclBuilderContainer* pParent, ViewShellBase& rBase );
    ~TableDesignWidget();

    // callbacks
    void onSelectionChanged();

    void ApplyOptions();
    void ApplyStyle();

private:
    void addListener();
    void removeListener();
    void updateControls();

    void FillDesignPreviewControl();

    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent&, void);
    DECL_LINK(implValueSetHdl, ValueSet*, void);
    DECL_LINK(implCheckBoxHdl, Button*, void);

private:
    ViewShellBase& mrBase;

    VclPtr<TableValueSet> m_pValueSet;
    VclPtr<CheckBox> m_aCheckBoxes[CB_COUNT];

    css::uno::Reference< css::beans::XPropertySet > mxSelectedTable;
    css::uno::Reference< css::drawing::XDrawView > mxView;
    css::uno::Reference< css::container::XIndexAccess > mxTableFamily;
};

class TableDesignPane : public PanelLayout
{
private:
    TableDesignWidget const aImpl;
public:
    TableDesignPane( vcl::Window* pParent, ViewShellBase& rBase )
        : PanelLayout(pParent, "TableDesignPanel",
        "modules/simpress/ui/tabledesignpanel.ui", css::uno::Reference<css::frame::XFrame>())
        , aImpl(this, rBase)
    {
    }
    TableDesignPane( vcl::Window* pParent, ViewShellBase& rBase, bool )
        : PanelLayout(pParent, "TableDesignPanel",
        "modules/simpress/ui/tabledesignpanelhorizontal.ui", css::uno::Reference<css::frame::XFrame>())
        , aImpl(this, rBase)
    {
    }
};

}

#endif // _SD_TABLEFORMATPANE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
