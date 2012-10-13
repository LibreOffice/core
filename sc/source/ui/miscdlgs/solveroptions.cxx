/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "solveroptions.hxx"
#include "solveroptions.hrc"
#include "scresid.hxx"
#include "global.hxx"
#include "miscuno.hxx"
#include "solverutil.hxx"

#include <rtl/math.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/localedatawrapper.hxx>

#include <algorithm>

#include <com/sun/star/sheet/Solver.hpp>
#include <com/sun/star/sheet/XSolverDescription.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

using namespace com::sun::star;

//==================================================================

/// Helper for sorting properties
struct ScSolverOptionsEntry
{
    sal_Int32       nPosition;
    rtl::OUString   aDescription;

    ScSolverOptionsEntry() : nPosition(0) {}

    bool operator< (const ScSolverOptionsEntry& rOther) const
    {
        return ( ScGlobal::GetCollator()->compareString( aDescription, rOther.aDescription ) == COMPARE_LESS );
    }
};

//------------------------------------------------------------------

class ScSolverOptionsString : public SvLBoxString
{
    bool        mbIsDouble;
    double      mfDoubleValue;
    sal_Int32   mnIntValue;

public:
    ScSolverOptionsString( SvLBoxEntry* pEntry, sal_uInt16 nFlags, const String& rStr ) :
        SvLBoxString( pEntry, nFlags, rStr ),
        mbIsDouble( false ),
        mfDoubleValue( 0.0 ),
        mnIntValue( 0 ) {}

    bool      IsDouble() const        { return mbIsDouble; }
    double    GetDoubleValue() const  { return mfDoubleValue; }
    sal_Int32 GetIntValue() const     { return mnIntValue; }

    void      SetDoubleValue( double fNew ) { mbIsDouble = true; mfDoubleValue = fNew; }
    void      SetIntValue( sal_Int32 nNew ) { mbIsDouble = false; mnIntValue = nNew; }

    virtual void Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry );
};

void ScSolverOptionsString::Paint( const Point& rPos, SvTreeListBox& rDev, sal_uInt16, SvLBoxEntry* /* pEntry */ )
{
    //! move position? (SvxLinguTabPage: aPos.X() += 20)
    String aNormalStr( GetText() );
    aNormalStr.Append( (sal_Unicode) ':' );
    rDev.DrawText( rPos, aNormalStr );

    Point aNewPos( rPos );
    aNewPos.X() += rDev.GetTextWidth( aNormalStr );
    Font aOldFont( rDev.GetFont() );
    Font aFont( aOldFont );
    aFont.SetWeight( WEIGHT_BOLD );

    rtl::OUString sTxt( ' ' );
    if ( mbIsDouble )
        sTxt += rtl::math::doubleToUString( mfDoubleValue,
            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
            ScGlobal::GetpLocaleData()->getNumDecimalSep()[0], true );
    else
        sTxt += rtl::OUString::valueOf(mnIntValue);
    rDev.SetFont( aFont );
    rDev.DrawText( aNewPos, sTxt );

    rDev.SetFont( aOldFont );
}

//------------------------------------------------------------------

