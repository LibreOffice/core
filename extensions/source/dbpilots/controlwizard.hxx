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

#ifndef INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_CONTROLWIZARD_HXX
#define INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_CONTROLWIZARD_HXX

#include <svtools/wizardmachine.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <vcl/weld.hxx>
#include "dbptypes.hxx"
#include <strings.hrc>
#include <componentmodule.hxx>
#include "wizardcontext.hxx"

class ResId;

namespace dbp
{
    struct OControlWizardSettings
    {
        OUString      sControlLabel;
    };

    class OControlWizard;
    typedef ::svt::OWizardPage OControlWizardPage_Base;
    class OControlWizardPage : public OControlWizardPage_Base
    {
        VclPtr<FixedText>      m_pFormDatasourceLabel;
        VclPtr<FixedText>      m_pFormDatasource;
        VclPtr<FixedText>      m_pFormContentTypeLabel;
        VclPtr<FixedText>      m_pFormContentType;
        VclPtr<FixedText>      m_pFormTableLabel;
        VclPtr<FixedText>      m_pFormTable;

    protected:
        OControlWizard*                 getDialog();
        const OControlWizard*           getDialog() const;
        const OControlWizardContext&    getContext();
        bool                        updateContext();
        void                            setFormConnection(const css::uno::Reference< css::sdbc::XConnection >& _rxConn, bool _bAutoDispose = true );
        css::uno::Reference< css::sdbc::XConnection >
                                        getFormConnection() const;
    public:
        OControlWizardPage( OControlWizard* _pParent, const OString& rID, const OUString& rUIXMLDescription );
        virtual ~OControlWizardPage() override;
        virtual void dispose() override;

    protected:
        static void fillListBox(
            ListBox& _rList,
            const css::uno::Sequence< OUString >& _rItems);
        static void fillListBox(
            ComboBox& _rList,
            const css::uno::Sequence< OUString >& _rItems);

    protected:
        void enableFormDatasourceDisplay();

    protected:
        // OWizardPage overridables
        virtual void        initializePage() override;
    };

    struct OAccessRegulator;

    typedef ::svt::OWizardMachine OControlWizard_Base;
    class OControlWizard : public OControlWizard_Base
    {
    private:
        OControlWizardContext   m_aContext;
        css::uno::Reference< css::uno::XComponentContext >
                                m_xContext;

    public:
        OControlWizard(
            vcl::Window* _pParent,
            const css::uno::Reference< css::beans::XPropertySet >& _rxObjectModel,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );
        virtual ~OControlWizard() override;

        // make the same base class methods public
        using OControlWizard_Base::travelNext;

    public:
        const css::uno::Reference< css::uno::XComponentContext >&
            getComponentContext() const { return m_xContext; }

        const OControlWizardContext&    getContext() const { return m_aContext; }
        bool                        updateContext(const OAccessRegulator&);
        void                            setFormConnection(const OAccessRegulator&, const css::uno::Reference< css::sdbc::XConnection >& _rxConn, bool _bAutoDispose );
            css::uno::Reference< css::sdbc::XConnection >
                                        getFormConnection(const OAccessRegulator&) const;

        /** returns the com.sun.star.task.InteractionHandler
            @param  _pWindow    The window will be used when an error message has to be shown.
        */
        css::uno::Reference< css::task::XInteractionHandler > getInteractionHandler(weld::Window* _pWindow) const;

    protected:
        // initialize the derivees settings (which have to be derived from OControlWizardSettings)
        // with some common data extracted from the control model
        void initControlSettings(OControlWizardSettings* _pSettings);
        // commit the control-relevant settings
        void commitControlSettings(OControlWizardSettings const * _pSettings);

        bool needDatasourceSelection();

        css::uno::Reference< css::sdbc::XConnection >
                                        getFormConnection() const;

        virtual bool approveControl(sal_Int16 _nClassId) = 0;

        // ModalDialog overridables
        virtual short   Execute() override;

    private:
        bool initContext();

        void implGetDSContext();
        void implDetermineForm();
        void implDeterminePage();
        void implDetermineShape();

        // made private. Not to be used by derived (or external) classes
        using OControlWizard_Base::ActivatePage;
    };


}   // namespace dbp


#endif // INCLUDED_EXTENSIONS_SOURCE_DBPILOTS_CONTROLWIZARD_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
