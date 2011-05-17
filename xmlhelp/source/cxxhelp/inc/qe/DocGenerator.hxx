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
#ifndef _XMLSEARCH_QE_DOCGENERATOR_HXX_
#define _XMLSEARCH_QE_DOCGENERATOR_HXX_

#include <rtl/ref.hxx>
#include  <rtl/ustring.hxx>
#include <excep/XmlSearchExceptions.hxx>
#include <util/CompressorIterator.hxx>
#include <util/Decompressor.hxx>


namespace xmlsearch {

    namespace qe {


        class Query;


        class NonnegativeIntegerGenerator
        {
        public:

            static const sal_Int32 END;

            virtual ~NonnegativeIntegerGenerator() { };
            virtual sal_Int32 first() throw( xmlsearch::excep::XmlSearchException ) = 0;
            virtual sal_Int32 next() throw( xmlsearch::excep::XmlSearchException ) = 0;
        };


        class RoleFiller
        {
        public:

            static RoleFiller* STOP() { return &roleFiller_; }

            RoleFiller();

            ~RoleFiller();

            void acquire() { ++m_nRefcount; }
            void release() { if( ! --m_nRefcount ) delete this; }

        private:

            static RoleFiller roleFiller_;

            sal_uInt32     m_nRefcount;
            sal_uInt8      fixedRole_;
            sal_Int16      filled_;
            sal_Int32      begin_,end_,parentContext_,limit_;

            RoleFiller*    next_;
            std::vector< RoleFiller* > fillers_;
        };


        class GeneratorHeap
        {
        public:

            GeneratorHeap()
                : heapSize_( 0 ),
                  free_( 0 )
            { }
        private:

            sal_Int32   heapSize_,free_;
        };



    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
