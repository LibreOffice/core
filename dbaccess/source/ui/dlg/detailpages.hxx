/*************************************************************************
 *
 *  $RCSfile: detailpages.hxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 17:52:22 $
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

#ifndef _DBAUI_DETAILPAGES_HXX_
#define _DBAUI_DETAILPAGES_HXX_

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/imagebtn.hxx>
#endif


//.........................................................................
namespace dbaui
{
//.........................................................................

    //=========================================================================
    //= OCommonBehaviourTabPage
    //=========================================================================
    #define     CBTP_NONE                   0x0000
    #define     CBTP_USE_UIDPWD             0x0001
    #define     CBTP_USE_CHARSET            0x0002
    #define     CBTP_USE_OPTIONS            0x0004
    #define     CBTP_USE_SQL92CHECK         0x0010
    #define     CBTP_USE_AUTOINCREMENT      0x0020

    /** eases the implementation of tab pages handling user/password and/or character
        set and/or generic options input
        <BR>
        The controls to be used habe to be defined within the resource, as usual, but
        this class does all the handling necessary.
    */
    class OCommonBehaviourTabPage : public OGenericAdministrationPage
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xORB;

        FixedText*          m_pUserNameLabel;
        Edit*               m_pUserName;
        FixedText*          m_pPasswordLabel;
        Edit*               m_pPassword;
        CheckBox*           m_pPasswordRequired;

        FixedText*          m_pOptionsLabel;
        Edit*               m_pOptions;

        FixedText*          m_pCharsetLabel;
        ListBox*            m_pCharset;

        CheckBox*           m_pIsSQL92Check;

        CheckBox*           m_pAutoRetrievingEnabled;
        FixedText*          m_pAutoIncrementLabel;
        Edit*               m_pAutoIncrement;
        FixedText*          m_pAutoRetrievingLabel;
        Edit*               m_pAutoRetrieving;

        OCharsetDisplay     m_aCharsets;

        USHORT              m_nControlFlags;

        DECL_LINK( OnCheckBoxClick, CheckBox * );

    public:
        virtual BOOL        FillItemSet (SfxItemSet& _rCoreAttrs);
        virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        virtual void setServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        {
            m_xORB = _rxORB;
        }

    protected:
        OCommonBehaviourTabPage(Window* pParent, USHORT nResId, const SfxItemSet& _rCoreAttrs, USHORT nControlFlags);
            // nControlFlags ist eine Kombination der CBTP_xxx-Konstanten
        virtual ~OCommonBehaviourTabPage();
    };

    //========================================================================
    //= ODbaseDetailsPage
    //========================================================================
    class ODbaseDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        /// get the SfxPoolItem ids used by this tab page
        static sal_Int32* getDetailIds();

    private:
        FixedLine           m_aLine1;
        FixedLine           m_aLine2;
        CheckBox            m_aShowDeleted;
        PushButton          m_aIndexes;

        String              m_sDsn;

    protected:
        ODbaseDetailsPage(Window* pParent, const SfxItemSet& _rCoreAttrs);
        ~ODbaseDetailsPage();

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

    private:
        DECL_LINK( OnButtonClicked, Button * );
    };

    //========================================================================
    //= OJdbcDetailsPage
    //========================================================================
    class OJdbcDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        virtual void setServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        {
            OCommonBehaviourTabPage::setServiceFactory(_rxORB);
            m_aJdbcUrl.initializeTypeCollection(_rxORB);
        }

        /// get the SfxPoolItem ids used by this tab page
        static sal_Int32* getDetailIds();

    private:
        FixedText           m_aDriverLabel;
        Edit                m_aDriver;
        FixedText           m_aJdbcUrlLabel;
        OConnectionURLEdit  m_aJdbcUrl;
        FixedLine           m_aSeparator1;

        OJdbcDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
        ~OJdbcDetailsPage();

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
    };

    //========================================================================
    //= OAdoDetailsPage
    //========================================================================
    class OAdoDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        virtual void setServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        {
            OCommonBehaviourTabPage::setServiceFactory(_rxORB);
            m_aAdoUrl.initializeTypeCollection(_rxORB);
        }

        /// get the SfxPoolItem ids used by this tab page
        static sal_Int32* getDetailIds();

    private:
        FixedText           m_aAdoUrlLabel;
        OConnectionURLEdit  m_aAdoUrl;
        FixedLine           m_aSeparator1;
        FixedLine           m_aSeparator2;

        OAdoDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
        ~OAdoDetailsPage();

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
    };

    //========================================================================
    //= OOdbcDetailsPage
    //========================================================================
    class OOdbcDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        /// get the SfxPoolItem ids used by this tab page
        static sal_Int32* getDetailIds();

    private:
        FixedLine           m_aSeparator1;
        CheckBox            m_aUseCatalog;

        OOdbcDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
    };


    //========================================================================
    //= OUserDriverDetailsPage
    //========================================================================
    class OUserDriverDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        /// get the SfxPoolItem ids used by this tab page
        static sal_Int32* getDetailIds();

    private:
        FixedLine           m_aSeparator1;
        CheckBox            m_aUseCatalog;

        OUserDriverDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
    };

    //========================================================================
    //= OMySQLDetailsPage
    //========================================================================
    class OMySQLDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        virtual void setServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        {
            OCommonBehaviourTabPage::setServiceFactory(_rxORB);
            m_aUrl.initializeTypeCollection(_rxORB);
        }

        /// get the SfxPoolItem ids used by this tab page
        static sal_Int32* getDetailIds();

    private:
        FixedLine           m_aSeparator1;
        RadioButton         m_aUseODBC;
        RadioButton         m_aUseJDBC;
        FixedText           m_aFTDriverClass;
        Edit                m_aEDDriverClass;
        FixedLine           m_aSeparator2;
        FixedText           m_aUrlLabel;
        OConnectionURLEdit  m_aUrl;
        PushButton          m_aBrowseConnection;
        String              m_sJDBCDefaultUrl;
        String              m_sOldODBCUrl;
        String              m_sOldJDBCUrl;

        OMySQLDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        DECL_LINK( OnToggle, RadioButton * );
        DECL_LINK(OnBrowseConnections, PushButton*);
    };

    //========================================================================
    //= OAdabasDetailsPage
    //========================================================================
    class OAdabasDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );

        /// get the SfxPoolItem ids used by this tab page
        static sal_Int32* getDetailIds();

    private:
        FixedLine           m_aSeparator1;

        OAdabasDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    //========================================================================
    //= OOdbcDetailsPage
    //========================================================================
    class OLDAPDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        /// get the SfxPoolItem ids used by this tab page
        static sal_Int32* getDetailIds();

    private:
        FixedText           m_aHostname;
        Edit                m_aETHostname;
        FixedText           m_aBaseDN;
        Edit                m_aETBaseDN;
        FixedLine           m_aSeparator2;
        FixedText           m_aPortNumber;
        NumericField        m_aNFPortNumber;
        FixedText           m_aFTRowCount;
        NumericField        m_aNFRowCount;

        OLDAPDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
    };

    //========================================================================
    //= OTextDetailsPage
    //========================================================================
    class OTextDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        /// get the SfxPoolItem ids used by this tab page
        static sal_Int32* getDetailIds();

    private:
        FixedLine   m_aLineFormat;
        CheckBox    m_aHeader;
        FixedText   m_aFieldSeparatorLabel;
        ComboBox    m_aFieldSeparator;
        FixedText   m_aTextSeparatorLabel;
        ComboBox    m_aTextSeparator;
        FixedText   m_aDecimalSeparatorLabel;
        ComboBox    m_aDecimalSeparator;
        FixedText   m_aThousandsSeparatorLabel;
        ComboBox    m_aThousandsSeparator;
        FixedLine   m_aSeparator1;
        FixedText   m_aExtensionLabel;
        ComboBox    m_aExtension;
        FixedLine   m_aSeparator3;

        String      m_aFieldSeparatorList;
        String      m_aTextSeparatorList;
        String      m_aTextNone;

        OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
        ~OTextDetailsPage();

    private:
        String      GetSeparator( const ComboBox& rBox, const String& rList );
        void        SetSeparator( ComboBox& rBox, const String& rList, const String& rVal );

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual sal_Bool checkItems();
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DETAILPAGES_HXX_
