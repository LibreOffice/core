/*************************************************************************
 *
 *  $RCSfile: abspilot.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:00:44 $
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

#ifndef EXTENSIONS_ABSPILOT_HXX
#define EXTENSIONS_ABSPILOT_HXX

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
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
    typedef ::svt::OWizardMachine OAddessBookSourcePilot_Base;
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

    protected:
        // OWizardMachine overridables
        virtual ::svt::OWizardPage* createPage(sal_uInt16 _nState);
        virtual void                enterState(sal_uInt16 _nState);
        virtual sal_Bool            leaveState(sal_uInt16 _nState);
        virtual sal_uInt16          determineNextState(sal_uInt16 _nCurrentState);
        virtual sal_Bool            onFinish(sal_Int32 _nResult);

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

        /// check if with the current settings, we would need to invoke he administration dialog for more details about the data source
        inline sal_Bool needAdminInvokationPage() const
        {
            return  (   (AST_LDAP == m_aSettings.eType)
                    ||  (AST_OTHER == m_aSettings.eType)
                    );
        }

        /// checks if we need a manual (user-guided) field mapping
        inline sal_Bool needManualFieldMapping() const
        {
            return ( AST_OTHER == m_aSettings.eType );
        }

        void implCleanup();
        void implCommitAll();
    };

//.........................................................................
}   // namespace abp
//.........................................................................

#endif // EXTENSIONS_ABSPILOT_HXX

