/*************************************************************************
 *
 *  $RCSfile: JoinController.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:53:14 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef DBAUI_JOINCONTROLLER_HXX
#define DBAUI_JOINCONTROLLER_HXX

#ifndef DBAUI_SINGLEDOCCONTROLLER_HXX
#include "singledoccontroller.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOBJECTINPUTSTREAM_HPP_
#include <com/sun/star/io/XObjectInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef DBAUI_JOINTABLEVIEW_HXX
#include "JoinTableView.hxx"
#endif
#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif
#ifndef _MEMORY_
#include <memory>
#endif

class VCLXWindow;
namespace dbaui
{
    class OTableConnectionData;
    class OTableWindowData;
    class OAddTableDlg;
    class OTableWindow;
    typedef OSingleDocumentController OJoinController_BASE;
    class OJoinController : public OJoinController_BASE
    {
    protected:
        ::std::vector<OTableConnectionData*>    m_vTableConnectionData;
        ::std::vector<OTableWindowData*>        m_vTableData;

        Fraction                                m_aZoom;
        ::dbtools::SQLExceptionInfo             m_aExceptionInfo;

        OAddTableDlg*   m_pAddTabDlg;       // is set by the first call of execute, the owner is the design view

        sal_Bool        m_bViewsAllowed;    // true when the Add Table dialog should also show views

        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId);

        /** loads the information for the windows.
            @param  _aViewProps
                The properties which comes from the layout information.
        */
        void loadTableWindows(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& _aViewProps);

        /** loads the information for one window.
            @param  _rTable
                The properties which comes from the layout information.
        */
        void loadTableWindow(const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& _rTable);

        /** saves the TableWindows structure in a sequence of property values
            @param  _rViewProps
                Contains the new sequence.
        */
        void saveTableWindows(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& _rViewProps);

        virtual ~OJoinController();
    public:
        OJoinController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        virtual void setModified(sal_Bool _bModified=sal_True);

        /** erase the data in the data vector
            @param  _pData
                    the data whioch should be erased
        */
        void removeConnectionData(OTableConnectionData* _pData);

        ::std::vector< OTableWindowData*>*      getTableWindowData()        { return &m_vTableData; }
        ::std::vector< OTableConnectionData*>*  getTableConnectionData()    { return &m_vTableConnectionData;}

        void SaveTabWinsPosSize( OJoinTableView::OTableWindowMap* pTabWinList, long nOffsetX, long nOffsetY );

        // should the statement be parsed by our own sql parser
        inline sal_Bool     isViewAllowed()         const { return m_bViewsAllowed; }

        void            SaveTabWinPosSize(OTableWindow* pTabWin, long nOffsetX, long nOffsetY);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XComponent
        virtual void    SAL_CALL disposing();
        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );


        /**
            only defines a method to save a SQLException in d&d methods to show the error at a later state
            set the internal member m_aExceptionInfo to _rInfo
        */
        void setErrorOccured(const ::dbtools::SQLExceptionInfo& _rInfo)
        {
            m_aExceptionInfo = _rInfo;
        }
        /**
            just returns the internal member and clears it
        */
        ::dbtools::SQLExceptionInfo clearOccuredError()
        {
            ::dbtools::SQLExceptionInfo aInfo = m_aExceptionInfo;
            m_aExceptionInfo = ::dbtools::SQLExceptionInfo();
            return aInfo;
        }
    protected:
        virtual OTableWindowData* createTableWindowData() = 0;
        // ask the user if the design should be saved when it is modified
        virtual short saveModified() = 0;
        // called when the orignal state should be reseted (first time load)
        virtual void reset()         = 0;
        virtual void AddSupportedFeatures();

        virtual OJoinDesignView*    getJoinView();
    };
}
#endif // DBAUI_JOINCONTROLLER_HXX

