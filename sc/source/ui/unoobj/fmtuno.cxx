/*************************************************************************
 *
 *  $RCSfile: fmtuno.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:07 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include <tools/debug.hxx>
#include <rtl/uuid.h>

#include <com/sun/star/sheet/ValidationAlertStyle.hpp>
#include <com/sun/star/sheet/ValidationType.hpp>

#include "fmtuno.hxx"
#include "miscuno.hxx"
#include "conditio.hxx"
#include "validat.hxx"
#include "document.hxx"
#include "unoguard.hxx"
#include "unonames.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  Map nur fuer PropertySetInfo

const SfxItemPropertyMap* lcl_GetValidatePropertyMap()
{
    static SfxItemPropertyMap aValidatePropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ERRALSTY), 0,  &getCppuType((sheet::ValidationAlertStyle*)0),  0},
        {MAP_CHAR_LEN(SC_UNONAME_ERRMESS),  0,  &getCppuType((rtl::OUString*)0),                0},
        {MAP_CHAR_LEN(SC_UNONAME_ERRTITLE), 0,  &getCppuType((rtl::OUString*)0),                0},
        {MAP_CHAR_LEN(SC_UNONAME_IGNOREBL), 0,  &getBooleanCppuType(),                          0},
        {MAP_CHAR_LEN(SC_UNONAME_INPMESS),  0,  &getCppuType((rtl::OUString*)0),                0},
        {MAP_CHAR_LEN(SC_UNONAME_INPTITLE), 0,  &getCppuType((rtl::OUString*)0),                0},
        {MAP_CHAR_LEN(SC_UNONAME_SHOWERR),  0,  &getBooleanCppuType(),                          0},
        {MAP_CHAR_LEN(SC_UNONAME_SHOWINP),  0,  &getBooleanCppuType(),                          0},
        {MAP_CHAR_LEN(SC_UNONAME_TYPE),     0,  &getCppuType((sheet::ValidationType*)0),        0},
        {0,0,0,0}
    };
    return aValidatePropertyMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScTableConditionalEntry, "ScTableConditionalEntry", "com.sun.star.sheet.TableConditionalEntry" )
SC_SIMPLE_SERVICE_INFO( ScTableConditionalFormat, "ScTableConditionalFormat", "com.sun.star.sheet.TableConditionalFormat" )
SC_SIMPLE_SERVICE_INFO( ScTableValidationObj, "ScTableValidationObj", "com.sun.star.sheet.TableValidation" )

//------------------------------------------------------------------------

sheet::ConditionOperator lcl_ConditionModeToOperator( ScConditionMode eMode )
{
    sheet::ConditionOperator eOper = sheet::ConditionOperator_NONE;
    switch (eMode)
    {
        case SC_COND_EQUAL:      eOper = sheet::ConditionOperator_EQUAL;         break;
        case SC_COND_LESS:       eOper = sheet::ConditionOperator_LESS;          break;
        case SC_COND_GREATER:    eOper = sheet::ConditionOperator_GREATER;       break;
        case SC_COND_EQLESS:     eOper = sheet::ConditionOperator_LESS_EQUAL;    break;
        case SC_COND_EQGREATER:  eOper = sheet::ConditionOperator_GREATER_EQUAL; break;
        case SC_COND_NOTEQUAL:   eOper = sheet::ConditionOperator_NOT_EQUAL;     break;
        case SC_COND_BETWEEN:    eOper = sheet::ConditionOperator_BETWEEN;       break;
        case SC_COND_NOTBETWEEN: eOper = sheet::ConditionOperator_NOT_BETWEEN;   break;
        case SC_COND_DIRECT:     eOper = sheet::ConditionOperator_FORMULA;       break;
    }
    return eOper;
}

ScConditionMode lcl_ConditionOperatorToMode( sheet::ConditionOperator eOper )
{
    ScConditionMode eMode = SC_COND_NONE;
    switch (eOper)
    {
        case sheet::ConditionOperator_EQUAL:         eMode = SC_COND_EQUAL;      break;
        case sheet::ConditionOperator_LESS:          eMode = SC_COND_LESS;       break;
        case sheet::ConditionOperator_GREATER:       eMode = SC_COND_GREATER;    break;
        case sheet::ConditionOperator_LESS_EQUAL:    eMode = SC_COND_EQLESS;     break;
        case sheet::ConditionOperator_GREATER_EQUAL: eMode = SC_COND_EQGREATER;  break;
        case sheet::ConditionOperator_NOT_EQUAL:     eMode = SC_COND_NOTEQUAL;   break;
        case sheet::ConditionOperator_BETWEEN:       eMode = SC_COND_BETWEEN;    break;
        case sheet::ConditionOperator_NOT_BETWEEN:   eMode = SC_COND_NOTBETWEEN; break;
        case sheet::ConditionOperator_FORMULA:       eMode = SC_COND_DIRECT;     break;
    }
    return eMode;
}

//------------------------------------------------------------------------

ScTableConditionalFormat::ScTableConditionalFormat()
{
}

ScTableConditionalFormat::ScTableConditionalFormat(ScDocument* pDoc, ULONG nKey, BOOL bEnglish)
{
    //  Eintrag aus dem Dokument lesen...

    if ( pDoc && nKey )
    {
        ScConditionalFormatList* pList = pDoc->GetCondFormList();
        if (pList)
        {
            const ScConditionalFormat* pFormat = pList->GetFormat( nKey );
            if (pFormat)
            {
                USHORT nEntryCount = pFormat->Count();
                for (USHORT i=0; i<nEntryCount; i++)
                {
                    const ScCondFormatEntry* pFormatEntry = pFormat->GetEntry(i);
                    ScConditionMode eMode = pFormatEntry->GetOperation();
                    ScAddress aPos = pFormatEntry->GetSrcPos();
                    String aExpr1 = pFormatEntry->GetExpression( aPos, 0, 0, bEnglish );
                    String aExpr2 = pFormatEntry->GetExpression( aPos, 1, 0, bEnglish );
                    String aStyle = pFormatEntry->GetStyle();

                    AddEntry_Impl( eMode, aExpr1, aExpr2, aPos, aStyle );
                }
            }
        }
    }
}

void ScTableConditionalFormat::FillFormat( ScConditionalFormat& rFormat,
                                            ScDocument* pDoc, BOOL bEnglish ) const
{
    //  ScConditionalFormat = Core-Struktur, muss leer sein

    DBG_ASSERT( rFormat.IsEmpty(), "FillFormat: Format nicht leer" );
    USHORT nCount = (USHORT)aEntries.Count();
    for (USHORT i=0; i<nCount; i++)
    {
        ScTableConditionalEntry* pEntry = (ScTableConditionalEntry*)aEntries.GetObject(i);
        if (pEntry)
        {
            USHORT nMode;
            String aExpr1, aExpr2, aStyle;
            ScAddress aPos;
            pEntry->GetData( nMode, aExpr1, aExpr2, aPos, aStyle );
            ScCondFormatEntry aCoreEntry( (ScConditionMode)nMode,
                                aExpr1, aExpr2, pDoc, aPos, aStyle, bEnglish );
            rFormat.AddEntry( aCoreEntry );
        }
    }
}

ScTableConditionalFormat::~ScTableConditionalFormat()
{
    ScTableConditionalEntry* pEntry;
    aEntries.First();
    while ( pEntry = (ScTableConditionalEntry*)aEntries.Remove() )
        pEntry->release();
}

void ScTableConditionalFormat::AddEntry_Impl( USHORT nMode,
                        const String& rExpr1, const String& rExpr2,
                        const ScAddress& rPos, const String& rStyle )
{
    ScTableConditionalEntry* pNew = new ScTableConditionalEntry(
                                        this, nMode, rExpr1, rExpr2, rPos, rStyle );
    pNew->acquire();
    aEntries.Insert( pNew, LIST_APPEND );
}

void ScTableConditionalFormat::DataChanged()
{
    //  wenn's mal das "lebende Objekt" ist, muss hier was passieren...
}

// XSheetConditionalFormat

ScTableConditionalEntry* ScTableConditionalFormat::GetObjectByIndex_Impl(USHORT nIndex) const
{
    return (ScTableConditionalEntry*)aEntries.GetObject(nIndex);
}

void SAL_CALL ScTableConditionalFormat::addNew(
                    const uno::Sequence<beans::PropertyValue >& aConditionalEntry )
                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScConditionMode eMode = SC_COND_NONE;
    String aExpr1;
    String aExpr2;
    ScAddress aPos;
    String aStyle;

    const beans::PropertyValue* pPropArray = aConditionalEntry.getConstArray();
    long nPropCount = aConditionalEntry.getLength();
    for (long i = 0; i < nPropCount; i++)
    {
        const beans::PropertyValue& rProp = pPropArray[i];
        String aPropName = rProp.Name;

        if ( aPropName.EqualsAscii( SC_UNONAME_OPERATOR ) )
        {
            sheet::ConditionOperator eOper = (sheet::ConditionOperator)
                            ScUnoHelpFunctions::GetEnumFromAny( rProp.Value );
            eMode = lcl_ConditionOperatorToMode( eOper );
        }
        else if ( aPropName.EqualsAscii( SC_UNONAME_FORMULA1 ) )
        {
            rtl::OUString aStrVal;
            if ( rProp.Value >>= aStrVal )
                aExpr1 = String( aStrVal );
        }
        else if ( aPropName.EqualsAscii( SC_UNONAME_FORMULA2 ) )
        {
            rtl::OUString aStrVal;
            if ( rProp.Value >>= aStrVal )
                aExpr2 = String( aStrVal );
        }
        else if ( aPropName.EqualsAscii( SC_UNONAME_SOURCEPOS ) )
        {
            table::CellAddress aAddress;
            if ( rProp.Value >>= aAddress )
                aPos = ScAddress( aAddress.Column, aAddress.Row, aAddress.Sheet );
        }
        else if ( aPropName.EqualsAscii( SC_UNONAME_STYLENAME ) )
        {
            rtl::OUString aStrVal;
            if ( rProp.Value >>= aStrVal )
                aStyle = String( aStrVal );
        }
        else
        {
            DBG_ERROR("falsche Property");
            //! Exception...
        }
    }

    AddEntry_Impl( eMode, aExpr1, aExpr2, aPos, aStyle );
    DataChanged();
}

void SAL_CALL ScTableConditionalFormat::removeByIndex( sal_Int32 nIndex )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTableConditionalEntry* pEntry = (ScTableConditionalEntry*)aEntries.GetObject(nIndex);
    if (pEntry)
    {
        aEntries.Remove(pEntry);
        pEntry->release();
        DataChanged();
    }
}

void SAL_CALL ScTableConditionalFormat::clear() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScTableConditionalEntry* pEntry;
    aEntries.First();
    while ( pEntry = (ScTableConditionalEntry*)aEntries.Remove() )
        pEntry->release();

    DataChanged();
}

// XEnumerationAccess

uno::Reference<container::XEnumeration> SAL_CALL ScTableConditionalFormat::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

// XIndexAccess

sal_Int32 SAL_CALL ScTableConditionalFormat::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aEntries.Count();
}

uno::Any SAL_CALL ScTableConditionalFormat::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<sheet::XSheetConditionalEntry> xEntry = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    aAny <<= xEntry;
    return aAny;
}

uno::Type SAL_CALL ScTableConditionalFormat::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<sheet::XSheetConditionalEntry>*)0);
}

sal_Bool SAL_CALL ScTableConditionalFormat::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

uno::Any SAL_CALL ScTableConditionalFormat::getByName( const rtl::OUString& aName )
            throw(container::NoSuchElementException,
                    lang::WrappedTargetException, uno::RuntimeException)
{
    //! was fuer Namen ???
    return uno::Any();
}

uno::Sequence<rtl::OUString> SAL_CALL ScTableConditionalFormat::getElementNames()
                                                    throw(uno::RuntimeException)
{
    //! was fuer Namen ???
    return uno::Sequence<rtl::OUString>(0);
}

sal_Bool SAL_CALL ScTableConditionalFormat::hasByName( const rtl::OUString& aName )
                                                    throw(uno::RuntimeException)
{
    //! was fuer Namen ???
    return FALSE;
}

// XUnoTunnel

sal_Int64 SAL_CALL ScTableConditionalFormat::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScTableConditionalFormat::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScTableConditionalFormat* ScTableConditionalFormat::getImplementation(
                                const uno::Reference<sheet::XSheetConditionalEntries> xObj )
{
    ScTableConditionalFormat* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScTableConditionalFormat*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------

ScTableConditionalEntry::ScTableConditionalEntry() :
    pParent( NULL )
{
}

ScTableConditionalEntry::ScTableConditionalEntry( ScTableConditionalFormat* pPar,
                             USHORT nM, const String& rEx1, const String& rEx2,
                             const ScAddress& rPos, const String& rSt ) :
    pParent( pPar ),
    nMode( nM ),
    aExpr1( rEx1 ),
    aExpr2( rEx2 ),
    aSrcPos( rPos ),
    aStyle( rSt )
{
    if (pParent)
        pParent->acquire();
}

ScTableConditionalEntry::~ScTableConditionalEntry()
{
    if (pParent)
        pParent->release();
}

void ScTableConditionalEntry::GetData( USHORT& rM, String& rEx1, String& rEx2,
                                        ScAddress& rPos, String& rSt ) const
{
    rM   = nMode;
    rEx1 = aExpr1;
    rEx2 = aExpr2;
    rPos = aSrcPos;
    rSt  = aStyle;
}

// XSheetCondition

sheet::ConditionOperator SAL_CALL ScTableConditionalEntry::getOperator()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return lcl_ConditionModeToOperator( (ScConditionMode)nMode );
}

void SAL_CALL ScTableConditionalEntry::setOperator( sheet::ConditionOperator nOperator )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    nMode = lcl_ConditionOperatorToMode( nOperator );
    if (pParent)
        pParent->DataChanged();
}

rtl::OUString SAL_CALL ScTableConditionalEntry::getFormula1() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aExpr1;
}

void SAL_CALL ScTableConditionalEntry::setFormula1( const rtl::OUString& aFormula1 )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aExpr1 = String( aFormula1 );
    if (pParent)
        pParent->DataChanged();
}

rtl::OUString SAL_CALL ScTableConditionalEntry::getFormula2() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aExpr2;
}

void SAL_CALL ScTableConditionalEntry::setFormula2( const rtl::OUString& aFormula2 )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aExpr2 = String( aFormula2 );
    if (pParent)
        pParent->DataChanged();
}

table::CellAddress SAL_CALL ScTableConditionalEntry::getSourcePosition() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellAddress aRet;
    aRet.Column = aSrcPos.Col();
    aRet.Row    = aSrcPos.Row();
    aRet.Sheet  = aSrcPos.Tab();
    return aRet;
}

void SAL_CALL ScTableConditionalEntry::setSourcePosition( const table::CellAddress& aSourcePosition )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aSrcPos.Set( aSourcePosition.Column, aSourcePosition.Row, aSourcePosition.Sheet );
    if (pParent)
        pParent->DataChanged();
}

// XSheetConditionalEntry

rtl::OUString SAL_CALL ScTableConditionalEntry::getStyleName() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aStyle;
}

void SAL_CALL ScTableConditionalEntry::setStyleName( const rtl::OUString& aStyleName )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aStyle = String( aStyleName );
    if (pParent)
        pParent->DataChanged();
}

//------------------------------------------------------------------------

ScTableValidationObj::ScTableValidationObj() :
    aPropSet( lcl_GetValidatePropertyMap() )
{
}

ScTableValidationObj::ScTableValidationObj(ScDocument* pDoc, ULONG nKey, BOOL bEnglish) :
    aPropSet( lcl_GetValidatePropertyMap() )
{
    //  Eintrag aus dem Dokument lesen...

    BOOL bFound = FALSE;
    if ( pDoc && nKey )
    {
        const ScValidationData* pData = pDoc->GetValidationEntry( nKey );
        if (pData)
        {
            nMode = pData->GetOperation();
            aSrcPos = pData->GetSrcPos();
            aExpr1 = pData->GetExpression( aSrcPos, 0, 0, bEnglish );
            aExpr2 = pData->GetExpression( aSrcPos, 1, 0, bEnglish );
            nValMode = pData->GetDataMode();
            bIgnoreBlank = pData->IsIgnoreBlank();
            bShowInput = pData->GetInput( aInputTitle, aInputMessage );
            ScValidErrorStyle eStyle;
            bShowError = pData->GetErrMsg( aErrorTitle, aErrorMessage, eStyle );
            nErrorStyle = eStyle;

            bFound = TRUE;
        }
    }
    if (!bFound)
        ClearData_Impl();       // Defaults
}

ScValidationData* ScTableValidationObj::CreateValidationData( ScDocument* pDoc, BOOL bEnglish ) const
{
    //  ScValidationData = Core-Struktur

    ScValidationData* pRet = new ScValidationData( (ScValidationMode)nValMode,
                                                   (ScConditionMode)nMode,
                                                   aExpr1, aExpr2, pDoc, aSrcPos, bEnglish );
    pRet->SetIgnoreBlank(bIgnoreBlank);
    if (bShowInput)
        pRet->SetInput( aInputTitle, aInputMessage );
    else
        pRet->ResetInput();
    if (bShowError)
        pRet->SetError( aErrorTitle, aErrorMessage, (ScValidErrorStyle)nErrorStyle );
    else
        pRet->ResetError();

    return pRet;
}

void ScTableValidationObj::ClearData_Impl()
{
    nMode        = SC_COND_NONE;
    nValMode     = SC_VALID_ANY;
    bIgnoreBlank = TRUE;
    bShowInput   = FALSE;
    bShowError   = FALSE;
    nErrorStyle  = SC_VALERR_STOP;
    aSrcPos.Set(0,0,0);
    aExpr1.Erase();
    aExpr2.Erase();
    aInputTitle.Erase();
    aInputMessage.Erase();
    aErrorTitle.Erase();
    aErrorMessage.Erase();
}

ScTableValidationObj::~ScTableValidationObj()
{
}

void ScTableValidationObj::DataChanged()
{
    //  wenn's mal das "lebende Objekt" ist, muss hier was passieren...
}

// XSheetCondition

sheet::ConditionOperator SAL_CALL ScTableValidationObj::getOperator()
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return lcl_ConditionModeToOperator( (ScConditionMode)nMode );
}

void SAL_CALL ScTableValidationObj::setOperator( sheet::ConditionOperator nOperator )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    nMode = lcl_ConditionOperatorToMode( nOperator );
    DataChanged();
}

rtl::OUString SAL_CALL ScTableValidationObj::getFormula1() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aExpr1;
}

void SAL_CALL ScTableValidationObj::setFormula1( const rtl::OUString& aFormula1 )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aExpr1 = String( aFormula1 );
    DataChanged();
}

rtl::OUString SAL_CALL ScTableValidationObj::getFormula2() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return aExpr2;
}

void SAL_CALL ScTableValidationObj::setFormula2( const rtl::OUString& aFormula2 )
                                                throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aExpr2 = String( aFormula2 );
    DataChanged();
}

table::CellAddress SAL_CALL ScTableValidationObj::getSourcePosition() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    table::CellAddress aRet;
    aRet.Column = aSrcPos.Col();
    aRet.Row    = aSrcPos.Row();
    aRet.Sheet  = aSrcPos.Tab();
    return aRet;
}

void SAL_CALL ScTableValidationObj::setSourcePosition( const table::CellAddress& aSourcePosition )
                                            throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    aSrcPos.Set( aSourcePosition.Column, aSourcePosition.Row, aSourcePosition.Sheet );
    DataChanged();
}

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScTableValidationObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScTableValidationObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aPropertyName;

    if ( aString.EqualsAscii( SC_UNONAME_SHOWINP ) )       bShowInput = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if ( aString.EqualsAscii( SC_UNONAME_SHOWERR ) )  bShowError = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if ( aString.EqualsAscii( SC_UNONAME_IGNOREBL ) ) bIgnoreBlank = ScUnoHelpFunctions::GetBoolFromAny( aValue );
    else if ( aString.EqualsAscii( SC_UNONAME_INPTITLE ) )
    {
        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            aInputTitle = String( aStrVal );
    }
    else if ( aString.EqualsAscii( SC_UNONAME_INPMESS ) )
    {
        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            aInputMessage = String( aStrVal );
    }
    else if ( aString.EqualsAscii( SC_UNONAME_ERRTITLE ) )
    {
        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            aErrorTitle = String( aStrVal );
    }
    else if ( aString.EqualsAscii( SC_UNONAME_ERRMESS ) )
    {
        rtl::OUString aStrVal;
        if ( aValue >>= aStrVal )
            aErrorMessage = String( aStrVal );
    }
    else if ( aString.EqualsAscii( SC_UNONAME_TYPE ) )
    {
        sheet::ValidationType eType = (sheet::ValidationType)
                                ScUnoHelpFunctions::GetEnumFromAny( aValue );
        switch (eType)
        {
            case sheet::ValidationType_ANY:      nValMode = SC_VALID_ANY;     break;
            case sheet::ValidationType_WHOLE:    nValMode = SC_VALID_WHOLE;   break;
            case sheet::ValidationType_DECIMAL:  nValMode = SC_VALID_DECIMAL; break;
            case sheet::ValidationType_DATE:     nValMode = SC_VALID_DATE;    break;
            case sheet::ValidationType_TIME:     nValMode = SC_VALID_TIME;    break;
            case sheet::ValidationType_TEXT_LEN: nValMode = SC_VALID_TEXTLEN; break;
            case sheet::ValidationType_LIST:     nValMode = SC_VALID_LIST;    break;
            case sheet::ValidationType_CUSTOM:   nValMode = SC_VALID_CUSTOM;  break;
        }
    }
    else if ( aString.EqualsAscii( SC_UNONAME_ERRALSTY ) )
    {
        sheet::ValidationAlertStyle eStyle = (sheet::ValidationAlertStyle)
                                ScUnoHelpFunctions::GetEnumFromAny( aValue );
        switch (eStyle)
        {
            case sheet::ValidationAlertStyle_STOP:    nErrorStyle = SC_VALERR_STOP;    break;
            case sheet::ValidationAlertStyle_WARNING: nErrorStyle = SC_VALERR_WARNING; break;
            case sheet::ValidationAlertStyle_INFO:    nErrorStyle = SC_VALERR_INFO;    break;
            case sheet::ValidationAlertStyle_MACRO:   nErrorStyle = SC_VALERR_MACRO;   break;
        }
    }

    DataChanged();
}

uno::Any SAL_CALL ScTableValidationObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aString = aPropertyName;
    uno::Any aRet;

    if ( aString.EqualsAscii( SC_UNONAME_SHOWINP ) )       ScUnoHelpFunctions::SetBoolInAny( aRet, bShowInput );
    else if ( aString.EqualsAscii( SC_UNONAME_SHOWERR ) )  ScUnoHelpFunctions::SetBoolInAny( aRet, bShowError );
    else if ( aString.EqualsAscii( SC_UNONAME_IGNOREBL ) ) ScUnoHelpFunctions::SetBoolInAny( aRet, bIgnoreBlank );
    else if ( aString.EqualsAscii( SC_UNONAME_INPTITLE ) ) aRet <<= rtl::OUString( aInputTitle );
    else if ( aString.EqualsAscii( SC_UNONAME_INPMESS ) )  aRet <<= rtl::OUString( aInputMessage );
    else if ( aString.EqualsAscii( SC_UNONAME_ERRTITLE ) ) aRet <<= rtl::OUString( aErrorTitle );
    else if ( aString.EqualsAscii( SC_UNONAME_ERRMESS ) )  aRet <<= rtl::OUString( aErrorMessage );
    else if ( aString.EqualsAscii( SC_UNONAME_TYPE ) )
    {
        sheet::ValidationType eType = sheet::ValidationType_ANY;
        switch (nValMode)
        {
            case SC_VALID_ANY:      eType = sheet::ValidationType_ANY;      break;
            case SC_VALID_WHOLE:    eType = sheet::ValidationType_WHOLE;    break;
            case SC_VALID_DECIMAL:  eType = sheet::ValidationType_DECIMAL;  break;
            case SC_VALID_DATE:     eType = sheet::ValidationType_DATE;     break;
            case SC_VALID_TIME:     eType = sheet::ValidationType_TIME;     break;
            case SC_VALID_TEXTLEN:  eType = sheet::ValidationType_TEXT_LEN; break;
            case SC_VALID_LIST:     eType = sheet::ValidationType_LIST;     break;
            case SC_VALID_CUSTOM:   eType = sheet::ValidationType_CUSTOM;   break;
        }
        aRet <<= eType;
    }
    else if ( aString.EqualsAscii( SC_UNONAME_ERRALSTY ) )
    {
        sheet::ValidationAlertStyle eStyle = sheet::ValidationAlertStyle_STOP;
        switch (nErrorStyle)
        {
            case SC_VALERR_STOP:    eStyle = sheet::ValidationAlertStyle_STOP;    break;
            case SC_VALERR_WARNING: eStyle = sheet::ValidationAlertStyle_WARNING; break;
            case SC_VALERR_INFO:    eStyle = sheet::ValidationAlertStyle_INFO;    break;
            case SC_VALERR_MACRO:   eStyle = sheet::ValidationAlertStyle_MACRO;   break;
        }
        aRet <<= eStyle;
    }

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScTableValidationObj )

// XUnoTunnel

sal_Int64 SAL_CALL ScTableValidationObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return (sal_Int64)this;
    }
    return 0;
}

// static
const uno::Sequence<sal_Int8>& ScTableValidationObj::getUnoTunnelId()
{
    static uno::Sequence<sal_Int8> * pSeq = 0;
    if( !pSeq )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pSeq )
        {
            static uno::Sequence< sal_Int8 > aSeq( 16 );
            rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
            pSeq = &aSeq;
        }
    }
    return *pSeq;
}

// static
ScTableValidationObj* ScTableValidationObj::getImplementation(
                                const uno::Reference<beans::XPropertySet> xObj )
{
    ScTableValidationObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScTableValidationObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------




