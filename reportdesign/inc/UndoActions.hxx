/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UndoActions.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 09:47:39 $
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
#ifndef RPTUI_UNDOACTIONS_HXX
#define RPTUI_UNDOACTIONS_HXX

#ifndef _SVDUNDO_HXX
#include <svx/svdundo.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_UTIL_XMODIFYLISTENER_HPP_
#include <com/sun/star/util/XModifyListener.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYCHANGEEVENT_HPP_
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENT_HPP_
#include <com/sun/star/script/ScriptEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XSCRIPTLISTENER_HPP_
#include <com/sun/star/script/XScriptListener.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_SCRIPTEVENTDESCRIPTOR_HPP_
#include <com/sun/star/script/ScriptEventDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_CONTAINEREVENT_HPP_
#include <com/sun/star/container/ContainerEvent.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCOMPONENT_HPP_
#include <com/sun/star/report/XReportComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTDEFINITION_HPP_
#include <com/sun/star/report/XReportDefinition.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XGROUP_HPP_
#include <com/sun/star/report/XGroup.hpp>
#endif
/** === end UNO includes === **/

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#include <comphelper/sequence.hxx>
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif
#ifndef _SVDOUNO_HXX
#include <svx/svdouno.hxx>
#endif
#ifndef REPORT_RPTMODEL_HXX
#include "RptModel.hxx"
#endif
#include "dllapi.h"
#include <functional>
#include <memory>
#include <boost/shared_ptr.hpp>

FORWARD_DECLARE_INTERFACE(awt,XControl)
FORWARD_DECLARE_INTERFACE(awt,XControlContainer)
namespace dbaui
{
    class IController;
}
namespace rptui
{
    class OObjectBase;

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

        static ::std::mem_fun_t< ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection> , OReportHelper> getMemberFunction(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XSection >& _xSection);
    };

    class REPORTDESIGN_DLLPUBLIC UndoManagerListAction
    {
    private:
        SfxUndoManager& m_rManager;

    public:
        UndoManagerListAction( SfxUndoManager& _rManager, const String& _rListActionComment )
            :m_rManager( _rManager )
        {
            m_rManager.EnterListAction( _rListActionComment, String() );
        }
        ~UndoManagerListAction()
        {
            m_rManager.LeaveListAction();
        }
    };

    class REPORTDESIGN_DLLPUBLIC OCommentUndoAction : public SdrUndoAction
    {
    protected:
        String                  m_strComment; // undo, redo comment
        ::dbaui::IController*   m_pController;

    public:
        TYPEINFO();
        OCommentUndoAction( SdrModel& rMod
                            ,USHORT nCommentID);
        virtual ~OCommentUndoAction();

        virtual UniString   GetComment() const { return m_strComment; }
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
                            ,USHORT _nCommentId);
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
                            ,USHORT _nCommentId);

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
                            ,USHORT _nCommentId);

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

        virtual String          GetComment() const;
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

