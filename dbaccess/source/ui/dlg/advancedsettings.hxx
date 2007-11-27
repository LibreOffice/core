/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: advancedsettings.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-27 12:10:30 $
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

#ifndef DBACCESS_ADVANCEDSETTINGS_HXX
#define DBACCESS_ADVANCEDSETTINGS_HXX

#include "adminpages.hxx"
#include "dsmeta.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <svtools/dialogcontrolling.hxx>

#include <vcl/lstbox.hxx>

#include <vector>

//........................................................................
namespace dbaui
{
//........................................................................

    struct BooleanSettingDesc;
    typedef ::std::vector< BooleanSettingDesc > BooleanSettingDescs;

    //====================================================================
    //= SpecialSettingsPage
    //====================================================================
    /** implements the "Special Settings" page of the advanced database settings
    */
    class SpecialSettingsPage : public OGenericAdministrationPage
    {
    protected:
        FixedLine           m_aTopLine;
        CheckBox*           m_pIsSQL92Check;
        CheckBox*           m_pAppendTableAlias;
        CheckBox*           m_pAsBeforeCorrelationName;
        CheckBox*           m_pEnableOuterJoin;
        CheckBox*           m_pIgnoreDriverPrivileges;
        CheckBox*           m_pParameterSubstitution;
        CheckBox*           m_pSuppressVersionColumn;
        CheckBox*           m_pCatalog;
        CheckBox*           m_pSchema;
        CheckBox*           m_pIndexAppendix;
        CheckBox*           m_pDosLineEnds;
        CheckBox*           m_pCheckRequiredFields;
        CheckBox*           m_pIgnoreCurrency;

        FixedText*          m_pBooleanComparisonModeLabel;
        ListBox*            m_pBooleanComparisonMode;

        ::svt::ControlDependencyManager
                            m_aControlDependencies;

        BooleanSettingDescs m_aBooleanSettings;

        AdvancedSettingsSupport
                            m_aSupported;

    public:
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        SpecialSettingsPage(Window* pParent, const SfxItemSet& _rCoreAttrs, const DataSourceMetaData& _rDSMeta );

    protected:
        virtual ~SpecialSettingsPage();

        // OGenericAdministrationPage overridables
        virtual void implInitControls (const SfxItemSet& _rSet, sal_Bool _bSaveValue );

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    private:
        void    impl_initBooleanSettings();
    };

    //====================================================================
    //= GeneratedValuesPage
    //====================================================================
    class GeneratedValuesPage : public OGenericAdministrationPage
    {
    protected:

        FixedLine   m_aAutoFixedLine;
        CheckBox    m_aAutoRetrievingEnabled;
        FixedText   m_aAutoIncrementLabel;
        Edit        m_aAutoIncrement;
        FixedText   m_aAutoRetrievingLabel;
        Edit        m_aAutoRetrieving;

        ::svt::ControlDependencyManager
                    m_aControlDependencies;

    public:
        virtual BOOL        FillItemSet (SfxItemSet& _rCoreAttrs);

        GeneratedValuesPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:

            // nControlFlags ist eine Kombination der CBTP_xxx-Konstanten
        virtual ~GeneratedValuesPage();

        // must be overloaded by subclasses, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    private:
        DECL_LINK( OnCheckBoxClick, CheckBox * );
    };

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_ADVANCEDSETTINGS_HXX
