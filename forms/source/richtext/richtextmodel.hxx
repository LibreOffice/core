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
#ifndef INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTMODEL_HXX
#define INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTMODEL_HXX

#include "FormComponent.hxx"
#include "formcontrolfont.hxx"
#include "richtextunowrapper.hxx"
#include <comphelper/propertycontainerhelper.hxx>
#include <comphelper/listenernotification.hxx>

#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <cppuhelper/implbase.hxx>
#include <tools/link.hxx>
#include <memory>

class EditEngine;

namespace frm
{


    class RichTextEngine;

    //= ORichTextModel

    typedef ::cppu::ImplHelper  <   css::awt::XControlModel
                                ,   css::lang::XUnoTunnel
                                ,   css::util::XModifyBroadcaster
                                >   ORichTextModel_BASE;

    class ORichTextModel
            :public OControlModel
            ,public FontControlModel
            ,public IEngineTextChangeListener
            ,public ::comphelper::OPropertyContainerHelper
            ,public ORichTextModel_BASE
    {
    public:
        DECLARE_DEFAULT_LEAF_XTOR( ORichTextModel );

    private:
        // <properties>
        css::uno::Reference< css::awt::XDevice >
                                    m_xReferenceDevice;
        css::uno::Any  m_aTabStop;
        css::uno::Any  m_aBackgroundColor;
        css::uno::Any  m_aBorderColor;
        css::uno::Any  m_aVerticalAlignment;
        OUString             m_sDefaultControl;
        OUString             m_sHelpText;
        OUString             m_sHelpURL;
        OUString             m_sLastKnownEngineText;
        sal_Int16                   m_nLineEndFormat;
        sal_Int16                   m_nTextWritingMode;
        sal_Int16                   m_nContextWritingMode;
        sal_Int16                   m_nBorder;
        bool                    m_bEnabled;
        bool                    m_bEnableVisible;
        bool                    m_bHardLineBreaks;
        bool                    m_bHScroll;
        bool                    m_bVScroll;
        bool                    m_bReadonly;
        bool                    m_bPrintable;
        bool                    m_bReallyActAsRichText; // despite the class name, the RichTextControl later on
                                                            // will create "ordinary" text peers depending on this property
        bool                    m_bHideInactiveSelection;
        // </properties>

        // <properties_for_awt_edit_compatibility>
        css::uno::Any  m_aAlign;
        sal_Int16                   m_nEchoChar;
        sal_Int16                   m_nMaxTextLength;
        bool                    m_bMultiLine;
        // </properties_for_awt_edit_compatibility>

        ::std::unique_ptr<RichTextEngine>
                                    m_pEngine;
        bool                        m_bSettingEngineText;

        ::cppu::OInterfaceContainerHelper
                                    m_aModifyListeners;

    public:
        static  RichTextEngine* getEditEngine( const css::uno::Reference< css::awt::XControlModel >& _rxModel );

    private:

        // UNO
        DECLARE_UNO3_AGG_DEFAULTS( ORichTextModel, OControlModel )
        virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) throw (css::uno::RuntimeException, std::exception) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw (::css::uno::RuntimeException, std::exception) override;
        virtual ::css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw (::css::uno::RuntimeException, std::exception) override;

        // XPersistObject
        DECLARE_XPERSISTOBJECT()

        // XTypeProvider
        DECLARE_XTYPEPROVIDER()

        // XCloneable
        virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

        // XUnoTunnel
        virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) throw (css::uno::RuntimeException, std::exception) override;

        // XModifyBroadcaster
        virtual void SAL_CALL addModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL removeModifyListener( const css::uno::Reference< css::util::XModifyListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

        // XPropertySet and friends
        virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle ) const override;
        virtual sal_Bool SAL_CALL convertFastPropertyValue(css::uno::Any& rConvertedValue, css::uno::Any& rOldValue,
                                            sal_Int32 nHandle, const css::uno::Any& rValue )
                                        throw(css::lang::IllegalArgumentException) override;
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const css::uno::Any& rValue) throw ( css::uno::Exception, std::exception) override;
        virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const override;

        // OControlModel's property handling
        virtual void describeFixedProperties(
            css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
        ) const override;
        virtual void describeAggregateProperties(
            css::uno::Sequence< css::beans::Property >& /* [out] */ _rAggregateProps
        ) const override;

        // prevent method hiding
        using OControlModel::disposing;
        using OControlModel::getFastPropertyValue;

        // OComponentHelper
        virtual void SAL_CALL disposing() override;

        // IEngineTextChangeListener
        virtual void potentialTextChange( ) override;

    private:
        void    implInit();
        void    implDoAggregation();
        void    implRegisterProperties();

        /** propagates a new text to the EditEngine

            This method needs to lock the global solar mutex, so our own mutex must not
            be locked when calling.

        @precond
            our mutex is not locked
        */
        void    impl_smlock_setEngineText( const OUString& _rText );

        DECL_LINK_TYPED( OnEngineContentModified, LinkParamNone*, void );

        static  css::uno::Sequence< sal_Int8 > getEditEngineTunnelId();

    private:
        ORichTextModel( const ORichTextModel& ) = delete;
        ORichTextModel& operator=( const ORichTextModel& ) = delete;
    };


} // namespace frm


#endif // INCLUDED_FORMS_SOURCE_RICHTEXT_RICHTEXTMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
