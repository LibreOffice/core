/*************************************************************************
 *
 *  $RCSfile: adminpages.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: fs $ $Date: 2000-10-30 15:22:44 $
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

#ifndef _DBAUI_ADMINPAGES_HXX_
#define _DBAUI_ADMINPAGES_HXX_

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
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
#include <vcl/imagebtn.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef _DBAUI_CHARSETS_HXX_
#include "charsets.hxx"
#endif
#ifndef _DBAUI_CURLEDIT_HXX_
#include "curledit.hxx"
#endif
#ifndef _DBAUI_TABLETREE_HXX_
#include "tabletree.hxx"
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OGenericAdministrationPage
//=========================================================================
class OGenericAdministrationPage : public SfxTabPage
{
protected:
    Link        m_aModifiedHandler;     /// to be called if something on the page has been modified

public:
    OGenericAdministrationPage(Window* _pParent, const ResId& _rId, const SfxItemSet& _rAttrSet);

    /// set a handler which gets called every time something on the page has been modified
    void            SetModifiedHandler(const Link& _rHandler) { m_aModifiedHandler = _rHandler; }

protected:
    /// default implementation: call FillItemSet, call checkItems,
    virtual int DeactivatePage(SfxItemSet* pSet);
    /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_False
    virtual void Reset(const SfxItemSet& _rCoreAttrs);
    /// default implementation: call implInitControls with the given item set and _bSaveValue = sal_True
    virtual void ActivatePage(const SfxItemSet& _rSet);

protected:
    void callModifiedHdl() const { if (m_aModifiedHandler.IsSet()) m_aModifiedHandler.Call((void*)this); }

    /// called from within DeactivatePage. The page is allowed to be deactivated if this method returns sal_True
    virtual sal_Bool checkItems(const SfxItemSet& _rSet) { return sal_True; }

    /** called from within Reset and ActivatePage, use to initialize the controls with the items from the given set
        @param      _bSaveValue     if set to sal_True, the implementation should call SaveValue on all relevant controls
    */
    virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue) { }

    /// analyze the invalid and the readonly flag which may be present in the set
    void getFlags(const SfxItemSet& _rSet, sal_Bool& _rValid, sal_Bool& _rReadonly);

protected:
    /** This link be used for controls where the tabpage does not need to take any special action when the control
        is modified. The implementation just calls callModifiedHdl.
    */
    DECL_LINK(OnControlModified, Control*);

    /// may be used in SetXXXHdl calls to controls, is a link to <method>OnControlModified</method>
    Link getControlModifiedLink() { return LINK(this, OGenericAdministrationPage, OnControlModified); }
};

//=========================================================================
//= OGeneralPage
//=========================================================================
/**
*/
class OGeneralPage : public OGenericAdministrationPage
{
    OGeneralPage(Window* pParent, const SfxItemSet& _rItems);

private:
    // dialog controls
    FixedText           m_aNameLabel;
    Edit                m_aName;
    GroupBox            m_aTypeBox;
    FixedText           m_aDatasourceTypeLabel;
    ListBox             m_aDatasourceType;
    FixedText           m_aConnectionLabel;
    OConnectionURLEdit  m_aConnection;
    PushButton          m_aBrowseConnection;
//  FixedText           m_aTimeoutLabel;
//  Edit                m_aTimeoutNumber;
//  ListBox             m_aTimeoutUnit;

    FixedText           m_aSpecialMessage;

    ODsnTypeCollection* m_pCollection;  /// the DSN type collection instance
    DECLARE_STL_MAP(DATASOURCE_TYPE, String, ::std::less< DATASOURCE_TYPE >, SelectionHistory);
    DATASOURCE_TYPE     m_eCurrentSelection;    /// currently selected type
    SelectionHistory    m_aSelectionHistory;    /// last selected ConnectURLs for all types

    Link                m_aTypeSelectHandler;   /// to be called if a new type is selected
    Link                m_aNameModifiedHandler; /// to be called whenever the name of the data source is changed by the user

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

protected:
    // SfxTabPage overridables
    virtual BOOL FillItemSet(SfxItemSet& _rCoreAttrs);
    virtual void Reset(const SfxItemSet& _rCoreAttrs);

    virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

    virtual void GetFocus();

protected:
    void onTypeSelected(DATASOURCE_TYPE _eType);
    void initializeHistory();

    DECL_LINK(OnDatasourceTypeSelected, ListBox*);
    DECL_LINK(OnBrowseConnections, PushButton*);
    DECL_LINK(OnNameModified, Edit*);
};


//=========================================================================
//= OCommonBehaviourTabPage
//=========================================================================
#define     CBTP_NONE           0x0000
#define     CBTP_USE_UIDPWD     0x0001
#define     CBTP_USE_CHARSET    0x0002
#define     CBTP_USE_OPTIONS    0x0004

