/*************************************************************************
 *
 *  $RCSfile: indexcollection.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-16 16:22:20 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _DBAUI_INDEXCOLLECTION_HXX_
#include "indexcollection.hxx"
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XAPPEND_HPP_
#include <com/sun/star/sdbcx/XAppend.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDATADESCRIPTORFACTORY_HPP_
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XDROP_HPP_
#include <com/sun/star/sdbcx/XDrop.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

//......................................................................
namespace dbaui
{
//......................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::sdbc;

    //==================================================================
    //= OIndexCollection
    //==================================================================
    //------------------------------------------------------------------
    OIndexCollection::OIndexCollection()
    {
    }

    //------------------------------------------------------------------
    OIndexCollection::OIndexCollection(const OIndexCollection& _rSource)
    {
        *this = _rSource;
    }

    //------------------------------------------------------------------
    OIndexCollection::OIndexCollection(const Reference< XNameAccess >& _rxIndexes)
    {
        implConstructFrom(_rxIndexes);
    }

    //------------------------------------------------------------------
    const OIndexCollection& OIndexCollection::operator=(const OIndexCollection& _rSource)
    {
        detach();
        m_xIndexes = _rSource.m_xIndexes;
        m_aIndexes = _rSource.m_aIndexes;
        return *this;
    }

    //------------------------------------------------------------------
    void OIndexCollection::attach(const Reference< XNameAccess >& _rxIndexes)
    {
        implConstructFrom(_rxIndexes);
    }

    //------------------------------------------------------------------
    void OIndexCollection::detach()
    {
        m_xIndexes.clear();
        m_aIndexes.clear();
    }

    //------------------------------------------------------------------
    OIndexCollection::const_iterator OIndexCollection::find(const String& _rName) const
    {
        ::rtl::OUString sNameCompare(_rName);

        // loop'n'compare
        const_iterator aSearch = m_aIndexes.begin();
        for (; aSearch != m_aIndexes.end(); ++aSearch)
            if (aSearch->sName == sNameCompare)
                break;

        return aSearch;
    }

    //------------------------------------------------------------------
    OIndexCollection::iterator OIndexCollection::find(const String& _rName)
    {
        ::rtl::OUString sNameCompare(_rName);

        // loop'n'compare
        iterator aSearch = m_aIndexes.begin();
        for (; aSearch != m_aIndexes.end(); ++aSearch)
            if (aSearch->sName == sNameCompare)
                break;

        return aSearch;
    }

    //------------------------------------------------------------------
    OIndexCollection::const_iterator OIndexCollection::findOriginal(const String& _rName) const
    {
        ::rtl::OUString sNameCompare(_rName);

        // loop'n'compare
        const_iterator aSearch = m_aIndexes.begin();
        for (; aSearch != m_aIndexes.end(); ++aSearch)
            if (aSearch->getOriginalName() == sNameCompare)
                break;

        return aSearch;
    }

    //------------------------------------------------------------------
    OIndexCollection::iterator OIndexCollection::findOriginal(const String& _rName)
    {
        ::rtl::OUString sNameCompare(_rName);

        // loop'n'compare
        iterator aSearch = m_aIndexes.begin();
        for (; aSearch != m_aIndexes.end(); ++aSearch)
            if (aSearch->getOriginalName() == sNameCompare)
                break;

        return aSearch;
    }

    //------------------------------------------------------------------
    void OIndexCollection::commitNewIndex(const iterator& _rPos) SAL_THROW((SQLException))
    {
        OSL_ENSURE(_rPos->isNew(), "OIndexCollection::commitNewIndex: index must be new!");

        try
        {
            Reference< XDataDescriptorFactory > xIndexFactory(m_xIndexes, UNO_QUERY);
            Reference< XAppend > xAppendIndex(xIndexFactory, UNO_QUERY);
            if (!xAppendIndex.is())
            {
                OSL_ENSURE(sal_False, "OIndexCollection::commitNewIndex: missing an interface of the index container!");
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
                OSL_ENSURE(sal_False, "OIndexCollection::commitNewIndex: invalid index descriptor returned!");
                return;
            }

            // set the properties
            static const ::rtl::OUString s_sUniquePropertyName = ::rtl::OUString::createFromAscii("IsUnique");
            static const ::rtl::OUString s_sSortPropertyName = ::rtl::OUString::createFromAscii("IsAscending");
            static const ::rtl::OUString s_sNamePropertyName = ::rtl::OUString::createFromAscii("Name");
            // the index' own props
            xIndexDescriptor->setPropertyValue(s_sUniquePropertyName, ::cppu::bool2any(_rPos->bUnique));
            xIndexDescriptor->setPropertyValue(s_sNamePropertyName, makeAny(_rPos->sName));

            // the fields
            for (   ConstIndexFieldsIterator aFieldLoop = _rPos->aFields.begin();
                    aFieldLoop != _rPos->aFields.end();
                    ++aFieldLoop
                )
            {
                Reference< XPropertySet > xColDescriptor = xColumnFactory->createDataDescriptor();
                OSL_ENSURE(xColDescriptor.is(), "OIndexCollection::commitNewIndex: invalid column descriptor!");
                if (xColDescriptor.is())
                {
                    xColDescriptor->setPropertyValue(s_sSortPropertyName, ::cppu::bool2any(aFieldLoop->bSortAscending));
                    xColDescriptor->setPropertyValue(s_sNamePropertyName, makeAny(::rtl::OUString(aFieldLoop->sFieldName)));
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
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OIndexCollection::commitNewIndex: caught a strange exception ....");
            return;
        }
    }

    //------------------------------------------------------------------
    sal_Bool OIndexCollection::dropNoRemove(const iterator& _rPos) SAL_THROW((SQLException))
    {
        try
        {
            OSL_ENSURE(m_xIndexes->hasByName(_rPos->getOriginalName()), "OIndexCollection::drop: invalid name!");

            Reference< XDrop > xDropIndex(m_xIndexes, UNO_QUERY);
            if (!xDropIndex.is())
            {
                OSL_ENSURE(sal_False, "OIndexCollection::drop: no XDrop interface!");
                return sal_False;
            }

            xDropIndex->dropByName(_rPos->getOriginalName());
        }
        catch(SQLException&)
        {   // allowed to pass
            throw;
        }
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OIndexCollection::drop: caught a strange exception ....");
            return sal_False;
        }

        // adjust the OIndex structure
        iterator aDropped = findOriginal(_rPos->getOriginalName());
        OSL_ENSURE(aDropped != m_aIndexes.end(), "OIndexCollection::drop: invalid original name, but successfull commit?!");
        aDropped->flagAsNew(GrantIndexAccess());

        return sal_True;
    }

    //------------------------------------------------------------------
    sal_Bool OIndexCollection::drop(const iterator& _rPos) SAL_THROW((SQLException))
    {
        OSL_ENSURE((_rPos >= m_aIndexes.begin()) && (_rPos < m_aIndexes.end()),
            "OIndexCollection::drop: invalid position (fasten your seatbelt .... this will crash)!");

        if (!_rPos->isNew())
            if (!dropNoRemove(_rPos))
                return sal_False;

        // adjust the index array
        m_aIndexes.erase(_rPos);
        return sal_True;
    }

    //------------------------------------------------------------------
    void OIndexCollection::implFillIndexInfo(OIndex& _rIndex) SAL_THROW((Exception))
    {
        // get the UNO descriptor for the index
        Reference< XPropertySet > xIndex;
        m_xIndexes->getByName(_rIndex.getOriginalName()) >>= xIndex;
        if (!xIndex.is())
        {
            OSL_ENSURE(sal_False, "OIndexCollection::implFillIndexInfo: got an invalid index object!");
        }
        else
            implFillIndexInfo(_rIndex, xIndex);
    }

    //------------------------------------------------------------------
    void OIndexCollection::implFillIndexInfo(OIndex& _rIndex, Reference< XPropertySet > _rxDescriptor) SAL_THROW((Exception))
    {
        static const ::rtl::OUString s_sPrimaryIndexPropertyName = ::rtl::OUString::createFromAscii("IsPrimaryKeyIndex");
        static const ::rtl::OUString s_sUniquePropertyName = ::rtl::OUString::createFromAscii("IsUnique");
        static const ::rtl::OUString s_sSortPropertyName = ::rtl::OUString::createFromAscii("IsAscending");
        static const ::rtl::OUString s_sCatalogPropertyName = ::rtl::OUString::createFromAscii("Catalog");

        _rIndex.bPrimaryKey = ::cppu::any2bool(_rxDescriptor->getPropertyValue(s_sPrimaryIndexPropertyName));
        _rIndex.bUnique = ::cppu::any2bool(_rxDescriptor->getPropertyValue(s_sUniquePropertyName));
        _rxDescriptor->getPropertyValue(s_sCatalogPropertyName) >>= _rIndex.sDescription;

        // the columns
        Reference< XColumnsSupplier > xSuppCols(_rxDescriptor, UNO_QUERY);
        Reference< XNameAccess > xCols;
        if (xSuppCols.is())
            xCols = xSuppCols->getColumns();
        OSL_ENSURE(xCols.is(), "OIndexCollection::implFillIndexInfo: the index does not have columns!");
        if (xCols.is())
        {
            Sequence< ::rtl::OUString > aFieldNames = xCols->getElementNames();
            _rIndex.aFields.resize(aFieldNames.getLength());

            const ::rtl::OUString* pFieldNames = aFieldNames.getConstArray();
            const ::rtl::OUString* pFieldNamesEnd = pFieldNames + aFieldNames.getLength();
            OIndexField* pCopyTo = _rIndex.aFields.begin();

            Reference< XPropertySet > xIndexColumn;
            for (;pFieldNames < pFieldNamesEnd; ++pFieldNames, ++pCopyTo)
            {
                // extract the column
                xIndexColumn.clear();
                xCols->getByName(*pFieldNames) >>= xIndexColumn;
                if (!xIndexColumn.is())
                {
                    OSL_ENSURE(sal_False, "OIndexCollection::implFillIndexInfo: invalid index column!");
                    --pCopyTo;
                    continue;
                }

                // get the relevant properties
                pCopyTo->sFieldName = *pFieldNames;
                pCopyTo->bSortAscending = ::cppu::any2bool(xIndexColumn->getPropertyValue(s_sSortPropertyName));
            }

            _rIndex.aFields.resize(pCopyTo - _rIndex.aFields.begin());
                // (just in case some fields were invalid ...)
        }
    }

    //------------------------------------------------------------------
    void OIndexCollection::resetIndex(const iterator& _rPos) SAL_THROW((SQLException))
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
        catch(Exception&)
        {
            OSL_ENSURE(sal_False, "OIndexCollection::resetIndex: caught an unknown exception!");
        }
    }

    //------------------------------------------------------------------
    OIndexCollection::iterator OIndexCollection::insert(const String& _rName)
    {
        OSL_ENSURE(end() == find(_rName), "OIndexCollection::insert: invalid new name!");

        OIndex aNewIndex(String()); // the empty string indicates the index is a new one
        aNewIndex.sName = _rName;
        m_aIndexes.push_back(aNewIndex);
        return m_aIndexes.end() - 1;    // the last element is the new one ...
    }

    //------------------------------------------------------------------
    void OIndexCollection::implConstructFrom(const Reference< XNameAccess >& _rxIndexes)
    {
        detach();

        m_xIndexes = _rxIndexes;
        if (m_xIndexes.is())
        {
            try
            {
                // loop through all the indexes
                Sequence< ::rtl::OUString > aNames = m_xIndexes->getElementNames();
                const ::rtl::OUString* pNames = aNames.getConstArray();
                const ::rtl::OUString* pEnd = pNames + aNames.getLength();
                for (; pNames < pEnd; ++pNames)
                {
                    // extract the index object
                    Reference< XPropertySet > xIndex;
                    m_xIndexes->getByName(*pNames) >>= xIndex;
                    if (!xIndex.is())
                    {
                        OSL_ENSURE(sal_False, "OIndexCollection::implConstructFrom: got an invalid index object ... ignoring!");
                        continue;
                    }

                    // fill the OIndex structure
                    OIndex aCurrentIndex(*pNames);
                    implFillIndexInfo(aCurrentIndex);
                    m_aIndexes.push_back(aCurrentIndex);
                }
            }
            catch(Exception&)
            {
                OSL_ENSURE(sal_False, "OIndexCollection::implConstructFrom: could not retrieve basic information from the UNO collection!");
            }
        }
    }

//......................................................................
}   // namespace dbaui
//......................................................................


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 07.03.01 15:09:42  fs
 ************************************************************************/

