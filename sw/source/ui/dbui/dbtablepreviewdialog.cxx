/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dbtablepreviewdialog.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 11:31:14 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#ifndef _SWTYPES_HXX
#include <swtypes.hxx>
#endif

#ifndef _DBTABLEPREVIEWDIALOG_HXX
#include <dbtablepreviewdialog.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/unohlp.hxx>
#endif

#include <dbui.hrc>
#include <dbtablepreviewdialog.hrc>
#include <unomid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::rtl;

/*-- 08.04.2004 15:12:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwDBTablePreviewDialog::SwDBTablePreviewDialog(Window* pParent, uno::Sequence< beans::PropertyValue>& rValues ) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_DBTABLEPREVIEWDIALOG)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aDescriptionFI( this, SW_RES(        FI_DESCRIPTION)),
    m_pBeamerWIN( new Window(this, SW_RES( WIN_BEAMER ))),
    m_aOK( this, SW_RES(                   PB_OK  ))
#ifdef MSC
#pragma warning (default : 4355)
#endif
{
    FreeResource();
    const beans::PropertyValue* pValues = rValues.getConstArray();
    for(sal_Int32 nValue = 0; nValue < rValues.getLength(); ++nValue        )
    {
        if(pValues[nValue].Name.equalsAscii("Command"))
        {
            String sDescription = m_aDescriptionFI.GetText();
            OUString sTemp;
            pValues[nValue].Value >>= sTemp;
            sDescription.SearchAndReplaceAscii("%1", sTemp);
            m_aDescriptionFI.SetText(sDescription);
            break;
        }
    }

    try
    {
        // create a frame wrapper for myself
        uno::Reference< lang::XMultiServiceFactory >
                                    xMgr = comphelper::getProcessServiceFactory();
        m_xFrame = uno::Reference< frame::XFrame >(xMgr->createInstance(C2U("com.sun.star.frame.Frame")), uno::UNO_QUERY);
        if(m_xFrame.is())
        {
            m_xFrame->initialize( VCLUnoHelper::GetInterface ( m_pBeamerWIN ) );
        }
    }
    catch (uno::Exception const &)
    {
        m_xFrame.clear();
    }
    if(m_xFrame.is())
    {
        uno::Reference<frame::XDispatchProvider> xDP(m_xFrame, uno::UNO_QUERY);
        util::URL aURL;
        aURL.Complete = C2U(".component:DB/DataSourceBrowser");
        uno::Reference<frame::XDispatch> xD = xDP->queryDispatch(aURL,
                    C2U(""),
                    0x0C);
        if(xD.is())
        {
            xD->dispatch(aURL, rValues);
            m_pBeamerWIN->Show();
        }
/*        uno::Reference<???::XController> xController = m_xFrame->getController();
        pImpl->xFController = uno::Reference<???::XFormController>(xController, uno::UNO_QUERY);
        if(pImpl->xFController.is())
        {
            uno::Reference< awt::XControl > xCtrl = pImpl->xFController->getCurrentControl(  );
            pImpl->xSelSupp = uno::Reference<???::XSelectionSupplier>(xCtrl, uno::UNO_QUERY);
            if(pImpl->xSelSupp.is())
            {
                pImpl->xChgLstnr = new SwXSelChgLstnr_Impl(*this);
                pImpl->xSelSupp->addSelectionChangeListener(  pImpl->xChgLstnr );
            }
        }
*/    }


}
/*-- 08.04.2004 15:12:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwDBTablePreviewDialog::~SwDBTablePreviewDialog()
{
    if(m_xFrame.is())
    {
        m_xFrame->setComponent(NULL, NULL);
        m_xFrame->dispose();
    }
    else
        delete m_pBeamerWIN;
}
