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
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#define _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_

#include  <rtl/ustring.hxx>


namespace xmlsearch {

    namespace excep {


        class XmlSearchException
        {
        public:

            XmlSearchException( const rtl::OUString& message )
                : _message( message )
            {
            }

            rtl::OUString getMessage() const
            {
                return _message;
            }


        private:

            rtl::OUString _message;
        };


        class IOException
            : public virtual XmlSearchException
        {
        public:

            IOException( const rtl::OUString& message )
                : XmlSearchException( message )
            {
            }
        };


        class NoFactoryException
            : public virtual XmlSearchException
        {
        public:
            NoFactoryException( const rtl::OUString& message )
                : XmlSearchException( message )
            {
            }
        };


        class NoSuchBlock
            : public virtual XmlSearchException
        {
        public:
            NoSuchBlock( const rtl::OUString& message )
                : XmlSearchException( message )
            {
            }
        };


        class IllegalIndexException
            : public virtual XmlSearchException
        {
        public:
            IllegalIndexException( const rtl::OUString& message )
                : XmlSearchException( message )
            {
            }
        };

    }
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
