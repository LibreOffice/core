/*************************************************************************
 *
 *  $RCSfile: dbfld.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 14:19:12 $
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


#pragma hdrstop

#include <float.h>

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif
#ifndef _ZFORLIST_HXX
#include <svtools/zforlist.hxx>
#endif
#ifndef _SVX_PAGEITEM_HXX
#include <svx/pageitem.hxx>
#endif
#ifndef _SVX_DATACCESSDESCRIPTOR_HXX_
#include <svx/dataaccessdescriptor.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _UNOTOOLS_TRANSLITERATIONWRAPPER_HXX
#include <unotools/transliterationwrapper.hxx>
#endif

#ifndef _FMTFLD_HXX
#include <fmtfld.hxx>
#endif
#ifndef _TXTFLD_HXX
#include <txtfld.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _FRAME_HXX
#include <frame.hxx>
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>
#endif
#ifndef _PAM_HXX
#include <pam.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _DBFLD_HXX
#include <dbfld.hxx>
#endif
#ifndef _DBMGR_HXX
#include <dbmgr.hxx>
#endif
#ifndef _DOCFLD_HXX
#include <docfld.hxx>
#endif
#ifndef _EXPFLD_HXX
#include <expfld.hxx>
#endif
#ifndef _TXTATR_HXX
#include <txtatr.hxx>
#endif
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif


using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star;
using namespace ::rtl;

/*--------------------------------------------------------------------
    Beschreibung: Datenbanktrenner durch Punkte fuer Anzeige ersetzen
 --------------------------------------------------------------------*/

String lcl_DBTrennConv(const String& aContent)
{
    String sTmp(aContent);
    sal_Unicode* pStr = sTmp.GetBufferAccess();
    for( USHORT i = sTmp.Len(); i; --i, ++pStr )
        if( DB_DELIM == *pStr )
            *pStr = '.';
    return sTmp;
}

/*--------------------------------------------------------------------
    Beschreibung: DatenbankFeldTyp
 --------------------------------------------------------------------*/

SwDBFieldType::SwDBFieldType(SwDoc* pDocPtr, const String& rNam, const SwDBData& rDBData ) :
    SwValueFieldType( pDocPtr, RES_DBFLD ),
    aDBData(rDBData),
    nRefCnt(0),
    sColumn(rNam)
{
    if(aDBData.sDataSource.getLength() || aDBData.sCommand.getLength())
    {
        sName =  aDBData.sDataSource;
        sName += DB_DELIM;
        sName += (String)aDBData.sCommand;
        sName += DB_DELIM;
    }
    sName += GetColumnName();
}
//------------------------------------------------------------------------------

SwFieldType* SwDBFieldType::Copy() const
{
    SwDBFieldType* pTmp = new SwDBFieldType(GetDoc(), sColumn, aDBData);
    return pTmp;
}

//------------------------------------------------------------------------------
const String& SwDBFieldType::GetName() const
{
    return sName;
}

//------------------------------------------------------------------------------

void SwDBFieldType::ReleaseRef()
{
    ASSERT(nRefCnt > 0, "RefCount kleiner 0!");

    if (--nRefCnt <= 0)
    {
        USHORT nPos = GetDoc()->GetFldTypes()->GetPos(this);

        if (nPos != USHRT_MAX)
        {
            GetDoc()->RemoveFldType(nPos);
            delete this;
        }
    }
}

/* -----------------24.02.99 14:51-------------------
 *
 * --------------------------------------------------*/
BOOL SwDBFieldType::QueryValue( com::sun::star::uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= aDBData.sCommand;
        break;
    case FIELD_PROP_PAR3:
        rAny <<= OUString(sColumn);
        break;
    case FIELD_PROP_SHORT1:
        rAny <<= aDBData.nCommandType;
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}
/* -----------------24.02.99 14:51-------------------
 *
 * --------------------------------------------------*/
