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

#ifndef INCLUDED_SVTOOLS_GENERICUNODIALOG_HXX
#define INCLUDED_SVTOOLS_GENERICUNODIALOG_HXX

#include <svtools/svtdllapi.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/lang/NotInitializedException.hpp>

#include <cppuhelper/implbase.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/broadcasthelper.hxx>
#include <tools/link.hxx>
#include <vcl/dialog.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/weld.hxx>

namespace com :: sun :: star :: awt { class XWindow; }
namespace com :: sun :: star :: uno { class XComponentContext; }

class VclWindowEvent;


namespace svt
{


#define     UNODIALOG_PROPERTY_ID_TITLE     1
#define     UNODIALOG_PROPERTY_ID_PARENT    2

#define     UNODIALOG_PROPERTY_TITLE        "Title"
#define     UNODIALOG_PROPERTY_PARENT       "ParentWindow"

    typedef cppu::WeakImplHelper< css::ui::dialogs::XExecutableDialog,
                                  css::lang::XServiceInfo,
                                  css::lang::XInitialization > OGenericUnoDialogBase;

    /** abstract base class for implementing UNO objects representing dialogs (com.sun.star.awt::XDialog)
    */
    class SVT_DLLPUBLIC OGenericUnoDialog
            :public OGenericUnoDialogBase
            ,public ::comphelper::OMutexAndBroadcastHelper
            ,public ::comphelper::OPropertyContainer
    {
    public:
        struct Dialog
        {
            VclPtr<::Dialog> m_xVclDialog;
            std::unique_ptr<weld::DialogController> m_xWeldDialog;

            Dialog()
            {
            }

            Dialog(const VclPtr<::Dialog>& rVclDialog)
                : m_xVclDialog(rVclDialog)
            {
            }

            Dialog(std::unique_ptr<weld::DialogController> pWeldDialog)
                : m_xWeldDialog(std::move(pWeldDialog))
            {
            }

            explicit operator bool() const
            {
                return m_xVclDialog || m_xWeldDialog;
            }

            void set_title(const OUString& rTitle)
            {
                if (m_xWeldDialog)
                    m_xWeldDialog->set_title(rTitle);
                else if (m_xVclDialog)
                    m_xVclDialog->SetText(rTitle);
            }

            OString get_help_id() const
            {
                if (m_xWeldDialog)
                    return m_xWeldDialog->get_help_id();
                else if (m_xVclDialog)
                    return m_xVclDialog->GetHelpId();
                return OString();
            }

            void set_help_id(const OString& rHelpId)
            {
                if (m_xWeldDialog)
                    return m_xWeldDialog->set_help_id(rHelpId);
                else if (m_xVclDialog)
                    return m_xVclDialog->SetHelpId(rHelpId);
            }
        };
    protected:
        OGenericUnoDialog::Dialog   m_aDialog;                  /// the dialog to execute
        bool                        m_bExecuting : 1;           /// we're currently executing the dialog
        bool                        m_bTitleAmbiguous : 1;      /// m_sTitle has not been set yet
        bool                        m_bInitialized : 1;         /// has "initialize" been called?
        bool                        m_bNeedInitialization : 1;  /// do we need to be initialized before any other API call is allowed?

        // <properties>
        OUString                                         m_sTitle;   /// title of the dialog
        css::uno::Reference<css::awt::XWindow>           m_xParent;  /// parent window
        // </properties>

        css::uno::Reference<css::uno::XComponentContext> m_aContext;

    public:
        bool needInitialization() const { return m_bNeedInitialization && !m_bInitialized; }

    protected:
        OGenericUnoDialog(const css::uno::Reference< css::uno::XComponentContext >& _rxContext);
        virtual ~OGenericUnoDialog() override;

    public:
        // UNO
        DECLARE_UNO3_DEFAULTS(OGenericUnoDialog, OGenericUnoDialogBase)
        virtual css::uno::Any SAL_CALL queryInterface(const css::uno::Type& _rType) override;

        // XTypeProvider
        virtual css::uno::Sequence<css::uno::Type> SAL_CALL getTypes(  ) override;
        virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override = 0;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override = 0;
        virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
        virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override = 0;

        // OPropertySetHelper
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue ) override;
        virtual sal_Bool SAL_CALL convertFastPropertyValue( css::uno::Any& rConvertedValue, css::uno::Any& rOldValue, sal_Int32 nHandle, const css::uno::Any& rValue) override;

        // XExecutableDialog
        virtual void SAL_CALL setTitle( const OUString& aTitle ) override;
        virtual sal_Int16 SAL_CALL execute(  ) override;

        // XInitialization
        virtual void SAL_CALL initialize( const css::uno::Sequence< css::uno::Any >& aArguments ) override;

    protected:
        /** create the concrete dialog instance. Note that m_aMutex is not locked when this method get called,
            but the application-wide solar mutex is (to guard the not thread-safe ctor of the dialog).
            @param      pParent     the parent window for the new dialog
        */
        virtual OGenericUnoDialog::Dialog createDialog(const css::uno::Reference<css::awt::XWindow>& rParent) = 0;

        /// called to destroy the dialog used. deletes m_pDialog and resets it to NULL
        void destroyDialog();

        /** called after the dialog has been executed
            @param      _nExecutionResult       the execution result as returned by Dialog::Execute
        */
        virtual void executedDialog(sal_Int16 /*_nExecutionResult*/) { }

        /** smaller form of <method>initialize</method>.<p/>
            The <method>initialize</method> method is called with a sequence of com.sun.star.uno::Any's,
            which is split up into the single elements, which are passed to implInitialize. The default implementation
            tries to extract an com.sun.star.beans::PropertyValue from the value an pass it to the
            com.sun.star.beans::XPropertySet interface of the object.
        */
        virtual void implInitialize(const css::uno::Any& _rValue);

    private:
        DECL_LINK( OnDialogDying, VclWindowEvent&, void );

        /** ensures that m_pDialog is not <NULL/>

            This method does nothing if m_pDialog is already non-<NULL/>. Else, it calls createDialog and does
            all necessary initializations of the new dialog instance.

            @precond
                m_aMutex is locked

            @return
                <TRUE/> if and only if m_pDialog is non-<NULL/> upon returning from the method. Note that the only
                case where m_pDialog is <NULL/> is when createDialog returned <NULL/>, which is will fire an assertion
                in non-product builds.
        */
        bool    impl_ensureDialog_lck();
    };

    /// helper class for guarding access to methods of a OGenericUnoDialog
    class UnoDialogEntryGuard
    {
    public:
        UnoDialogEntryGuard( OGenericUnoDialog& _rDialog )
            :m_aGuard( _rDialog.GetMutex() )
        {
            if ( _rDialog.needInitialization() )
                throw css::lang::NotInitializedException();
        }

    private:
        ::osl::MutexGuard   m_aGuard;
    };


}   // namespace svt


#endif // INCLUDED_SVTOOLS_GENERICUNODIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
