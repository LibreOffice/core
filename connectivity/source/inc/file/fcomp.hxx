/*************************************************************************
 *
 *  $RCSfile: fcomp.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:13:40 $
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
#ifndef _CONNECTIVITY_FILE_FCOMP_HXX_
#define _CONNECTIVITY_FILE_FCOMP_HXX_

#ifndef _CONNECTIVITY_FILE_FCODE_HXX_
#include "file/fcode.hxx"
#endif
#ifndef _LIST_
#include <list>
#endif

namespace connectivity
{
    class OSQLParseNode;
    namespace file
    {
        class OCode;
        class OOperand;
        typedef::std::vector<OCode*> OCodeList;

        class OPredicateCompiler : public ::vos::OReference
        {
            friend class OPredicateInterpreter;
            friend class OSQLAnalyzer;

            OCodeList                               m_aCodeList;
            OFileColumns                            m_orgColumns; // in filecurs this are the filecolumns
            OSQLAnalyzer*                           m_pAnalyzer;
            ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess> m_xIndexes;
            sal_Int32                               m_nParamCounter;
            sal_Bool                                m_bORCondition;
        public:
            OPredicateCompiler(OSQLAnalyzer* pAnalyzer);

            virtual ~OPredicateCompiler();

            inline static void * SAL_CALL operator new( size_t nSize ) SAL_THROW( () )
                { return ::rtl_allocateMemory( nSize ); }
            inline static void * SAL_CALL operator new( size_t nSize,void* _pHint ) SAL_THROW( () )
                { return _pHint; }
            inline static void SAL_CALL operator delete( void * pMem ) SAL_THROW( () )
                { ::rtl_freeMemory( pMem ); }
            inline static void SAL_CALL operator delete( void * pMem,void* _pHint ) SAL_THROW( () )
                {  }
            void dispose();

            void start(connectivity::OSQLParseNode* pSQLParseNode);
            OOperand* execute(connectivity::OSQLParseNode* pPredicateNode);

            void Clean();
            sal_Bool isClean() const {return m_aCodeList.empty();}
            sal_Bool hasCode() const {return !isClean();}
            sal_Bool hasORCondition() const {return m_bORCondition;}
            void     setOrigColumns(const OFileColumns& rCols) { m_orgColumns = rCols; }
            const OFileColumns getOrigColumns() const { return m_orgColumns; }
        protected:
            OOperand* execute_COMPARE(connectivity::OSQLParseNode* pPredicateNode) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_LIKE(connectivity::OSQLParseNode* pPredicateNode) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_BETWEEN(connectivity::OSQLParseNode* pPredicateNode) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_ISNULL(connectivity::OSQLParseNode* pPredicateNode) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_Operand(connectivity::OSQLParseNode* pPredicateNode) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* execute_Fold(OSQLParseNode* pPredicateNode) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            OOperand* executeFunction(OSQLParseNode* pPredicateNode) throw( ::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
        };


        class OPredicateInterpreter : public ::vos::OReference
        {
            OCodeStack          m_aStack;
            ::vos::ORef<OPredicateCompiler> m_rCompiler;

        public:
            OPredicateInterpreter(const ::vos::ORef<OPredicateCompiler>& rComp) : m_rCompiler(rComp){}
            virtual ~OPredicateInterpreter();

            sal_Bool    evaluate(OCodeList& rCodeList);
            void        evaluateSelection(OCodeList& rCodeList,ORowSetValueDecoratorRef& _rVal);

            inline sal_Bool start()
            {
                return evaluate(m_rCompiler->m_aCodeList);
            }

            inline void startSelection(ORowSetValueDecoratorRef& _rVal)
            {
                return evaluateSelection(m_rCompiler->m_aCodeList,_rVal);
            }


        };
    }
}
#endif // _CONNECTIVITY_FILE_FCOMP_HXX_

