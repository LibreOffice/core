/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef DBTOOLS_WARNINGSCONTAINER_HXX
#define DBTOOLS_WARNINGSCONTAINER_HXX

/** ==== begin UNO includes === **/
#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>
/** ==== end UNO includes === **/

#include "connectivity/dbtoolsdllapi.hxx"

//.........................................................................
namespace dbtools
{
//.........................................................................

    //=====================================================================
    //= IWarningsContainer
    //=====================================================================
    class SAL_NO_VTABLE IWarningsContainer
    {
    public:
        virtual void appendWarning(const ::com::sun::star::sdbc::SQLException& _rWarning) = 0;
        virtual void appendWarning(const ::com::sun::star::sdbc::SQLWarning& _rWarning) = 0;
        virtual void appendWarning(const ::com::sun::star::sdb::SQLContext& _rContext) = 0;
    };

    //====================================================================
    //= WarningsContainer
    //====================================================================
    /** helper class for implementing XWarningsSupplier, which mixes own warnings with
        warnings obtained from an external instance
    */
    class OOO_DLLPUBLIC_DBTOOLS WarningsContainer : public IWarningsContainer
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XWarningsSupplier >   m_xExternalWarnings;
        ::com::sun::star::uno::Any                                                      m_aOwnWarnings;

    public:
        WarningsContainer() { }
        WarningsContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XWarningsSupplier >& _rxExternalWarnings )
            :m_xExternalWarnings( _rxExternalWarnings )
        {
        }
        virtual ~WarningsContainer();

        void setExternalWarnings( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XWarningsSupplier >& _rxExternalWarnings )
        {
            m_xExternalWarnings = _rxExternalWarnings;
        }

        // convenience
        /** appends an SQLWarning instance to the chain
            @param  _rWarning
                the warning message
            @param  _pAsciiSQLState
                the SQLState of the warning
            @param  _rxContext
                the context of the warning
        */
        void appendWarning(
            const ::rtl::OUString& _rWarning,
            const sal_Char* _pAsciiSQLState,
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxContext );

        // IWarningsContainer
        virtual void appendWarning(const ::com::sun::star::sdbc::SQLException& _rWarning);
        virtual void appendWarning(const ::com::sun::star::sdbc::SQLWarning& _rWarning);
        virtual void appendWarning(const ::com::sun::star::sdb::SQLContext& _rContext);

        // XWarningsSupplier equivalents
        ::com::sun::star::uno::Any SAL_CALL getWarnings(  ) const;
        void SAL_CALL clearWarnings(  );
    };

//.........................................................................
}   // namespace dbtools
//.........................................................................

#endif // DBTOOLS_WARNINGSCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
