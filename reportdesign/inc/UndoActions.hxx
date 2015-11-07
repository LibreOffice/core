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
#ifndef INCLUDED_REPORTDESIGN_INC_UNDOACTIONS_HXX
#define INCLUDED_REPORTDESIGN_INC_UNDOACTIONS_HXX

#include "dllapi.h"

#include "RptModel.hxx"

#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include <com/sun/star/script/ScriptEvent.hpp>
#include <com/sun/star/script/XScriptListener.hpp>
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/ContainerEvent.hpp>
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XGroup.hpp>
#include <com/sun/star/document/XUndoManager.hpp>

#include <comphelper/uno3.hxx>
#include <comphelper/sequence.hxx>
#include <svl/lstner.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdundo.hxx>

#include <functional>

namespace dbaui
{
    class IController;
}
namespace rptui
{
    enum Action
    {
        Inserted = 1,
        Removed  = 2
    };

    /** Helper class to allow std::mem_fun for SAL_CALL
    */
    class REPORTDESIGN_DLLPUBLIC OGroupHelper
    {
        css::uno::Reference< css::report::XGroup > m_xGroup;
        OGroupHelper(const OGroupHelper&) = delete;
        OGroupHelper& operator=(const OGroupHelper&) = delete;
    public:
        OGroupHelper(const css::uno::Reference< css::report::XGroup >& _xGroup)
            :m_xGroup(_xGroup)
        {
        }
        inline css::uno::Reference< css::report::XSection >   getHeader() { return m_xGroup->getHeader(); }
        inline css::uno::Reference< css::report::XSection >   getFooter() { return m_xGroup->getFooter(); }
        inline css::uno::Reference< css::report::XGroup >     getGroup() { return m_xGroup; }

        inline bool getHeaderOn() { return m_xGroup->getHeaderOn(); }
        inline bool getFooterOn() { return m_xGroup->getFooterOn(); }

        static ::std::mem_fun_t< css::uno::Reference< css::report::XSection> , OGroupHelper> getMemberFunction(const css::uno::Reference< css::report::XSection >& _xSection);

    };

    /** Helper class to allow std::mem_fun for SAL_CALL
    */
    class REPORTDESIGN_DLLPUBLIC OReportHelper
    {
        css::uno::Reference< css::report::XReportDefinition > m_xReport;
    public:
        OReportHelper(const css::uno::Reference< css::report::XReportDefinition >& _xReport)
            :m_xReport(_xReport)
        {
        }
        inline css::uno::Reference< css::report::XSection > getReportHeader() { return m_xReport->getReportHeader(); }
        inline css::uno::Reference< css::report::XSection > getReportFooter() { return m_xReport->getReportFooter(); }
        inline css::uno::Reference< css::report::XSection > getPageHeader()   { return m_xReport->getPageHeader(); }
        inline css::uno::Reference< css::report::XSection > getPageFooter()   { return m_xReport->getPageFooter(); }
        inline css::uno::Reference< css::report::XSection > getDetail()       { return m_xReport->getDetail(); }

        inline bool getReportHeaderOn() { return m_xReport->getReportHeaderOn(); }
        inline bool getReportFooterOn() { return m_xReport->getReportFooterOn(); }
        inline bool getPageHeaderOn() { return m_xReport->getPageHeaderOn(); }
        inline bool getPageFooterOn() { return m_xReport->getPageFooterOn(); }

        static ::std::mem_fun_t< css::uno::Reference< css::report::XSection> , OReportHelper> getMemberFunction(const css::uno::Reference< css::report::XSection >& _xSection);
    };


    //= UndoContext

    class UndoContext
    {
    public:
        UndoContext( SfxUndoManager& i_undoManager, const OUString& i_undoTitle )
            :m_rUndoManager( i_undoManager )
        {
            m_rUndoManager.EnterListAction( i_undoTitle, OUString() );
        }

        ~UndoContext()
        {
            m_rUndoManager.LeaveListAction();
        }

    private:
        SfxUndoManager& m_rUndoManager;
    };


    //= UndoSuppressor

    class UndoSuppressor
    {
    public:
        UndoSuppressor( SfxUndoManager& i_undoManager )
            :m_rUndoManager( i_undoManager )
        {
            m_rUndoManager.EnableUndo( false );
        }

        ~UndoSuppressor()
        {
            m_rUndoManager.EnableUndo( true );
        }

    private:
        SfxUndoManager& m_rUndoManager;
    };


    //= OCommentUndoAction

    class REPORTDESIGN_DLLPUBLIC OCommentUndoAction : public SdrUndoAction
    {
    protected:
        OUString                m_strComment; // undo, redo comment
        ::dbaui::IController*   m_pController;

    public:
        OCommentUndoAction( SdrModel& rMod
                            ,sal_uInt16 nCommentID);
        virtual ~OCommentUndoAction();

        virtual OUString GetComment() const override { return m_strComment; }
        virtual void        Undo() override;
        virtual void        Redo() override;
    };

    // OUndoContainerAction

