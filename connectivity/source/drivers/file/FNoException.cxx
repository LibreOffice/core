/*************************************************************************
 *
 *  $RCSfile: FNoException.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-04 08:25:04 $
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
#ifndef CONNECTIVITY_TKEYVALUE_HXX
#include "TKeyValue.hxx"
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
    m_aCompiler->Clean();
}
// -----------------------------------------------------------------------------
void OSQLAnalyzer::bindParameterRow(OValueRefRow& _pRow)
{
    OCodeList& rCodeList    = m_aCompiler->m_aCodeList;
    for(OCodeList::iterator aIter = rCodeList.begin(); aIter != rCodeList.end();++aIter)
    {
        OOperandParam* pParam = PTR_CAST(OOperandParam,(*aIter));
        if ( pParam )
            pParam->bindValue(_pRow);
    }
}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::isCount() const
{
    return (m_pParseTree &&
            m_pParseTree->count() > 2                                                       &&
            SQL_ISRULE(m_pParseTree->getChild(2),scalar_exp_commalist)                      &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0),derived_column)               &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0)->getChild(0),general_set_fct) &&
            m_pParseTree->getChild(2)->getChild(0)->getChild(0)->count() == 4
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
OKeyValue* OResultSet::GetOrderbyKeyValue(OValueRefRow& _rRow)
{
    UINT32 nBookmarkValue = Abs((sal_Int32)(*_rRow)[0]->getValue());

    OKeyValue* pKeyValue = OKeyValue::createKeyValue((UINT32)nBookmarkValue);

    ::std::vector<sal_Int32>::iterator aIter = m_aOrderbyColumnNumber.begin();
    for (;aIter != m_aOrderbyColumnNumber.end(); ++aIter)
    {
        OSL_ENSURE(*aIter < static_cast<sal_Int32>(_rRow->size()),"Invalid index for orderkey values!");
        pKeyValue->pushKey(new ORowSetValueDecorator((*_rRow)[*aIter]->getValue()));
    }

    return pKeyValue;
}
// -----------------------------------------------------------------------------








