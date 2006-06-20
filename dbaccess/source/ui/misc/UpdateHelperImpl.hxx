/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UpdateHelperImpl.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:21:24 $
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

