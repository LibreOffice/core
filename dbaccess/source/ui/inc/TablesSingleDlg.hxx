/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TablesSingleDlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:39:31 $
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

