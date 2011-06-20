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
        virtual void PageCreated(sal_uInt16 _nId, SfxTabPage& _rPage);

    public:
        AdvancedSettingsDialog( Window* _pParent
                            ,SfxItemSet* _pItems
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
                            ,const ::com::sun::star::uno::Any& _aDataSourceName);

        virtual ~AdvancedSettingsDialog();

        /// determines whether or not the given data source type has any advanced setting
        static  bool    doesHaveAnyAdvancedSettings( const ::rtl::OUString& _sURL );

        virtual const SfxItemSet* getOutputSet() const;
        virtual SfxItemSet* getWriteOutputSet();

        virtual short   Execute();

        // forwards to ODbDataSourceAdministrationHelper
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > getORB() const;
        virtual ::std::pair< ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >,sal_Bool> createConnection();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDriver > getDriver();
        virtual ::rtl::OUString getDatasourceType(const SfxItemSet& _rSet) const;
        virtual void clearPassword();
        virtual sal_Bool saveDatasource();
        virtual void setTitle(const ::rtl::OUString& _sTitle);
        virtual void enableConfirmSettings( bool _bEnable );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_ADVANCEDSETTINGSDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