/** eases the implementation of tab pages handling user/password and/or character
    set and/or generic options input
    <BR>
    The controls to be used habe to be defined within the resource, as usual, but
    this class does all the handling necessary.
*/
class OCommonBehaviourTabPage : public OGenericAdministrationPage
{
protected:
    FixedText*          m_pUserNameLabel;
    Edit*               m_pUserName;
    FixedText*          m_pPasswordLabel;
    Edit*               m_pPassword;
    CheckBox*           m_pPasswordRequired;

    FixedText*          m_pOptionsLabel;
    Edit*               m_pOptions;

    FixedText*          m_pCharsetLabel;
    ListBox*            m_pCharset;

    OCharsetCollection  m_aCharsets;

    USHORT              m_nControlFlags;

public:
    virtual BOOL        FillItemSet (SfxItemSet& _rCoreAttrs);
    virtual void        implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);

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
    GroupBox            m_aFrame;
    CheckBox            m_aShowDeleted;
    CheckBox            m_aLongTableNames;
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
//= OOdbcDetailsPage
//========================================================================
class OOdbcDetailsPage : public OCommonBehaviourTabPage
{
public:
    static  SfxTabPage* Create( Window* pParent, const SfxItemSet& _rAttrSet );

    /// get the SfxPoolItem ids used by this tab page
    static sal_Int32* getDetailIds();

private:
    FixedLine           m_aSeparator1;

    OOdbcDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
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

    OTextDetailsPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    ~OTextDetailsPage();

private:
    String      GetSeparator( const ComboBox& rBox, const String& rList );
    void        SetSeparator( ComboBox& rBox, const String& rList, const String& rVal );

    virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
    virtual sal_Bool checkItems(const SfxItemSet& _rSet);
};

//========================================================================
//= OTableSubscriptionPage
//========================================================================
class ODbAdminDialog;
class OTableSubscriptionPage : public OGenericAdministrationPage
{
    friend class ODbAdminDialog;

private:
    GroupBox                m_aTables;
    RadioButton             m_aIncludeAll;
    RadioButton             m_aIncludeNone;
    RadioButton             m_aIncludeSelected;
    OTableTreeListBox       m_aTablesList;
    CheckBox                m_aSuppressVersionColumns;
    sal_Bool                m_bCheckedAll : 1;
    sal_Bool                m_bCatalogAtStart : 1;
    sal_Bool                m_bInitializingControls : 1;
    ::rtl::OUString         m_sCatalogSeparator;
    ODbAdminDialog*         m_pAdminDialog;     /** needed for translating an SfxItemSet into Sequence< PropertyValue >
                                                    (for building an XConnection)
                                                */

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >
                            m_xCurrentConnection;   /// valid as long as the page is active
    ::com::sun::star::uno::Sequence< ::rtl::OUString >
                            m_aLastDetailedSelection;
    RadioButton*            m_pLastCheckedButton;

public:
    static  SfxTabPage* Create( Window* _pParent, const SfxItemSet& _rAttrSet);
    virtual BOOL        FillItemSet(SfxItemSet& _rCoreAttrs);
    virtual void        ActivatePage(const SfxItemSet& _rSet);
    virtual int         DeactivatePage(SfxItemSet* _pSet);

    void setServiceFactory(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > _rxORB)
        { m_aTablesList.setServiceFactory(_rxORB); }

protected:
    void SetAdminDialog(ODbAdminDialog* _pDialog) { m_pAdminDialog = _pDialog; }

private:
    OTableSubscriptionPage( Window* pParent, const SfxItemSet& _rCoreAttrs );
    ~OTableSubscriptionPage();

    /** check the tables in <member>m_aTablesList</member> according to <arg>_rTables</arg>
    */
    void implCheckTables(const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rTables);

    /** return the current selection in <member>m_aTablesList</member>
    */
    ::com::sun::star::uno::Sequence< ::rtl::OUString > collectDetailedSelection() const;

    void CheckAll( BOOL bCheck=TRUE );
    DECL_LINK( OnRadioButtonClicked, Button* );

    virtual void implInitControls(const SfxItemSet& _rSet, sal_Bool _bSaveValue);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_ADMINPAGES_HXX_

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.7  2000/10/24 12:11:15  fs
 *  functionality added: browsing for system data sources (ODBC/Adabas/dbase/text)
 *
 *  Revision 1.6  2000/10/20 09:53:17  fs
 *  handling for the SuppresVersionColumns property of a data source
 *
 *  Revision 1.5  2000/10/13 16:03:34  fs
 *  Separator changed to string / getDetailIds
 *
 *  Revision 1.4  2000/10/12 16:20:42  fs
 *  new implementations ... still under construction
 *
 *  Revision 1.3  2000/10/11 11:31:03  fs
 *  new implementations - still under construction
 *
 *  Revision 1.2  2000/10/09 12:39:29  fs
 *  some (a lot of) new imlpementations - still under development
 *
 *  Revision 1.1  2000/10/05 10:04:22  fs
 *  initial checkin
 *
 *
 *  Revision 1.0 26.09.00 11:46:15  fs
 ************************************************************************/

