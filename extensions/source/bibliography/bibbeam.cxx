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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include <osl/mutex.hxx>
#include <tools/urlobj.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/PosSize.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include "bibliography.hrc"
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <tools/debug.hxx>
#include "bibbeam.hxx"
#include "toolbar.hrc"
#include "bibresid.hxx"
#include "datman.hxx"
#include "bibtools.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

#define C2U(cChar) OUString::createFromAscii(cChar)

#define PROPERTY_FRAME                      1
#define ID_TOOLBAR                          1
#define ID_GRIDWIN                          2

//.........................................................................
namespace bib
{
//.........................................................................

    using namespace ::com::sun::star::uno;

    void HandleTaskPaneList( Window* pWindow, sal_Bool bAddToList )
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
                    DBG_ASSERT( m_xControl.is(), "no GridControl created" );
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
    IMPL_LINK( BibBeamer, RecalcLayout_Impl, void*, /*pVoid*/ )
    {
        long nHeight = pToolBar->GetSizePixel().Height();
        SetItemSize( ID_TOOLBAR, nHeight );
        return 0L;
    }

//.........................................................................
}   // namespace bib
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
