/*************************************************************************
 *
 *  $RCSfile: fanalyzer.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-20 16:49:26 $
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
#ifndef _CONNECTIVITY_FILE_FANALYZER_HXX_
#include "file/fanalyzer.hxx"
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include "connectivity/sqlparse.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif


using namespace connectivity;
using namespace connectivity::file;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;

typedef ::std::list<OEvaluateSet*>      OEvaluateSetList;
//------------------------------------------------------------------
OSQLAnalyzer::OSQLAnalyzer()
               : m_aCompiler(this)
               , m_aInterpreter(m_aCompiler)
               //   ,m_rCursor(rCurs)
{}
//------------------------------------------------------------------
void OSQLAnalyzer::start(OSQLParseNode* pSQLParseNode)
{
    if (!pSQLParseNode) return;

    // Parse Tree analysieren (je nach Statement-Typ)
    // und Zeiger auf WHERE-Klausel setzen:
    OSQLParseNode * pWhereClause = NULL;
    OSQLParseNode * pOrderbyClause = NULL;

    if (SQL_ISRULE(pSQLParseNode,select_statement))
    {
        OSL_ENSURE(pSQLParseNode->count() >= 4,"OFILECursor: Fehler im Parse Tree");

        OSQLParseNode * pTableExp = pSQLParseNode->getChild(3);
        OSL_ENSURE(pTableExp != NULL,"Fehler im Parse Tree");
        OSL_ENSURE(SQL_ISRULE(pTableExp,table_exp)," Fehler im Parse Tree");
        OSL_ENSURE(pTableExp->count() == 5,"Fehler im Parse Tree");

        pWhereClause    = pTableExp->getChild(1);
        pOrderbyClause  = pTableExp->getChild(4);
    }
    else if (SQL_ISRULE(pSQLParseNode,update_statement_searched))
    {
        OSL_ENSURE(pSQLParseNode->count() == 5,"OFILECursor: Fehler im Parse Tree");
        pWhereClause = pSQLParseNode->getChild(4);
    }
    else if (SQL_ISRULE(pSQLParseNode,update_statement_positioned))
    {
        // nyi
        DBG_ERROR("OPredicateCompiler: update positioned nyi");
    }
    else if (SQL_ISRULE(pSQLParseNode,delete_statement_searched))
    {
        OSL_ENSURE(pSQLParseNode->count() == 4,"Fehler im Parse Tree");
        pWhereClause = pSQLParseNode->getChild(3);
    }
    else if (SQL_ISRULE(pSQLParseNode,delete_statement_positioned))
    {
        // nyi
        DBG_ERROR("OPredicateCompiler: positioned nyi");
    }
    else
            // Anderes Statement. Keine Selektionskriterien.
        return;

    if (SQL_ISRULE(pWhereClause,where_clause))
    {
        // Wenn es aber eine where_clause ist, dann darf sie nicht leer sein:
        OSL_ENSURE(pWhereClause->count() == 2,"OFILECursor: Fehler im Parse Tree");

        OSQLParseNode * pComparisonPredicate = pWhereClause->getChild(1);
        OSL_ENSURE(pComparisonPredicate != NULL,"OFILECursor: Fehler im Parse Tree");

        m_aCompiler.execute(pComparisonPredicate);
    }
    else
    {
        // Die Where Clause ist meistens optional, d. h. es koennte sich auch
        // um "optional_where_clause" handeln.
        OSL_ENSURE(SQL_ISRULE(pWhereClause,opt_where_clause),"OFILECursor: Fehler im Parse Tree");
    }
}

//------------------------------------------------------------------
::std::vector<sal_Int32>* OSQLAnalyzer::bindResultRow(OValueRow _pRow)
{
    OCodeList& rCodeList    = m_aCompiler.m_aCodeList;
    // Zaehlen, wieviele Kriterien
    // wenn nur ein Kriterium, und das entsprechende Feld ist indiziert
    // dann wird der Index verwendet

    OEvaluateSetList    aEvaluateSetList;
    OEvaluateSet*       pEvaluateSet = NULL;
    ::std::vector<sal_Int32>*   pKeySet = NULL;

    for (OCodeList::iterator aIter = rCodeList.begin(); aIter != rCodeList.end(); ++aIter)
    {
        OOperandAttr* pAttr = PTR_CAST(OOperandAttr,(*aIter));
        if (pAttr)
        {
            if (pAttr->isIndexed() && !m_aCompiler.hasORCondition())
            {
                OCode* pCode1 = *(aIter + 1);
                OCode* pCode2 = *(aIter + 2);

                if (PTR_CAST(OOperand,pCode1))
                    pEvaluateSet = pAttr->preProcess(PTR_CAST(OBoolOperator,pCode2), PTR_CAST(OOperand,pCode1));
                else
                    pEvaluateSet = pAttr->preProcess(PTR_CAST(OBoolOperator,pCode1));
            }

            if (pEvaluateSet)
            {
                aEvaluateSetList.push_back(pEvaluateSet);
                pEvaluateSet = NULL;
            }
            else
                pAttr->bindValue(_pRow);
        }
    }

    // Keyset erzeugen mit kleinster Liste
    if(aEvaluateSetList.size())
    {
        // welche Liste hat den kleinsten count ?
        OEvaluateSetList::iterator i = aEvaluateSetList.begin();
        pEvaluateSet = *(i);
        for(++i; i != aEvaluateSetList.end();++i)
        {
            OEvaluateSet*   pEvaluateSetComp = (*i);
            for(OEvaluateSet::reverse_iterator j = pEvaluateSet->rbegin(); j != pEvaluateSet->rend(); ++j)
            {
                if (pEvaluateSetComp->find(j->second) != pEvaluateSetComp->end())
                    pEvaluateSet->erase(j->second);
            }
        }
        pKeySet = new ::std::vector<sal_Int32>(pEvaluateSet->size());
        sal_Int32 k=0;
        for(OEvaluateSet::iterator j = pEvaluateSet->begin(); j != pEvaluateSet->end(); ++j,++k)
        {
            (*pKeySet)[k] = j->second;
        }

        // alle loeschen
        for(i = aEvaluateSetList.begin(); i != aEvaluateSetList.end();++i)
            delete (*i);
    }

    return pKeySet;
}

//------------------------------------------------------------------
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


//------------------------------------------------------------------
void OSQLAnalyzer::describeParam(::vos::ORef<OSQLColumns> rParameterColumns)
{
    OCodeList& rCodeList    = m_aCompiler.m_aCodeList;
    OCodeStack aCodeStack;

    if (!rCodeList.size())
        return;     // kein Prädikat
    if (!rParameterColumns->size())
        return; // keine Parameter

    // Anlegen von Columns, die eine genauere Beschreibung für die enthalten
    ::vos::ORef<OSQLColumns> aNewParamColumns = new OSQLColumns(*rParameterColumns);

    // Anlegen einer Testzeile, wird benötigt um die Parameter zu beschreiben
    OValueRow aTestRow = new OValueVector(Reference< XIndexAccess>(m_aCompiler.getOrigColumns(),UNO_QUERY)->getCount());
    bindResultRow(aTestRow);                    // Binden der Attribute an die Values

    OValueRow aParameterRow  = new OValueVector(rParameterColumns->size());
    bindParameterRow(aParameterRow);

    for(OCodeList::iterator aIter = rCodeList.begin(); aIter != rCodeList.end(); ++aIter)
    {
        OOperand* pOperand = PTR_CAST(OOperand,(*aIter));
        OOperator* pOperator = PTR_CAST(OOperator,(*aIter));
        if (pOperand)
            aCodeStack.push(pOperand);
        else
        {
            if (pOperator->getRequestedOperands() == 2)     // bei zwei Operatoren ist es möglich
            {                                               // einen Parameter weiter zu spezifizieren
                OOperandParam *pParam  = PTR_CAST(OOperandParam,aCodeStack.top());
                if (pParam)  // Anpassen des ParameterTyps, wenn der linke Operand ein Attribut ist
                {
                    OOperandAttr *pLeft  = PTR_CAST(OOperandAttr,*(rCodeList.end() - 2));
                    if (pLeft)
                    {
                        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xCol;
                        ::cppu::extractInterface(xCol,Reference< XIndexAccess>(m_aCompiler.getOrigColumns(),UNO_QUERY)->getByIndex(pLeft->getRowPos()));
                        OSL_ENSURE(xCol.is(), "Ungültige Struktur");
                        pParam->describe(xCol, aNewParamColumns);
                    }
                }
            }
            pOperator->Exec(aCodeStack);
        }
    }
    OOperand* pOperand = aCodeStack.top();
    aCodeStack.pop();

    OSL_ENSURE(aCodeStack.size() == 0, "StackFehler");
    OSL_ENSURE(pOperand, "StackFehler");
    if (IS_TYPE(OOperandResult,pOperand))
        delete pOperand;

    rParameterColumns = aNewParamColumns;
    m_aCompiler.setParameterColumns(rParameterColumns);
}

//------------------------------------------------------------------
void OSQLAnalyzer::clean()
{
    m_aCompiler.Clean();
}


