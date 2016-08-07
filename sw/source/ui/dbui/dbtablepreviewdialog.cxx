/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#ifdef SW_DLLIMPLEMENTATION
#undef SW_DLLIMPLEMENTATION
#endif


#include <swtypes.hxx>
#include <dbtablepreviewdialog.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
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
