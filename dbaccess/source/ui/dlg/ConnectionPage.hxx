/*************************************************************************
 *
 *  $RCSfile: ConnectionPage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:40:29 $
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
 *  Contributor(s): Ocke Janssen
 *
 *
 ************************************************************************/

#ifndef DBAUI_CONNECTIONPAGE_HXX
#define DBAUI_CONNECTIONPAGE_HXX

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _DBAUI_CURLEDIT_HXX_
#include "curledit.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

    // #106016# --------------
    enum IS_PATH_EXIST
    {
        PATH_NOT_EXIST = 0,
        PATH_EXIST,
        PATH_NOT_KNOWN
    };

    class IAdminHelper;
    //=========================================================================
    //= OConnectionTabPage
    //=========================================================================

    /** implements the connection page of teh data source properties dialog.
    */
    class OConnectionTabPage : public OGenericAdministrationPage
    {
        ODsnTypeCollection* m_pCollection;  /// the DSN type collection instance
        sal_Bool            m_bUserGrabFocus : 1;
    protected:
        // connection
        FixedLine           m_aFL1;
        FixedText*          m_pConnectionLabel;
        OConnectionURLEdit  m_aConnection;
        PushButton          m_aBrowseConnection;

        // user authentification
        FixedLine           m_aFL2;
        FixedText           m_aUserNameLabel;
        Edit                m_aUserName;
        CheckBox            m_aPasswordRequired;

        // jdbc driver
        FixedLine           m_aFL3;
        FixedText           m_aJavaDriverLabel;
        Edit                m_aJavaDriver;
        PushButton          m_aTestJavaDriver;

        // connection test
        PushButton          m_aTestConnection;

        DATASOURCE_TYPE     m_eType; // the type can't be changed in this class, so we hold it as member.

        // called when the test connection button was clicked
        DECL_LINK(OnTestConnectionClickHdl,PushButton*);
        DECL_LINK(OnBrowseConnections, PushButton*);
        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual BOOL        FillItemSet (SfxItemSet& _rCoreAttrs);
        virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        virtual void SetServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        {
            OGenericAdministrationPage::SetServiceFactory(_rxORB);
            m_aConnection.initializeTypeCollection(m_xORB);
        }

        inline void enableConnectionURL() { m_aConnection.SetReadOnly(sal_False); }
        inline void disableConnectionURL() { m_aConnection.SetReadOnly(); }

        /** changes the connection URL.
            <p>The new URL must be of the type which is currently selected, only the parts which do not
            affect the type may be changed (compared to the previous URL).</p>
        */
        void    changeConnectionURL( const String& _rNewDSN );
        String  getConnectionURL( ) const;
    protected:
        OConnectionTabPage(Window* pParent, const SfxItemSet& _rCoreAttrs);
            // nControlFlags ist eine Kombination der CBTP_xxx-Konstanten
        virtual ~OConnectionTabPage();

        virtual long PreNotify( NotifyEvent& _rNEvt );

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
    private:
        // setting/retrieving the current connection URL
        // necessary because for some types, the URL must be decoded for display purposes
        String      getURL( ) const;
        void        setURL( const String& _rURL );
        String      getURLNoPrefix( ) const;
        void        setURLNoPrefix( const String& _rURL );

        String      implGetURL( sal_Bool _bPrefix ) const;
        void        implSetURL( const String& _rURL, sal_Bool _bPrefix );

        /** checks if the path is existence
            @param  _rURL
                The URL to check.
        */
        sal_Int32       checkPathExistence(const String& _rURL);

        StringBag getInstalledAdabasDBDirs(const String &_rPath,const ::ucb::ResultSetInclude& _reResultSetInclude);
        StringBag getInstalledAdabasDBs(const String &_rConfigDir,const String &_rWorkDir);

        // #106016# ----------------
        IS_PATH_EXIST   pathExists(const ::rtl::OUString& _rURL, sal_Bool bIsFile) const;
        sal_Bool        createDirectoryDeep(const String& _rPathNormalized);
        sal_Bool        commitURL();


        /** enables the test connection button, if allowed
        */
        void checkTestConnection();


        /** opens the FileOpen dialog and asks for a FileName
            @param  _sFilterName
                The filter name.
            @param  _sExtension
                The filter extension.
        */
        void askForFileName(const ::rtl::OUString& _sFilterName, const ::rtl::OUString& _sExtension);
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DETAILPAGES_HXX_
