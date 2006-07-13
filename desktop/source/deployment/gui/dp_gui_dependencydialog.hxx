/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_dependencydialog.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 17:02:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2006 by Sun Microsystems, Inc.
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

#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DEPENDENCYDIALOG_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_DEPENDENCYDIALOG_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#include <vector>

#ifndef _SV_GEN_HXX
#include "tools/gen.hxx"
#endif
#ifndef _SV_BUTTON_HXX
#include "vcl/button.hxx"
#endif
#ifndef _SV_DIALOG_HXX
#include "vcl/dialog.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include "vcl/fixed.hxx"
#endif
#ifndef _SV_LSTBOX_HXX
#include "vcl/lstbox.hxx"
#endif

class Window;
namespace rtl { class OUString; }

namespace dp_gui {

class DependencyDialog: public ModalDialog {
public:
    DependencyDialog(
        Window * parent, std::vector< rtl::OUString > const & dependencies);

    ~DependencyDialog();

private:
    DependencyDialog(DependencyDialog &); // not defined
    void operator =(DependencyDialog &); // not defined

    virtual void Resize();

    FixedText m_text;
    ListBox m_list;
    OKButton m_ok;
    Size m_listDelta;
};

}

#endif
