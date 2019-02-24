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

#include <indexcollection.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/sdbcx/XAppend.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#include <comphelper/extract.hxx>
#include <com/sun/star/sdbcx/XDrop.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

namespace dbaui
{

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdbc;

    // OIndexCollection
    OIndexCollection::OIndexCollection()
    {
    }

    OIndexCollection::OIndexCollection(const OIndexCollection& _rSource)
    {
        *this = _rSource;
    }

    OIndexCollection& OIndexCollection::operator=(const OIndexCollection& _rSource)
    {
        detach();
        m_xIndexes = _rSource.m_xIndexes;
        m_aIndexes = _rSource.m_aIndexes;
        return *this;
    }

    void OIndexCollection::attach(const Reference< XNameAccess >& _rxIndexes)
    {
        implConstructFrom(_rxIndexes);
    }

    void OIndexCollection::detach()
    {
        m_xIndexes.clear();
        m_aIndexes.clear();
    }

    Indexes::const_iterator OIndexCollection::find(const OUString& _rName) const
    {
        // loop'n'compare
        return std::find_if(m_aIndexes.cbegin(), m_aIndexes.cend(),
            [&_rName](const OIndex& rIndex) { return rIndex.sName == _rName; });
    }

    Indexes::iterator OIndexCollection::find(const OUString& _rName)
    {
        // loop'n'compare
        return std::find_if(m_aIndexes.begin(), m_aIndexes.end(),
            [&_rName](const OIndex& rIndex) { return rIndex.sName == _rName; });
    }

    Indexes::const_iterator OIndexCollection::findOriginal(const OUString& _rName) const
    {
        // loop'n'compare
        return std::find_if(m_aIndexes.cbegin(), m_aIndexes.cend(),
            [&_rName](const OIndex& rIndex) { return rIndex.getOriginalName() == _rName; });
    }

    Indexes::iterator OIndexCollection::findOriginal(const OUString& _rName)
    {
        // loop'n'compare
        return std::find_if(m_aIndexes.begin(), m_aIndexes.end(),
            [&_rName](const OIndex& rIndex) { return rIndex.getOriginalName() == _rName; });
    }

