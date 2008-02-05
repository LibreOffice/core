/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: solveroptions.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2008-02-05 15:46:35 $
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

#ifndef SC_SOLVEROPTIONS_HXX
#define SC_SOLVEROPTIONS_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif

#ifndef _SVX_CHECKLBX_HXX //autogen
#include <svx/checklbx.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace beans { struct PropertyValue; }
} } }



class ScSolverOptionsDialog : public ModalDialog
{
    FixedText       maFtEngine;
    ListBox         maLbEngine;
    FixedText       maFtSettings;
    SvxCheckListBox maLbSettings;
    PushButton      maBtnEdit;
    FixedLine       maFlButtons;
    HelpButton      maBtnHelp;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

    SvLBoxButtonData* mpCheckButtonData;
    com::sun::star::uno::Sequence<rtl::OUString> maImplNames;
    com::sun::star::uno::Sequence<rtl::OUString> maDescriptions;
    String          maEngine;
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> maProperties;

    DECL_LINK( EngineSelectHdl, ListBox* );
    DECL_LINK( SettingsSelHdl, SvxCheckListBox* );
    DECL_LINK( SettingsDoubleClickHdl, SvTreeListBox* );
    DECL_LINK( ButtonHdl, PushButton* );

    void    ReadFromComponent();
    void    FillListBox();
    void    EditOption();

public:
    ScSolverOptionsDialog( Window* pParent,
                           const com::sun::star::uno::Sequence<rtl::OUString>& rImplNames,
                           const com::sun::star::uno::Sequence<rtl::OUString>& rDescriptions,
                           const String& rEngine,
                           const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProperties );
    ~ScSolverOptionsDialog();

    const String& GetEngine() const;
    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& GetProperties();
};


class ScSolverIntegerDialog : public ModalDialog
{
    FixedText       maFtName;
    NumericField    maNfValue;
    FixedLine       maFlButtons;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

public:
    ScSolverIntegerDialog( Window * pParent );
    ~ScSolverIntegerDialog();

    void        SetOptionName( const String& rName );
    void        SetValue( sal_Int32 nValue );
    sal_Int32   GetValue() const;
};

class ScSolverValueDialog : public ModalDialog
{
    FixedText       maFtName;
    Edit            maEdValue;
    FixedLine       maFlButtons;
    OKButton        maBtnOk;
    CancelButton    maBtnCancel;

public:
    ScSolverValueDialog( Window * pParent );
    ~ScSolverValueDialog();

    void        SetOptionName( const String& rName );
    void        SetValue( double fValue );
    double      GetValue() const;
};

#endif

