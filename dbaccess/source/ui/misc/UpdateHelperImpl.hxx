/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UpdateHelperImpl.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef DBAUI_UPDATEHELPERIMPL_HXX
#define DBAUI_UPDATEHELPERIMPL_HXX

#ifndef _COM_SUN_STAR_SDBC_XRESULTSETUPDATE_HPP_
#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWUPDATE_HPP_
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPARAMETERS_HPP_
#include <com/sun/star/sdbc/XParameters.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XPREPAREDSTATEMENT_HPP_
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROWSET_HPP_
#include <com/sun/star/sdbc/XRowSet.hpp>
#endif
#ifndef DBAUI_IUPDATEHELPER_HXX
#include "IUpdateHelper.hxx"
#endif

namespace dbaui
{
    class ORowUpdateHelper : public IUpdateHelper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowUpdate >          m_xRowUpdate;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate >    m_xResultSetUpdate; //
    public:
        ORowUpdateHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _xRowSet)
            :m_xRowUpdate(_xRowSet,::com::sun::star::uno::UNO_QUERY)
            ,m_xResultSetUpdate(_xRowSet,::com::sun::star::uno::UNO_QUERY)
        {
        }
        virtual ~ORowUpdateHelper() {}
        virtual void updateString(sal_Int32 _nPos, const ::rtl::OUString& _sValue)
        {
            m_xRowUpdate->updateString(_nPos, _sValue);
        }
        virtual void updateDouble(sal_Int32 _nPos,const double& _nValue)
        {
            m_xRowUpdate->updateDouble(_nPos, _nValue);
        }
        virtual void updateInt(sal_Int32 _nPos,const sal_Int32& _nValue)
        {
            m_xRowUpdate->updateInt(_nPos, _nValue);
        }
        virtual void updateNull(sal_Int32 _nPos, ::sal_Int32)
        {
            m_xRowUpdate->updateNull(_nPos);
        }
        virtual void moveToInsertRow()
        {
            m_xResultSetUpdate->moveToInsertRow();
        }
        virtual void insertRow()
        {
            m_xResultSetUpdate->insertRow();
        }
    };

    class OParameterUpdateHelper : public IUpdateHelper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement >  m_xPrepared;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters >         m_xParameters;

    public:
        OParameterUpdateHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement >& _xPrepared)
           :m_xPrepared(_xPrepared)
           ,m_xParameters(_xPrepared,::com::sun::star::uno::UNO_QUERY)
        {
        }
        virtual ~OParameterUpdateHelper() {}
        virtual void updateString(sal_Int32 _nPos, const ::rtl::OUString& _sValue)
        {
            m_xParameters->setString(_nPos, _sValue);
        }
        virtual void updateDouble(sal_Int32 _nPos,const double& _nValue)
        {
            m_xParameters->setDouble(_nPos, _nValue);
        }
        virtual void updateInt(sal_Int32 _nPos,const sal_Int32& _nValue)
        {
            m_xParameters->setInt(_nPos, _nValue);
        }
        virtual void updateNull(sal_Int32 _nPos, ::sal_Int32 sqlType)
        {
            m_xParameters->setNull(_nPos,sqlType);
        }
        virtual void moveToInsertRow()
        {
        }
        virtual void insertRow()
        {
            m_xPrepared->executeUpdate();
        }
    };
}

#endif // DBAUI_UPDATEHELPERIMPL_HXX

