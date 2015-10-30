/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <osl/mutex.hxx>
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
#include "bibview.hxx"
#include "toolbar.hrc"
#include "bibresid.hxx"
#include "datman.hxx"
#include "bibtools.hxx"

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;


#define ID_TOOLBAR                          1
#define ID_GRIDWIN                          2

namespace bib
{

    using namespace ::com::sun::star::uno;

    void HandleTaskPaneList( vcl::Window* pWindow, bool bAddToList )
    {
        vcl::Window*             pParent = pWindow->GetParent();

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


    class BibGridwin
                :public vcl::Window //DockingWindow
    {
    private:
            Reference< awt::XWindow >           m_xGridWin;
            Reference< awt::XControlModel >     m_xGridModel;
            Reference< awt::XControl >          m_xControl;
            Reference< awt::XControlContainer > m_xControlContainer;
            Reference< frame::XDispatchProviderInterception> m_xDispatchProviderInterception;

    protected:

            virtual void        Resize() override;

    public:

            BibGridwin(vcl::Window* pParent, WinBits nStyle = WB_3DLOOK );
            virtual ~BibGridwin();
            virtual void dispose() override;

            void createGridWin(const Reference< awt::XControlModel > & xDbForm);
            void disposeGridWin();

            const Reference< awt::XControlContainer >& getControlContainer() const { return m_xControlContainer; }
            const Reference< frame::XDispatchProviderInterception>& getDispatchProviderInterception() const { return m_xDispatchProviderInterception; }

            virtual void GetFocus() override;
    };

    BibGridwin::BibGridwin( vcl::Window* _pParent, WinBits _nStyle ) : Window( _pParent, _nStyle )
    {
        m_xControlContainer = VCLUnoHelper::CreateControlContainer(this);

        AddToTaskPaneList( this );
    }

    BibGridwin::~BibGridwin()
    {
        disposeOnce();
    }

    void BibGridwin::dispose()
    {
        RemoveFromTaskPaneList( this );

        disposeGridWin();
        vcl::Window::dispose();
    }

    void BibGridwin::Resize()
    {
        if(m_xGridWin.is())
        {
            ::Size aSize = GetOutputSizePixel();
            m_xGridWin->setPosSize(0, 0, aSize.Width(),aSize.Height(), awt::PosSize::SIZE);
        }
    }

    void BibGridwin::createGridWin(const uno::Reference< awt::XControlModel > & xGModel)
    {
        m_xGridModel = xGModel;

        if( m_xControlContainer.is())
        {
            uno::Reference< uno::XComponentContext > xContext = comphelper::getProcessComponentContext();

            if ( m_xGridModel.is())
            {
                uno::Reference< XPropertySet >  xPropSet( m_xGridModel, UNO_QUERY );

                if ( xPropSet.is() && m_xGridModel.is() )
                {
                    uno::Any aAny = xPropSet->getPropertyValue( "DefaultControl" );
                    OUString aControlName;
                    aAny >>= aControlName;

                    m_xControl.set( xContext->getServiceManager()->createInstanceWithContext(aControlName, xContext), UNO_QUERY_THROW );
                    m_xControl->setModel( m_xGridModel );
                }

                if ( m_xControl.is() )
                {
                    // Peer as Child to the FrameWindow
                    m_xControlContainer->addControl("GridControl", m_xControl);
                    m_xGridWin.set(m_xControl, UNO_QUERY );
                    m_xDispatchProviderInterception.set(m_xControl, UNO_QUERY );
                    m_xGridWin->setVisible( sal_True );
                    m_xControl->setDesignMode( sal_True );
                    // initially switch on the design mode - switch it off _after_ loading the form

                    ::Size aSize = GetOutputSizePixel();
                    m_xGridWin->setPosSize(0, 0, aSize.Width(),aSize.Height(), awt::PosSize::POSSIZE);
                }
            }
        }
    }

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

    void BibGridwin::GetFocus()
    {
        if(m_xGridWin.is())
            m_xGridWin->setFocus();
    }

    BibBeamer::BibBeamer( vcl::Window* _pParent, BibDataManager* _pDM, WinBits _nStyle )
        :BibSplitWindow( _pParent, _nStyle | WB_NOSPLITDRAW )
        ,pDatMan( _pDM )
        ,pToolBar( NULL )
        ,pGridWin( NULL )
    {
        createToolBar();
        createGridWin();
        pDatMan->SetToolbar(pToolBar);
        pGridWin->Show();
        connectForm( pDatMan );
    }

    BibBeamer::~BibBeamer()
    {
        disposeOnce();
    }

    void BibBeamer::dispose()
    {
        if ( isFormConnected() )
            disconnectForm();

        if ( m_xToolBarRef.is() )
            m_xToolBarRef->dispose();

        if ( pToolBar )
            pDatMan->SetToolbar(0);

        pToolBar.disposeAndClear();
        pGridWin.disposeAndClear();
        BibSplitWindow::dispose();
    }

    void BibBeamer::createToolBar()
    {
        pToolBar= VclPtr<BibToolBar>::Create(this, LINK( this, BibBeamer, RecalcLayout_Impl ));
        ::Size aSize=pToolBar->GetSizePixel();
        InsertItem(ID_TOOLBAR, pToolBar, aSize.Height(), 0, 0, SplitWindowItemFlags::Fixed );
        if ( m_xController.is() )
            pToolBar->SetXController( m_xController );
    }

    void BibBeamer::createGridWin()
    {
        pGridWin = VclPtr<BibGridwin>::Create(this,0);

        InsertItem(ID_GRIDWIN, pGridWin, 40, 1, 0, SplitWindowItemFlags::RelativeSize );

        pGridWin->createGridWin( pDatMan->updateGridModel() );
    }

    Reference< awt::XControlContainer > BibBeamer::getControlContainer()
    {
        Reference< awt::XControlContainer > xReturn;
        if ( pGridWin )
            xReturn = pGridWin->getControlContainer();
        return xReturn;
    }

    Reference< frame::XDispatchProviderInterception > BibBeamer::getDispatchProviderInterception()
    {
        Reference< frame::XDispatchProviderInterception > xReturn;
        if ( pGridWin )
            xReturn = pGridWin->getDispatchProviderInterception();
        return xReturn;
    }

    void BibBeamer::SetXController(const uno::Reference< frame::XController > & xCtr)
    {
        m_xController = xCtr;

        if ( pToolBar )
            pToolBar->SetXController( m_xController );

    }

    void BibBeamer::GetFocus()
    {
        if( pGridWin )
            pGridWin->GrabFocus();
    }

    IMPL_LINK_NOARG_TYPED( BibBeamer, RecalcLayout_Impl, void*, void )
    {
        long nHeight = pToolBar->GetSizePixel().Height();
        SetItemSize( ID_TOOLBAR, nHeight );
    }

}   // namespace bib

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
