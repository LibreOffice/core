/*************************************************************************
 *
 *  $RCSfile: optjava.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:57:42 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_OPTJAVA_HXX
#define _SVX_OPTJAVA_HXX

// include ---------------------------------------------------------------

#include <vector>

#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_TIMER_HXX
#include <vcl/timer.hxx>
#endif
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SVX_SIMPTABL_HXX
#include "simptabl.hxx"
#endif

// forward ---------------------------------------------------------------

typedef struct _JavaInfo JavaInfo;

class   SvxJavaParameterDlg;
class   SvxJavaClassPathDlg;

// class SvxJavaTable ----------------------------------------------------

class SvxJavaTable : public SvxSimpleTable
{
private:
    Point               m_aCurMousePoint;

protected:
    virtual void        SetTabs();
    virtual void        MouseButtonUp( const MouseEvent& _rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );

public:
    SvxJavaTable( Window* _pParent, const ResId& _rId );
    ~SvxJavaTable();


    inline Point        GetCurMousePoint() { return m_aCurMousePoint; }
};

// class SvxJavaOptionsPage ----------------------------------------------

class SvxJavaOptionsPage : public SfxTabPage
{
private:
    FixedLine               m_aJavaLine;
    CheckBox                m_aJavaEnableCB;
    FixedText               m_aJavaFoundLabel;
    SvxJavaTable            m_aJavaList;
    FixedText               m_aJavaPathText;
    PushButton              m_aAddBtn;
    PushButton              m_aParameterBtn;
    PushButton              m_aClassPathBtn;

    SvxJavaParameterDlg*    m_pParamDlg;
    SvxJavaClassPathDlg*    m_pPathDlg;

    JavaInfo**              m_parJavaInfo;
    rtl_uString**           m_parParameters;
    rtl_uString*            m_pClassPath;
    sal_Int32               m_nInfoSize;
    sal_Int32               m_nParamSize;
    String                  m_sInstallText;
    String                  m_sAccessibilityText;
    String                  m_sAddDialogText;
    Timer                   m_aResetTimer;

    ::std::vector< JavaInfo* >
                            m_aAddedInfos;

    DECL_LINK(              EnableHdl_Impl, CheckBox * );
    DECL_LINK(              CheckHdl_Impl, SvxSimpleTable * );
    DECL_LINK(              SelectHdl_Impl, SvxSimpleTable * );
    DECL_LINK(              AddHdl_Impl, PushButton * );
    DECL_LINK(              ParameterHdl_Impl, PushButton * );
    DECL_LINK(              ClassPathHdl_Impl, PushButton * );
    DECL_LINK(              ResetHdl_Impl, Timer * );

    void                    ClearJavaInfo();
    void                    ClearJavaList();
    void                    LoadJREs();
    void                    AddJRE( JavaInfo* _pInfo );
    void                    HandleCheckEntry( SvLBoxEntry* _pEntry );

public:
    SvxJavaOptionsPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxJavaOptionsPage();

    static SfxTabPage*      Create( Window* pParent, const SfxItemSet& rSet );

    virtual BOOL            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );
    virtual void            FillUserData();
};

// class SvxJavaParameterDlg ---------------------------------------------

class SvxJavaParameterDlg : public ModalDialog
{
private:
    FixedText               m_aParameterLabel;
    Edit                    m_aParameterEdit;
    PushButton              m_aAssignBtn;

    FixedText               m_aAssignedLabel;
    ListBox                 m_aAssignedList;
    PushButton              m_aRemoveBtn;

    FixedLine               m_aButtonsLine;
    OKButton                m_aOKBtn;
    CancelButton            m_aCancelBtn;
    HelpButton              m_aHelpBtn;

    DECL_LINK(              ModifyHdl_Impl, Edit * );
    DECL_LINK(              AssignHdl_Impl, PushButton * );
    DECL_LINK(              SelectHdl_Impl, ListBox * );
    DECL_LINK(              RemoveHdl_Impl, PushButton * );

    inline void             EnableRemoveButton()
                                { m_aRemoveBtn.Enable(
                                    m_aAssignedList.GetSelectEntryPos()
                                    != LISTBOX_ENTRY_NOTFOUND ); }


public:
    SvxJavaParameterDlg( Window* pParent );
    ~SvxJavaParameterDlg();

    virtual short           Execute();

    ::com::sun::star::uno::Sequence< ::rtl::OUString > GetParameters() const;
    void SetParameters( ::com::sun::star::uno::Sequence< ::rtl::OUString >& rParams );
};

// class SvxJavaClassPathDlg ---------------------------------------------

class SvxJavaClassPathDlg : public ModalDialog
{
private:
    FixedText               m_aPathLabel;
    ListBox                 m_aPathList;
    PushButton              m_aAddArchiveBtn;
    PushButton              m_aAddPathBtn;
    PushButton              m_aRemoveBtn;

    FixedLine               m_aButtonsLine;
    OKButton                m_aOKBtn;
    CancelButton            m_aCancelBtn;
    HelpButton              m_aHelpBtn;

    String                  m_sOldPath;

    DECL_LINK(              AddArchiveHdl_Impl, PushButton * );
    DECL_LINK(              AddPathHdl_Impl, PushButton * );
    DECL_LINK(              RemoveHdl_Impl, PushButton * );

    bool                    IsPathDuplicate( const String& _rPath );

public:
    SvxJavaClassPathDlg( Window* pParent );
    ~SvxJavaClassPathDlg();

    inline const String&    GetOldPath() const { return m_sOldPath; }
    inline void             SetFocus() { m_aPathList.GrabFocus(); }

    String                  GetClassPath() const;
    void                    SetClassPath( const String& _rPath );
};

#endif // #ifndef _SVX_OPTJAVA_HXX

