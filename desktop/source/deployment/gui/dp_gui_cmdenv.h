/*************************************************************************
 *
 *  $RCSfile: dp_gui_cmdenv.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-06-11 12:03:21 $
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

#if ! defined INCLUDED_DP_GUI_CMDENV_H
#define INCLUDED_DP_GUI_CMDENV_H

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

public:
    virtual ~ProgressCommandEnv();
    inline ProgressCommandEnv( DialogImpl * mainDialog,
                               ::rtl::OUString const & title )
        : m_mainDialog( mainDialog ),
          m_title( title ),
          m_aborted( false )
        {}

    void showProgress( sal_Int32 progressSections );
    void progressSection(
        String const & text,
        css::uno::Reference<css::task::XAbortChannel> const & xAbortChannel =
        css::uno::Reference<css::task::XAbortChannel>() );
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
