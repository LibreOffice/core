/*************************************************************************
 *
 *  $RCSfile: unoadmin.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-25 12:49:14 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_UNOADMIN_
#define _DBAUI_UNOADMIN_

#ifndef _DBAUI_GENERICUNODIALOG_HXX_
#include "genericunodialog.hxx"
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif

class SfxItemSet;
class SfxItemPool;
class SfxPoolItem;

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
typedef OGenericUnoDialog ODatabaseAdministrationDialogBase;
class ODatabaseAdministrationDialog
        :public ODatabaseAdministrationDialogBase
        ,public ::comphelper::OPropertyArrayUsageHelper< ODatabaseAdministrationDialog >
        ,public OModuleClient
{
protected:
    SfxItemSet*             m_pDatasourceItems;     // item set for the dialog
    SfxItemPool*            m_pItemPool;            // item pool for the item set for the dialog
    SfxPoolItem**           m_pItemPoolDefaults;    // pool defaults
    ODsnTypeCollection      m_aCollection;          // datasource type collection

protected:
    ODatabaseAdministrationDialog(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);
    ~ODatabaseAdministrationDialog();

public:
    // XTypeProvider
    virtual staruno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(staruno::RuntimeException);

    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(staruno::RuntimeException);
    virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(staruno::RuntimeException);

    // XServiceInfo - static methods
    static staruno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( staruno::RuntimeException );
    static ::rtl::OUString getImplementationName_Static(void) throw( staruno::RuntimeException );
    static staruno::Reference< staruno::XInterface >
            SAL_CALL Create(const staruno::Reference< starlang::XMultiServiceFactory >&);

    // XPropertySet
    virtual staruno::Reference<starbeans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(staruno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // OPropertyArrayUsageHelper
    virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

protected:
// OGenericUnoDialog overridables
    virtual Dialog* createDialog(Window* _pParent);
    virtual void destroyDialog();
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_UNOADMIN_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2  2000/10/11 11:31:03  fs
 *  new implementations - still under construction
 *
 *  Revision 1.1  2000/10/05 10:07:32  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 20.09.00 12:15:14  fs
 ************************************************************************/