ScSolverOptionsDialog::ScSolverOptionsDialog( Window* pParent,
                        const uno::Sequence<rtl::OUString>& rImplNames,
                        const uno::Sequence<rtl::OUString>& rDescriptions,
                        const String& rEngine,
                        const uno::Sequence<beans::PropertyValue>& rProperties )
    : ModalDialog( pParent, ScResId( RID_SCDLG_SOLVEROPTIONS ) ),
    maFtEngine      ( this, ScResId( FT_ENGINE ) ),
    maLbEngine      ( this, ScResId( LB_ENGINE ) ),
    maFtSettings    ( this, ScResId( FT_SETTINGS ) ),
    maLbSettings    ( this, ScResId( LB_SETTINGS ) ),
    maBtnEdit       ( this, ScResId( BTN_EDIT ) ),
    maFlButtons     ( this, ScResId( FL_BUTTONS ) ),
    maBtnHelp       ( this, ScResId( BTN_HELP ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) ),
    mpCheckButtonData( NULL ),
    maImplNames( rImplNames ),
    maDescriptions( rDescriptions ),
    maEngine( rEngine ),
    maProperties( rProperties )
{
    maLbEngine.SetSelectHdl( LINK( this, ScSolverOptionsDialog, EngineSelectHdl ) );

    maBtnEdit.SetClickHdl( LINK( this, ScSolverOptionsDialog, ButtonHdl ) );

    maLbSettings.SetStyle( maLbSettings.GetStyle()|WB_CLIPCHILDREN|WB_FORCE_MAKEVISIBLE );
    maLbSettings.SetHelpId( HID_SC_SOLVEROPTIONS_LB );
    maLbSettings.SetHighlightRange();

    maLbSettings.SetSelectHdl( LINK( this, ScSolverOptionsDialog, SettingsSelHdl ) );
    maLbSettings.SetDoubleClickHdl( LINK( this, ScSolverOptionsDialog, SettingsDoubleClickHdl ) );

    sal_Int32 nSelect = -1;
    sal_Int32 nImplCount = maImplNames.getLength();
    for (sal_Int32 nImpl=0; nImpl<nImplCount; ++nImpl)
    {
        String aImplName( maImplNames[nImpl] );
        String aDescription( maDescriptions[nImpl] );   // user-visible descriptions in list box
        maLbEngine.InsertEntry( aDescription );
        if ( aImplName == maEngine )
            nSelect = nImpl;
    }
    if ( nSelect < 0 )                  // no (valid) engine given
    {
        if ( nImplCount > 0 )
        {
            maEngine = maImplNames[0];  // use first implementation
            nSelect = 0;
        }
        else
            maEngine.Erase();
        maProperties.realloc(0);        // don't use options from different engine
    }
    if ( nSelect >= 0 )                 // select in list box
        maLbEngine.SelectEntryPos( static_cast<sal_uInt16>(nSelect) );

    if ( !maProperties.getLength() )
        ReadFromComponent();            // fill maProperties from component (using maEngine)
    FillListBox();                      // using maProperties

    FreeResource();
}

ScSolverOptionsDialog::~ScSolverOptionsDialog()
{
    delete mpCheckButtonData;
}

const String& ScSolverOptionsDialog::GetEngine() const
{
    return maEngine;    // already updated in selection handler
}

const uno::Sequence<beans::PropertyValue>& ScSolverOptionsDialog::GetProperties()
{
    // update maProperties from list box content
    // order of entries in list box and maProperties is the same
    sal_Int32 nEntryCount = maProperties.getLength();
    SvLBoxTreeList* pModel = maLbSettings.GetModel();
    if ( nEntryCount == (sal_Int32)pModel->GetEntryCount() )
    {
        for (sal_Int32 nEntryPos=0; nEntryPos<nEntryCount; ++nEntryPos)
        {
            uno::Any& rValue = maProperties[nEntryPos].Value;
            SvLBoxEntry* pEntry = pModel->GetEntry(nEntryPos);

            bool bHasData = false;
            sal_uInt16 nItemCount = pEntry->ItemCount();
            for (sal_uInt16 nItemPos=0; nItemPos<nItemCount && !bHasData; ++nItemPos)
            {
                SvLBoxItem* pItem = pEntry->GetItem( nItemPos );
                ScSolverOptionsString* pStringItem = dynamic_cast<ScSolverOptionsString*>(pItem);
                if ( pStringItem )
                {
                    if ( pStringItem->IsDouble() )
                        rValue <<= pStringItem->GetDoubleValue();
                    else
                        rValue <<= pStringItem->GetIntValue();
                    bHasData = true;
                }
            }
            if ( !bHasData )
                ScUnoHelpFunctions::SetBoolInAny( rValue,
                                    maLbSettings.GetCheckButtonState( pEntry ) == SV_BUTTON_CHECKED );
        }
    }
    else
    {
        OSL_FAIL( "wrong count" );
    }

    return maProperties;
}

