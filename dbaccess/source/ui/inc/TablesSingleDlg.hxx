/*************************************************************************
 *
 *  $RCSfile: TablesSingleDlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:54:13 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

#ifndef _DBAUI_TABLESSINGLEDLG_HXX_
#define _DBAUI_TABLESSINGLEDLG_HXX_

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif
#ifndef DBAUI_ITEMSETHELPER_HXX
#include "IItemSetHelper.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#include <memory>

FORWARD_DECLARE_INTERFACE(beans,XPropertySet)
FORWARD_DECLARE_INTERFACE(sdbc,XConnection)
FORWARD_DECLARE_INTERFACE(lang,XMultiServiceFactory)

//.........................................................................
namespace dbaui
{
//.........................................................................
class ODbDataSourceAdministrationHelper;
    //========================================================================
    //= OTableSubscriptionDialog
    //========================================================================
    class OTableSubscriptionDialog : public SfxSingleTabDialog, public IItemSetHelper
    {
        ::std::auto_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
        sal_Bool m_bStopExecution; // set when the dialog should not be executed

        DECL_LINK( OKClickHdl, OKButton* );
        SfxItemSet*             m_pOutSet;
    public:

        OTableSubscriptionDialog(Window* pParent
            ,SfxItemSet* _pItems
            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
            ,const ::com::sun::star::uno::Any& _aDataSourceName
        );
        virtual ~OTableSubscriptionDialog();

        // forwards from ODbDataSourceAdministrationHelper
        void        successfullyConnected();
        sal_Bool    getCurrentSettings(::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& _rDriverParams);
        void        clearPassword();
        String      getConnectionURL() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >   getCurrentDataSource();
        inline void endExecution() { m_bStopExecution = sal_True; }

        virtual const SfxItemSet* getOutputSet() const;
        virtual SfxItemSet* getWriteOutputSet();

        virtual short   Execute();
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_TABLESSINGLEDLG_HXX_

