/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: detailpages.hxx,v $
 *
 *  $Revision: 1.29 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 11:23:07 $
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

#ifndef _DBAUI_DETAILPAGES_HXX_
#define _DBAUI_DETAILPAGES_HXX_

#ifndef _DBAUI_ADMINPAGES_HXX_
#include "adminpages.hxx"
#endif
#ifndef _DBAUI_CHARSETS_HXX_
#include "charsets.hxx"
#endif
#ifndef CHARSETLISTBOX_HXX
#include "charsetlistbox.hxx"
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
#ifndef DBAUI_TEXTCONNECTIONHELPER_HXX
#include "TextConnectionHelper.hxx"
#endif

#include <svtools/dialogcontrolling.hxx>


//.........................................................................
namespace dbaui
{
//.........................................................................
    //=========================================================================
    //= OCommonBehaviourTabPage
    //=========================================================================
    #define     CBTP_NONE                           0x00000000
    #define     CBTP_USE_CHARSET                    0x00000002
    #define     CBTP_USE_OPTIONS                    0x00000004

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
        CharSetListBox*     m_pCharset;

        FixedLine*          m_pAutoFixedLine;
        CheckBox*           m_pAutoRetrievingEnabled;
        FixedText*          m_pAutoIncrementLabel;
        Edit*               m_pAutoIncrement;
        FixedText*          m_pAutoRetrievingLabel;
        Edit*               m_pAutoRetrieving;

        sal_uInt32          m_nControlFlags;

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
                                        , const char* _pDriverName);

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
        bool                m_bUseClass;
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
    //= OMozillaDetailsPage Detail page for Mozilla and Thunderbird addressbook
    //========================================================================
    class OMozillaDetailsPage : public OCommonBehaviourTabPage
    {
    protected:
        virtual ~OMozillaDetailsPage();
    public:

        OMozillaDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    };

    //========================================================================
    //= OTextDetailsPage
    //========================================================================
    class OTextDetailsPage : public OCommonBehaviourTabPage
    {
    public:
        virtual BOOL        FillItemSet ( SfxItemSet& _rCoreAttrs );

        OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
        OTextConnectionHelper*  m_pTextConnectionHelper;

    private:

        String      m_aFieldSeparatorList;
        String      m_aTextSeparatorList;
        String      m_aTextNone;
    protected:
        virtual ~OTextDetailsPage();
        virtual sal_Bool prepareLeave();

        virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
        virtual void fillControls(::std::vector< ISaveValueWrapper* >& _rControlList);
        virtual void fillWindows(::std::vector< ISaveValueWrapper* >& _rControlList);

    private:
    };

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DETAILPAGES_HXX_
