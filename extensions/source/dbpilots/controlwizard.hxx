/*************************************************************************
 *
 *  $RCSfile: controlwizard.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-03 12:42:48 $
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

#ifndef _EXTENSIONS_DBP_CONTROLWIZARD_HXX
#define _EXTENSIONS_DBP_CONTROLWIZARD_HXX

#ifndef _SVTOOLS_WIZARDMACHINE_HXX_
#include <svtools/wizardmachine.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCONNECTION_HPP_
#include <com/sun/star/sdbc/XConnection.hpp>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _EXTENSIONS_DBP_DBPTYPES_HXX_
#include "dbptypes.hxx"
#endif
#ifndef _EXTENSIONS_DBP_DBPRESID_HRC_
#include "dbpresid.hrc"
#endif
#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#include "componentmodule.hxx"
#endif
#ifndef _EXTENSIONS_DBP_WIZARDCONTEXT_HXX_
#include "wizardcontext.hxx"
#endif

class ResId;
//.........................................................................
namespace dbp
{
//.........................................................................

    //=====================================================================
    //= OControlWizardSettings
    //=====================================================================
    struct OControlWizardSettings
    {
        String      sControlLabel;
    };

    //=====================================================================
    //= OControlWizardPage
    //=====================================================================
    class OControlWizard;
    class OControlWizardPage : public ::svt::OWizardPage
    {
    protected:
        OControlWizard*                 getDialog();
        const OControlWizard*           getDialog() const;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                                        getServiceFactory();
        const OControlWizardContext&    getContext();
        sal_Bool                        updateContext();
        void                            setFormConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                                        getFormConnection() const;

    public:
        OControlWizardPage( OControlWizard* _pParent, const ResId& _rResId );

    protected:
        void fillListBox(
            ListBox& _rList,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rItems,
            sal_Bool _bClear = sal_True);
        void fillListBox(
            ComboBox& _rList,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rItems,
            sal_Bool _bClear = sal_True);
    };

    struct OAccessRegulator;
    //=====================================================================
    //= OControlWizard
    //=====================================================================
    typedef ::svt::OWizardMachine OControlWizard_Base;
    class OControlWizard : public OControlWizard_Base
    {
    private:
        OControlWizardContext   m_aContext;

    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;

    public:
        OControlWizard(
            Window* _pParent,
            const ResId& _rId,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxObjectModel,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB
        );
        ~OControlWizard();

        // make the some base class methods public
        sal_Bool    travelNext() { return OControlWizard_Base::travelNext(); }

    public:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
            getServiceFactory() const { return m_xORB; }

        const OControlWizardContext&    getContext() const { return m_aContext; }
        sal_Bool                        updateContext(const OAccessRegulator&);
        void                            setFormConnection(const OAccessRegulator&, const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConn);
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                                        getFormConnection(const OAccessRegulator&) const;

    protected:
        // initialize the derivees settings (which have to be derived from OControlWizardSettings)
        // with some common data extracted from the control model
        void initControlSettings(OControlWizardSettings* _pSettings);
        // commit the control-relevant settings
        void commitControlSettings(OControlWizardSettings* _pSettings);

        sal_Bool needDatasourceSelection();

        virtual sal_Bool approveControl(sal_Int16 _nClassId) = 0;

        // ModalDialog overridables
        virtual short   Execute();

    private:
        sal_Bool initContext();

        void implGetDSContext();
        void implDetermineForm();
        void implDeterminePage();
        void implDetermineShape();

        // made private. Not to be used by derived (or external) classes
        virtual void ActivatePage();
    };

//.........................................................................
}   // namespace dbp
//.........................................................................

#endif // _EXTENSIONS_DBP_CONTROLWIZARD_HXX

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4  2001/03/05 14:53:13  fs
 *  finished the grid control wizard
 *
 *  Revision 1.3  2001/02/28 09:18:30  fs
 *  finalized the list/combo wizard
 *
 *  Revision 1.2  2001/02/23 15:19:08  fs
 *  some changes / centralizations - added the list-/combobox wizard
 *
 *  Revision 1.1  2001/02/21 09:22:18  fs
 *  initial checkin - form control auto pilots
 *
 *
 *  Revision 1.0 14.02.01 10:02:52  fs
 ************************************************************************/

