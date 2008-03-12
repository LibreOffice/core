/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TableDesignPane.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:46:59 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
#include <svtools/valueset.hxx>

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
    DECL_LINK(implValueSetHdl, Control* );
    DECL_LINK(implCheckBoxHdl, Control* );

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
    ViewShellBase& mrBase;
    boost::scoped_ptr< TableDesignPane > mpDesignPane;

    boost::scoped_ptr< FixedLine >       mxFlSep1;
    boost::scoped_ptr< FixedLine >       mxFlSep2;
    boost::scoped_ptr< HelpButton >      mxHelpButton;
    boost::scoped_ptr< OKButton >        mxOkButton;
    boost::scoped_ptr< CancelButton >    mxCancelButton;
};

}

#endif // _SD_TABLEFORMATPANE_HXX