void ScSolverOptionsDialog::FillListBox()
{
    // get property descriptions, sort by them

    uno::Reference<sheet::XSolverDescription> xDesc( ScSolverUtil::GetSolver( maEngine ), uno::UNO_QUERY );
    sal_Int32 nCount = maProperties.getLength();
    std::vector<ScSolverOptionsEntry> aDescriptions( nCount );
    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
    {
        rtl::OUString aPropName( maProperties[nPos].Name );
        rtl::OUString aVisName;
        if ( xDesc.is() )
            aVisName = xDesc->getPropertyDescription( aPropName );
        if ( aVisName.isEmpty() )
            aVisName = aPropName;
        aDescriptions[nPos].nPosition = nPos;
        aDescriptions[nPos].aDescription = aVisName;
    }
    std::sort( aDescriptions.begin(), aDescriptions.end() );

    // also update maProperties to the order of descriptions

    uno::Sequence<beans::PropertyValue> aNewSeq;
    aNewSeq.realloc( nCount );
    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
        aNewSeq[nPos] = maProperties[ aDescriptions[nPos].nPosition ];
    maProperties = aNewSeq;

    // fill the list box

    maLbSettings.SetUpdateMode(false);
    maLbSettings.Clear();

    String sEmpty;
    if (!mpCheckButtonData)
        mpCheckButtonData = new SvLBoxButtonData( &maLbSettings );

    SvLBoxTreeList* pModel = maLbSettings.GetModel();
    SvLBoxEntry* pEntry = NULL;

    for (sal_Int32 nPos=0; nPos<nCount; nPos++)
    {
        rtl::OUString aVisName = aDescriptions[nPos].aDescription;

        uno::Any aValue = maProperties[nPos].Value;
        uno::TypeClass eClass = aValue.getValueTypeClass();
        if ( eClass == uno::TypeClass_BOOLEAN )
        {
            // check box entry
            pEntry = new SvLBoxEntry;
            SvLBoxButton* pButton = new SvLBoxButton( pEntry, SvLBoxButtonKind_enabledCheckbox, 0, mpCheckButtonData );
            if ( ScUnoHelpFunctions::GetBoolFromAny( aValue ) )
                pButton->SetStateChecked();
            else
                pButton->SetStateUnchecked();
            pEntry->AddItem( pButton );
            pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0 ) );
            pEntry->AddItem( new SvLBoxString( pEntry, 0, aVisName ) );
        }
        else
        {
            // value entry
            pEntry = new SvLBoxEntry;
            pEntry->AddItem( new SvLBoxString( pEntry, 0, sEmpty ) );                   // empty column
            pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0 ) );
            ScSolverOptionsString* pItem = new ScSolverOptionsString( pEntry, 0, aVisName );
            if ( eClass == uno::TypeClass_DOUBLE )
            {
                double fDoubleValue = 0.0;
                if ( aValue >>= fDoubleValue )
                    pItem->SetDoubleValue( fDoubleValue );
            }
            else
            {
                sal_Int32 nIntValue = 0;
                if ( aValue >>= nIntValue )
                    pItem->SetIntValue( nIntValue );
            }
            pEntry->AddItem( pItem );
        }
        pModel->Insert( pEntry );
    }

    maLbSettings.SetUpdateMode(sal_True);
}

void ScSolverOptionsDialog::ReadFromComponent()
{
    maProperties = ScSolverUtil::GetDefaults( maEngine );
}

void ScSolverOptionsDialog::EditOption()
{
    SvLBoxEntry* pEntry = maLbSettings.GetCurEntry();
    if (pEntry)
    {
        sal_uInt16 nItemCount = pEntry->ItemCount();
        for (sal_uInt16 nPos=0; nPos<nItemCount; ++nPos)
        {
            SvLBoxItem* pItem = pEntry->GetItem( nPos );
            ScSolverOptionsString* pStringItem = dynamic_cast<ScSolverOptionsString*>(pItem);
            if ( pStringItem )
            {
                if ( pStringItem->IsDouble() )
                {
                    ScSolverValueDialog aValDialog( this );
                    aValDialog.SetOptionName( pStringItem->GetText() );
                    aValDialog.SetValue( pStringItem->GetDoubleValue() );
                    if ( aValDialog.Execute() == RET_OK )
                    {
                        pStringItem->SetDoubleValue( aValDialog.GetValue() );
                        maLbSettings.InvalidateEntry( pEntry );
                    }
                }
                else
                {
                    ScSolverIntegerDialog aIntDialog( this );
                    aIntDialog.SetOptionName( pStringItem->GetText() );
                    aIntDialog.SetValue( pStringItem->GetIntValue() );
                    if ( aIntDialog.Execute() == RET_OK )
                    {
                        pStringItem->SetIntValue( aIntDialog.GetValue() );
                        maLbSettings.InvalidateEntry( pEntry );
                    }
                }
            }
        }
    }
}

