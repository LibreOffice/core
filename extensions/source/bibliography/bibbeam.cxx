/*************************************************************************
 *
 *  $RCSfile: bibbeam.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: os $ $Date: 2000-11-14 11:06:35 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_AWT_POSSIZE_HPP_
#include <com/sun/star/awt/PosSize.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XURLTransformer.hpp>
#endif

#include <svtools/iniprop.hxx>
#include <extensio.hrc>

#ifndef _SV_LSTBOX_HXX //autogen wg. form::component::ListBox
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_EDIT_HXX //autogen wg. Edit
#include <vcl/edit.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#include "bibbeam.hxx"
#include "toolbar.hrc"
#include "bibresid.hxx"
#include "datman.hxx"

using namespace rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)

#define PROPERTY_FRAME                      1
#define ID_TOOLBAR                          1
#define ID_GRIDWIN                          2
#define ID_ALPHAWIN                         3

rtl::OUString gGridModelCommand( OUString::createFromAscii(".uno:Bib/newGridModel"));

BibGridwin::BibGridwin(Window* pParent, WinBits nStyle):
    DockingWindow(pParent,nStyle)
{
    aCtrContainer = VCLUnoHelper::CreateControlContainer(this);
}

BibGridwin::~BibGridwin()
{
}

void BibGridwin::Resize()
{
    if(xGridWinRef.is())
    {
        ::Size aSize = GetOutputSizePixel();
        xGridWinRef->setPosSize(0, 0, aSize.Width(),aSize.Height(), PosSize::SIZE);
    }
}
void BibGridwin::createGridWin(const uno::Reference< XControlModel > & xGModel)
{
    xGridModel=xGModel;

    if( aCtrContainer.is())
    {
        uno::Reference< lang::XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();

        if ( xGridModel.is() && xMgr.is())
        {
            uno::Reference< XPropertySet >  xPropSet( xGridModel, UNO_QUERY );

            if ( xPropSet.is() && xGridModel.is() )
            {
                uno::Any aAny = xPropSet->getPropertyValue( C2U("DefaultControl") );
                rtl::OUString aControlName;
                aAny >>= aControlName;

                xControl=uno::Reference< XControl > (xMgr->createInstance( aControlName ), UNO_QUERY );
                if ( xControl.is() )
                {
                    xControl->setModel( xGridModel);
                }
            }

            if ( xControl.is() )
            {
                // Peer als Child zu dem FrameWindow
                aCtrContainer->addControl(C2U("Bla"), xControl);
                xGridWinRef=uno::Reference< XWindow > (xControl, UNO_QUERY );
                xGridWinRef->setVisible( sal_True );
                xControl->setDesignMode(sal_False);
                ::Size aSize = GetOutputSizePixel();
                xGridWinRef->setPosSize(0, 0, aSize.Width(),aSize.Height(), PosSize::POSSIZE);
//              Window* pWindow = ::GetWindow( xGridWinRef );
//              pWindow->SetHelpId(HID_BIB_DB_GRIDCTRL);

            }
        }
    }
}

void BibGridwin::disposeGridWin()
{
    if ( xControl.is() )
    {
        aCtrContainer->removeControl(xControl);
        xControl->dispose();
    }
}

void BibGridwin::changeGridModel(const uno::Reference< XControlModel > & xGModel)
{
    xGridModel=xGModel;

    if ( xControl.is() )
    {
        xControl->setModel( xGridModel);
    }
}


void BibGridwin::statusChanged(const frame::FeatureStateEvent& rEvt)throw( uno::RuntimeException )
{
    if(rEvt.FeatureURL.Complete == gGridModelCommand)
    {
        if(IsEnabled() != rEvt.IsEnabled) Enable(rEvt.IsEnabled);

        uno::Any aModel=rEvt.State;
        if(aModel.getValueType()==::getCppuType((Reference< XControlModel>*)0))
        {
            uno::Reference< XControlModel >  xGModel=*(uno::Reference< XControlModel > *) aModel.getValue();
            changeGridModel(xGModel);
        }
    }
};




BibBeamer::BibBeamer(Window* pParent,BibDataManager* pDM, WinBits nStyle):
    SplitWindow(pParent,nStyle|WB_NOSPLITDRAW),
    pDatMan(pDM),
    pToolBar (NULL),
    pGridWin (NULL),
    pAlphaWin(NULL)
{
    createToolBar();
    createGridWin();
    createAlphaWin();
    pDatMan->SetToolbar(pToolBar);
    pDatMan->SetGridWin(pGridWin);
}

BibBeamer::~BibBeamer()
{
    if ( xToolBarRef.is() ) xToolBarRef->dispose();
    if ( xGridRef.is() ) xGridRef->dispose();
    if ( xAlphaRef.is() ) xAlphaRef->dispose();


    if(pToolBar)
    {
        if(pDatMan)
        {
            pDatMan->SetToolbar(0);
            pDatMan->SetGridWin(0);
        }

        delete pToolBar;
    }

    if(pGridWin)
    {
        pGridWin->disposeGridWin();
//      delete pGridWin;
        xpGridWin = 0;
    }

    if(pAlphaWin) delete pAlphaWin;
}

void BibBeamer::createToolBar()
{
    pToolBar= new BibToolBar(this);
    ::Size aSize=pToolBar->GetSizePixel();
    InsertItem(ID_TOOLBAR, pToolBar, aSize.Height(), 0, 0, SWIB_FIXED );
    if(xController.is()) pToolBar->SetXController(xController);
}
void BibBeamer::createGridWin()
{
    xpGridWin = pGridWin= new BibGridwin(this,0);

    InsertItem(ID_GRIDWIN, pGridWin, 40, 1, 0, SWIB_RELATIVESIZE );


    pGridWin->createGridWin(pDatMan->createGridModel());
}

void BibBeamer::createAlphaWin()
{
}

void BibBeamer::SetXController(const uno::Reference< frame::XController > & xCtr)
{
    xController=xCtr;

    if(xController.is())
    {
        uno::Reference< frame::XDispatch >  xDisp(xController, UNO_QUERY);
        uno::Reference< lang::XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();
        uno::Reference< util::XURLTransformer >  xTrans ( xMgr->createInstance( C2U("com.sun.star.util.URLTransformer") ), UNO_QUERY );
        if( xTrans.is() && xDisp.is())
        {
            util::URL aURL;
            aURL.Complete =gGridModelCommand;

            xTrans->parseStrict( aURL );
            xDisp->addStatusListener(uno::Reference< frame::XStatusListener > (pGridWin),aURL);
        }
    }

    if(pToolBar)
    {
        pToolBar->SetXController(xController);
    }

}


