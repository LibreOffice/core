/*************************************************************************
 *
 *  $RCSfile: RowSetRow.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:15:38 $
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
#ifndef DBACCESS_CORE_API_ROWSETROW_HXX
#define DBACCESS_CORE_API_ROWSETROW_HXX

#ifndef _VOS_REF_HXX_
#include <vos/ref.hxx>
#endif
#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include <connectivity/CommonTools.hxx>
#endif
#ifndef DBACCESS_CORE_API_ROWSETVALUE_HXX
#include "RowSetValue.hxx"
#endif

namespace dbaccess
{
    typedef ::vos::ORef< connectivity::ORowVector< ORowSetValue > >     ORowSetRow;
    typedef ::std::vector< ORowSetRow >                                 ORowSetMatrix;

    class ORowSetValueCompare
    {
        const ::com::sun::star::uno::Any& m_rAny;
    public:
        ORowSetValueCompare(const ::com::sun::star::uno::Any& _rAny) : m_rAny(_rAny){}

        sal_Bool operator ()(const ORowSetRow& _rRH)
        {
            switch((*_rRH)[0].getTypeKind())
            {
                case ::com::sun::star::sdbc::DataType::TINYINT:
                case ::com::sun::star::sdbc::DataType::SMALLINT:
                case ::com::sun::star::sdbc::DataType::INTEGER:
                    return connectivity::getINT32(m_rAny) == (sal_Int32)(*_rRH)[0];
                    break;
                default:
                {
                    ::com::sun::star::uno::Sequence<sal_Int8> aSeq;
                    m_rAny >>= aSeq;
                    return aSeq == (*_rRH)[0];
                }
            }
        }
    };
}
#endif // DBACCESS_CORE_API_ROWSETROW_HXX
/*------------------------------------------------------------------------

    $Log: not supported by cvs2svn $
    Revision 1.2  2000/09/18 14:52:47  willem.vandorp
    OpenOffice header added.

    Revision 1.1  2000/09/01 15:24:38  oj
    rowset addon

    Revision 1.0 27.07.2000 13:23:15  oj
------------------------------------------------------------------------*/

