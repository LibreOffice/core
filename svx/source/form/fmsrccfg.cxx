/*************************************************************************
 *
 *  $RCSfile: fmsrccfg.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:17 $
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

#ifndef _FMSRCCF_HXX_
#include "fmsrccfg.hxx"
#endif // _FMSRCCF_HXX_

#ifndef _SFXFILEREC_HXX //autogen
#include <svtools/filerec.hxx>
#endif

// ===================================================================================================
// = struct FmSearchParams - Parameter einer Suche
// ===================================================================================================

//------------------------------------------------------------------------
BOOL FmSearchParams::operator ==(const FmSearchParams& rComp) const
{
    if (!strHistory.Equals(rComp.strHistory))
        return FALSE;
    if (nSearchForType != rComp.nSearchForType)
        return FALSE;
    if (bAllFields != rComp.bAllFields)
        return FALSE;
    if (nPosition != rComp.nPosition)
        return FALSE;
    if (bUseFormatter != rComp.bUseFormatter)
        return FALSE;
    if (bCaseSensitive != rComp.bCaseSensitive)
        return FALSE;
    if (bBackwards != rComp.bBackwards)
        return FALSE;
    if (bWildcard != rComp.bWildcard)
        return FALSE;
    if (bRegular != rComp.bRegular)
        return FALSE;
    if (bApproxSearch != rComp.bApproxSearch)
        return FALSE;
    if (bLevRelaxed != rComp.bLevRelaxed)
        return FALSE;
    if (nLevOther != rComp.nLevOther)
        return FALSE;
    if (nLevShorter != rComp.nLevShorter)
        return FALSE;
    if (nLevLonger != rComp.nLevLonger)
        return FALSE;

    // strSingleSearchField wird nicht mit verglichen : dieser Operator ist nur fuer die persistenten Eigenschaften gedacht,
    // und strSingleSearchField ist nicht persistent
    return TRUE;
}

// ===================================================================================================
// = class FmSearchConfigItem - ein ConfigItem, dass sich Suchparameter merkt
// ===================================================================================================

#define FM_SEARCH_TAG_PARAMS                1
#define FM_SEARCH_CONFIG_VERSION_SO50       1       // nur in internen Versionen und der 5.1 beta benutzt worden
#define FM_SEARCH_CONFIG_VERSION_SO51       2       // zur 5.1 eingesetzt
#define FM_SEARCH_CONFIG_VERSION_SO52       3       // ab der SRC557

//------------------------------------------------------------------------
FmSearchConfigItem::FmSearchConfigItem() : SfxConfigItem(SFX_ITEMTYPE_SVXSEARCHPARAMS)
{
}

//------------------------------------------------------------------------
INT32 FmSearchConfigItem::CalcCheckSum(const FmSearchParams& aParams, short nFormatVersion)
{
    // ziemlich billig, aber es reicht ...
    sal_Int32 nReturn = 0;
    for (xub_StrLen i=0; i<aParams.strHistory.Len(); ++i)
        nReturn += (sal_Int32)aParams.strHistory.GetChar(i);

    nReturn += aParams.bAllFields;
    nReturn += aParams.nPosition;
    nReturn += aParams.bUseFormatter;
    nReturn += aParams.bCaseSensitive;
    nReturn += aParams.bBackwards;
    nReturn += aParams.bWildcard;
    nReturn += aParams.bRegular;
    nReturn += aParams.bApproxSearch;
    nReturn += aParams.bLevRelaxed;
    nReturn += aParams.nLevOther;
    nReturn += aParams.nLevShorter;
    nReturn += aParams.nLevLonger;

    if (nFormatVersion >= FM_SEARCH_CONFIG_VERSION_SO52)
        nReturn += aParams.nSearchForType;

    return nReturn;
}

//------------------------------------------------------------------------
int FmSearchConfigItem::Load(SvStream& rStore)
{
    SfxSingleRecordReader aRecord(&rStore);

    if (aRecord.GetTag() == FM_SEARCH_TAG_PARAMS)
    {
        if (aRecord.HasVersion(FM_SEARCH_CONFIG_VERSION_SO51))  // 5.1 oder groesser
        {
            INT32 nCheckSum;
            *aRecord >> nCheckSum;

            FmSearchParams aParams;
            aParams.strSingleSearchField.Erase();

            (*aRecord).ReadByteString(aParams.strHistory, gsl_getSystemTextEncoding());

            INT16 nBitField;
            *aRecord >> nBitField;
            nBitField >>= 0; aParams.bAllFields     = nBitField & 0x0001;
            nBitField >>= 1; aParams.nPosition      = nBitField & 0x0003;
            nBitField >>= 2; aParams.bUseFormatter  = nBitField & 0x0001;
            nBitField >>= 1; aParams.bCaseSensitive = nBitField & 0x0001;
            nBitField >>= 1; aParams.bBackwards     = nBitField & 0x0001;
            nBitField >>= 1; aParams.bWildcard      = nBitField & 0x0001;
            nBitField >>= 1; aParams.bRegular       = nBitField & 0x0001;
            nBitField >>= 1; aParams.bApproxSearch  = nBitField & 0x0001;
            nBitField >>= 1; aParams.bLevRelaxed    = nBitField & 0x0001;

            *aRecord >> aParams.nLevOther;
            *aRecord >> aParams.nLevShorter;
            *aRecord >> aParams.nLevLonger;

            if (aRecord.HasVersion(FM_SEARCH_CONFIG_VERSION_SO52))  // 5.2 oder groesser
            {
                nBitField >>= 1; aParams.nSearchForType     = nBitField & 0x0003;
            }
            else
                aParams.nSearchForType = 0;


            if (nCheckSum == CalcCheckSum(aParams, aRecord.GetVersion()))
            {
                m_aParams = aParams;
                return ERR_OK;
            }
        }
        else if (aRecord.HasVersion(FM_SEARCH_CONFIG_VERSION_SO50))
        {
            INT32 nCheckSum;
            *aRecord >> nCheckSum;

            FmSearchParams aParams;
            aParams.strSingleSearchField.Erase();

            (*aRecord).ReadByteString(aParams.strHistory, gsl_getSystemTextEncoding());

            // kleine Verrenkung, da die Bools alle BitFields sind
            BOOL bTempBool;
            *aRecord >> bTempBool;  aParams.bAllFields = bTempBool;
            *aRecord >> bTempBool;  aParams.bUseFormatter = bTempBool;
            *aRecord >> bTempBool;  aParams.bCaseSensitive = bTempBool;
            *aRecord >> bTempBool;  aParams.bBackwards = bTempBool;
            *aRecord >> bTempBool;  aParams.bWildcard = bTempBool;
            *aRecord >> bTempBool;  aParams.bRegular = bTempBool;
            *aRecord >> bTempBool;  aParams.bApproxSearch = bTempBool;
            *aRecord >> bTempBool;  aParams.bLevRelaxed = bTempBool;

            *aRecord >> aParams.nLevOther;
            *aRecord >> aParams.nLevShorter;
            *aRecord >> aParams.nLevLonger;

            USHORT nTempShort;
            *aRecord >> nTempShort; aParams.nPosition = nTempShort;

            if (nCheckSum == CalcCheckSum(aParams, FM_SEARCH_CONFIG_VERSION_SO50))
            {
                m_aParams = aParams;
                return ERR_OK;
            }
        }
    }

    UseDefault();
    return ERR_READ;
}

//------------------------------------------------------------------------
BOOL FmSearchConfigItem::Store(SvStream& rStore)
{
    SfxSingleRecordWriter aRecord(&rStore, FM_SEARCH_TAG_PARAMS, FM_SEARCH_CONFIG_VERSION_SO52);

    // eine Checksumme an den Anfang
    INT32 nCheckSum = CalcCheckSum(m_aParams, FM_SEARCH_CONFIG_VERSION_SO52);
    *aRecord << nCheckSum;

    (*aRecord).WriteByteString(m_aParams.strHistory, gsl_getSystemTextEncoding());

    INT16 nBitField = 0;
    nBitField |= (INT16(m_aParams.bAllFields        & 0x0001)) << 0;
    nBitField |= (INT16(m_aParams.nPosition         & 0x0003)) << 1;
    nBitField |= (INT16(m_aParams.bUseFormatter     & 0x0001)) << 3;
    nBitField |= (INT16(m_aParams.bCaseSensitive    & 0x0001)) << 4;
    nBitField |= (INT16(m_aParams.bBackwards        & 0x0001)) << 5;
    nBitField |= (INT16(m_aParams.bWildcard         & 0x0001)) << 6;
    nBitField |= (INT16(m_aParams.bRegular          & 0x0001)) << 7;
    nBitField |= (INT16(m_aParams.bApproxSearch     & 0x0001)) << 8;
    nBitField |= (INT16(m_aParams.bLevRelaxed       & 0x0001)) << 9;
    nBitField |= (INT16(m_aParams.nSearchForType    & 0x0003)) << 10;
        // because of nSearchForType this is format version 5.2

    *aRecord << nBitField;

    *aRecord << m_aParams.nLevOther;
    *aRecord << m_aParams.nLevShorter;
    *aRecord << m_aParams.nLevLonger;

    return TRUE;
}

//------------------------------------------------------------------------
void FmSearchConfigItem::UseDefault()
{
    m_aParams.strHistory.Erase();
    m_aParams.strSingleSearchField = String();
    m_aParams.bAllFields = FALSE;
    m_aParams.nPosition = MATCHING_ANYWHERE;
    m_aParams.bUseFormatter = TRUE;
    m_aParams.bCaseSensitive = FALSE;
    m_aParams.bBackwards = FALSE;
    m_aParams.bWildcard = FALSE;
    m_aParams.bRegular = FALSE;
    m_aParams.bApproxSearch = FALSE;
    m_aParams.bLevRelaxed = TRUE;
    m_aParams.nLevOther = 2;
    m_aParams.nLevShorter = 2;
    m_aParams.nLevLonger = 2;

    SetDefault(TRUE);
}

// ===================================================================================================
// = class FmSearchConfigAdmin - verwaltet den Zugriff auf das einzige FmSearchConfigItem, das es gibt
// ===================================================================================================

// die statics der Klasse
FmSearchConfigItem* FmSearchConfigAdmin::s_pItem = NULL;
INT32 FmSearchConfigAdmin::s_nUsageCounter = 0;

// die Instanz, die durch ihre globale Instantiierung dafuer sorgt, dass das ConfigItem erst bei
// Programmende aufgeraeumt wird
//FmSearchConfigAdmin g_aEnsureLastPossibleDelete;
    // da der FmSearchConfigAdmin einen Usage-Counter hat, wird das von ihm verwaltete Item wirklich erst
    // mit dem Beenden des Office endgueltige aufgeraeumt, dann naemlich wird diese Instanz hier weggeworfen.
    // Geladen wird es erst bei Benutzung : sobald naemlich jemand von einer FmSearchConfigAdmin-Instanz
    // die Params erfragt. Also schadet dieses eine zusaetzliche Objekt niemandem.



DBG_NAME(FmSearchConfigAdmin);
//------------------------------------------------------------------------
FmSearchConfigAdmin::FmSearchConfigAdmin()
{
    DBG_CTOR(FmSearchConfigAdmin,NULL);

    ++s_nUsageCounter;
}

//------------------------------------------------------------------------
FmSearchConfigAdmin::~FmSearchConfigAdmin()
{
    if (--s_nUsageCounter == 0)
    {
        if (s_pItem)
        {
            s_pItem->StoreConfig(TRUE);
            delete s_pItem;
            s_pItem = NULL;
        }
    }

    DBG_DTOR(FmSearchConfigAdmin,NULL);
}

//------------------------------------------------------------------------
FmSearchConfigItem* FmSearchConfigAdmin::GetItem() const
{
    if (s_pItem)
        return s_pItem;

    s_pItem = new FmSearchConfigItem;
    s_pItem->Initialize();
    return s_pItem;
}

//------------------------------------------------------------------------
void FmSearchConfigAdmin::PutParams(const FmSearchParams& rParams)
{
    if (GetParams() != rParams)
    {
        GetItem()->m_aParams = rParams;
        GetItem()->SetDefault(FALSE);   // damit wird das Ding implizit auch modified gesetzt
    }
}

