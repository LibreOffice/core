/*************************************************************************
 *
 *  $RCSfile: bibbeam.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 16:12:31 $
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

#ifndef __EXTENSIONS_INC_EXTENSIO_HRC__
#include "extensio.hrc"
#endif

#ifndef _SV_LSTBOX_HXX //autogen wg. form::component::ListBox
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_EDIT_HXX //autogen wg. Edit
#include <vcl/edit.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX //autogen wg. DBG_ASSERT
#include <tools/debug.hxx>
#endif

#ifndef ADRBEAM_HXX
#include "bibbeam.hxx"
#endif
#include "toolbar.hrc"
#ifndef ADRRESID_HXX
#include "bibresid.hxx"
#endif
#ifndef _BIB_DATMAN_HXX
#include "datman.hxx"
#endif
#ifndef BIBTOOLS_HXX
#include "bibtools.hxx"
#endif

using namespace rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

#define C2U(cChar) OUString::createFromAscii(cChar)

#define PROPERTY_FRAME                      1
#define ID_TOOLBAR                          1
#define ID_GRIDWIN                          2

//.........................................................................
namespace bib
{
//.........................................................................

    using namespace ::com::sun::star::uno;

    void HandleTaskPaneList( Window* pWindow, BOOL bAddToList )
    {
        Window*             pParent = pWindow->GetParent();

        DBG_ASSERT( pParent, "-GetTaskPaneList(): everybody here should have a parent!" );

        SystemWindow*       pSysWin = pParent->GetSystemWindow();
        if( pSysWin )
        {
            TaskPaneList*   pTaskPaneList = pSysWin->GetTaskPaneList();
            if( pTaskPaneList )
            {
                if( bAddToList )
                    pTaskPaneList->AddWindow( pWindow );
                else
                    pTaskPaneList->RemoveWindow( pWindow );
            }
        }
    }

    //=====================================================================
    //= BibGridwin
    //=====================================================================
    class BibGridwin
                :public Window //DockingWindow
    {
    private:
            Reference< awt::XWindow >           m_xGridWin;
            Reference< awt::XControlModel >     m_xGridModel;
            Reference< awt::XControl >          m_xControl;
            Reference< awt::XControlContainer > m_xControlContainer;
            // #100312# ---------
            Reference< frame::XDispatchProviderInterception> m_xDispatchProviderInterception;

    protected:

            virtual void        Resize();

    public:

            BibGridwin(Window* pParent, WinBits nStyle = WB_3DLOOK );
            ~BibGridwin();

            void createGridWin(const Reference< awt::XControlModel > & xDbForm);
            void changeGridModel(const Reference< awt::XControlModel > & xGModel);
            void disposeGridWin();

            const Reference< awt::XControlContainer >& getControlContainer() const { return m_xControlContainer; }
            // #100312# ---------
            const Reference< frame::XDispatchProviderInterception>& getDispatchProviderInterception() const { return m_xDispatchProviderInterception; }

            virtual void GetFocus();
    };

    //---------------------------------------------------------------------
    BibGridwin::BibGridwin( Window* _pParent, WinBits _nStyle ) : Window( _pParent, _nStyle )
    {
        m_xControlContainer = VCLUnoHelper::CreateControlContainer(this);

        AddToTaskPaneList( this );
    }

    //---------------------------------------------------------------------
    BibGridwin::~BibGridwin()
    {
        RemoveFromTaskPaneList( this );

        disposeGridWin();
    }

    //---------------------------------------------------------------------
    void BibGridwin::Resize()
    {
        if(m_xGridWin.is())
        {
            ::Size aSize = GetOutputSizePixel();
            m_xGridWin->setPosSize(0, 0, aSize.Width(),aSize.Height(), awt::PosSize::SIZE);
        }
    }

    //---------------------------------------------------------------------
    void BibGridwin::createGridWin(const uno::Reference< awt::XControlModel > & xGModel)
    {
        m_xGridModel = xGModel;

        if( m_xControlContainer.is())
        {
            uno::Reference< lang::XMultiServiceFactory >  xMgr = comphelper::getProcessServiceFactory();

            if ( m_xGridModel.is() && xMgr.is())
            {
                uno::Reference< XPropertySet >  xPropSet( m_xGridModel, UNO_QUERY );

                if ( xPropSet.is() && m_xGridModel.is() )
                {
                    uno::Any aAny = xPropSet->getPropertyValue( C2U("DefaultControl") );
                    rtl::OUString aControlName;
                    aAny >>= aControlName;

                    m_xControl = Reference< awt::XControl > (xMgr->createInstance( aControlName ), UNO_QUERY );
                    DBG_ASSERT( m_xControl.is(), "no GridControl created" )
                    if ( m_xControl.is() )
                        m_xControl->setModel( m_xGridModel );
                }

                if ( m_xControl.is() )
                {
                    // Peer als Child zu dem FrameWindow
                    m_xControlContainer->addControl(C2U("GridControl"), m_xControl);
                    m_xGridWin=uno::Reference< awt::XWindow > (m_xControl, UNO_QUERY );
                    // #100312# -----
                    m_xDispatchProviderInterception=uno::Reference< frame::XDispatchProviderInterception > (m_xControl, UNO_QUERY );
                    m_xGridWin->setVisible( sal_True );
                    m_xControl->setDesignMode( sal_True );
                        // initially switch on the desing mode - switch it off _after_ loading the form
                        // 17.10.2001 - 93107 - frank.schoenheit@sun.com

                    ::Size aSize = GetOutputSizePixel();
                    m_xGridWin->setPosSize(0, 0, aSize.Width(),aSize.Height(), awt::PosSize::POSSIZE);
                }
            }
        }
    }

    //---------------------------------------------------------------------
    void BibGridwin::disposeGridWin()
    {
        if ( m_xControl.is() )
        {
            Reference< awt::XControl > xDel( m_xControl );
            m_xControl = NULL;
            m_xGridWin = NULL;

            m_xControlContainer->removeControl( xDel );
            xDel->dispose();
        }
    }

    //---------------------------------------------------------------------
    void BibGridwin::changeGridModel(const uno::Reference< awt::XControlModel > & xGModel)
    {
        m_xGridModel = xGModel;

        if ( m_xControl.is() )
            m_xControl->setModel( m_xGridModel );
    }

    //---------------------------------------------------------------------
    void BibGridwin::GetFocus()
    {
        if(m_xGridWin.is())
            m_xGridWin->setFocus();
    }

    //---------------------------------------------------------------------
    BibBeamer::BibBeamer( Window* _pParent, BibDataManager* _pDM, WinBits _nStyle )
        :BibSplitWindow( _pParent, _nStyle | WB_NOSPLITDRAW )
        ,pDatMan( _pDM )
        ,pToolBar( NULL )
        ,pGridWin( NULL )
    {
        createToolBar();
        createGridWin();
        if ( pDatMan )
            pDatMan->SetToolbar(pToolBar);
        pGridWin->Show();

        if ( pDatMan )
            connectForm( pDatMan );
    }

    //---------------------------------------------------------------------
    BibBeamer::~BibBeamer()
    {
        if ( isFormConnected() )
            disconnectForm();

        if ( m_xToolBarRef.is() )
            m_xToolBarRef->dispose();

        if ( pToolBar )
        {
            if ( pDatMan )
                pDatMan->SetToolbar(0);

            DELETEZ( pToolBar );
        }

        if( pGridWin )
        {
            BibGridwin* pDel = pGridWin;
            pGridWin = NULL;
            pDel->disposeGridWin();
            delete pDel;
        }

    }

    //---------------------------------------------------------------------
    void BibBeamer::createToolBar()
    {
        pToolBar= new BibToolBar(this, LINK( this, BibBeamer, RecalcLayout_Impl ));
        ::Size aSize=pToolBar->GetSizePixel();
        InsertItem(ID_TOOLBAR, pToolBar, aSize.Height(), 0, 0, SWIB_FIXED );
        if ( m_xController.is() )
            pToolBar->SetXController( m_xController );
    }

    //---------------------------------------------------------------------
    void BibBeamer::createGridWin()
    {
        pGridWin = new BibGridwin(this,0);

        InsertItem(ID_GRIDWIN, pGridWin, 40, 1, 0, SWIB_RELATIVESIZE );

        pGridWin->createGridWin( pDatMan->updateGridModel() );
    }

    //---------------------------------------------------------------------
    Reference< awt::XControlContainer > BibBeamer::getControlContainer()
    {
        Reference< awt::XControlContainer > xReturn;
        if ( pGridWin )
            xReturn = pGridWin->getControlContainer();
        return xReturn;
    }

    // #100312# -----------------------------------------------------------
    Reference< frame::XDispatchProviderInterception > BibBeamer::getDispatchProviderInterception()
    {
        Reference< frame::XDispatchProviderInterception > xReturn;
        if ( pGridWin )
            xReturn = pGridWin->getDispatchProviderInterception();
        return xReturn;
    }

    //---------------------------------------------------------------------
    void BibBeamer::SetXController(const uno::Reference< frame::XController > & xCtr)
    {
        m_xController = xCtr;

        if ( pToolBar )
            pToolBar->SetXController( m_xController );

    }

    //---------------------------------------------------------------------
    void BibBeamer::GetFocus()
    {
        if( pGridWin )
            pGridWin->GrabFocus();
    }

    //---------------------------------------------------------------------
    IMPL_LINK( BibBeamer, RecalcLayout_Impl, void*, pVoid )
    {
        long nHeight = pToolBar->GetSizePixel().Height();
        SetItemSize( ID_TOOLBAR, pToolBar->GetSizePixel().Height() );
        return 0L;
    }

//.........................................................................
}   // namespace bib
//.........................................................................
