/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RelationController.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-04 14:01:45 $
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
#ifndef DBAUI_RELATIONCONTROLLER_HXX
#define DBAUI_RELATIONCONTROLLER_HXX

#ifndef DBAUI_JOINCONTROLLER_HXX
#include "JoinController.hxx"
#endif
#ifndef DBAUI_RELATIONDESIGNVIEW_HXX
#include "RelationDesignView.hxx"
#endif

class VCLXWindow;
namespace dbaui
{
    class OTableConnectionData;
    class OTableWindowData;
    class OAddTableDlg;
    class OTableFieldDesc;
    class OTableWindow;
    class ORelationController : public OJoinController
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    m_xTables;
        sal_Bool    m_bRelationsPossible;
    protected:
        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        ORelationDesignView*    getRelationView() { return static_cast<ORelationDesignView*>(m_pView); }
        void loadData();
        TTableWindowData::value_type existsTable(const ::rtl::OUString& _rComposedTableName) const;

        // load the window positions out of the datasource
        void loadLayoutInformation();
        void loadTableData(const ::com::sun::star::uno::Any& _aTable);
    public:
        ORelationController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        ~ORelationController();
        // temp
        void SaveTabWinsPosSize( OJoinTableView::OTableWindowMap* pTabWinList, long nOffsetX, long nOffsetY );

        virtual sal_Bool Construct(Window* pParent);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
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
        virtual ::rtl::OUString getPrivateTitle( ) const;
    };
}
#endif // DBAUI_RELATIONCONTROLLER_HXX

