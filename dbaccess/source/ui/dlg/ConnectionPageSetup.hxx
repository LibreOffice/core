/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ConnectionPageSetup.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-06 18:15:41 $
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


        virtual void SetServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        {
            OConnectionHelper::SetServiceFactory(_rxORB);
//          m_aET_Connection.initializeTypeCollection(m_xORB);
        }


        virtual BOOL        FillItemSet (SfxItemSet& _rCoreAttrs);

        virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual sal_Bool    commitPage( CommitPageReason _eReason );


        inline void enableConnectionURL() { m_aET_Connection.SetReadOnly(sal_False); }
        inline void disableConnectionURL() { m_aET_Connection.SetReadOnly(); }

        /** changes the connection URL.
            <p>The new URL must be of the type which is currently selected, only the parts which do not
            affect the type may be changed (compared to the previous URL).</p>
        */
        void    changeConnectionURL( const String& _rNewDSN );
        String  getConnectionURL( ) const;


    protected:
        OConnectionTabPageSetup(Window* pParent, USHORT _rId, const SfxItemSet& _rCoreAttrs, USHORT _nHelpTextResId, USHORT _nHeaderResId, USHORT _nUrlResId);
        virtual bool checkTestConnection();
            // nControlFlags ist eine Kombination der CBTP_xxx-Konstanten
        virtual ~OConnectionTabPageSetup();
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif
