/*************************************************************************
 *
 *  $RCSfile: FNoException.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-24 06:08:38 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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
#ifndef _CONNECTIVITY_FILE_CATALOG_HXX_
#include "file/FCatalog.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_FCOMP_HXX_
#include "file/fcomp.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_FANALYZER_HXX_
#include "file/fanalyzer.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_FRESULTSET_HXX_
#include "file/FResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_OPREPAREDSTATEMENT_HXX_
#include "file/FPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_FILE_VALUE_HXX_
#include "FValue.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace connectivity;
using namespace connectivity::file;
// -----------------------------------------------------------------------------
void OFileCatalog::refreshViews()
{}
void OFileCatalog::refreshGroups()
{}
void OFileCatalog::refreshUsers()
{
}
// -----------------------------------------------------------------------------
OPredicateInterpreter::~OPredicateInterpreter()
{
    while(!m_aStack.empty())
    {
        delete m_aStack.top();
        m_aStack.pop();
    }
    //  m_aStack.clear();
}
// -----------------------------------------------------------------------------
void OPredicateCompiler::Clean()
{
    for(OCodeList::reverse_iterator aIter = m_aCodeList.rbegin(); aIter != m_aCodeList.rend();++aIter)
    {
        delete *aIter;
        m_aCodeList.pop_back();
    }
    //  m_aCodeList.clear();
}
// -----------------------------------------------------------------------------
void OSQLAnalyzer::clean()
{
    m_aCompiler.Clean();
}
// -----------------------------------------------------------------------------
void OSQLAnalyzer::bindParameterRow(OValueRow _pRow)
{
    OCodeList& rCodeList    = m_aCompiler.m_aCodeList;
    for(OCodeList::iterator aIter = rCodeList.begin(); aIter != rCodeList.end();++aIter)
    {
        OOperandParam* pParam = PTR_CAST(OOperandParam,(*aIter));
        if (pParam)
            pParam->bindValue(_pRow);
    }
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::isCount() const
{
    return (m_pParseTree &&
            m_pParseTree->count() > 2 &&
            SQL_ISRULE(m_pParseTree->getChild(2),scalar_exp_commalist) &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0),derived_column) &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0)->getChild(0),general_set_fct)
            );
}
// -----------------------------------------------------------------------------
void OPreparedStatement::scanParameter(OSQLParseNode* pParseNode,::std::vector< OSQLParseNode*>& _rParaNodes)
{
    DBG_ASSERT(pParseNode != NULL,"OResultSet: interner Fehler: ungueltiger ParseNode");

    // Parameter Name-Regel gefunden?
    if (SQL_ISRULE(pParseNode,parameter))
    {
        DBG_ASSERT(pParseNode->count() >= 1,"OResultSet: Parse Tree fehlerhaft");
        DBG_ASSERT(pParseNode->getChild(0)->getNodeType() == SQL_NODE_PUNCTUATION,"OResultSet: Parse Tree fehlerhaft");

        _rParaNodes.push_back(pParseNode);
        // Weiterer Abstieg nicht erforderlich
        return;
    }

    // Weiter absteigen im Parse Tree
    for (UINT32 i = 0; i < pParseNode->count(); i++)
        scanParameter(pParseNode->getChild(i),_rParaNodes);
}
// -----------------------------------------------------------------------------
//------------------------------------------------------------------
int
#if defined(WIN) || defined(WNT)
__cdecl
#endif
#if defined(ICC) && defined(OS2)
_Optlink
#endif
connectivity::file::OFILEKeyCompare(const void * elem1, const void * elem2)
{
    const OFILESortIndex * pIndex = OFILESortIndex::pCurrentIndex;
    const OFILEKeyValue * pKeyValue1 = (OFILEKeyValue *) * (OFILEKeyValue **) elem1;
    const OFILEKeyValue * pKeyValue2 = (OFILEKeyValue *) * (OFILEKeyValue **) elem2;

    // Ueber die (max.) drei ORDER BY-Columns iterieren. Abbruch des Vergleiches, wenn Ungleichheit erkannt
    // oder alle Columns gleich.
    for (UINT16 i = 0; i < SQL_ORDERBYKEYS && pIndex->eKeyType[i] != SQL_ORDERBYKEY_NONE; i++)
    {
        const int nGreater = (pIndex->m_aAscending[i]) ? 1 : -1;
        const int nLess = - nGreater;

        // Vergleich (je nach Datentyp):
        switch (pIndex->eKeyType[i])
        {
            case SQL_ORDERBYKEY_STRING:
            {
                INT32 nRes = pKeyValue1->GetKeyString(i)->compareTo(*pKeyValue2->GetKeyString(i));
                if (nRes < 0)
                    return nLess;
                else if (nRes > 0)
                    return nGreater;
            }
            break;
            case SQL_ORDERBYKEY_DOUBLE:
            {
                double d1 = pKeyValue1->GetKeyDouble(i);
                double d2 = pKeyValue2->GetKeyDouble(i);

                if (d1 < d2)
                    return nLess;
                else if (d1 > d2)
                    return nGreater;
            }
            break;
        }
    }

    // Wenn wir bis hierher gekommen sind, waren alle Werte gleich:
    return 0;
}
// -----------------------------------------------------------------------------
//------------------------------------------------------------------
OKeySet* OFILESortIndex::CreateKeySet()
{

    OSL_ENSURE(! bFrozen,"OFILESortIndex::Freeze: already frozen!");

    // Kritischer Bereich: Hinterlegung von this in statischer Variable.
    // Zugriff auf diese Variable von der OFILECompare-Funktion aus.
    // Da dies NUR waehrend der Ausfuehrung der qsort-Funktion stattfindet,
    // ist dies aber unkritisch: unter Windows 3.x ist diese Ausfuehrung
    // UNUNTERBRECHBAR; unter NT, OS/2, Unix, ... hat jede DLL ihr
    // eigenes Datensegment.
    pCurrentIndex = this;
    eCurrentCharSet = eCharSet;

    // Sortierung:
    if (eKeyType[0] != SQL_ORDERBYKEY_NONE)
        // Sortierung, wenn mindestens nach dem ersten Key sortiert werden soll:
        qsort(ppKeyValueArray,nCount,sizeof(void *),&OFILEKeyCompare);


    // Ende des kritischen Bereiches
    pCurrentIndex = NULL;


    OKeySet* pKeySet = new OKeySet(nCount);
    OKeySet::iterator aIter = pKeySet->begin();
    for (INT32 i = 0; i < nCount; i++,++aIter)
    {
        OFILEKeyValuePtr pKeyValue = ppKeyValueArray[i];

        OSL_ENSURE(pKeyValue != NULL,"OFILESortIndex::Freeze: pKeyValue == NULL");
        (*aIter) = pKeyValue->GetValue();       // Wert holen ...

        // Strings in KeyValue freigeben!
        for (int j = 0; j < SQL_ORDERBYKEYS; j++)
        {
            if (eKeyType[j] == SQL_ORDERBYKEY_STRING)
                delete pKeyValue->GetKeyString(j);
        }
        delete pKeyValue;
    }
    bFrozen = TRUE;
    pKeySet->setFrozen();
    return pKeySet;
}
// -----------------------------------------------------------------------------
OFILESortIndex * OFILESortIndex::pCurrentIndex;
CharSet OFILESortIndex::eCurrentCharSet;
//------------------------------------------------------------------
OFILESortIndex::OFILESortIndex(const OKeyType eKeyType2[],  // Genau 3 Eintraege!
                           const ::std::vector<sal_Int16>& _aAscending,     // Genau 3 Eintraege!
                           INT32 nMaxNumberOfRows, rtl_TextEncoding eSet)   // Obere Schranke fuer die Anzahl indizierbarer Zeilen
    : nMaxCount(nMaxNumberOfRows),
      nCount(0),
      bFrozen(FALSE),
      eCharSet(eSet),
      m_aAscending(_aAscending)
{
    for (int j = 0; j < SQL_ORDERBYKEYS; j++)
    {
        eKeyType[j] = eKeyType2[j];
    }

#if defined MAX_KEYSET_SIZE
    // Zur Sicherheit Maximalgroesse nochmal pruefen:
    if (nMaxCount > MAX_KEYSET_SIZE)
    {
        DBG_WARNING("OFILESortIndex::OFILESortIndex: nMaxNumberOfRows zur Zeit auf <16K beschraenkt!");
        nMaxCount = MAX_KEYSET_SIZE;
    }
#endif
    if (nMaxCount <= 0)
        nMaxCount = USHORT(-1);

    ppKeyValueArray = new OFILEKeyValuePtr[nMaxCount];

    for (INT32 i = 0; i < nMaxCount; i++)
        ppKeyValueArray[i] = NULL;
}

//------------------------------------------------------------------
OFILESortIndex::~OFILESortIndex()
{
    __DELETE(nMaxCount) ppKeyValueArray;
}


//------------------------------------------------------------------
BOOL OFILESortIndex::AddKeyValue(OFILEKeyValue * pKeyValue)
{
    if (nCount < nMaxCount)
    {
        if (bFrozen)                            // wenn der Index schon eingefroren
                                                // dann wird der Key einfach ans Ende gehaengt
        {
            OSL_ENSURE(pKeyValue != NULL,"OFILESortIndex::Freeze: pKeyValue == NULL");
            INT32 nValue = pKeyValue->GetValue();       // Wert holen ...

            // Strings in KeyValue freigeben!
            for (int j = 0; j < SQL_ORDERBYKEYS; j++)
            {
                if (eKeyType[j] == SQL_ORDERBYKEY_STRING)
                    delete pKeyValue->GetKeyString(j);
            }
            delete pKeyValue;
            ppKeyValueArray[nCount++] = (OFILEKeyValuePtr) nValue;
        }
        else
            ppKeyValueArray[nCount++] = pKeyValue;
        return TRUE;
    }
    else
        return FALSE;
}


//------------------------------------------------------------------
void OFILESortIndex::Freeze()
{
    OSL_ENSURE(! bFrozen,"OFILESortIndex::Freeze: already frozen!");

    // Kritischer Bereich: Hinterlegung von this in statischer Variable.
    // Zugriff auf diese Variable von der OFILECompare-Funktion aus.
    // Da dies NUR waehrend der Ausfuehrung der qsort-Funktion stattfindet,
    // ist dies aber unkritisch: unter Windows 3.x ist diese Ausfuehrung
    // UNUNTERBRECHBAR; unter NT, OS/2, Unix, ... hat jede DLL ihr
    // eigenes Datensegment.
    pCurrentIndex = this;
    eCurrentCharSet = eCharSet;

    // Sortierung:
    if (eKeyType[0] != SQL_ORDERBYKEY_NONE)
        // Sortierung, wenn mindestens nach dem ersten Key sortiert werden soll:
        qsort(ppKeyValueArray,nCount,sizeof(void *),&OFILEKeyCompare);


    // Ende des kritischen Bereiches
    pCurrentIndex = NULL;

    // Wert auslesen, KeyValue loeschen und in den void * den Value
    // reinschreiben (uebler Trick mit Typecast!)
    for (INT32 i = 0; i < nCount; i++)
    {
        OFILEKeyValuePtr pKeyValue = ppKeyValueArray[i];

        OSL_ENSURE(pKeyValue != NULL,"OFILESortIndex::Freeze: pKeyValue == NULL");
        INT32 nValue = pKeyValue->GetValue();       // Wert holen ...

        // Strings in KeyValue freigeben!
        for (int j = 0; j < SQL_ORDERBYKEYS; j++)
        {
            if (eKeyType[j] == SQL_ORDERBYKEY_STRING)
                delete pKeyValue->GetKeyString(j);
        }
        delete pKeyValue;
        ppKeyValueArray[i] = (OFILEKeyValuePtr) nValue;
    }

    bFrozen = TRUE;
}

//------------------------------------------------------------------
INT32 OFILESortIndex::GetValue(INT32 nPos) const
{
    OSL_ENSURE(nPos > 0,"OFILESortIndex::GetValue: nPos == 0");
    OSL_ENSURE(nPos <= nCount,"OFILESortIndex::GetValue: Zugriff ausserhalb der Array-Grenzen");

//  OSL_ENSURE(ppKeyValueArray[nPos-1] != NULL,"OFILESortIndex::GetValue: interner Fehler: kein KeyValue an dieser Stelle");
//  return ppKeyValueArray[nPos-1]->GetValue();

    if (!bFrozen)
    {
        if (eKeyType[0] == SQL_ORDERBYKEY_NONE)  // wenn keine Sortierung vorliegt
                                                 // darf auf die Values schon vorher zugegriffen werden
            return ppKeyValueArray[nPos-1]->GetValue();
        else
        {
            OSL_ASSERT("OFILESortIndex::GetValue: Invalid use of index!");
            return 0;
        }
    }
    else
        return (INT32) ppKeyValueArray[nPos-1]; // Trick: nach Freeze sind hier nur noch Values, keine KeyValue-Strukturen mehr!

}
// -----------------------------------------------------------------------------
OFILEKeyValue* OResultSet::GetOrderbyKeyValue(OValueRow _rRow)
{
    UINT32 nBookmarkValue = Abs((sal_Int32)(*_rRow)[0]);

    OFILEKeyValue* pKeyValue = new OFILEKeyValue((UINT32)nBookmarkValue);
    for (int i = 0; i < m_aOrderbyColumnNumber.size(); ++i)
    {
        if (m_aOrderbyColumnNumber[i] == SQL_COLUMN_NOTFOUND) break;

        ORowSetValue xKey = (*_rRow)[m_aOrderbyColumnNumber[i]];
        switch (xKey.getTypeKind())
        {
            case ::com::sun::star::sdbc::DataType::VARCHAR:
            case ::com::sun::star::sdbc::DataType::CHAR:
                pKeyValue->SetKey(i,(rtl::OUString)xKey);
                break;
            default:
                pKeyValue->SetKey(i,(double)xKey);
                break;
        }
    }
    return pKeyValue;
}
// -----------------------------------------------------------------------------