BOOL SwDBFieldType::PutValue( const com::sun::star::uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= aDBData.sCommand;
        break;
    case FIELD_PROP_PAR3:
        {
            String sTmp;
            ::GetString( rAny, sTmp );
            if( sTmp != sColumn )
            {
                sColumn = sTmp;
                SwClientIter aIter( *this );
                SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
                while(pFld)
                {
                    // Feld im Undo?
                    SwTxtFld *pTxtFld = pFld->GetTxtFld();
                    if(pTxtFld && pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
                    {
                        SwDBField* pDBField = (SwDBField*)pFld->GetFld();
                        pDBField->ClearInitialized();
                        pDBField->InitContent();
                     }
                    pFld = (SwFmtFld*)aIter.Next();
                }
            }
        }
        break;
    case FIELD_PROP_SHORT1:
        rAny >>= aDBData.nCommandType;
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}
/*--------------------------------------------------------------------
    Beschreibung: SwDBField
 --------------------------------------------------------------------*/

SwDBField::SwDBField(SwDBFieldType* pTyp, ULONG nFmt)
    :   SwValueField(pTyp, nFmt),
        bValidValue(FALSE),
        bIsInBodyTxt(TRUE),
        bInitialized(FALSE),
        nSubType(0)
{
    if (GetTyp())
        ((SwDBFieldType*)GetTyp())->AddRef();
    InitContent();
}

//------------------------------------------------------------------------------

SwDBField::~SwDBField()
{
    if (GetTyp())
        ((SwDBFieldType*)GetTyp())->ReleaseRef();
}

//------------------------------------------------------------------------------

void SwDBField::InitContent()
{
    if (!IsInitialized())
    {
        aContent = '<';
        aContent += ((SwDBFieldType*)GetTyp())->GetColumnName();
        aContent += '>';
    }
}

//------------------------------------------------------------------------------

void SwDBField::InitContent(const String& rExpansion)
{
    if (rExpansion.Len() > 2)
    {
        if (rExpansion.GetChar(0) == '<' &&
            rExpansion.GetChar(rExpansion.Len() - 1) == '>')
        {
            String sColumn( rExpansion.Copy( 1, rExpansion.Len() - 2 ) );
            if( ::GetAppCmpStrIgnore().isEqual( sColumn,
                            ((SwDBFieldType *)GetTyp())->GetColumnName() ))
            {
                InitContent();
                return;
            }
        }
    }
    SetExpansion( rExpansion );
}

//------------------------------------------------------------------------------

String SwDBField::GetOldContent()
{
    String sOldExpand = Expand();
    String sNewExpand = sOldExpand;
    BOOL bOldInit = bInitialized;

    bInitialized = FALSE;
    InitContent();
    bInitialized = bOldInit;

    if( ::GetAppCmpStrIgnore().isEqual( sNewExpand, Expand() ) )
    {
        sNewExpand = '<';
        sNewExpand += ((SwDBFieldType *)GetTyp())->GetColumnName();
        sNewExpand += '>';
    }
    SetExpansion( sOldExpand );

    return sNewExpand;
}

//------------------------------------------------------------------------------

String SwDBField::Expand() const
{
    String sRet;

    if(0 ==(GetSubType() & SUB_INVISIBLE))
        sRet = lcl_DBTrennConv(aContent);
    return sRet;
}

//------------------------------------------------------------------------------

SwField* SwDBField::Copy() const
{
    SwDBField *pTmp = new SwDBField((SwDBFieldType*)GetTyp(), GetFormat());
    pTmp->aContent      = aContent;
    pTmp->bIsInBodyTxt  = bIsInBodyTxt;
    pTmp->bValidValue   = bValidValue;
    pTmp->bInitialized  = bInitialized;
    pTmp->nSubType      = nSubType;
    pTmp->SetValue(GetValue());

    return pTmp;
}

String SwDBField::GetCntnt(BOOL bName) const
{
    if(bName)
    {
        const String& rDBName = ((SwDBFieldType*)GetTyp())->GetName();
        String sContent( SFX_APP()->LocalizeDBName(INI2NATIONAL,
                                            rDBName.GetToken(0, DB_DELIM)));

        if (sContent.Len() > 1)
        {
            sContent += DB_DELIM;
            sContent += rDBName.GetToken(1, DB_DELIM);
            sContent += DB_DELIM;
            sContent += rDBName.GetToken(2, DB_DELIM);
        }
        return lcl_DBTrennConv(sContent);
    }
    return Expand();
}

//------------------------------------------------------------------------------

void SwDBField::ChgValue( double d, BOOL bVal )
{
    bValidValue = bVal;
    SetValue(d);

    if( bValidValue )
        aContent = ((SwValueFieldType*)GetTyp())->ExpandValue(d, GetFormat(), GetLanguage());
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

SwFieldType* SwDBField::ChgTyp( SwFieldType* pNewType )
{
    SwFieldType* pOld = SwValueField::ChgTyp( pNewType );

    ((SwDBFieldType*)pNewType)->AddRef();
    ((SwDBFieldType*)pOld)->ReleaseRef();

    return pOld;
}

/*--------------------------------------------------------------------
    Beschreibung: Aktuellen Field-Value holen und chachen
 --------------------------------------------------------------------*/

void SwDBField::Evaluate()
{
    SwNewDBMgr* pMgr = GetDoc()->GetNewDBMgr();

    // erstmal loeschen
    bValidValue = FALSE;
    double nValue = DBL_MAX;
    const SwDBData& aTmpData = GetDBData();

    if(!pMgr || !pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, sal_True))
        return ;

    ULONG nFmt;

    // Passenden Spaltennamen suchen
    String aColNm( ((SwDBFieldType*)GetTyp())->GetColumnName() );

    SvNumberFormatter* pDocFormatter = GetDoc()->GetNumberFormatter();
    pMgr->GetMergeColumnCnt(aColNm, GetLanguage(), aContent, &nValue, &nFmt);
    if( !( nSubType & SUB_OWN_FMT ) )
        SetFormat( nFmt = pMgr->GetColumnFmt( aTmpData.sDataSource, aTmpData.sCommand,
                                        aColNm, pDocFormatter, GetLanguage() ));

    if( DBL_MAX != nValue )
    {
        sal_Int32 nColumnType = pMgr->GetColumnType(aTmpData.sDataSource, aTmpData.sCommand, aColNm);
        if( DataType::DATE == nColumnType  || DataType::TIME == nColumnType  ||
                 DataType::TIMESTAMP  == nColumnType)

        {
            Date aStandard(1,1,1900);
            if (*pDocFormatter->GetNullDate() != aStandard)
                nValue += (aStandard - *pDocFormatter->GetNullDate());
        }
        bValidValue = TRUE;
        SetValue(nValue);
        aContent = ((SwValueFieldType*)GetTyp())->ExpandValue(nValue, GetFormat(), GetLanguage());
    }
    else
    {
        SwSbxValue aVal;
        aVal.PutString( aContent );

        if (aVal.IsNumeric())
        {
            SetValue(aVal.GetDouble());

            SvNumberFormatter* pFormatter = GetDoc()->GetNumberFormatter();
            if (nFmt && nFmt != ULONG_MAX && !pFormatter->IsTextFormat(nFmt))
                bValidValue = TRUE; // Wegen Bug #60339 nicht mehr bei allen Strings
        }
        else
        {
            // Bei Strings TRUE wenn Laenge > 0 sonst FALSE
            SetValue(aContent.Len() ? 1 : 0);
        }
    }
    bInitialized = TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung: Namen erfragen
 --------------------------------------------------------------------*/

const String& SwDBField::GetPar1() const
{
    return ((SwDBFieldType*)GetTyp())->GetName();
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

USHORT SwDBField::GetSubType() const
{
    return nSubType;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

void SwDBField::SetSubType(USHORT nType)
{
    nSubType = nType;
}

/*-----------------06.03.98 16:15-------------------

--------------------------------------------------*/
BOOL SwDBField::QueryValue( com::sun::star::uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_BOOL1:
        {
            BOOL bTemp = 0 == (GetSubType()&SUB_OWN_FMT);
            rAny.setValue(&bTemp, ::getBooleanCppuType());
        }
        break;
    case FIELD_PROP_BOOL2:
    {
        sal_Bool bVal = 0 == (GetSubType() & SUB_INVISIBLE);
        rAny.setValue(&bVal, ::getBooleanCppuType());
    }
    break;
    case FIELD_PROP_FORMAT:
        rAny <<= (sal_Int32)GetFormat();
        break;
    case FIELD_PROP_PAR1:
        rAny <<= OUString(aContent);
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;

}
/*-----------------06.03.98 16:15-------------------

--------------------------------------------------*/
BOOL SwDBField::PutValue( const com::sun::star::uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_BOOL1:
        if( *(sal_Bool*)rAny.getValue() )
            SetSubType(GetSubType()&~SUB_OWN_FMT);
        else
            SetSubType(GetSubType()|SUB_OWN_FMT);
        break;
    case FIELD_PROP_BOOL2:
    {
        USHORT nSubType = GetSubType();
        sal_Bool bVisible;
        if(!(rAny >>= bVisible))
            return FALSE;
        if(bVisible)
            nSubType &= ~SUB_INVISIBLE;
        else
            nSubType |= SUB_INVISIBLE;
        SetSubType(nSubType);
        //invalidate text node
        if(GetTyp())
        {
            SwClientIter aIter( *GetTyp() );
            SwFmtFld* pFld = (SwFmtFld*)aIter.First( TYPE( SwFmtFld ));
            while(pFld)
            {
                SwTxtFld *pTxtFld = pFld->GetTxtFld();
                if(pTxtFld && (SwDBField*)pFld->GetFld() == this )
                {
                    //notify the change
                    pTxtFld->NotifyContentChange(*pFld);
                    break;
                }
                pFld = (SwFmtFld*)aIter.Next();
            }
        }
    }
    break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nTemp;
            rAny >>= nTemp;
            SetFormat(nTemp);
        }
        break;
    case FIELD_PROP_PAR1:
        ::GetString( rAny, aContent );
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}

/*--------------------------------------------------------------------
    Beschreibung: Basisklasse fuer alle weiteren Datenbankfelder
 --------------------------------------------------------------------*/

SwDBNameInfField::SwDBNameInfField(SwFieldType* pTyp, const SwDBData& rDBData, ULONG nFmt) :
    SwField(pTyp, nFmt),
    aDBData(rDBData),
    nSubType(0)
{
}

//------------------------------------------------------------------------------

SwDBData SwDBNameInfField::GetDBData(SwDoc* pDoc)
{
    SwDBData aRet;
    if(aDBData.sDataSource.getLength())
        aRet = aDBData;
    else
        aRet = pDoc->GetDBData();
    return aRet;
}

// #111840#
void SwDBNameInfField::SetDBData(const SwDBData & rDBData)
{
    aDBData = rDBData;
}

//------------------------------------------------------------------------------

String SwDBNameInfField::GetCntnt(BOOL bName) const
{
    String sStr(SwField::GetCntnt(bName));

    if(bName)
    {
        if (aDBData.sDataSource.getLength())
        {
            sStr += ':';
            sStr += String(aDBData.sDataSource);
            sStr += DB_DELIM;
            sStr += String(aDBData.sCommand);
        }
    }
    return lcl_DBTrennConv(sStr);
}

/*-----------------06.03.98 16:55-------------------

--------------------------------------------------*/
BOOL SwDBNameInfField::QueryValue( com::sun::star::uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= aDBData.sCommand;
        break;
    case FIELD_PROP_SHORT1:
        rAny <<= aDBData.nCommandType;
        break;
    case FIELD_PROP_BOOL2:
    {
        sal_Bool bVal = 0 == (GetSubType() & SUB_INVISIBLE);
        rAny.setValue(&bVal, ::getBooleanCppuType());
    }
    break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}
/*-----------------06.03.98 16:55-------------------

--------------------------------------------------*/
BOOL SwDBNameInfField::PutValue( const com::sun::star::uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= aDBData.sDataSource;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= aDBData.sCommand;
        break;
    case FIELD_PROP_SHORT1:
        rAny >>= aDBData.nCommandType;
        break;
    case FIELD_PROP_BOOL2:
    {
        USHORT nSubType = GetSubType();
        sal_Bool bVisible;
        if(!(rAny >>= bVisible))
            return FALSE;
        if(bVisible)
            nSubType &= ~SUB_INVISIBLE;
        else
            nSubType |= SUB_INVISIBLE;
        SetSubType(nSubType);
    }
    break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}
/* -----------------4/10/2003 15:03------------------

 --------------------------------------------------*/
USHORT SwDBNameInfField::GetSubType() const
{
    return nSubType;
}
/* -----------------4/10/2003 15:03------------------

 --------------------------------------------------*/
void SwDBNameInfField::SetSubType(USHORT nType)
{
    nSubType = nType;
}

/*--------------------------------------------------------------------
    Beschreibung: NaechsterDatensatz
 --------------------------------------------------------------------*/

SwDBNextSetFieldType::SwDBNextSetFieldType()
    : SwFieldType( RES_DBNEXTSETFLD )
{
}

//------------------------------------------------------------------------------

SwFieldType* SwDBNextSetFieldType::Copy() const
{
    SwDBNextSetFieldType* pTmp = new SwDBNextSetFieldType();
    return pTmp;
}
/*--------------------------------------------------------------------
    Beschreibung: SwDBSetField
 --------------------------------------------------------------------*/

SwDBNextSetField::SwDBNextSetField(SwDBNextSetFieldType* pTyp,
                                   const String& rCond,
                                   const String& rDummy,
                                   const SwDBData& rDBData) :
    SwDBNameInfField(pTyp, rDBData), aCond(rCond), bCondValid(TRUE)
{}

//------------------------------------------------------------------------------

String SwDBNextSetField::Expand() const
{
    return aEmptyStr;
}

//------------------------------------------------------------------------------

SwField* SwDBNextSetField::Copy() const
{
    SwDBNextSetField *pTmp = new SwDBNextSetField((SwDBNextSetFieldType*)GetTyp(),
                                         aCond, aEmptyStr, GetDBData());
    pTmp->SetSubType(GetSubType());
    pTmp->bCondValid = bCondValid;
    return pTmp;
}
//------------------------------------------------------------------------------

void SwDBNextSetField::Evaluate(SwDoc* pDoc)
{
    SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();
    const SwDBData& rData = GetDBData();
    if( !bCondValid ||
            !pMgr || !pMgr->IsDataSourceOpen(rData.sDataSource, rData.sCommand, sal_False))
        return ;
    pMgr->ToNextRecord(rData.sDataSource, rData.sCommand);
}

/*--------------------------------------------------------------------
    Beschreibung: Bedingung
 --------------------------------------------------------------------*/

const String& SwDBNextSetField::GetPar1() const
{
    return aCond;
}

void SwDBNextSetField::SetPar1(const String& rStr)
{
    aCond = rStr;
}
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
BOOL SwDBNextSetField::QueryValue( uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    BOOL bRet = TRUE;
    switch( nMId )
    {
    case FIELD_PROP_PAR3:
        rAny <<= OUString(aCond);
        break;
    default:
        bRet = SwDBNameInfField::QueryValue( rAny, nMId );
    }
    return bRet;
}
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
BOOL SwDBNextSetField::PutValue( const com::sun::star::uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    BOOL bRet = TRUE;
    switch( nMId )
    {
    case FIELD_PROP_PAR3:
        ::GetString( rAny, aCond );
        break;
    default:
        bRet = SwDBNameInfField::PutValue( rAny, nMId );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/
/*
String SwDBNextSetField::GetPar2() const
{
    return GetDBName();
}

void SwDBNextSetField::SetPar2(const String& rStr)
{
    GetDBName() = rStr;
}
*/

/*--------------------------------------------------------------------
    Beschreibung: Datensatz mit bestimmter ID
 --------------------------------------------------------------------*/

SwDBNumSetFieldType::SwDBNumSetFieldType() :
    SwFieldType( RES_DBNUMSETFLD )
{
}

//------------------------------------------------------------------------------

SwFieldType* SwDBNumSetFieldType::Copy() const
{
    SwDBNumSetFieldType* pTmp = new SwDBNumSetFieldType();
    return pTmp;
}

/*--------------------------------------------------------------------
    Beschreibung: SwDBSetField
 --------------------------------------------------------------------*/

SwDBNumSetField::SwDBNumSetField(SwDBNumSetFieldType* pTyp,
                                 const String& rCond,
                                 const String& rDBNum,
                                 const SwDBData& rDBData) :
    SwDBNameInfField(pTyp, rDBData),
    aCond(rCond),
    aPar2(rDBNum),
    bCondValid(TRUE)
{}

//------------------------------------------------------------------------------

String SwDBNumSetField::Expand() const
{
    return aEmptyStr;
}

//------------------------------------------------------------------------------

SwField* SwDBNumSetField::Copy() const
{
    SwDBNumSetField *pTmp = new SwDBNumSetField((SwDBNumSetFieldType*)GetTyp(),
                                         aCond, aPar2, GetDBData());
    pTmp->bCondValid = bCondValid;
    pTmp->SetSubType(GetSubType());
    return pTmp;
}

void SwDBNumSetField::Evaluate(SwDoc* pDoc)
{
    SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();
    const SwDBData& aTmpData = GetDBData();

    if( bCondValid && pMgr && pMgr->IsInMerge() &&
                        pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, sal_True))
    {   // Bedingug OK -> aktuellen Set einstellen
        pMgr->ToRecordId(Max((USHORT)aPar2.ToInt32(), USHORT(1))-1);
    }
}

/*--------------------------------------------------------------------
    Beschreibung: LogDBName
 --------------------------------------------------------------------*/

const String& SwDBNumSetField::GetPar1() const
{
    return aCond;
}

void SwDBNumSetField::SetPar1(const String& rStr)
{
    aCond = rStr;
}

/*--------------------------------------------------------------------
    Beschreibung: Bedingung
 --------------------------------------------------------------------*/

String SwDBNumSetField::GetPar2() const
{
    return aPar2;
}

void SwDBNumSetField::SetPar2(const String& rStr)
{
    aPar2 = rStr;
}
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
BOOL SwDBNumSetField::QueryValue( com::sun::star::uno::Any& rAny, BYTE nMId ) const
{
    nMId &= ~CONVERT_TWIPS;
    BOOL bRet = TRUE;
    switch( nMId )
    {
    case FIELD_PROP_PAR3:
        rAny <<= OUString(aCond);
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= (sal_Int32)aPar2.ToInt32();
        break;
    default:
        bRet = SwDBNameInfField::QueryValue(rAny, nMId );
    }
    return bRet;
}
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
BOOL    SwDBNumSetField::PutValue( const com::sun::star::uno::Any& rAny, BYTE nMId )
{
    nMId &= ~CONVERT_TWIPS;
    BOOL bRet = TRUE;
    switch( nMId )
    {
    case FIELD_PROP_PAR3:
        ::GetString( rAny, aCond );
        break;
    case FIELD_PROP_FORMAT:
        {
            sal_Int32 nVal;
            rAny >>= nVal;
            aPar2 = String::CreateFromInt32(nVal);
        }
        break;
    default:
        bRet = SwDBNameInfField::PutValue(rAny, nMId );
    }
    return bRet;
}

/*--------------------------------------------------------------------
    Beschreibung: SwDBNameFieldType
 --------------------------------------------------------------------*/

SwDBNameFieldType::SwDBNameFieldType(SwDoc* pDocument)
    : SwFieldType( RES_DBNAMEFLD )
{
    pDoc = pDocument;
}
//------------------------------------------------------------------------------

String SwDBNameFieldType::Expand(ULONG nFmt) const
{
    ASSERT( nFmt >= FF_BEGIN && nFmt < FF_END, "Expand: kein guelt. Fmt!" );
    const SwDBData aData = pDoc->GetDBData();
    String sRet(aData.sDataSource);
    sRet += '.';
    sRet += (String)aData.sCommand;
    return sRet;
}
//------------------------------------------------------------------------------

SwFieldType* SwDBNameFieldType::Copy() const
{
    SwDBNameFieldType *pTmp = new SwDBNameFieldType(pDoc);
    return pTmp;
}

//------------------------------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung: Name der angedockten DB
 --------------------------------------------------------------------*/

SwDBNameField::SwDBNameField(SwDBNameFieldType* pTyp, const SwDBData& rDBData, ULONG nFmt)
    : SwDBNameInfField(pTyp, rDBData, nFmt)
{}

//------------------------------------------------------------------------------

String SwDBNameField::Expand() const
{
    String sRet;
    if(0 ==(GetSubType() & SUB_INVISIBLE))
        sRet = ((SwDBNameFieldType*)GetTyp())->Expand(GetFormat());
    return sRet;
}

//------------------------------------------------------------------------------

SwField* SwDBNameField::Copy() const
{
    SwDBNameField *pTmp = new SwDBNameField((SwDBNameFieldType*)GetTyp(), GetDBData());
    pTmp->ChangeFormat(GetFormat());
    pTmp->SetLanguage(GetLanguage());
    pTmp->SetSubType(GetSubType());
    return pTmp;
}

/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
BOOL SwDBNameField::QueryValue( com::sun::star::uno::Any& rAny, BYTE nMId ) const
{
    return SwDBNameInfField::QueryValue(rAny, nMId );
}
/*-----------------06.03.98 16:16-------------------

--------------------------------------------------*/
BOOL SwDBNameField::PutValue( const com::sun::star::uno::Any& rAny, BYTE nMId )
{
    return SwDBNameInfField::PutValue(rAny, nMId );
}
/*--------------------------------------------------------------------
    Beschreibung: SwDBNameFieldType
 --------------------------------------------------------------------*/

SwDBSetNumberFieldType::SwDBSetNumberFieldType()
    : SwFieldType( RES_DBSETNUMBERFLD )
{
}

//------------------------------------------------------------------------------

SwFieldType* SwDBSetNumberFieldType::Copy() const
{
    SwDBSetNumberFieldType *pTmp = new SwDBSetNumberFieldType;
    return pTmp;
}

//------------------------------------------------------------------------------

/*--------------------------------------------------------------------
    Beschreibung: SetNumber der angedockten DB
 --------------------------------------------------------------------*/

SwDBSetNumberField::SwDBSetNumberField(SwDBSetNumberFieldType* pTyp,
                                       const SwDBData& rDBData,
                                       ULONG nFmt)
    : SwDBNameInfField(pTyp, rDBData, nFmt), nNumber(0)
{}

//------------------------------------------------------------------------------

String SwDBSetNumberField::Expand() const
{
    if(0 !=(GetSubType() & SUB_INVISIBLE) || nNumber == 0)
        return aEmptyStr;
    else
        return FormatNumber((USHORT)nNumber, GetFormat());
    //return(nNumber == 0 ? aEmptyStr : FormatNumber(nNumber, GetFormat()));
}

//------------------------------------------------------------------------------

void SwDBSetNumberField::Evaluate(SwDoc* pDoc)
{
    SwNewDBMgr* pMgr = pDoc->GetNewDBMgr();

    const SwDBData& aTmpData = GetDBData();
    if (!pMgr || !pMgr->IsInMerge() ||
        !pMgr->IsDataSourceOpen(aTmpData.sDataSource, aTmpData.sCommand, sal_False))
        return;
    nNumber = pMgr->GetSelectedRecordId();
}


//------------------------------------------------------------------------------

SwField* SwDBSetNumberField::Copy() const
{
    SwDBSetNumberField *pTmp =
        new SwDBSetNumberField((SwDBSetNumberFieldType*)GetTyp(), GetDBData(), GetFormat());
    pTmp->SetLanguage(GetLanguage());
    pTmp->SetSetNumber(nNumber);
    pTmp->SetSubType(GetSubType());
    return pTmp;
}
/*-----------------06.03.98 16:15-------------------

--------------------------------------------------*/
BOOL SwDBSetNumberField::QueryValue( com::sun::star::uno::Any& rAny, BYTE nMId ) const
{
    BOOL bRet = TRUE;
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_USHORT1:
        rAny <<= (sal_Int16)GetFormat();
        break;
    case FIELD_PROP_FORMAT:
        rAny <<= nNumber;
        break;
    default:
        bRet = SwDBNameInfField::QueryValue( rAny, nMId );
    }
    return bRet;
}
/*-----------------06.03.98 16:15-------------------

--------------------------------------------------*/
BOOL SwDBSetNumberField::PutValue( const com::sun::star::uno::Any& rAny, BYTE nMId )
{
    BOOL bRet = TRUE;
    nMId &= ~CONVERT_TWIPS;
    switch( nMId )
    {
    case FIELD_PROP_USHORT1:
        {
            sal_Int16 nSet;
            rAny >>= nSet;
            if(nSet < (INT16) SVX_NUMBER_NONE )
                SetFormat(nSet);
            else
                //exception(wrong_value)
                ;
        }
        break;
    case FIELD_PROP_FORMAT:
        rAny >>= nNumber;
        break;
    default:
        bRet = SwDBNameInfField::PutValue( rAny, nMId );
    }
    return bRet;
}


