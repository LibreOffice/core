/*************************************************************************
 *
 *  $RCSfile: DCode.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: oj $ $Date: 2001-05-07 10:46:47 $
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
#ifndef CONNECTIVITY_DBASE_DCODE_HXX
#define CONNECTIVITY_DBASE_DCODE_HXX

#ifndef _CONNECTIVITY_FILE_FANALYZER_HXX_
#include "file/fanalyzer.hxx"
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _CONNECTIVITY_FILE_FCODE_HXX_
#include "file/fcode.hxx"
#endif

namespace connectivity
{
    namespace dbase
    {
        class OFILEAnalyzer : public file::OSQLAnalyzer
        {
        public:
            OFILEAnalyzer() : file::OSQLAnalyzer(){}
            virtual file::OOperandAttr* createOperandAttr(sal_Int32 _nPos,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xCol,
                                                    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xIndexes=NULL);
        };

        // Attribute aus einer Ergebniszeile
        class OFILEOperandAttr : public file::OOperandAttr
        {
            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> m_xIndex;
        public:
            OFILEOperandAttr(sal_uInt16 _nPos,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xColumn,
                             const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xIndexes=NULL);

            virtual sal_Bool isIndexed() const;
            virtual file::OEvaluateSet* preProcess(file::OBoolOperator* pOp, file::OOperand* pRight = 0);
            TYPEINFO();
        };
    }
}

#endif // CONNECTIVITY_DBASE_DCODE_HXX

