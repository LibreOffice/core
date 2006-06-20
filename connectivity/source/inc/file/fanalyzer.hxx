/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fanalyzer.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:01:48 $
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

#ifndef _CONNECTIVITY_FILE_FANALYZER_HXX_
#define _CONNECTIVITY_FILE_FANALYZER_HXX_

#ifndef _CONNECTIVITY_FILE_FCOMP_HXX_
#include "file/fcomp.hxx"
#endif

namespace connectivity
{
    namespace file
    {

        class OSQLAnalyzer
        {
            typedef ::std::list<OEvaluateSet*>      OEvaluateSetList;
            typedef ::std::pair< ::vos::ORef<OPredicateCompiler>,::vos::ORef<OPredicateInterpreter> > TPredicates;

            ::std::vector< TPredicates >        m_aSelectionEvaluations;
            ::vos::ORef<OPredicateCompiler>     m_aCompiler;
            ::vos::ORef<OPredicateInterpreter>  m_aInterpreter;

            mutable sal_Bool                    m_bHasSelectionCode;
            mutable sal_Bool                    m_bSelectionFirstTime;

            void bindRow(OCodeList& rCodeList,const OValueRefRow& _pRow,OEvaluateSetList& _rEvaluateSetList);

        public:
            OSQLAnalyzer();
            virtual ~OSQLAnalyzer();
            inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
                { return ::rtl_allocateMemory( nSize ); }
            inline static void * SAL_CALL operator new( size_t /*nSize*/,void* _pHint ) SAL_THROW( () )
                { return _pHint; }
            inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
                { ::rtl_freeMemory( pMem ); }
            inline static void SAL_CALL operator delete( void * /*pMem*/,void* /*_pHint*/ ) SAL_THROW( () )
                {  }

            void describeParam(::vos::ORef<OSQLColumns> rParameterColumns); // genauere Beschreibung der Parameter
            ::std::vector<sal_Int32>* bindEvaluationRow(OValueRefRow& _pRow);                   // Anbinden einer Ergebniszeile an die Restrictions
            /** bind the select columns if they contain a function which needs a row value
                @param  _pRow   the result row
            */
            void bindSelectRow(const OValueRefRow& _pRow);

            /** binds the row to parameter for the restrictions
                @param  _pRow   the parameter row
            */
            void bindParameterRow(OValueRefRow& _pRow);

            void setIndexes(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xIndexes);

            void dispose();
            void start(OSQLParseNode* pSQLParseNode);
            void clean();
            virtual BOOL hasRestriction() const;
            virtual BOOL hasFunctions() const;
            inline BOOL evaluateRestriction()   { return m_aInterpreter->start(); }
            void setSelectionEvaluationResult(OValueRefRow& _pRow,const ::std::vector<sal_Int32>& _rColumnMapping);
            void setOrigColumns(const OFileColumns& rCols);
            virtual OOperandAttr* createOperandAttr(sal_Int32 _nPos,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xCol,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xIndexes=NULL);
        };
    }
}
#endif // _CONNECTIVITY_FILE_FANALYZER_HXX_

