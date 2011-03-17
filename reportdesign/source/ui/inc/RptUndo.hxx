/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef RPTUI_UNDO_HXX
#define RPTUI_UNDO_HXX

#include "UndoActions.hxx"
#include <functional>

FORWARD_DECLARE_INTERFACE(awt,XControl)
FORWARD_DECLARE_INTERFACE(awt,XControlContainer)
FORWARD_DECLARE_INTERFACE(drawing,XShape)
namespace dbaui
{
    class IController;
}
namespace rptui
{
    class OObjectBase;


    /** \class OSectionUndo
     * Undo class for section add and remove.
     */
    class OSectionUndo : public OCommentUndoAction
    {
        OSectionUndo(const OSectionUndo&);
        void operator =(const OSectionUndo&);
    protected:
        ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape> >
                                                    m_aControls;
        ::std::vector< ::std::pair< ::rtl::OUString ,::com::sun::star::uno::Any> >
                                                    m_aValues;
        Action                                      m_eAction;
        sal_uInt16                                  m_nSlot;
        bool                                        m_bInserted;

        virtual void    implReInsert( ) = 0;
        virtual void    implReRemove( ) = 0;

        void collectControls(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
    public:
        TYPEINFO();
        OSectionUndo(   OReportModel& rMod
                        ,sal_uInt16 _nSlot
                        ,Action _eAction
                        ,sal_uInt16 nCommentID);
        virtual ~OSectionUndo();

        virtual void        Undo();
        virtual void        Redo();
    };

    /** Undo action for the group header, footer, page header, footer
    */
    class OReportSectionUndo : public OSectionUndo
    {
        OReportHelper                               m_aReportHelper;
        ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                    ,OReportHelper> m_pMemberFunction;

        void    implReInsert( );
        void    implReRemove( );
        OReportSectionUndo(const OReportSectionUndo&);
        void operator =(const OReportSectionUndo&);
    public:
        TYPEINFO();
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

        mutable ::rtl::OUString                     m_sName;

        void    implReInsert( );
        void    implReRemove( );
        OGroupSectionUndo(const OGroupSectionUndo&);
        void operator =(const OGroupSectionUndo&);
    public:
        TYPEINFO();
        //OGroupSectionUndo(     const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection
        OGroupSectionUndo(  OReportModel& rMod
                            ,sal_uInt16 _nSlot
                            ,::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >
                                            ,OGroupHelper> _pMemberFunction
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup >& _xGroup
                            ,Action _eAction
                            ,sal_uInt16 nCommentID);

        virtual String      GetComment() const;
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
        TYPEINFO();
        OGroupUndo(OReportModel& rMod
                    ,sal_uInt16 nCommentID
                    ,Action _eAction
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XGroup>& _xGroup
                    ,const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportDefinition >& _xReportDefinition);
        virtual void        Undo();
        virtual void        Redo();
    };
}
#endif // RPTUI_UNDO_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