IMPL_LINK( ScSolverOptionsDialog, ButtonHdl, PushButton*, pBtn )
{
    if ( pBtn == &maBtnEdit )
        EditOption();

    return 0;
}

IMPL_LINK_NOARG(ScSolverOptionsDialog, SettingsDoubleClickHdl)
{
    EditOption();
    return 0;
}

IMPL_LINK_NOARG(ScSolverOptionsDialog, EngineSelectHdl)
{
    sal_uInt16 nSelectPos = maLbEngine.GetSelectEntryPos();
    if ( nSelectPos < maImplNames.getLength() )
    {
        String aNewEngine( maImplNames[nSelectPos] );
        if ( aNewEngine != maEngine )
        {
            maEngine = aNewEngine;
            ReadFromComponent();            // fill maProperties from component (using maEngine)
            FillListBox();                  // using maProperties
        }
    }
    return 0;
}

IMPL_LINK_NOARG(ScSolverOptionsDialog, SettingsSelHdl)
{
    sal_Bool bCheckbox = false;

    SvLBoxEntry* pEntry = maLbSettings.GetCurEntry();
    if (pEntry)
    {
        SvLBoxItem* pItem = pEntry->GetFirstItem(SV_ITEM_ID_LBOXBUTTON);
        if ( pItem && pItem->IsA() == SV_ITEM_ID_LBOXBUTTON )
            bCheckbox = sal_True;
    }

    maBtnEdit.Enable( !bCheckbox );

    return 0;
}

//------------------------------------------------------------------

ScSolverIntegerDialog::ScSolverIntegerDialog( Window * pParent )
    : ModalDialog( pParent, ScResId( RID_SCDLG_SOLVER_INTEGER ) ),
    maFtName        ( this, ScResId( FT_OPTIONNAME ) ),
    maNfValue       ( this, ScResId( NF_VALUE ) ),
    maFlButtons     ( this, ScResId( FL_BUTTONS ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) )
{
    FreeResource();
}

ScSolverIntegerDialog::~ScSolverIntegerDialog()
{
}

void ScSolverIntegerDialog::SetOptionName( const String& rName )
{
    maFtName.SetText( rName );
}

void ScSolverIntegerDialog::SetValue( sal_Int32 nValue )
{
    maNfValue.SetValue( nValue );
}

sal_Int32 ScSolverIntegerDialog::GetValue() const
{
    sal_Int64 nValue = maNfValue.GetValue();
    if ( nValue < SAL_MIN_INT32 )
        return SAL_MIN_INT32;
    if ( nValue > SAL_MAX_INT32 )
        return SAL_MAX_INT32;
    return (sal_Int32) nValue;
}

//------------------------------------------------------------------

ScSolverValueDialog::ScSolverValueDialog( Window * pParent )
    : ModalDialog( pParent, ScResId( RID_SCDLG_SOLVER_DOUBLE ) ),
    maFtName        ( this, ScResId( FT_OPTIONNAME ) ),
    maEdValue       ( this, ScResId( ED_VALUE ) ),
    maFlButtons     ( this, ScResId( FL_BUTTONS ) ),
    maBtnOk         ( this, ScResId( BTN_OK ) ),
    maBtnCancel     ( this, ScResId( BTN_CANCEL ) )
{
    FreeResource();
}

ScSolverValueDialog::~ScSolverValueDialog()
{
}

void ScSolverValueDialog::SetOptionName( const String& rName )
{
    maFtName.SetText( rName );
}

void ScSolverValueDialog::SetValue( double fValue )
{
    maEdValue.SetText( rtl::math::doubleToUString( fValue,
            rtl_math_StringFormat_Automatic, rtl_math_DecimalPlaces_Max,
            ScGlobal::GetpLocaleData()->getNumDecimalSep()[0], true ) );
}

double ScSolverValueDialog::GetValue() const
{
    String aInput = maEdValue.GetText();

    const LocaleDataWrapper* pLocaleData = ScGlobal::GetpLocaleData();
    rtl_math_ConversionStatus eStatus = rtl_math_ConversionStatus_Ok;
    double fValue = rtl::math::stringToDouble( aInput,
                            pLocaleData->getNumDecimalSep()[0],
                            pLocaleData->getNumThousandSep()[0],
                            &eStatus, NULL );
    return fValue;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