    void OIndexCollection::commitNewIndex(const Indexes::iterator& _rPos)
    {
        OSL_ENSURE(_rPos->isNew(), "OIndexCollection::commitNewIndex: index must be new!");

        try
        {
            Reference< XDataDescriptorFactory > xIndexFactory(m_xIndexes, UNO_QUERY);
            Reference< XAppend > xAppendIndex(xIndexFactory, UNO_QUERY);
            if (!xAppendIndex.is())
            {
                OSL_FAIL("OIndexCollection::commitNewIndex: missing an interface of the index container!");
                return;
            }

            Reference< XPropertySet > xIndexDescriptor = xIndexFactory->createDataDescriptor();
            Reference< XColumnsSupplier > xColsSupp(xIndexDescriptor, UNO_QUERY);
            Reference< XNameAccess > xCols;
            if (xColsSupp.is())
                xCols = xColsSupp->getColumns();

            Reference< XDataDescriptorFactory > xColumnFactory(xCols, UNO_QUERY);
            Reference< XAppend > xAppendCols(xColumnFactory, UNO_QUERY);
            if (!xAppendCols.is())
            {
                OSL_FAIL("OIndexCollection::commitNewIndex: invalid index descriptor returned!");
                return;
            }

            // set the properties
            static const char s_sNamePropertyName[] = "Name";
            // the index' own props
            xIndexDescriptor->setPropertyValue("IsUnique", css::uno::makeAny(_rPos->bUnique));
            xIndexDescriptor->setPropertyValue(s_sNamePropertyName, makeAny(_rPos->sName));

            // the fields
            for (auto const& field : _rPos->aFields)
            {
                OSL_ENSURE(!xCols->hasByName(field.sFieldName), "OIndexCollection::commitNewIndex: double column name (need to prevent this outside)!");

                Reference< XPropertySet > xColDescriptor = xColumnFactory->createDataDescriptor();
                OSL_ENSURE(xColDescriptor.is(), "OIndexCollection::commitNewIndex: invalid column descriptor!");
                if (xColDescriptor.is())
                {
                    xColDescriptor->setPropertyValue("IsAscending", css::uno::makeAny(field.bSortAscending));
                    xColDescriptor->setPropertyValue(s_sNamePropertyName, makeAny(field.sFieldName));
                    xAppendCols->appendByDescriptor(xColDescriptor);
                }
            }

            xAppendIndex->appendByDescriptor(xIndexDescriptor);

            _rPos->flagAsCommitted(GrantIndexAccess());
            _rPos->clearModified();
        }
        catch(SQLException&)
        {   // allowed to pass
            throw;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }

    bool OIndexCollection::dropNoRemove(const Indexes::iterator& _rPos)
    {
        try
        {
            OSL_ENSURE(m_xIndexes->hasByName(_rPos->getOriginalName()), "OIndexCollection::drop: invalid name!");

            Reference< XDrop > xDropIndex(m_xIndexes, UNO_QUERY);
            if (!xDropIndex.is())
            {
                OSL_FAIL("OIndexCollection::drop: no XDrop interface!");
                return false;
            }

            xDropIndex->dropByName(_rPos->getOriginalName());
        }
        catch(SQLException&)
        {   // allowed to pass
            throw;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
            return false;
        }

        // adjust the OIndex structure
        Indexes::iterator aDropped = findOriginal(_rPos->getOriginalName());
        OSL_ENSURE(aDropped != m_aIndexes.end(), "OIndexCollection::drop: invalid original name, but successful commit?!");
        aDropped->flagAsNew(GrantIndexAccess());

        return true;
    }

    bool OIndexCollection::drop(const Indexes::iterator& _rPos)
    {
        OSL_ENSURE((_rPos >= m_aIndexes.begin()) && (_rPos < m_aIndexes.end()),
            "OIndexCollection::drop: invalid position (fasten your seatbelt .... this will crash)!");

        if (!_rPos->isNew())
            if (!dropNoRemove(_rPos))
                return false;

        // adjust the index array
        m_aIndexes.erase(_rPos);
        return true;
    }

    void OIndexCollection::implFillIndexInfo(OIndex& _rIndex)
    {
        // get the UNO descriptor for the index
        Reference< XPropertySet > xIndex;
        m_xIndexes->getByName(_rIndex.getOriginalName()) >>= xIndex;
        if (!xIndex.is())
        {
            OSL_FAIL("OIndexCollection::implFillIndexInfo: got an invalid index object!");
        }
        else
            implFillIndexInfo(_rIndex, xIndex);
    }

    void OIndexCollection::implFillIndexInfo(OIndex& _rIndex, const Reference< XPropertySet >& _rxDescriptor)
    {
        _rIndex.bPrimaryKey = ::cppu::any2bool(_rxDescriptor->getPropertyValue("IsPrimaryKeyIndex"));
        _rIndex.bUnique = ::cppu::any2bool(_rxDescriptor->getPropertyValue("IsUnique"));
        _rxDescriptor->getPropertyValue("Catalog") >>= _rIndex.sDescription;

        // the columns
        Reference< XColumnsSupplier > xSuppCols(_rxDescriptor, UNO_QUERY);
        Reference< XNameAccess > xCols;
        if (xSuppCols.is())
            xCols = xSuppCols->getColumns();
        OSL_ENSURE(xCols.is(), "OIndexCollection::implFillIndexInfo: the index does not have columns!");
        if (xCols.is())
        {
            Sequence< OUString > aFieldNames = xCols->getElementNames();
            _rIndex.aFields.resize(aFieldNames.getLength());

            const OUString* pFieldNames = aFieldNames.getConstArray();
            const OUString* pFieldNamesEnd = pFieldNames + aFieldNames.getLength();
            IndexFields::iterator aCopyTo = _rIndex.aFields.begin();

            Reference< XPropertySet > xIndexColumn;
            for (;pFieldNames < pFieldNamesEnd; ++pFieldNames, ++aCopyTo)
            {
                // extract the column
                xIndexColumn.clear();
                xCols->getByName(*pFieldNames) >>= xIndexColumn;
                if (!xIndexColumn.is())
                {
                    OSL_FAIL("OIndexCollection::implFillIndexInfo: invalid index column!");
                    --aCopyTo;
                    continue;
                }

                // get the relevant properties
                aCopyTo->sFieldName = *pFieldNames;
                aCopyTo->bSortAscending = ::cppu::any2bool(xIndexColumn->getPropertyValue("IsAscending"));
            }

            _rIndex.aFields.resize(aCopyTo - _rIndex.aFields.begin());
                // (just in case some fields were invalid ...)
        }
    }

    void OIndexCollection::resetIndex(const Indexes::iterator& _rPos)
    {
        OSL_ENSURE(_rPos >= m_aIndexes.begin() && _rPos < m_aIndexes.end(),
            "OIndexCollection::resetIndex: invalid position!");

        try
        {
            _rPos->sName = _rPos->getOriginalName();
            implFillIndexInfo(*_rPos);

            _rPos->clearModified();
            _rPos->flagAsCommitted(GrantIndexAccess());
        }
        catch(SQLException&)
        {   // allowed to pass
            throw;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
    }

    Indexes::iterator OIndexCollection::insert(const OUString& _rName)
    {
        OSL_ENSURE(end() == find(_rName), "OIndexCollection::insert: invalid new name!");
        OIndex aNewIndex((OUString()));  // the empty string indicates the index is a new one
        aNewIndex.sName = _rName;
        m_aIndexes.push_back(aNewIndex);
        return m_aIndexes.end() - 1;    // the last element is the new one ...
    }

    void OIndexCollection::implConstructFrom(const Reference< XNameAccess >& _rxIndexes)
    {
        detach();

        m_xIndexes = _rxIndexes;
        if (m_xIndexes.is())
        {
            // loop through all the indexes
            Sequence< OUString > aNames = m_xIndexes->getElementNames();
            const OUString* pNames = aNames.getConstArray();
            const OUString* pEnd = pNames + aNames.getLength();
            for (; pNames < pEnd; ++pNames)
            {
                // extract the index object
                Reference< XPropertySet > xIndex;
                m_xIndexes->getByName(*pNames) >>= xIndex;
                if (!xIndex.is())
                {
                    OSL_FAIL("OIndexCollection::implConstructFrom: got an invalid index object ... ignoring!");
                    continue;
                }

                // fill the OIndex structure
                OIndex aCurrentIndex(*pNames);
                implFillIndexInfo(aCurrentIndex);
                m_aIndexes.push_back(aCurrentIndex);
            }
        }
    }

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
