/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef DBAUI_RELATIONCONTROLLER_HXX
#define DBAUI_RELATIONCONTROLLER_HXX

#include "JoinController.hxx"
#include "RelationDesignView.hxx"

class WaitObject;
namespace dbaui
{
    class ORelationController : public OJoinController
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xTables;
        ::std::auto_ptr<WaitObject> m_pWaitObject;
        sal_uLong       m_nThreadEvent;
        sal_Bool    m_bRelationsPossible;
    protected:
        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        ORelationDesignView*    getRelationView() { return static_cast<ORelationDesignView*>( getView() ); }
        void loadData();
        TTableWindowData::value_type existsTable(const OUString& _rComposedTableName,sal_Bool _bCase) const;

        // load the window positions out of the datasource
        void loadLayoutInformation();
        void loadTableData(const ::com::sun::star::uno::Any& _aTable);
    public:
        ORelationController(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM);

        ~ORelationController();
        // temp
        void SaveTabWinsPosSize( OJoinTableView::OTableWindowMap* pTabWinList, long nOffsetX, long nOffsetY );

        void mergeData(const TTableConnectionData& _aConnectionData);

        virtual sal_Bool Construct(Window* pParent);

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        // OJoinController overridables
        virtual bool allowViews() const;
        virtual bool allowQueries() const;

    private:
        // ask the user if the design should be saved when it is modified
        virtual short saveModified();
        virtual void reset();
        virtual void impl_initialize();
        virtual OUString getPrivateTitle( ) const;
        DECL_LINK( OnThreadFinished, void* );
    };
}
#endif // DBAUI_RELATIONCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
