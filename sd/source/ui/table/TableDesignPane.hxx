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

#ifndef _SD_TABLEDESIGNPANE_HXX
#define _SD_TABLEDESIGNPANE_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ui/XUIElement.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>

#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>

#include <boost/scoped_ptr.hpp>

#include "TableDesignPane.hrc"

namespace sd
{

namespace tools {
class EventMultiplexerEvent;
}

class ViewShellBase;

// --------------------------------------------------------------------

class TableDesignPane : public Control
{
public:
    TableDesignPane( ::Window* pParent, ViewShellBase& rBase, bool bModal );
    virtual ~TableDesignPane();

    // callbacks
    void onSelectionChanged();

    // Control
    virtual void Resize();

    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    void ApplyOptions();
    void ApplyStyle();

    bool isStyleChanged() const { return mbStyleSelected; }
    bool isOptionsChanged() const { return mbOptionsChanged; }

private:
    void addListener();
    void removeListener();
    void updateLayout();
    void updateControls();

    void FillDesignPreviewControl();

    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);
    DECL_LINK(implValueSetHdl, void *);
    DECL_LINK(implCheckBoxHdl, void *);

private:
    ViewShellBase& mrBase;
    const rtl::OUString msTableTemplate;

    boost::scoped_ptr< Control > mxControls[DESIGNPANE_CONTROL_COUNT];
    int mnOrgOffsetY[DESIGNPANE_CONTROL_COUNT];

    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > mxSelectedTable;
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawView > mxView;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > mxTableFamily;
    ::com::sun::star::uno::Reference< ::com::sun::star::ui::XUIElement > mxToolbar;

    bool mbModal;
    bool mbStyleSelected;
    bool mbOptionsChanged;
};

// --------------------------------------------------------------------

class TableDesignDialog : public ModalDialog
{
public:
    TableDesignDialog( ::Window* pParent, ViewShellBase& rBase );

    virtual short Execute();
private:
    boost::scoped_ptr< TableDesignPane > mpDesignPane;

    boost::scoped_ptr< FixedLine >       mxFlSep1;
    boost::scoped_ptr< FixedLine >       mxFlSep2;
    boost::scoped_ptr< HelpButton >      mxHelpButton;
    boost::scoped_ptr< OKButton >        mxOkButton;
    boost::scoped_ptr< CancelButton >    mxCancelButton;
};

}

#endif // _SD_TABLEFORMATPANE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
