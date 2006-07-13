/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_dependencydialog.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 17:02:37 $
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

#include "sal/config.h"

#include <algorithm>
#include <vector>

#include "rtl/ustring.hxx"
#include "tools/gen.hxx"
#include "tools/resid.hxx"
#include "tools/resmgr.hxx"
#include "tools/solar.h"
#include "tools/string.hxx"
#include "vcl/dialog.hxx"

#include "dp_gui.hrc"
#include "dp_gui_dependencydialog.hxx"

class Window;

using dp_gui::DependencyDialog;

DependencyDialog::DependencyDialog(
    Window * parent, std::vector< rtl::OUString > const & dependencies):
    ModalDialog(
        parent,
        ResId(
            RID_DLG_DEPENDENCIES,
            ResMgr::CreateResMgr("deploymentgui" LIBRARY_SOLARUPD()))),
    m_text(this, ResId(RID_DLG_DEPENDENCIES_TEXT)),
    m_list(this, ResId(RID_DLG_DEPENDENCIES_LIST)),
    m_ok(this, ResId(RID_DLG_DEPENDENCIES_OK)),
    m_listDelta(
        GetOutputSizePixel().Width() - m_list.GetSizePixel().Width(),
        GetOutputSizePixel().Height() - m_list.GetSizePixel().Height())
{
    SetMinOutputSizePixel(GetOutputSizePixel());
    m_list.SetReadOnly();
    for (std::vector< rtl::OUString >::const_iterator i(dependencies.begin());
         i != dependencies.end(); ++i)
    {
        m_list.InsertEntry(
            i->getLength() == 0
            ? String(ResId(RID_DLG_DEPENDENCIES_UNKNOWN)) : String(*i));
    }
}

DependencyDialog::~DependencyDialog() {}

void DependencyDialog::Resize() {
    long n = m_ok.GetPosPixel().Y() -
        (m_list.GetPosPixel().Y() + m_list.GetSizePixel().Height());
    m_list.SetSizePixel(
        Size(
            GetOutputSizePixel().Width() - m_listDelta.Width(),
            GetOutputSizePixel().Height() - m_listDelta.Height()));
    m_ok.SetPosPixel(
        Point(
            (m_list.GetPosPixel().X() +
             (m_list.GetSizePixel().Width() - m_ok.GetSizePixel().Width()) / 2),
            m_list.GetPosPixel().Y() + m_list.GetSizePixel().Height() + n));
}
