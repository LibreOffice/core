/*************************************************************************
 *
 *  $RCSfile: fielduno.cxx,v $
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
#ifndef _SFXSMPLHINT_HXX //autogen
#include <svtools/smplhint.hxx>
#endif

#pragma hdrstop

#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#include <svx/editeng.hxx>
#include <svx/editobj.hxx>
#include <svx/flditem.hxx>
#include <rtl/uuid.h>

#include "fielduno.hxx"
#include "textuno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "hints.hxx"
#include "editsrc.hxx"
#include "cellsuno.hxx"
#include "servuno.hxx"      // fuer IDs
#include "unoguard.hxx"
#include "unonames.hxx"
#include "editutil.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  alles ohne Which-ID, Map nur fuer PropertySetInfo

const SfxItemPropertyMap* lcl_GetURLPropertyMap()
{
    static SfxItemPropertyMap aURLPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_REPR),     0,  &getCppuType((rtl::OUString*)0),    0},
        {MAP_CHAR_LEN(SC_UNONAME_TARGET),   0,  &getCppuType((rtl::OUString*)0),    0},
        {MAP_CHAR_LEN(SC_UNONAME_URL),      0,  &getCppuType((rtl::OUString*)0),    0},
        {0,0,0,0}
    };
    return aURLPropertyMap_Impl;
}

const SfxItemPropertyMap* lcl_GetHeaderFieldPropertyMap()
{
    static SfxItemPropertyMap aHeaderFieldPropertyMap_Impl[] =
    {
        //! Format bei Dateinamen?
        {0,0,0,0}
    };
    return aHeaderFieldPropertyMap_Impl;
}

//------------------------------------------------------------------------

SC_SIMPLE_SERVICE_INFO( ScCellFieldObj, "ScCellFieldObj", "com.sun.star.text.TextField" )
SC_SIMPLE_SERVICE_INFO( ScCellFieldsObj, "ScCellFieldsObj", "stardiv.unknown" )
SC_SIMPLE_SERVICE_INFO( ScHeaderFieldObj, "ScHeaderFieldObj", "com.sun.star.text.TextField" )
SC_SIMPLE_SERVICE_INFO( ScHeaderFieldsObj, "ScHeaderFieldsObj", "stardiv.unknown" )

//------------------------------------------------------------------------

//  ScUnoEditEngine nur um aus einer EditEngine die Felder herauszubekommen...

enum ScUnoCollectMode
{
    SC_UNO_COLLECT_NONE,
    SC_UNO_COLLECT_COUNT,
    SC_UNO_COLLECT_FINDINDEX,
    SC_UNO_COLLECT_FINDPOS
};

class ScUnoEditEngine : public ScEditEngineDefaulter
{
    ScUnoCollectMode    eMode;
    USHORT              nFieldCount;
    TypeId              aFieldType;
    SvxFieldData*       pFound;         // lokale Kopie
    USHORT              nFieldPar;
    xub_StrLen          nFieldPos;
    USHORT              nFieldIndex;

public:
                ScUnoEditEngine(ScEditEngineDefaulter* pSource);
                ~ScUnoEditEngine();

                    //! nPos should be xub_StrLen
    virtual String  CalcFieldValue( const SvxFieldItem& rField, USHORT nPara, USHORT nPos,
                                    Color*& rTxtColor, Color*& rFldColor );

    USHORT          CountFields(TypeId aType);
    SvxFieldData*   FindByIndex(USHORT nIndex, TypeId aType);
    SvxFieldData*   FindByPos(USHORT nPar, xub_StrLen nPos, TypeId aType);

    USHORT          GetFieldPar() const     { return nFieldPar; }
    xub_StrLen      GetFieldPos() const     { return nFieldPos; }
};

ScUnoEditEngine::ScUnoEditEngine(ScEditEngineDefaulter* pSource) :
    ScEditEngineDefaulter( *pSource ),
    eMode( SC_UNO_COLLECT_NONE ),
    nFieldCount( 0 ),
    aFieldType( NULL ),
    pFound( NULL )
{
    if (pSource)
    {
        EditTextObject* pData = pSource->CreateTextObject();
        SetText( *pData );
        delete pData;
    }
}

ScUnoEditEngine::~ScUnoEditEngine()
{
    delete pFound;
}

String ScUnoEditEngine::CalcFieldValue( const SvxFieldItem& rField,
            USHORT nPara, USHORT nPos, Color*& rTxtColor, Color*& rFldColor )
{
    String aRet = EditEngine::CalcFieldValue( rField, nPara, nPos, rTxtColor, rFldColor );
    if (eMode != SC_UNO_COLLECT_NONE)
    {
        const SvxFieldData* pFieldData = rField.GetField();
        if ( pFieldData )
        {
            if ( !aFieldType || pFieldData->Type() == aFieldType )
            {
                if ( eMode == SC_UNO_COLLECT_FINDINDEX && !pFound && nFieldCount == nFieldIndex )
                {
                    pFound = pFieldData->Clone();
                    nFieldPar = nPara;
                    nFieldPos = nPos;
                }
                if ( eMode == SC_UNO_COLLECT_FINDPOS && !pFound &&
                        nPara == nFieldPar && nPos == nFieldPos )
                {
                    pFound = pFieldData->Clone();
                    nFieldIndex = nFieldCount;
                }
                ++nFieldCount;
            }
        }
    }
    return aRet;
}

USHORT ScUnoEditEngine::CountFields(TypeId aType)
{
    eMode = SC_UNO_COLLECT_COUNT;
    aFieldType = aType;
    nFieldCount = 0;
    UpdateFields();
    aFieldType = NULL;
    eMode = SC_UNO_COLLECT_NONE;

    return nFieldCount;
}

SvxFieldData* ScUnoEditEngine::FindByIndex(USHORT nIndex, TypeId aType)
{
    eMode = SC_UNO_COLLECT_FINDINDEX;
    nFieldIndex = nIndex;
    aFieldType = aType;
    nFieldCount = 0;
    UpdateFields();
    aFieldType = NULL;
    eMode = SC_UNO_COLLECT_NONE;

    return pFound;
}

SvxFieldData* ScUnoEditEngine::FindByPos(USHORT nPar, xub_StrLen nPos, TypeId aType)
{
    eMode = SC_UNO_COLLECT_FINDPOS;
    nFieldPar = nPar;
    nFieldPos = nPos;
    aFieldType = aType;
    nFieldCount = 0;
    UpdateFields();
    aFieldType = NULL;
    eMode = SC_UNO_COLLECT_NONE;

    return pFound;
}

//------------------------------------------------------------------------

ScCellFieldsObj::ScCellFieldsObj(ScDocShell* pDocSh, const ScAddress& rPos) :
    pDocShell( pDocSh ),
    aCellPos( rPos )
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    pEditSource = new ScCellEditSource( pDocShell, aCellPos );
}

ScCellFieldsObj::~ScCellFieldsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    delete pEditSource;
}

void ScCellFieldsObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }

    //  EditSource hat sich selber als Listener angemeldet
}

// XIndexAccess (via XTextFields)

ScCellFieldObj* ScCellFieldsObj::GetObjectByIndex_Impl(INT32 Index) const
{
    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    if ( aTempEngine.FindByIndex( (USHORT)Index, NULL ) )   // in der Zelle ist der Typ egal
    {
        USHORT nPar = aTempEngine.GetFieldPar();
        xub_StrLen nPos = aTempEngine.GetFieldPos();
        ESelection aSelection( nPar, nPos, nPar, nPos+1 );      // Feld ist 1 Zeichen
        return new ScCellFieldObj( pDocShell, aCellPos, aSelection );
    }
    return NULL;
}

sal_Int32 SAL_CALL ScCellFieldsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    return aTempEngine.CountFields(NULL);       // Felder zaehlen, in Zelle ist der Typ egal
}

uno::Any SAL_CALL ScCellFieldsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<text::XTextField> xField = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    aAny <<= xField;
    return aAny;
}

uno::Type SAL_CALL ScCellFieldsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<text::XTextField>*)0);
}

sal_Bool SAL_CALL ScCellFieldsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

uno::Reference<container::XEnumeration> SAL_CALL ScCellFieldsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

void SAL_CALL ScCellFieldsObj::addContainerListener(
                                const uno::Reference<container::XContainerListener>& xListener )
                                    throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellFieldsObj::removeContainerListener(
                                const uno::Reference<container::XContainerListener>& xListener )
                                    throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

//------------------------------------------------------------------------

//  Default-ctor wird fuer SMART_REFLECTION_IMPLEMENTATION gebraucht

ScCellFieldObj::ScCellFieldObj() :
    aPropSet( lcl_GetURLPropertyMap() ),
    pDocShell( NULL )
{
    pEditSource = NULL;
}

ScCellFieldObj::ScCellFieldObj(ScDocShell* pDocSh, const ScAddress& rPos,
                                            const ESelection& rSel) :
    aPropSet( lcl_GetURLPropertyMap() ),
    pDocShell( pDocSh ),
    aCellPos( rPos ),
    aSelection( rSel )
{
    //  pDocShell ist Null, wenn per ServiceProvider erzeugt

    if (pDocShell)
    {
        pDocShell->GetDocument()->AddUnoObject(*this);
        pEditSource = new ScCellEditSource( pDocShell, aCellPos );
    }
    else
        pEditSource = NULL;
}

void ScCellFieldObj::InitDoc( ScDocShell* pDocSh, const ScAddress& rPos,
                                        const ESelection& rSel )
{
    if ( pDocSh && !pEditSource )
    {
        aCellPos = rPos;
        aSelection = rSel;
        pDocShell = pDocSh;

        pDocShell->GetDocument()->AddUnoObject(*this);
        pEditSource = new ScCellEditSource( pDocShell, aCellPos );
    }
}

ScCellFieldObj::~ScCellFieldObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    delete pEditSource;
}

void ScCellFieldObj::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    //! Updates fuer aSelection (muessen erst noch erzeugt werden) !!!!!!

    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;

        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }

    //  EditSource hat sich selber als Listener angemeldet
}

// per getImplementation gerufen:

SvxFieldItem ScCellFieldObj::CreateFieldItem()
{
    DBG_ASSERT( !pEditSource, "CreateFieldItem mit eingefuegtem Feld" );

    SvxURLField aField;
    aField.SetFormat(SVXURLFORMAT_APPDEFAULT);
    aField.SetURL( aUrl );
    aField.SetRepresentation( aRepresentation );
    aField.SetTargetFrame( aTarget );
    return SvxFieldItem( aField );
}

void ScCellFieldObj::DeleteField()
{
    if (pEditSource)
    {
        SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
//      pEditEngine->QuickDelete( aSelection );
        pForwarder->QuickInsertText( String(), aSelection );
        pEditSource->UpdateData();

        aSelection.nEndPara = aSelection.nStartPara;
        aSelection.nEndPos  = aSelection.nStartPos;

        //! Broadcast, um Selektion in anderen Objekten anzupassen
        //! (auch bei anderen Aktionen)
    }
}

// XTextField

rtl::OUString SAL_CALL ScCellFieldObj::getPresentation( sal_Bool bShowCommand )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aRet;

    if (pEditSource)
    {
        //! Feld-Funktionen muessen an den Forwarder !!!
        ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  Typ egal (in Zellen gibts nur URLs)
        SvxFieldData* pField = aTempEngine.FindByPos( aSelection.nStartPara, aSelection.nStartPos, 0 );
        DBG_ASSERT(pField,"getPresentation: Feld nicht gefunden");
        if (pField)
        {
            SvxURLField* pURL = (SvxURLField*)pField;
            if (bShowCommand)
                aRet = pURL->GetURL();
            else
                aRet = pURL->GetRepresentation();
        }
    }

    return aRet;
}

// XTextContent

void SAL_CALL ScCellFieldObj::attach( const uno::Reference<text::XTextRange>& xTextRange )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (xTextRange.is())
    {
        uno::Reference<text::XText> xText = xTextRange->getText();
        if (xText.is())
        {
            xText->insertTextContent( xTextRange, this, TRUE );
        }
    }
}

uno::Reference<text::XTextRange> SAL_CALL ScCellFieldObj::getAnchor() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (pEditSource)
        return (SvxUnoTextRangeBase*)new SvxUnoText(
            pEditSource, ScCellObj::GetEditPropertyMap(), uno::Reference<text::XText>() );
    return NULL;
}

// XComponent

void SAL_CALL ScCellFieldObj::dispose() throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellFieldObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScCellFieldObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& aListener )
                                                    throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellFieldObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScCellFieldObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aNameString = aPropertyName;
    rtl::OUString aStrVal;
    if (pEditSource)
    {
        //! Feld-Funktionen muessen an den Forwarder !!!
        ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  Typ egal (in Zellen gibts nur URLs)
        SvxFieldData* pField = aTempEngine.FindByPos( aSelection.nStartPara, aSelection.nStartPos, 0 );
        DBG_ASSERT(pField,"setPropertyValue: Feld nicht gefunden");
        if (pField)
        {
            SvxURLField* pURL = (SvxURLField*)pField;   // ist eine Kopie in der ScUnoEditEngine

            BOOL bOk = TRUE;
            if ( aNameString.EqualsAscii( SC_UNONAME_URL ) )
            {
                if (aValue >>= aStrVal)
                    pURL->SetURL( aStrVal );
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_REPR ) )
            {
                if (aValue >>= aStrVal)
                    pURL->SetRepresentation( aStrVal );
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_TARGET ) )
            {
                if (aValue >>= aStrVal)
                    pURL->SetTargetFrame( aStrVal );
            }
            else
                bOk = FALSE;

            if (bOk)
            {
                pEditEngine->QuickInsertField( SvxFieldItem(*pField), aSelection );
                pEditSource->UpdateData();
            }
        }
    }
    else        // noch nicht eingefuegt
    {
        if ( aNameString.EqualsAscii( SC_UNONAME_URL ) )
        {
            if (aValue >>= aStrVal)
                aUrl = String( aStrVal );
        }
        else if ( aNameString.EqualsAscii( SC_UNONAME_REPR ) )
        {
            if (aValue >>= aStrVal)
                aRepresentation = String( aStrVal );
        }
        else if ( aNameString.EqualsAscii( SC_UNONAME_TARGET ) )
        {
            if (aValue >>= aStrVal)
                aTarget = String( aStrVal );
        }
    }
}

uno::Any SAL_CALL ScCellFieldObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Any aRet;
    String aNameString = aPropertyName;

    if (pEditSource)
    {
        //! Feld-Funktionen muessen an den Forwarder !!!
        ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  Typ egal (in Zellen gibts nur URLs)
        SvxFieldData* pField = aTempEngine.FindByPos( aSelection.nStartPara, aSelection.nStartPos, 0 );
        DBG_ASSERT(pField,"getPropertyValue: Feld nicht gefunden");
        if (pField)
        {
            SvxURLField* pURL = (SvxURLField*)pField;

            if ( aNameString.EqualsAscii( SC_UNONAME_URL ) )
                aRet <<= rtl::OUString( pURL->GetURL() );
            else if ( aNameString.EqualsAscii( SC_UNONAME_REPR ) )
                aRet <<= rtl::OUString( pURL->GetRepresentation() );
            else if ( aNameString.EqualsAscii( SC_UNONAME_TARGET ) )
                aRet <<= rtl::OUString( pURL->GetTargetFrame() );
        }
    }
    else        // noch nicht eingefuegt
    {
        if ( aNameString.EqualsAscii( SC_UNONAME_URL ) )
            aRet <<= rtl::OUString( aUrl );
        else if ( aNameString.EqualsAscii( SC_UNONAME_REPR ) )
            aRet <<= rtl::OUString( aRepresentation );
        else if ( aNameString.EqualsAscii( SC_UNONAME_TARGET ) )
            aRet <<= rtl::OUString( aTarget );
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScCellFieldObj )

// XUnoTunnel

sal_Int64 SAL_CALL ScCellFieldObj::getSomething(
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
const uno::Sequence<sal_Int8>& ScCellFieldObj::getUnoTunnelId()
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
ScCellFieldObj* ScCellFieldObj::getImplementation(
                                const uno::Reference<text::XTextContent> xObj )
{
    ScCellFieldObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScCellFieldObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------

ScHeaderFieldsObj::ScHeaderFieldsObj(ScHeaderFooterContentObj* pContent, USHORT nP, USHORT nT) :
    pContentObj( pContent ),
    nPart( nP ),
    nType( nT )
{
    DBG_ASSERT( pContentObj, "ScHeaderFieldsObj ohne Objekt?" );

    if (pContentObj)
    {
        pContentObj->acquire();     // darf nicht wegkommen
        pEditSource = new ScHeaderFooterEditSource( pContentObj, nPart );
    }
    else
        pEditSource = NULL;
}

ScHeaderFieldsObj::~ScHeaderFieldsObj()
{
    delete pEditSource;

    if (pContentObj)
        pContentObj->release();
}

// XIndexAccess (via XTextFields)

ScHeaderFieldObj* ScHeaderFieldsObj::GetObjectByIndex_Impl(INT32 Index) const
{
    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = ((ScHeaderFooterEditSource*)pEditSource)->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    TypeId aTypeId = NULL;
    switch (nType)
    {
        case SC_SERVICE_PAGEFIELD:  aTypeId = TYPE(SvxPageField);    break;
        case SC_SERVICE_PAGESFIELD: aTypeId = TYPE(SvxPagesField);   break;
        case SC_SERVICE_DATEFIELD:  aTypeId = TYPE(SvxDateField);    break;
        case SC_SERVICE_TIMEFIELD:  aTypeId = TYPE(SvxTimeField);    break;
        case SC_SERVICE_TITLEFIELD: aTypeId = TYPE(SvxFileField);    break;
        case SC_SERVICE_FILEFIELD:  aTypeId = TYPE(SvxExtFileField); break;
        case SC_SERVICE_SHEETFIELD: aTypeId = TYPE(SvxTableField);   break;
        // bei SC_SERVICE_INVALID bleibt TypeId Null
    }
    SvxFieldData* pData = aTempEngine.FindByIndex( (USHORT)Index, aTypeId );
    if ( pData )
    {
        USHORT nPar = aTempEngine.GetFieldPar();
        xub_StrLen nPos = aTempEngine.GetFieldPos();

        USHORT nFieldType = nType;
        if ( nFieldType == SC_SERVICE_INVALID )
        {
            if ( pData->ISA( SvxPageField ) )         nFieldType = SC_SERVICE_PAGEFIELD;
            else if ( pData->ISA( SvxPagesField ) )   nFieldType = SC_SERVICE_PAGESFIELD;
            else if ( pData->ISA( SvxDateField ) )    nFieldType = SC_SERVICE_DATEFIELD;
            else if ( pData->ISA( SvxTimeField ) )    nFieldType = SC_SERVICE_TIMEFIELD;
            else if ( pData->ISA( SvxFileField ) )    nFieldType = SC_SERVICE_TITLEFIELD;
            else if ( pData->ISA( SvxExtFileField ) ) nFieldType = SC_SERVICE_FILEFIELD;
            else if ( pData->ISA( SvxTableField ) )   nFieldType = SC_SERVICE_SHEETFIELD;
        }

        ESelection aSelection( nPar, nPos, nPar, nPos+1 );      // Feld ist 1 Zeichen
        return new ScHeaderFieldObj( pContentObj, nPart, nType, aSelection );
    }
    return NULL;
}

sal_Int32 SAL_CALL ScHeaderFieldsObj::getCount() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;

    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = ((ScHeaderFooterEditSource*)pEditSource)->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    TypeId aTypeId = NULL;
    switch (nType)
    {
        case SC_SERVICE_PAGEFIELD:  aTypeId = TYPE(SvxPageField);    break;
        case SC_SERVICE_PAGESFIELD: aTypeId = TYPE(SvxPagesField);   break;
        case SC_SERVICE_DATEFIELD:  aTypeId = TYPE(SvxDateField);    break;
        case SC_SERVICE_TIMEFIELD:  aTypeId = TYPE(SvxTimeField);    break;
        case SC_SERVICE_TITLEFIELD: aTypeId = TYPE(SvxFileField);    break;
        case SC_SERVICE_FILEFIELD:  aTypeId = TYPE(SvxExtFileField); break;
        case SC_SERVICE_SHEETFIELD: aTypeId = TYPE(SvxTableField);   break;
    }
    return aTempEngine.CountFields(aTypeId);        // Felder zaehlen
}

uno::Any SAL_CALL ScHeaderFieldsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference<text::XTextField> xField = GetObjectByIndex_Impl(nIndex);
    uno::Any aAny;
    aAny <<= xField;
    return aAny;
}

uno::Type SAL_CALL ScHeaderFieldsObj::getElementType() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return getCppuType((uno::Reference<text::XTextField>*)0);
}

sal_Bool SAL_CALL ScHeaderFieldsObj::hasElements() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return ( getCount() != 0 );
}

uno::Reference<container::XEnumeration> SAL_CALL ScHeaderFieldsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return new ScIndexEnumeration(this);
}

void SAL_CALL ScHeaderFieldsObj::addContainerListener(
                                const uno::Reference<container::XContainerListener>& xListener )
                                    throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScHeaderFieldsObj::removeContainerListener(
                                const uno::Reference<container::XContainerListener>& xListener )
                                    throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

//------------------------------------------------------------------------

//  Default-ctor wird fuer SMART_REFLECTION_IMPLEMENTATION gebraucht

ScHeaderFieldObj::ScHeaderFieldObj() :
    aPropSet( lcl_GetHeaderFieldPropertyMap() ),
    pContentObj( NULL ),
    nPart( 0 ),
    nType( 0 )
{
    pEditSource = NULL;
}

ScHeaderFieldObj::ScHeaderFieldObj(ScHeaderFooterContentObj* pContent, USHORT nP,
                                            USHORT nT, const ESelection& rSel) :
    aPropSet( lcl_GetHeaderFieldPropertyMap() ),
    pContentObj( pContent ),
    nPart( nP ),
    nType( nT ),
    aSelection( rSel )
{
    //  pContent ist Null, wenn per ServiceProvider erzeugt

    if (pContentObj)
    {
        pContentObj->acquire();     // darf nicht wegkommen
        pEditSource = new ScHeaderFooterEditSource( pContentObj, nPart );
    }
    else
        pEditSource = NULL;
}

void ScHeaderFieldObj::InitDoc( ScHeaderFooterContentObj* pContent, USHORT nP,
                                        const ESelection& rSel )
{
    if ( pContent && !pEditSource )
    {
        DBG_ASSERT( !pContentObj, "ContentObj, aber kein EditSource?" );

        aSelection = rSel;
        nPart = nP;
        pContentObj = pContent;

        pContentObj->acquire();     // darf nicht wegkommen
        pEditSource = new ScHeaderFooterEditSource( pContentObj, nPart );
    }
}

ScHeaderFieldObj::~ScHeaderFieldObj()
{
    delete pEditSource;

    if (pContentObj)
        pContentObj->release();
}

// per getImplementation gerufen:

SvxFieldItem ScHeaderFieldObj::CreateFieldItem()
{
    DBG_ASSERT( !pEditSource, "CreateFieldItem mit eingefuegtem Feld" );

    switch (nType)
    {
        case SC_SERVICE_PAGEFIELD:
            {
                SvxPageField aField;
                return SvxFieldItem( aField );
            }
        case SC_SERVICE_PAGESFIELD:
            {
                SvxPagesField aField;
                return SvxFieldItem( aField );
            }
        case SC_SERVICE_DATEFIELD:
            {
                SvxDateField aField;
                return SvxFieldItem( aField );
            }
        case SC_SERVICE_TIMEFIELD:
            {
                SvxTimeField aField;
                return SvxFieldItem( aField );
            }
        case SC_SERVICE_TITLEFIELD:
            {
                SvxFileField aField;
                return SvxFieldItem( aField );
            }
        case SC_SERVICE_FILEFIELD:
            {
                SvxExtFileField aField;
                return SvxFieldItem( aField );
            }
        case SC_SERVICE_SHEETFIELD:
            {
                SvxTableField aField;
                return SvxFieldItem( aField );
            }
    }

    return SvxFieldItem( SvxFieldData() );
}

void ScHeaderFieldObj::DeleteField()
{
    if (pEditSource)
    {
        SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
//      pEditEngine->QuickDelete( aSelection );
        pForwarder->QuickInsertText( String(), aSelection );
        pEditSource->UpdateData();

        aSelection.nEndPara = aSelection.nStartPara;
        aSelection.nEndPos  = aSelection.nStartPos;

        //! Broadcast, um Selektion in anderen Objekten anzupassen
        //! (auch bei anderen Aktionen)
    }
}

// XTextField

rtl::OUString SAL_CALL ScHeaderFieldObj::getPresentation( sal_Bool bShowCommand )
                                                    throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    String aRet;

    if (pEditSource)
    {
        // Feld von der EditEngine formatieren lassen, bShowCommand gibt's nicht

        SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
        aRet = pForwarder->GetText( aSelection );
    }

    return aRet;
}

// XTextContent

void SAL_CALL ScHeaderFieldObj::attach( const uno::Reference<text::XTextRange>& xTextRange )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (xTextRange.is())
    {
        uno::Reference<text::XText> xText = xTextRange->getText();
        if (xText.is())
        {
            xText->insertTextContent( xTextRange, this, TRUE );
        }
    }
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFieldObj::getAnchor() throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    //! Property-Map fuer Kopf-Fusszeilentext
    if (pEditSource)
        return (SvxUnoTextRangeBase*)new SvxUnoText(
            pEditSource, ScCellObj::GetEditPropertyMap(), uno::Reference<text::XText>() );
    return NULL;
}

// XComponent

void SAL_CALL ScHeaderFieldObj::dispose() throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScHeaderFieldObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

void SAL_CALL ScHeaderFieldObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& aListener )
                                                    throw(uno::RuntimeException)
{
    DBG_ERROR("not implemented");
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScHeaderFieldObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef =
        new SfxItemPropertySetInfo( aPropSet.getPropertyMap() );
    return aRef;
}

void SAL_CALL ScHeaderFieldObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    //! Properties?
}

uno::Any SAL_CALL ScHeaderFieldObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    //! Properties?
    uno::Any aRet;
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScHeaderFieldObj )

// XUnoTunnel

sal_Int64 SAL_CALL ScHeaderFieldObj::getSomething(
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
const uno::Sequence<sal_Int8>& ScHeaderFieldObj::getUnoTunnelId()
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
ScHeaderFieldObj* ScHeaderFieldObj::getImplementation(
                                const uno::Reference<text::XTextContent> xObj )
{
    ScHeaderFieldObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = (ScHeaderFieldObj*) xUT->getSomething( getUnoTunnelId() );
    return pRet;
}

//------------------------------------------------------------------------




