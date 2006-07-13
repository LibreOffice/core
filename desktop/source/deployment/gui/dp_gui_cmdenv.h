/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dp_gui_cmdenv.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-13 17:02:23 $
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

#if ! defined INCLUDED_DP_GUI_CMDENV_H
#define INCLUDED_DP_GUI_CMDENV_H

#include "tools/resmgr.hxx"
#include "osl/conditn.hxx"
#include "cppuhelper/implbase3.hxx"
#include "vcl/dialog.hxx"
#include "vcl/button.hxx"
#include "vcl/fixed.hxx"
#include "vcl/status.hxx"
#include "com/sun/star/deployment/thePackageManagerFactory.hpp"
#include <memory>


namespace css = ::com::sun::star;

namespace dp_gui
{

struct DialogImpl;

//==============================================================================
class ProgressCommandEnv
    : public ::cppu::WeakImplHelper3< css::ucb::XCommandEnvironment,
                                      css::task::XInteractionHandler,
                                      css::ucb::XProgressHandler >
{
    DialogImpl * m_mainDialog;
    ::rtl::OUString m_title;
    css::uno::Reference<css::task::XInteractionHandler> m_xHandler;
    sal_Int32 m_currentInnerProgress;
    sal_Int32 m_currentProgressSection;
    sal_Int32 m_progressSections;
    void updateProgress( ::rtl::OUString const & text = ::rtl::OUString() );
    css::uno::Reference<css::task::XAbortChannel> m_xAbortChannel;
    bool m_aborted;
    // shared or user, may be empty string
    ::rtl::OUString m_sContext;

    struct ProgressDialog : public Dialog
    {
        struct CancelButtonImpl : public CancelButton
        {
            ProgressDialog * m_dialog;
            inline CancelButtonImpl( ProgressDialog * dialog )
                : CancelButton( dialog ),
                  m_dialog( dialog )
                {}
            virtual void Click();
        };
        friend struct CancelButtonImpl;

        ProgressCommandEnv * m_cmdEnv;
        ::std::auto_ptr<FixedText> m_ftCurrentAction;
        ::std::auto_ptr<StatusBar> m_statusBar;
        ::std::auto_ptr<CancelButtonImpl> m_cancelButton;

        ProgressDialog( ProgressCommandEnv * cmdEnv );
    };
    friend struct ProgressDialog;
    friend struct ProgressDialog::CancelButtonImpl;

    ::std::auto_ptr<ProgressDialog> m_progressDialog;
    DECL_LINK( executeDialog, ::osl::Condition * );

    void update_( css::uno::Any const & Status )
        throw (css::uno::RuntimeException);

    void solarthread_dtor();

    Dialog * activeDialog(); // either m_progressDialog or m_mainDialog

public:
    virtual ~ProgressCommandEnv();
    inline ProgressCommandEnv( DialogImpl * mainDialog,
                               ::rtl::OUString const & title,
                               ::rtl::OUString const & context)
        : m_mainDialog( mainDialog ),
          m_title( title ),
          m_aborted( false ),
          m_sContext(context)
        {}

    void showProgress( sal_Int32 progressSections );
    void progressSection(
        String const & text,
        css::uno::Reference< css::task::XAbortChannel > const & xAbortChannel = 0 );
    inline bool isAborted() const { return m_aborted; }

    // XCommandEnvironment
    virtual css::uno::Reference<css::task::XInteractionHandler > SAL_CALL
    getInteractionHandler() throw (css::uno::RuntimeException);
    virtual css::uno::Reference<css::ucb::XProgressHandler >
    SAL_CALL getProgressHandler() throw (css::uno::RuntimeException);

    // XInteractionHandler
    virtual void SAL_CALL handle(
        css::uno::Reference<css::task::XInteractionRequest > const & xRequest )
        throw (css::uno::RuntimeException);

    // XProgressHandler
    virtual void SAL_CALL push( css::uno::Any const & Status )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL update( css::uno::Any const & Status )
        throw (css::uno::RuntimeException);
    virtual void SAL_CALL pop() throw (css::uno::RuntimeException);
};

}

#endif
