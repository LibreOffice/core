/*************************************************************************
 *
 *  $RCSfile: JoinController.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-14 10:35:10 $
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

#ifndef DBAUI_GENERICCONTROLLER_HXX
#include "genericcontroller.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
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
    class OJoinController : public OGenericUnoController
    {
    protected:
        SfxUndoManager  m_aUndoManager;
        ::std::vector<OTableConnectionData*>    m_vTableConnectionData;
        ::std::vector<OTableWindowData*>        m_vTableData;

        Fraction                                m_aZoom;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >         m_xConnection;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xDataSource;

        ::rtl::OUString m_sDataSourceName;  // is set in initialze

        OAddTableDlg*   m_pAddTabDlg;       // is set by the first call of execute, the owner is the design view

        sal_Bool        m_bEditable;        // is the control readonly or not
        sal_Bool        m_bModified;        // is the data modified
        sal_Bool        m_bOwnConnection;   // is true when we created our own connection


        virtual void createNewConnection(sal_Bool _bUI = sal_False);

        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId);
        // execute a feature
        virtual void            Execute(sal_uInt16 nId);

    public:
        OJoinController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        ~OJoinController();
        // removes the connection from the vector and delete it
        void removeConnectionData(::std::auto_ptr<OTableConnectionData> _pData);
        ::std::vector< OTableWindowData*>*      getTableWindowData()        { return &m_vTableData; }
        ::std::vector< OTableConnectionData*>*  getTableConnectionData()    { return &m_vTableConnectionData;}

        void SaveTabWinsPosSize( OJoinTableView::OTableWindowMap* pTabWinList, long nOffsetX, long nOffsetY );
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection > getConnection() { return m_xConnection; }

        // should the statement be parsed by our own sql parser
        sal_Bool        isReadOnly()            const { return !m_bEditable; }
        sal_Bool        isModified()            const { return m_bModified; }

        virtual void    setModified(sal_Bool _bModified=sal_True);

        void            SaveTabWinPosSize(OTableWindow* pTabWin, long nOffsetX, long nOffsetY);

        // need for undo's and redo's
        SfxUndoManager* getUndoMgr();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL disposing();

        //
        virtual void Load(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxIn);
        virtual void Save(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOut);

    protected:
        virtual OTableWindowData* createTableWindowData() = 0;
    };
}
#endif // DBAUI_JOINCONTROLLER_HXX

