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
#ifndef RPTUI_UNDOACTIONS_HXX
#define RPTUI_UNDOACTIONS_HXX

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
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/report/XReportComponent.hpp>
#include <com/sun/star/report/XReportDefinition.hpp>
#include <com/sun/star/report/XGroup.hpp>
#include <com/sun/star/document/XUndoManager.hpp>

#include <cppuhelper/implbase3.hxx>
#include <comphelper/uno3.hxx>
#include <comphelper/sequence.hxx>
#include <svl/lstner.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdundo.hxx>
#include <tools/string.hxx>

#include <functional>
#include <memory>

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
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup > m_xGroup;
        OGroupHelper(const OGroupHelper&);
        OGroupHelper& operator=(const OGroupHelper&);
    public:
        OGroupHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >& _xGroup)
            :m_xGroup(_xGroup)
        {
        }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >   getHeader() { return m_xGroup->getHeader(); }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >   getFooter() { return m_xGroup->getFooter(); }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >     getGroup() { return m_xGroup; }

        inline ::sal_Bool getHeaderOn() { return m_xGroup->getHeaderOn(); }
        inline ::sal_Bool getFooterOn() { return m_xGroup->getFooterOn(); }

        static ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection> , OGroupHelper> getMemberFunction(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);

    };
    typedef com::sun::star::uno::Reference< ::com::sun::star::report::XSection > TSection;

    /** Helper class to allow std::mem_fun for SAL_CALL
    */
    class REPORTDESIGN_DLLPUBLIC OReportHelper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition > m_xReport;
    public:
        OReportHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xReport)
            :m_xReport(_xReport)
        {
        }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getReportHeader() { return m_xReport->getReportHeader(); }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getReportFooter() { return m_xReport->getReportFooter(); }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getPageHeader()   { return m_xReport->getPageHeader(); }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getPageFooter()   { return m_xReport->getPageFooter(); }
        inline ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection > getDetail()       { return m_xReport->getDetail(); }

        inline ::sal_Bool getReportHeaderOn() { return m_xReport->getReportHeaderOn(); }
        inline ::sal_Bool getReportFooterOn() { return m_xReport->getReportFooterOn(); }
        inline ::sal_Bool getPageHeaderOn() { return m_xReport->getPageHeaderOn(); }
        inline ::sal_Bool getPageFooterOn() { return m_xReport->getPageFooterOn(); }

        static ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection> , OReportHelper> getMemberFunction(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
    };

    //==================================================================================================================
    //= UndoContext
    //==================================================================================================================
    class UndoContext
    {
    public:
        UndoContext( SfxUndoManager& i_undoManager, const ::rtl::OUString& i_undoTitle )
            :m_rUndoManager( i_undoManager )
        {
            m_rUndoManager.EnterListAction( i_undoTitle, String() );
        }

        ~UndoContext()
        {
            m_rUndoManager.LeaveListAction();
        }

    private:
        SfxUndoManager& m_rUndoManager;
    };

    //==================================================================================================================
    //= UndoSuppressor
    //==================================================================================================================
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

    //==================================================================================================================
    //= OCommentUndoAction
    //==================================================================================================================
    class REPORTDESIGN_DLLPUBLIC OCommentUndoAction : public SdrUndoAction
    {
    protected:
        String                  m_strComment; // undo, redo comment
        ::dbaui::IController*   m_pController;

    public:
        TYPEINFO();
        OCommentUndoAction( SdrModel& rMod
                            ,sal_uInt16 nCommentID);
        virtual ~OCommentUndoAction();

        virtual rtl::OUString GetComment() const { return m_strComment; }
        virtual void        Undo();
        virtual void        Redo();
    };
    //==================================================================
    // OUndoContainerAction
    //==================================================================
    class REPORTDESIGN_DLLPUBLIC OUndoContainerAction: public OCommentUndoAction
    {
        OUndoContainerAction(OUndoContainerAction&);
        void operator =(OUndoContainerAction&);
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        m_xElement;     // object not owned by the action
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        m_xOwnElement;  // object owned by the action
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >
                        m_xContainer;
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                        m_xSection;
        Action          m_eAction;

    public:
        OUndoContainerAction(SdrModel& rMod
                            ,Action _eAction
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > _xContainer
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElem
                            ,sal_uInt16 _nCommentId);
        virtual ~OUndoContainerAction();

        virtual void Undo();
        virtual void Redo();

    protected:
        virtual void    implReInsert( ) SAL_THROW( ( ::com::sun::star::uno::Exception ) );
        virtual void    implReRemove( ) SAL_THROW( ( ::com::sun::star::uno::Exception ) );
    };

    //==================================================================
    // OUndoReportSectionAction
    //==================================================================
    class REPORTDESIGN_DLLPUBLIC OUndoReportSectionAction : public OUndoContainerAction
    {
        OReportHelper                               m_aReportHelper;
        ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                    ,OReportHelper> m_pMemberFunction;
    public:
        OUndoReportSectionAction(SdrModel& rMod
                            ,Action _eAction
                            ,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                ,OReportHelper> _pMemberFunction
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xReport
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElem
                            ,sal_uInt16 _nCommentId);

    protected:
        virtual void    implReInsert( ) SAL_THROW( ( ::com::sun::star::uno::Exception ) );
        virtual void    implReRemove( ) SAL_THROW( ( ::com::sun::star::uno::Exception ) );
    };

    //==================================================================
    // OUndoGroupSectionAction
    //==================================================================
    class REPORTDESIGN_DLLPUBLIC OUndoGroupSectionAction : public OUndoContainerAction
    {
        OGroupHelper                                m_aGroupHelper;
        ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                    ,OGroupHelper> m_pMemberFunction;
    public:
        OUndoGroupSectionAction(SdrModel& rMod
                            ,Action _eAction
                            ,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                            ,OGroupHelper> _pMemberFunction
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >& _xGroup
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElem
                            ,sal_uInt16 _nCommentId);

    protected:
        virtual void    implReInsert( ) SAL_THROW( ( ::com::sun::star::uno::Exception ) );
        virtual void    implReRemove( ) SAL_THROW( ( ::com::sun::star::uno::Exception ) );
    };

    ///==================================================================
    /// ORptUndoPropertyAction
    ///==================================================================
    class REPORTDESIGN_DLLPUBLIC ORptUndoPropertyAction: public OCommentUndoAction
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> m_xObj;
        ::rtl::OUString                     m_aPropertyName;
        ::com::sun::star::uno::Any          m_aNewValue;
        ::com::sun::star::uno::Any          m_aOldValue;

        /** sets either the old value or the new value again at the property set.
         *
         * \param _bOld If set to <TRUE/> than the old value will be set otherwise the new value will be set.
         */
        void setProperty(sal_Bool _bOld);
    protected:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getObject();

    public:
        ORptUndoPropertyAction(SdrModel& rMod, const ::com::sun::star::beans::PropertyChangeEvent& evt);

        virtual void Undo();
        virtual void Redo();

        virtual rtl::OUString GetComment() const;
    };

    //==================================================================
    // OUndoPropertyReportSectionAction
    //==================================================================
    class REPORTDESIGN_DLLPUBLIC OUndoPropertyReportSectionAction : public ORptUndoPropertyAction
    {
        OReportHelper                               m_aReportHelper;
        ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                    ,OReportHelper> m_pMemberFunction;
    protected:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getObject();
    public:
        OUndoPropertyReportSectionAction(SdrModel& rMod
                            ,const ::com::sun::star::beans::PropertyChangeEvent& evt
                            ,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                ,OReportHelper> _pMemberFunction
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xReport
                            );
    };

    //==================================================================
    // OUndoPropertyGroupSectionAction
    //==================================================================
    class REPORTDESIGN_DLLPUBLIC OUndoPropertyGroupSectionAction : public ORptUndoPropertyAction
    {
        OGroupHelper                                m_aGroupHelper;
        ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                    ,OGroupHelper> m_pMemberFunction;
    protected:
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> getObject();
    public:
        OUndoPropertyGroupSectionAction(SdrModel& rMod
                            ,const ::com::sun::star::beans::PropertyChangeEvent& evt
                            ,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                            ,OGroupHelper> _pMemberFunction
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >& _xGroup
                            );
    };

}
#endif //RPTUI_UNDOACTIONS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
