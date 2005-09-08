/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AutoRetrievingBase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:36:50 $
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
#ifndef _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_
#define _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace connectivity
{
    class OAutoRetrievingBase
    {
        ::rtl::OUString m_sGeneratedValueStatement; // contains the statement which should be used when query for automatically generated values
        sal_Bool        m_bAutoRetrievingEnabled; // set to when we should allow to query for generated values
    protected:
        OAutoRetrievingBase() : m_bAutoRetrievingEnabled(sal_False) {}
        virtual ~OAutoRetrievingBase(){}

        inline void enableAutoRetrievingEnabled(sal_Bool _bAutoEnable)          { m_bAutoRetrievingEnabled = _bAutoEnable; }
        inline void setAutoRetrievingStatement(const ::rtl::OUString& _sStmt)   { m_sGeneratedValueStatement = _sStmt; }
    public:
        inline sal_Bool                 isAutoRetrievingEnabled()       const { return m_bAutoRetrievingEnabled; }
        inline const ::rtl::OUString&   getAutoRetrievingStatement()    const { return m_sGeneratedValueStatement; }

        /** transform the statement to query for auto generated values
            @param  _sInsertStatement
                The "INSERT" statement, is used to query for column and table names
            @return
                The transformed generated statement.
        */
        ::rtl::OUString getTransformedGeneratedStatement(const ::rtl::OUString& _sInsertStatement) const;
    };
}
#endif // _CONNECTIVITY_AUTOKEYRETRIEVINGBASE_HXX_