    class REPORTDESIGN_DLLPUBLIC OUndoContainerAction: public OCommentUndoAction
    {
        OUndoContainerAction(OUndoContainerAction&) = delete;
        void operator =(OUndoContainerAction&) = delete;
    protected:
        css::uno::Reference< css::uno::XInterface >
                        m_xElement;     // object not owned by the action
        css::uno::Reference< css::uno::XInterface >
                        m_xOwnElement;  // object owned by the action
        css::uno::Reference< css::container::XIndexContainer >
                        m_xContainer;
        css::uno::Reference< css::report::XSection >
                        m_xSection;
        Action          m_eAction;

    public:
        OUndoContainerAction(SdrModel& rMod
                            ,Action _eAction
                            ,const css::uno::Reference< css::container::XIndexContainer >& rContainer
                            ,const css::uno::Reference< css::uno::XInterface>& xElem
                            ,sal_uInt16 _nCommentId);
        virtual ~OUndoContainerAction();

        virtual void Undo() override;
        virtual void Redo() override;

    protected:
        virtual void    implReInsert( );
        virtual void    implReRemove( );
    };


    // OUndoReportSectionAction

    class REPORTDESIGN_DLLPUBLIC OUndoReportSectionAction : public OUndoContainerAction
    {
        OReportHelper                               m_aReportHelper;
        ::std::mem_fun_t< css::uno::Reference< css::report::XSection >
                                    ,OReportHelper> m_pMemberFunction;
    public:
        OUndoReportSectionAction(SdrModel& rMod
                            ,Action _eAction
                            ,::std::mem_fun_t< css::uno::Reference< css::report::XSection >
                                ,OReportHelper> _pMemberFunction
                            ,const css::uno::Reference< css::report::XReportDefinition >& _xReport
                            ,const css::uno::Reference< css::uno::XInterface>& xElem
                            ,sal_uInt16 _nCommentId);

    protected:
        virtual void    implReInsert( ) override;
        virtual void    implReRemove( ) override;
    };


    // OUndoGroupSectionAction
    class REPORTDESIGN_DLLPUBLIC OUndoGroupSectionAction : public OUndoContainerAction
    {
        OGroupHelper                                m_aGroupHelper;
        ::std::mem_fun_t< css::uno::Reference< css::report::XSection >
                                    ,OGroupHelper> m_pMemberFunction;
    public:
        OUndoGroupSectionAction(SdrModel& rMod
                            ,Action _eAction
                            ,::std::mem_fun_t< css::uno::Reference< css::report::XSection >
                                            ,OGroupHelper> _pMemberFunction
                            ,const css::uno::Reference< css::report::XGroup >& _xGroup
                            ,const css::uno::Reference< css::uno::XInterface>& xElem
                            ,sal_uInt16 _nCommentId);

    protected:
        virtual void    implReInsert( ) override;
        virtual void    implReRemove( ) override;
    };

    // ORptUndoPropertyAction
    class REPORTDESIGN_DLLPUBLIC ORptUndoPropertyAction: public OCommentUndoAction
    {
        css::uno::Reference< css::beans::XPropertySet> m_xObj;
        OUString               m_aPropertyName;
        css::uno::Any          m_aNewValue;
        css::uno::Any          m_aOldValue;

        /** sets either the old value or the new value again at the property set.
         *
         * \param _bOld If set to <TRUE/> than the old value will be set otherwise the new value will be set.
         */
        void setProperty(bool _bOld);
    protected:
        virtual css::uno::Reference< css::beans::XPropertySet> getObject();

    public:
        ORptUndoPropertyAction(SdrModel& rMod, const css::beans::PropertyChangeEvent& evt);

        virtual void Undo() override;
        virtual void Redo() override;

        virtual OUString GetComment() const override;
    };


    // OUndoPropertyReportSectionAction

    class REPORTDESIGN_DLLPUBLIC OUndoPropertyReportSectionAction : public ORptUndoPropertyAction
    {
        OReportHelper                               m_aReportHelper;
        ::std::mem_fun_t< css::uno::Reference< css::report::XSection >
                                    ,OReportHelper> m_pMemberFunction;
    protected:
        virtual css::uno::Reference< css::beans::XPropertySet> getObject() override;
    public:
        OUndoPropertyReportSectionAction(SdrModel& rMod
                            ,const css::beans::PropertyChangeEvent& evt
                            ,::std::mem_fun_t< css::uno::Reference< css::report::XSection >
                                ,OReportHelper> _pMemberFunction
                            ,const css::uno::Reference< css::report::XReportDefinition >& _xReport
                            );
    };


    // OUndoPropertyGroupSectionAction

    class REPORTDESIGN_DLLPUBLIC OUndoPropertyGroupSectionAction : public ORptUndoPropertyAction
    {
        OGroupHelper                                m_aGroupHelper;
        ::std::mem_fun_t< css::uno::Reference< css::report::XSection >
                                    ,OGroupHelper> m_pMemberFunction;
    protected:
        virtual css::uno::Reference< css::beans::XPropertySet> getObject() override;
    public:
        OUndoPropertyGroupSectionAction(SdrModel& rMod
                            ,const css::beans::PropertyChangeEvent& evt
                            ,::std::mem_fun_t< css::uno::Reference< css::report::XSection >
                                            ,OGroupHelper> _pMemberFunction
                            ,const css::uno::Reference< css::report::XGroup >& _xGroup
                            );
    };

}
#endif // INCLUDED_REPORTDESIGN_INC_UNDOACTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
