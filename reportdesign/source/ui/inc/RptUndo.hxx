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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_UI_INC_RPTUNDO_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_UI_INC_RPTUNDO_HXX

#include "UndoActions.hxx"
#include <functional>

namespace com { namespace sun { namespace star {
    namespace drawing {
        class XShape;
    }
}}}

namespace rptui
{
    /** \class OSectionUndo
     * Undo class for section add and remove.
     */
    class OSectionUndo : public OCommentUndoAction
    {
        OSectionUndo(const OSectionUndo&) = delete;
        void operator =(const OSectionUndo&) = delete;
    protected:
        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape> >
                                                    m_aControls;
        ::std::vector< ::std::pair< OUString ,::com::sun::star::uno::Any> >
                                                    m_aValues;
        Action                                      m_eAction;
        sal_uInt16                                  m_nSlot;
        bool                                        m_bInserted;

        virtual void    implReInsert( ) = 0;
        virtual void    implReRemove( ) = 0;

        void collectControls(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
    public:
        TYPEINFO_OVERRIDE();
        OSectionUndo(   OReportModel& rMod
                        ,sal_uInt16 _nSlot
                        ,Action _eAction
                        ,sal_uInt16 nCommentID);
        virtual ~OSectionUndo();

        virtual void        Undo() SAL_OVERRIDE;
        virtual void        Redo() SAL_OVERRIDE;
    };

    /** Undo action for the group header, footer, page header, footer
    */
    class OReportSectionUndo : public OSectionUndo
    {
        OReportHelper                               m_aReportHelper;
        ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                    ,OReportHelper> m_pMemberFunction;

        void    implReInsert( ) SAL_OVERRIDE;
        void    implReRemove( ) SAL_OVERRIDE;
        OReportSectionUndo(const OReportSectionUndo&) = delete;
        void operator =(const OReportSectionUndo&) = delete;
    public:
        TYPEINFO_OVERRIDE();
        //OReportSectionUndo(    const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
        OReportSectionUndo( OReportModel& rMod
                            ,sal_uInt16 _nSlot
                            ,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                ,OReportHelper> _pMemberFunction
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xReport
                            ,Action _eAction
                            ,sal_uInt16 nCommentID);
        virtual ~OReportSectionUndo();
    };

    /** Undo action for the group header, footer
    */
    class OGroupSectionUndo : public OSectionUndo
    {
        OGroupHelper                                m_aGroupHelper;
        ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                    ,OGroupHelper> m_pMemberFunction;

        mutable OUString                     m_sName;

        void    implReInsert( ) SAL_OVERRIDE;
        void    implReRemove( ) SAL_OVERRIDE;
        OGroupSectionUndo(const OGroupSectionUndo&) = delete;
        void operator =(const OGroupSectionUndo&) = delete;
    public:
        TYPEINFO_OVERRIDE();
        //OGroupSectionUndo(     const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
        OGroupSectionUndo(  OReportModel& rMod
                            ,sal_uInt16 _nSlot
                            ,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                            ,OGroupHelper> _pMemberFunction
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >& _xGroup
                            ,Action _eAction
                            ,sal_uInt16 nCommentID);

        virtual OUString GetComment() const SAL_OVERRIDE;
    };

    /** /class OGroupUndo
     * \brief Undo action for removing a group object.
    */
    class OGroupUndo : public OCommentUndoAction
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>             m_xGroup; ///<! the group for the undo redo action
        ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition > m_xReportDefinition; ///<! the parent report definition
        Action                                                                          m_eAction; ///<! the current action
        sal_Int32                                                                       m_nLastPosition; ///<! the last position of the group

        void    implReInsert( );
        void    implReRemove( );
    public:
        TYPEINFO_OVERRIDE();
        OGroupUndo(OReportModel& rMod
                    ,sal_uInt16 nCommentID
                    ,Action _eAction
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>& _xGroup
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xReportDefinition);
        virtual void        Undo() SAL_OVERRIDE;
        virtual void        Redo() SAL_OVERRIDE;
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_UI_INC_RPTUNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
