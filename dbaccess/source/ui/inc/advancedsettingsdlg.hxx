/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: advancedsettingsdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-01 15:20:15 $
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

#ifndef DBACCESS_ADVANCEDSETTINGSDLG_HXX
#define DBACCESS_ADVANCEDSETTINGSDLG_HXX

#include "IItemSetHelper.hxx"
#include "moduledbu.hxx"

 /** === begin UNO includes === **/
/** === end UNO includes === **/

#include <sfx2/tabdlg.hxx>

#include <memory>

//........................................................................
namespace dbaui
{
//........................................................................

    //=========================================================================
    //= AdvancedSettingsDialog
    //=========================================================================
    class ODbDataSourceAdministrationHelper;
    /** implements the advanced page dlg of the data source properties.
    */
    class AdvancedSettingsDialog    :public SfxTabDialog
                                    ,public IItemSetHelper
                                    ,public IDatabaseSettingsDialog
    {
        OModuleClient                                       m_aModuleClient;
        ::std::auto_ptr<ODbDataSourceAdministrationHelper>  m_pImpl;
        SfxItemSet*                                         m_pItemSet;

    protected:
        virtual void PageCreated(USHORT _nId, SfxTabPage& _rPage);

    public:
        AdvancedSettingsDialog( Window* _pParent
                            ,SfxItemSet* _pItems
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
                            ,const ::com::sun::star::uno::Any& _aDataSourceName);

        virtual ~AdvancedSettingsDialog();

        /// determines whether or not the given data source type has any advanced setting
        static  bool    doesHaveAnyAdvancedSettings( DATASOURCE_TYPE _eType );

        virtual const SfxItemSet* getOutputSet() const;
        virtual SfxItemSet* getWriteOutputSet();

        virtual short   Execute();

        // forwards to ODbDataSourceAdministrationHelper
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const;
        virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > getDriver();
        virtual DATASOURCE_TYPE getDatasourceType(const SfxItemSet& _rSet) const;
        virtual void clearPassword();
        virtual sal_Bool saveDatasource();
        virtual void setTitle(const ::rtl::OUString& _sTitle);
        virtual void enableConfirmSettings( bool _bEnable );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_ADVANCEDSETTINGSDLG_HXX
