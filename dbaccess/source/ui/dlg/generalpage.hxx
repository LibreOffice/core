/*************************************************************************
 *
 *  $RCSfile: generalpage.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: fs $ $Date: 2001-08-01 08:30:41 $
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

#ifndef _DBAUI_GENERALPAGE_HXX_
#define _DBAUI_GENERALPAGE_HXX_

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    class ODbAdminDialog;
    //=========================================================================
    //= OGeneralPage
    //=========================================================================
    class OGeneralPage : public OGenericAdministrationPage
    {
        OGeneralPage(Window* pParent, const SfxItemSet& _rItems);

    private:
        // dialog controls
        FixedText           m_aNameLabel;
        Edit                m_aName;
        FixedLine           m_aTypeBox;
        FixedText           m_aDatasourceTypeLabel;
        ListBox             m_aDatasourceType;
        FixedText           m_aConnectionLabel;
        OConnectionURLEdit  m_aConnection;
        PushButton          m_aBrowseConnection;
        PushButton          m_aCreateDatabase;
    //  FixedText           m_aTimeoutLabel;
    //  Edit                m_aTimeoutNumber;
    //  ListBox             m_aTimeoutUnit;

        FixedText           m_aSpecialMessage;

        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                            m_xORB;

        ODsnTypeCollection* m_pCollection;  /// the DSN type collection instance
        DECLARE_STL_MAP(DATASOURCE_TYPE, String, ::std::less< DATASOURCE_TYPE >, SelectionHistory);
        DATASOURCE_TYPE     m_eCurrentSelection;    /// currently selected type
        DATASOURCE_TYPE     m_eNotSupportedKnownType;   /// if a data source of an unsupported, but known type is encountered ....
        SelectionHistory    m_aSelectionHistory;    /// last selected ConnectURLs for all types

        enum SPECIAL_MESSAGE
        {
            smNone,
            smInvalidName,
            smDatasourceDeleted,
            smUnsupportedType
        };
        SPECIAL_MESSAGE     m_eLastMessage;

        Link                m_aTypeSelectHandler;   /// to be called if a new type is selected
        Link                m_aNameModifiedHandler; /// to be called whenever the name of the data source is changed by the user
        ODbAdminDialog*     m_pAdminDialog;         /// we need the servicefactory
        String              m_sControlUser;         /// set by XExecutableDialog
        String              m_sControlPassword;     /// set by XExecutableDialog
        String              m_sUser;                /// set by XExecutableDialog
        String              m_sUserPassword;        /// set by XExecutableDialog
        sal_Int32           m_nCacheSize;           /// set by XExecutableDialog

        sal_Bool            m_bDisplayingInvalid : 1;   // the currently displayed data source is deleted
        sal_Bool            m_bUserGrabFocus : 1;

    public:
        static SfxTabPage*  Create(Window* pParent, const SfxItemSet& _rAttrSet);

        /// set a handler which gets called every time the user selects a new type
        void            SetTypeSelectHandler(const Link& _rHandler) { m_aTypeSelectHandler = _rHandler; }
        /// get the currently selected datasource type
        DATASOURCE_TYPE GetSelectedType() const { return m_eCurrentSelection; }

        /// set a handler which gets called every time the user changes the data source name
        void            SetNameModifyHandler(const Link& _rHandler) { m_aNameModifiedHandler = _rHandler; }
        /// get the current name the user wants the data source to have
        String          GetCurrentName() const { return m_aName.GetText(); }

        // set the parent dialog typesafe
        void SetAdminDialog(ODbAdminDialog* _pDialog) { m_pAdminDialog = _pDialog; }
        void setServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
            { m_xORB = _rxORB; }

        void enableConnectionURL() { m_aConnection.SetReadOnly(sal_False); }
        void disableConnectionURL() { m_aConnection.SetReadOnly(); }

        /** changes the connection URL.
            <p>The new URL must be of the type which is currently selected, only the parts which do not
            affect the type may be changed (compared to the previous URL).</p>
        */
        void    changeConnectionURL( const String& _rNewDSN );
        String  getConnectionURL( ) const;

    protected:
        // SfxTabPage overridables
        virtual BOOL FillItemSet(SfxItemSet& _rCoreAttrs);
        virtual void Reset(const SfxItemSet& _rCoreAttrs);
        virtual void ActivatePage(const SfxItemSet& _rSet);

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual sal_Bool checkItems();

        virtual void GetFocus();
        virtual long PreNotify( NotifyEvent& _rNEvt );

    protected:
        void onTypeSelected(const DATASOURCE_TYPE _eType);
        void initializeHistory();
        void initializeTypeList();

        void implSetCurrentType( const DATASOURCE_TYPE _eType );

        void switchMessage(const SPECIAL_MESSAGE _eType);
        void previousMessage();

        sal_Int32       checkPathExistence(const String& _rURL);
        sal_Bool        commitURL();
        sal_Bool        createDirectoryDeep(const String& _rPathNormalized);
        sal_Bool        directoryExists(const ::rtl::OUString& _rURL) const;
        sal_Bool        fileExists(const ::rtl::OUString& _rURL) const;

        void checkCreateDatabase(DATASOURCE_TYPE _eType);
        sal_Bool isBrowseable(DATASOURCE_TYPE _eType) const;
        StringBag getInstalledAdabasDBDirs(const String &_rPath,const ::ucb::ResultSetInclude& _reResultSetInclude);
        StringBag getInstalledAdabasDBs(const String &_rConfigDir,const String &_rWorkDir);

        DECL_LINK(OnDatasourceTypeSelected, ListBox*);
        DECL_LINK(OnBrowseConnections, PushButton*);
        DECL_LINK(OnCreateDatabase, PushButton*);
        DECL_LINK(OnNameModified, Edit*);
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................
#endif // _DBAUI_GENERALPAGE_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.4  2001/07/31 16:01:33  fs
 *  #88530# changes to operate the dialog in a mode where no type change is possible
 *
 *  Revision 1.3  2001/07/23 13:13:38  oj
 *  #90074# check if calc doc exists
 *
 *  Revision 1.2  2001/05/29 13:33:12  oj
 *  #87149# addressbook ui impl
 *
 *  Revision 1.1  2001/05/29 09:59:32  fs
 *  initial checkin - outsourced the class from commonpages
 *
 *
 *  Revision 1.0 29.05.01 11:31:35  fs
 ************************************************************************/

