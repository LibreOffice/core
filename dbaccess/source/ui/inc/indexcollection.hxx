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

#ifndef _DBAUI_INDEXCOLLECTION_HXX_
#define _DBAUI_INDEXCOLLECTION_HXX_

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "indexes.hxx"

//......................................................................
namespace dbaui
{
//......................................................................

    //==================================================================
    //= OIndexCollection
    //==================================================================
    class OIndexCollection
    {
    protected:
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                    m_xIndexes;

        // cached information
        Indexes     m_aIndexes;

    public:
        // construction
        OIndexCollection();
        OIndexCollection(const OIndexCollection& _rSource);
        //  OIndexCollection(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxIndexes);

        const OIndexCollection& operator=(const OIndexCollection& _rSource);

        // iterating through the collection
        typedef OIndex* iterator;
        typedef OIndex const* const_iterator;

        /// get access to the first element of the index collection
        Indexes::const_iterator begin() const { return m_aIndexes.begin(); }
        /// get access to the first element of the index collection
        Indexes::iterator begin() { return m_aIndexes.begin(); }
        /// get access to the (last + 1st) element of the index collection
        Indexes::const_iterator end() const { return m_aIndexes.end(); }
        /// get access to the (last + 1st) element of the index collection
        Indexes::iterator end() { return m_aIndexes.end(); }

        // searching
        Indexes::const_iterator find(const String& _rName) const;
        Indexes::iterator find(const String& _rName);
        Indexes::const_iterator findOriginal(const String& _rName) const;
        Indexes::iterator findOriginal(const String& _rName);

        // inserting without committing
        // the OriginalName of the newly inserted index will be empty, thus indicating that it's new
        Indexes::iterator insert(const String& _rName);
        // commit a new index, which is already part if the collection, but does not have an equivalent in the
        // data source, yet
        void commitNewIndex(const Indexes::iterator& _rPos) SAL_THROW((::com::sun::star::sdbc::SQLException));

        // reset the data for the given index
        void resetIndex(const Indexes::iterator& _rPos) SAL_THROW((::com::sun::star::sdbc::SQLException));

        // attach to a new key container
        void attach(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxIndexes);
        // detach from the container
        void detach();

        // is the object valid?
        sal_Bool    isValid() const { return m_xIndexes.is(); }
        // number of contained indexes
        sal_Int32   size() const { return m_aIndexes.size(); }

        /// drop an index, and remove it from the collection
        sal_Bool    drop(const Indexes::iterator& _rPos) SAL_THROW((::com::sun::star::sdbc::SQLException));
        /// simply drop the index described by the name, but don't remove the descriptor from the collection
        sal_Bool    dropNoRemove(const Indexes::iterator& _rPos) SAL_THROW((::com::sun::star::sdbc::SQLException));

    protected:
        void implConstructFrom(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& _rxIndexes);
        void implFillIndexInfo(OIndex& _rIndex, ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > _rxDescriptor) SAL_THROW((::com::sun::star::uno::Exception));
        void implFillIndexInfo(OIndex& _rIndex) SAL_THROW((::com::sun::star::uno::Exception));
    };


//......................................................................
}   // namespace dbaui
//......................................................................

#endif // _DBAUI_INDEXCOLLECTION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
