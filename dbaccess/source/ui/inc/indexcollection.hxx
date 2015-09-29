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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXCOLLECTION_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXCOLLECTION_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/SQLException.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "indexes.hxx"

namespace dbaui
{

    // OIndexCollection
    class OIndexCollection
    {
    protected:
        css::uno::Reference< css::container::XNameAccess >
                    m_xIndexes;

        // cached information
        Indexes     m_aIndexes;

    public:
        // construction
        OIndexCollection();
        OIndexCollection(const OIndexCollection& _rSource);
        //  OIndexCollection(const css::uno::Reference< css::container::XNameAccess >& _rxIndexes);

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
        Indexes::const_iterator find(const OUString& _rName) const;
        Indexes::iterator find(const OUString& _rName);
        Indexes::const_iterator findOriginal(const OUString& _rName) const;
        Indexes::iterator findOriginal(const OUString& _rName);

        // inserting without committing
        // the OriginalName of the newly inserted index will be empty, thus indicating that it's new
        Indexes::iterator insert(const OUString& _rName);
        // commit a new index, which is already part if the collection, but does not have an equivalent in the
        // data source, yet
        void commitNewIndex(const Indexes::iterator& _rPos);

        // reset the data for the given index
        void resetIndex(const Indexes::iterator& _rPos);

        // attach to a new key container
        void attach(const css::uno::Reference< css::container::XNameAccess >& _rxIndexes);
        // detach from the container
        void detach();

        // number of contained indexes
        sal_Int32   size() const { return m_aIndexes.size(); }

        /// drop an index, and remove it from the collection
        bool    drop(const Indexes::iterator& _rPos);
        /// simply drop the index described by the name, but don't remove the descriptor from the collection
        bool    dropNoRemove(const Indexes::iterator& _rPos);

    protected:
        void implConstructFrom(const css::uno::Reference< css::container::XNameAccess >& _rxIndexes);
        static void implFillIndexInfo(OIndex& _rIndex, css::uno::Reference< css::beans::XPropertySet > _rxDescriptor);
        void implFillIndexInfo(OIndex& _rIndex);
    };

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_INDEXCOLLECTION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
