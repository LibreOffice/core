/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2009 by Sun Microsystems, Inc.
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
************************************************************************/

#ifndef STORAGESTREAM_HXX
#define STORAGESTREAM_HXX

#include "dbaccessdllapi.h"

/** === begin UNO includes === **/
#include <com/sun/star/embed/XStorage.hpp>
/** === end UNO includes === **/

namespace comphelper
{
    class ComponentContext;
}

//........................................................................
namespace dbaccess
{
//........................................................................

    //====================================================================
    //= StorageOutputStream
    //====================================================================
    /** convenience wrapper around a stream living in a storage
    */
    class DBACCESS_DLLPRIVATE StorageOutputStream
    {
    public:
        StorageOutputStream(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const ::rtl::OUString& i_rStreamName
        );
        virtual ~StorageOutputStream();

        /** simply calls closeOutput on our output stream, override to extend/modify this behavior
        */
        virtual void close();

    protected:
        const ::comphelper::ComponentContext&   getContext() const { return m_rContext; }
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >&
                                                getOutputStream() const { return m_xOutputStream; }

    private:
        const ::comphelper::ComponentContext&   m_rContext;
              ::com::sun::star::uno::Reference< ::com::sun::star::io::XOutputStream >
                                                m_xOutputStream;
    };

    //====================================================================
    //= StorageInputStream
    //====================================================================
    /** convenience wrapper around a stream living in a storage
    */
    class DBACCESS_DLLPRIVATE StorageInputStream
    {
    public:
        StorageInputStream(
            const ::comphelper::ComponentContext& i_rContext,
            const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& i_rParentStorage,
            const ::rtl::OUString& i_rStreamName
        );
        virtual ~StorageInputStream();

        /** simply calls closeInput on our input stream, override to extend/modify this behavior
        */
        virtual void close();

    protected:
        const ::comphelper::ComponentContext&   getContext() const { return m_rContext; }
        const ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >&
                                                getInputStream() const { return m_xInputStream; }

    private:
        const ::comphelper::ComponentContext&   m_rContext;
              ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                                                m_xInputStream;
    };

//........................................................................
} // namespace dbaccess
//........................................................................

#endif // STORAGESTREAM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
