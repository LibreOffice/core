/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




#ifndef DBAUI_CONNECTIONPAGESETUP_HXX
#define DBAUI_CONNECTIONPAGESETUP_HXX
#ifndef DBAUI_CONNECTIONHELPER_HXX
#include "ConnectionHelper.hxx"
#endif

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _DBAUI_CURLEDIT_HXX_
#include "curledit.hxx"
#endif

#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif


//.........................................................................
namespace dbaui
{
//.........................................................................
/*  // #106016# --------------
    enum IS_PATH_EXIST
    {
        PATH_NOT_EXIST = 0,
        PATH_EXIST,
        PATH_NOT_KNOWN
    };
    */

    class IDatabaseSettingsDialog;
    //=========================================================================
    //= OConnectionTabPageSetup
    //=========================================================================

    /** implements the connection page of the data source properties dialog.
    */
    class OConnectionTabPageSetup : public OConnectionHelper
    {
        sal_Bool            m_bUserGrabFocus : 1;
    protected:

        FixedText           m_aFT_HelpText;

        // called when the test connection button was clicked
        DECL_LINK(OnEditModified,Edit*);

    public:
        static  OGenericAdministrationPage* CreateDbaseTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
        static  OGenericAdministrationPage* CreateMSAccessTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
        static  OGenericAdministrationPage* CreateAdabasTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
        static  OGenericAdministrationPage* CreateADOTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
        static  OGenericAdministrationPage* CreateODBCTabPage( Window* pParent, const SfxItemSet& _rAttrSet );
        static  OGenericAdministrationPage* CreateUserDefinedTabPage( Window* pParent, const SfxItemSet& _rAttrSet );


        virtual sal_Bool        FillItemSet (SfxItemSet& _rCoreAttrs);

        virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual sal_Bool    commitPage( ::svt::WizardTypes::CommitPageReason _eReason );


        inline void enableConnectionURL() { m_aConnectionURL.SetReadOnly(sal_False); }
        inline void disableConnectionURL() { m_aConnectionURL.SetReadOnly(); }

        /** changes the connection URL.
            <p>The new URL must be of the type which is currently selected, only the parts which do not
            affect the type may be changed (compared to the previous URL).</p>
        */
        void    changeConnectionURL( const String& _rNewDSN );
        String  getConnectionURL( ) const;


    protected:
        OConnectionTabPageSetup(Window* pParent, sal_uInt16 _rId, const SfxItemSet& _rCoreAttrs, sal_uInt16 _nHelpTextResId, sal_uInt16 _nHeaderResId, sal_uInt16 _nUrlResId);
        virtual bool checkTestConnection();
            // nControlFlags ist eine Kombination der CBTP_xxx-Konstanten
        virtual ~OConnectionTabPageSetup();
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif
