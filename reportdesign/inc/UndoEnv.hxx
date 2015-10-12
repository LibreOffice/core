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
#ifndef INCLUDED_REPORTDESIGN_INC_UNDOENV_HXX
#define INCLUDED_REPORTDESIGN_INC_UNDOENV_HXX

#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>

#include <cppuhelper/compbase.hxx>
#include <memory>
#include <svl/lstner.hxx>

namespace rptui
{
    class OXUndoEnvironmentImpl;


    class REPORTDESIGN_DLLPUBLIC OXUndoEnvironment
        : public ::cppu::WeakImplHelper<   ::com::sun::star::beans::XPropertyChangeListener
                                        ,   ::com::sun::star::container::XContainerListener
                                        ,   ::com::sun::star::util::XModifyListener
                                        >
        , public SfxListener
    {
        const ::std::unique_ptr<OXUndoEnvironmentImpl> m_pImpl;

        OXUndoEnvironment(const OXUndoEnvironment&) = delete;
        OXUndoEnvironment& operator=(const OXUndoEnvironment&) = delete;

    protected:
        virtual ~OXUndoEnvironment();

        void SetUndoMode(bool _bUndo);

    public:
        OXUndoEnvironment(OReportModel& _rModel);

        /**
           Create an object ob OUndoEnvLock locks the undo possibility
           As long as in the OUndoEnvLock scope, no undo is possible for manipulated object.
         */
        class OUndoEnvLock
        {
            OXUndoEnvironment& m_rUndoEnv;
        public:
            OUndoEnvLock(OXUndoEnvironment& _rUndoEnv): m_rUndoEnv(_rUndoEnv){m_rUndoEnv.Lock();}
            ~OUndoEnvLock(){ m_rUndoEnv.UnLock(); }
        };

        /**
           This is near the same as OUndoEnvLock but it is also possible to ask for the current mode.
           UndoMode will set if SID_UNDO is called in execute()
         */
        class OUndoMode
        {
            OXUndoEnvironment& m_rUndoEnv;
        public:
            OUndoMode(OXUndoEnvironment& _rUndoEnv)
                :m_rUndoEnv(_rUndoEnv)
            {
                m_rUndoEnv.Lock();
                m_rUndoEnv.SetUndoMode(true);
            }
            ~OUndoMode()
            {
                m_rUndoEnv.SetUndoMode(false);
                m_rUndoEnv.UnLock();
            }
        };

        void Lock();
        void UnLock();
        bool IsLocked() const;

        // returns sal_True is we are in UNDO
        bool IsUndoMode() const;

        // access control
        struct Accessor { friend class OReportModel; private: Accessor() { } };
        void Clear(const Accessor& _r);

        void AddElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);
        void RemoveElement(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);

        void AddSection( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection);
        void RemoveSection( const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection>& _xSection );
        /** removes the section from the page out of the undo env
        *
        * \param _pPage
        */
        void RemoveSection(OReportPage* _pPage);

    protected:
        // XEventListener
        virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;

        // XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XContainerListener
        virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
        virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

        // XModifyListener
        virtual void SAL_CALL modified( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

        void ModeChanged();

        virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    private:
        void TogglePropertyListening(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& Element);

        void    implSetModified();

        void    switchListening( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess >& _rxContainer, bool _bStartListening );
        void    switchListening( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxObject, bool _bStartListening );

        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild> >::const_iterator
            getSection(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XChild>& _xContainer) const;
    };

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
