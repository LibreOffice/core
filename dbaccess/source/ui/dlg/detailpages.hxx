/*************************************************************************
 *
 *  $RCSfile: detailpages.hxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:45:39 $
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
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
    //=========================================================================
    //= OCommonBehaviourTabPage
    //=========================================================================
    #define     CBTP_NONE                           0x00000000
    #define     CBTP_USE_APPENDTABLEALIAS           0x00000001
    #define     CBTP_USE_CHARSET                    0x00000002
    #define     CBTP_USE_OPTIONS                    0x00000004
    #define     CBTP_USE_SQL92CHECK                 0x00000010
    #define     CBTP_USE_AUTOINCREMENT              0x00000020
    #define     CBTP_USE_PARAMETERNAMESUBST         0x00000040
    #define     CBTP_USE_IGNOREDRIVER_PRIV          0x00000100
    #define     CBTP_USE_SUPPRESS_VERSION_COLUMN    0x00000200
    #define     CBTP_USE_BOOLEANCOMPARISON          0x00000400
    #define     CBTP_USE_ENABLEOUTERJOIN            0x00001000
    #define     CBTP_USE_CATALOG                    0x00002000
    #define     CBTP_USE_SCHEMA                     0x00004000
    #define     CBTP_USE_INDEXAPPENDIX              0x00010000
    #define     CBTP_USE_DOSLINEENDS                0x00020000

    /** eases the implementation of tab pages handling user/password and/or character
        set and/or generic options input
        <BR>
        The controls to be used habe to be defined within the resource, as usual, but
        this class does all the handling necessary.
    */
    class OCommonBehaviourTabPage : public OGenericAdministrationPage
    {
    protected:

        FixedText*          m_pOptionsLabel;
        Edit*               m_pOptions;

        FixedLine*          m_pDataConvertFixedLine;
        FixedText*          m_pCharsetLabel;
        ListBox*            m_pCharset;

        FixedLine*          m_pDSFixedLine;
        CheckBox*           m_pIsSQL92Check;
        CheckBox*           m_pAppendTableAlias;
        CheckBox*           m_pParameterSubstitution;
        CheckBox*           m_pIgnoreDriverPrivileges;
        CheckBox*           m_pSuppressVersionColumn;
        CheckBox*           m_pEnableOuterJoin;
        CheckBox*           m_pCatalog;
        CheckBox*           m_pSchema;
        CheckBox*           m_pIndexAppendix;
        CheckBox*           m_pDosLineEnds;

        FixedText*          m_pBooleanComprisonModeLabel;
        ListBox*            m_pBooleanComprisonMode;

        FixedLine*          m_pAutoFixedLine;
        CheckBox*           m_pAutoRetrievingEnabled;
        FixedText*          m_pAutoIncrementLabel;
        Edit*               m_pAutoIncrement;
        FixedText*          m_pAutoRetrievingLabel;
        Edit*               m_pAutoRetrieving;

        OCharsetDisplay     m_aCharsets;

        sal_uInt32          m_nControlFlags;

        DECL_LINK( OnCheckBoxClick, CheckBox * );

    public:
        virtual BOOL        FillItemSet (SfxItemSet& _rCoreAttrs);

        OCommonBehaviourTabPage(Window* pParent, USHORT nResId, const SfxItemSet& _rCoreAttrs, sal_uInt32 nControlFlags,bool _bFreeResource = true);
    protected:

            // nControlFlags ist eine Kombination der CBTP_xxx-Konstanten
        virtual ~OCommonBehaviourTabPage();

        // must be overloaded by subclasses, but it isn't pure virtual
        virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

        // <method>OGenericAdministrationPage::fillControls</method>
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);

        // <method>OGenericAdministrationPage::fillWindows</method>
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
    private:
        /// creates the fixed line before the autoincrement controls
        void createBehaviourFixedLine();
    };

    //========================================================================
    //= ODbaseDetailsPage
    //========================================================================
    class ODbaseDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        ODbaseDetailsPage(Window* pParent, const SfxItemSet& _rCoreAttrs);
    private:
        // please add new controls also to <method>fillControls</method> or <method>fillWindows</method>
        CheckBox            m_aShowDeleted;
        FixedLine           m_aFL_1;
        FixedText           m_aFT_Message;
        PushButton          m_aIndexes;

        String              m_sDsn;

    protected:

        virtual ~ODbaseDetailsPage();

    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    private:
        DECL_LINK( OnButtonClicked, Button * );
    };

    //========================================================================
    //= OAdoDetailsPage
    //========================================================================
    class OAdoDetailsPage : public OCommonBehaviourTabPage
    {
    protected:
        virtual ~OAdoDetailsPage();
    public:

        OAdoDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    //========================================================================
    //= OOdbcDetailsPage
    //========================================================================
    class OOdbcDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        OOdbcDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
    private:
        FixedLine           m_aFL_1;
        CheckBox            m_aUseCatalog;
    };


    //========================================================================
    //= OUserDriverDetailsPage
    //========================================================================
    class OUserDriverDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        OUserDriverDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
    private:
        FixedText           m_aFTHostname;
        Edit                m_aEDHostname;
        FixedText           m_aPortNumber;
        NumericField        m_aNFPortNumber;
        FixedLine           m_aSeparator2;
        CheckBox            m_aUseCatalog;
    };

    //========================================================================
    //= OMySQLODBCDetailsPage
    //========================================================================
    class OMySQLODBCDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        OMySQLODBCDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    //========================================================================
    //= OGeneralSpecialJDBCDetailsPage
    //========================================================================
    class OGeneralSpecialJDBCDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        OGeneralSpecialJDBCDetailsPage(   Window* pParent
                                        , USHORT _nResId
                                        , const SfxItemSet& _rCoreAttrs
                                        , USHORT _nPortId
                                        , char* _pDriverName);

    protected:


        virtual BOOL FillItemSet( SfxItemSet& _rCoreAttrs );
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

        DECL_LINK(OnTestJavaClickHdl,PushButton*);
        DECL_LINK(OnEditModified,Edit*);

        FixedLine           m_aFL_1;
        FixedText           m_aFTHostname;
        Edit                m_aEDHostname;
        FixedText           m_aPortNumber;
        NumericField        m_aNFPortNumber;

        FixedText           m_aFTDriverClass;
        Edit                m_aEDDriverClass;
        PushButton          m_aTestJavaDriver;

        String              m_sDefaultJdbcDriverName;
        USHORT              m_nPortId;
    };

    //========================================================================
    //= OAdabasDetailsPage
    //========================================================================
    class OAdabasDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual BOOL        FillItemSet (SfxItemSet& _rCoreAttrs);

        OAdabasDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    private:
        FixedText           m_aFTHostname;
        Edit                m_aEDHostname;
        FixedLine           m_aFL_1;
        FixedText           m_FT_CACHE_SIZE;
        NumericField        m_NF_CACHE_SIZE;

        FixedText           m_FT_DATA_INCREMENT;
        NumericField        m_NF_DATA_INCREMENT;

        FixedLine           m_aFL_2;
        FixedText           m_FT_CTRLUSERNAME;
        Edit                m_ET_CTRLUSERNAME;
        FixedText           m_FT_CTRLPASSWORD;
        Edit                m_ET_CTRLPASSWORD;

        CheckBox            m_CB_SHUTDB;
        PushButton          m_PB_STAT;
        String              m_sUser;
        BOOL                bAttrsChanged;

        DECL_LINK( AttributesChangedHdl,    void * );
        DECL_LINK( UserSettingsHdl,         void * );
        DECL_LINK( LoseFocusHdl,            Edit * );
        DECL_LINK( PBClickHdl,              Button *);
    };

    //========================================================================
    //= OOdbcDetailsPage
    //========================================================================
    class OLDAPDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        OLDAPDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    protected:
        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);
    private:
        FixedLine           m_aFL_1;
        FixedText           m_aBaseDN;
        Edit                m_aETBaseDN;
        CheckBox            m_aCBUseSSL;
        FixedText           m_aPortNumber;
        NumericField        m_aNFPortNumber;
        FixedText           m_aFTRowCount;
        NumericField        m_aNFRowCount;

        sal_Int32           m_iSSLPort;
        sal_Int32           m_iNormalPort;
        DECL_LINK( OnCheckBoxClick, CheckBox * );
    };

    //========================================================================
    //= OTextDetailsPage
    //========================================================================
    class OTextDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
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

        String      m_aFieldSeparatorList;
        String      m_aTextSeparatorList;
        String      m_aTextNone;
    protected:
        virtual ~OTextDetailsPage();

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    private:
        String      GetSeparator( const ComboBox& rBox, const String& rList );
        void        SetSeparator( ComboBox& rBox, const String& rList, const String& rVal );

        virtual sal_Bool checkItems();
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DETAILPAGES_HXX_
