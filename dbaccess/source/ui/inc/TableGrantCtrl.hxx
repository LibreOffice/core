/*************************************************************************
 *
 *  $RCSfile: TableGrantCtrl.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-06-20 06:59:32 $
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
#ifndef DBAUI_TABLEGRANTCONTROL_HXX
#define DBAUI_TABLEGRANTCONTROL_HXX

#ifndef _SVX_DBBROWSE_HXX
#include <svx/dbbrowse.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif

class Edit;
namespace dbaui
{

class OTableGrantControl : public DbBrowseBox
{
    DECLARE_STL_USTRINGACCESS_MAP(sal_Int32,TTablePrivilegeMap);

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xUsers;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xTables;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xORB;
    ::com::sun::star::uno::Sequence< ::rtl::OUString>                               m_aTableNames;

    TTablePrivilegeMap  m_aPrivMap;
    ::rtl::OUString     m_sUserName;
    DbCheckBoxCtrl*     m_pCheckCell;
    Edit*               m_pEdit;
    long                m_nDataPos;
    BOOL                m_bEnable;

public:
    OTableGrantControl( Window* pParent,const ResId& _RsId);
    virtual ~OTableGrantControl();
    void UpdateTables();
    void setUserName(const ::rtl::OUString _sUserName);

    void setTablesSupplier(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XTablesSupplier >& _xTablesSup);
    void setORB(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _xORB);

    virtual void Init();
protected:
    virtual void Resize();

    virtual long PreParentNotify(NotifyEvent& rNEvt );

    virtual BOOL IsTabAllowed(BOOL bForward) const;
    virtual void InitController( DbCellControllerRef& rController, long nRow, USHORT nCol );
    virtual DbCellController* GetController( long nRow, USHORT nCol );
    virtual void PaintCell( OutputDevice& rDev, const Rectangle& rRect, USHORT nColId ) const;
    virtual BOOL SeekRow( long nRow );
    virtual BOOL SaveModified();
    virtual String GetCellText( long nRow, USHORT nColId );

    virtual void CellModified();

private:
    ULONG       m_nDeActivateEvent;
    DECL_LINK( AsynchActivate, void* );
    DECL_LINK( AsynchDeactivate, void* );

    sal_Bool isAllowed(USHORT _nColumnId,sal_Int32 _nPrivilege) const;
    sal_Int32 fillPrivilege(sal_Int32 _nRow);
};

}

#endif // DBAUI_TABLEGRANTCONTROL_HXX