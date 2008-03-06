/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: abspilot.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:36:08 $
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

#ifndef EXTENSIONS_ABSPILOT_HXX
#define EXTENSIONS_ABSPILOT_HXX

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef SVTOOLS_INC_ROADMAPWIZARD_HXX
#include <svtools/roadmapwizard.hxx>
#endif
#ifndef EXTENSIONS_ABP_ADDRESSSETTINGS_HXX
#include "addresssettings.hxx"
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef EXTENSIONS_ABP_DATASOURCEHANDLING_HXX
#include "datasourcehandling.hxx"
#endif

//.........................................................................
namespace abp
{
//.........................................................................

    //=====================================================================
    //= OAddessBookSourcePilot
    //=====================================================================
    typedef ::svt::RoadmapWizard OAddessBookSourcePilot_Base;
    class OAddessBookSourcePilot : public OAddessBookSourcePilot_Base
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                m_xORB;
        AddressSettings         m_aSettings;

        ODataSource             m_aNewDataSource;
        AddressSourceType       m_eNewDataSourceType;

    public:
        /// ctor
        OAddessBookSourcePilot(
            Window* _pParent,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

        /// get the service factory which was used to create the dialog
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                getORB() { return m_xORB; }
        AddressSettings&        getSettings() { return m_aSettings; }
        const AddressSettings&  getSettings() const { return m_aSettings; }

        const ODataSource&      getDataSource() const { return m_aNewDataSource; }

        sal_Bool                connectToDataSource( sal_Bool _bForceReConnect );

        void                    travelNext( ) { OAddessBookSourcePilot_Base::travelNext(); }

        /// to be called when the selected type changed
        void                    typeSelectionChanged( AddressSourceType _eType );

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage* createPage( WizardState _nState );
        virtual void                enterState( WizardState _nState );
        virtual sal_Bool            prepareLeaveCurrentState( CommitPageReason _eReason );
        virtual sal_Bool            onFinish( sal_Int32 _nResult );

        // RoadmapWizard
        virtual String              getStateDisplayName( WizardState _nState ) const;

        virtual BOOL    Close();

    private:
        DECL_LINK( OnCancelClicked, void* );

        /** creates a new data source of the type indicated by m_aSettings
            <p>If another data source has been created before, this one is deleted.</p>
        */
        void implCreateDataSource();

        /// does an automatic field mapping (possible for all types except AST_OTHER)
        void implDoAutoFieldMapping();

        /// guesses a default for the table name, if no valid table is selected
        void implDefaultTableName();

        inline sal_Bool needAdminInvokationPage( AddressSourceType _eType ) const
        {
            return  (   ( AST_LDAP == _eType )
                    ||  ( AST_OTHER == _eType )
                    );
        }
        /// check if with the current settings, we would need to invoke he administration dialog for more details about the data source
        inline sal_Bool needAdminInvokationPage() const
        {
            return  needAdminInvokationPage( m_aSettings.eType );
        }

        inline sal_Bool needManualFieldMapping( AddressSourceType _eType ) const
        {
            return ( AST_OTHER == _eType ) || ( AST_KAB == _eType );
        }
        /// checks if we need a manual (user-guided) field mapping
        inline sal_Bool needManualFieldMapping() const
        {
            return needManualFieldMapping( m_aSettings.eType );
        }

        /// determines whether the given address book type does provide one table only
        inline bool needTableSelection( AddressSourceType _eType ) const
        {
            return  ( AST_LDAP != _eType ) && ( AST_KAB != _eType ) && ( AST_EVOLUTION != _eType )
                &&  ( AST_EVOLUTION_GROUPWISE != _eType ) && ( AST_EVOLUTION_LDAP != _eType );
        }

        inline bool needTableSelection() const
        {
            return needTableSelection( m_aSettings.eType );
        }

        void implCleanup();
        void implCommitAll();

        void implUpdateRoadmap( AddressSourceType _eType );
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABSPILOT_HXX

