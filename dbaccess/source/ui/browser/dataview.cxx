/*************************************************************************
 *
 *  $RCSfile: dataview.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 14:29:32 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _SFXAPP_HXX //autogen wg. SFX_APP
#include <sfx2/app.hxx>
#endif
#ifndef _SFXIMGMGR_HXX
#include <sfx2/imgmgr.hxx>
#endif

using namespace dbaui;
using namespace ::com::sun::star::uno;
//  using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
//  using namespace ::com::sun::star::container;

// -------------------------------------------------------------------------
ODataView::ODataView(Window* pParent, const Reference< XMultiServiceFactory >& _rFactory, WinBits nStyle)
    :Window(pParent,nStyle)
    ,m_pToolBox(NULL)
    ,m_xServiceFactory(_rFactory)
{
}
// -------------------------------------------------------------------------
void ODataView::Construct(const Reference< ::com::sun::star::awt::XControlModel >& xModel)
{
    try
    {
        // our UNO representation
        m_xMe = VCLUnoHelper::CreateControlContainer(this);
        DBG_ASSERT(m_xMe.is(), "ODataView::Construct : no UNO representation");
    }
    catch(Exception&)
    {
        ::comphelper::disposeComponent(m_xMe);
        throw;
    }
}
// -------------------------------------------------------------------------
ODataView::~ODataView()
{
    setToolBox(NULL);

    ::comphelper::disposeComponent(m_xMe);
}
// -------------------------------------------------------------------------
void ODataView::setToolBox(ToolBox* pTB)
{
    if (pTB == m_pToolBox)
        return;

    if (m_pToolBox)
    {
        SFX_IMAGEMANAGER()->ReleaseToolBox(m_pToolBox);
        delete m_pToolBox;
    }

    m_pToolBox = pTB;
    if (m_pToolBox)
    {
        SFX_IMAGEMANAGER()->RegisterToolBox(m_pToolBox, SFX_TOOLBOX_CHANGEOUTSTYLE);

        m_pToolBox->SetParent(this);
        m_pToolBox->Show();
    }

    // rearrange the grid and the TB
    Resize();
}
// -------------------------------------------------------------------------
void ODataView::Resize()
{
    Window::Resize();
    Rectangle aRect(Point(0,0),GetOutputSizePixel());
    resizeControl(aRect);

    // set the size of the toolbox
    if (m_pToolBox)
        m_pToolBox->SetPosSizePixel(aRect.TopLeft(),aRect.GetSize());
}
//------------------------------------------------------------------

