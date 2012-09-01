/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_
#define _XMLSEARCH_EXCEP_XMLSEARCHEXCEPTIONS_HXX_

#include  <rtl/ustring.hxx>


namespace xmlsearch {

    namespace excep {


        class XmlSearchException
        {
        public:

            XmlSearchException( const OUString& message )
                : _message( message )
            {
            }

            OUString getMessage() const
            {
                return _message;
            }


        private:

            OUString _message;
        };


        class IOException
            : public virtual XmlSearchException
        {
        public:

            IOException( const OUString& message )
                : XmlSearchException( message )
            {
            }
        };


        class NoFactoryException
            : public virtual XmlSearchException
        {
        public:
            NoFactoryException( const OUString& message )
                : XmlSearchException( message )
            {
            }
        };


        class NoSuchBlock
            : public virtual XmlSearchException
        {
        public:
            NoSuchBlock( const OUString& message )
                : XmlSearchException( message )
            {
            }
        };


        class IllegalIndexException
            : public virtual XmlSearchException
        {
        public:
            IllegalIndexException( const OUString& message )
                : XmlSearchException( message )
            {
            }
        };

    }
}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
