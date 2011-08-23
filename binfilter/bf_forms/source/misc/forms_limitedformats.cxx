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

#include "limitedformats.hxx"
#include "services.hxx"
#include <osl/diagnose.h>
#include <comphelper/types.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/form/FormComponentType.hpp>
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::beans;

    sal_Int32								OLimitedFormats::s_nInstanceCount(0);
    ::osl::Mutex							OLimitedFormats::s_aMutex;
    Reference< XNumberFormatsSupplier >		OLimitedFormats::s_xStandardFormats;

    //=====================================================================
    //=
    //=====================================================================
    //---------------------------------------------------------------------
    enum LocaleType
    {
        ltEnglishUS,
        ltGerman,
        ltSystem
    };

    //---------------------------------------------------------------------
    static const Locale& getLocale(LocaleType _eType)
    {
        static const Locale s_aEnglishUS( ::rtl::OUString::createFromAscii("en"), ::rtl::OUString::createFromAscii("us"), ::rtl::OUString() );
        static const Locale s_aGerman( ::rtl::OUString::createFromAscii("de"), ::rtl::OUString::createFromAscii("DE"), ::rtl::OUString() );
        static const ::rtl::OUString s_sEmptyString;
        static const Locale s_aSystem( s_sEmptyString, s_sEmptyString, s_sEmptyString );

        switch (_eType)
        {
            case ltEnglishUS:
                return s_aEnglishUS;

            case ltGerman:
                return s_aGerman;

            case ltSystem:
                return s_aSystem;
        }

        OSL_ENSURE(sal_False, "getLocale: invalid enum value!");
        return s_aSystem;
    }

    //---------------------------------------------------------------------
    struct FormatEntry
    {
        const sal_Char* pDescription;
        sal_Int32		nKey;
        LocaleType		eLocale;
    };

    //---------------------------------------------------------------------
    static const FormatEntry* lcl_getFormatTable(sal_Int16 nTableId)
    {
        switch (nTableId)
        {
            case FormComponentType::TIMEFIELD:
            {
                static FormatEntry s_aFormats[] = {
                    { "HH:MM", -1, ltEnglishUS },
                    { "HH:MM:SS", -1, ltEnglishUS },
                    { "HH:MM AM/PM", -1, ltEnglishUS },
                    { "HH:MM:SS AM/PM", -1, ltEnglishUS },
                    { NULL, -1 }
                };
                // don't switch this table here to const. The compiler could be tempted to really place this
                // in a non-writeable segment, but we want to fill in the format keys later ....
                return s_aFormats;
            }
            break;
            case FormComponentType::DATEFIELD:
            {
                static FormatEntry s_aFormats[] = {
                    { "T-M-JJ", -1, ltGerman },
                    { "TT-MM-JJ", -1, ltGerman },
                    { "TT-MM-JJJJ", -1, ltGerman },
                    { "NNNNT. MMMM JJJJ", -1, ltGerman },

                    { "DD/MM/YY", -1, ltEnglishUS },
                    { "MM/DD/YY", -1, ltEnglishUS },
                    { "YY/MM/DD", -1, ltEnglishUS },
                    { "DD/MM/YYYY", -1, ltEnglishUS },
                    { "MM/DD/YYYY", -1, ltEnglishUS },
                    { "YYYY/MM/DD", -1, ltEnglishUS },

                    { "JJ-MM-TT", -1, ltGerman },
                    { "JJJJ-MM-TT", -1, ltGerman },

                    { NULL, -1 }
                };
                return s_aFormats;
            }
        }

        OSL_ENSURE(sal_False, "lcl_getFormatTable: invalid id!");
        return NULL;
    }

    //=====================================================================
    //= OLimitedFormats
    //=====================================================================
    //---------------------------------------------------------------------
    OLimitedFormats::OLimitedFormats(const Reference< XMultiServiceFactory >& _rxORB, const sal_Int16 _nClassId)
        :m_nTableId(_nClassId)
        ,m_nFormatEnumPropertyHandle(-1)
    {
        OSL_ENSURE(_rxORB.is(), "OLimitedFormats::OLimitedFormats: invalid service factory!");
        acquireSupplier(_rxORB);
        ensureTableInitialized(m_nTableId);
    }

    //---------------------------------------------------------------------
    OLimitedFormats::~OLimitedFormats()
    {
        releaseSupplier();
    }

    //---------------------------------------------------------------------
    void OLimitedFormats::ensureTableInitialized(const sal_Int16 _nTableId)
    {
        const FormatEntry* pFormatTable = lcl_getFormatTable(_nTableId);
        if (-1 == pFormatTable->nKey)
        {
            ::osl::MutexGuard aGuard(s_aMutex);
            if (-1 == pFormatTable->nKey)
            {
                // initialize the keys
                Reference<XNumberFormats> xStandardFormats;
                if (s_xStandardFormats.is())
                    xStandardFormats = s_xStandardFormats->getNumberFormats();
                OSL_ENSURE(xStandardFormats.is(), "OLimitedFormats::ensureTableInitialized: don't have a formats supplier!");

                if (xStandardFormats.is())
                {
                    // loop through the table
                    FormatEntry* pLoopFormats = const_cast<FormatEntry*>(pFormatTable);
                    while (pLoopFormats->pDescription)
                    {
                        // get the key for the description
                        pLoopFormats->nKey = xStandardFormats->queryKey(
                            ::rtl::OUString::createFromAscii(pLoopFormats->pDescription),
                            getLocale(pLoopFormats->eLocale),
                            sal_False
                        );

                        if (-1 == pLoopFormats->nKey)
                        {
                            pLoopFormats->nKey = xStandardFormats->addNew(
                                ::rtl::OUString::createFromAscii(pLoopFormats->pDescription),
                                getLocale(pLoopFormats->eLocale)
                            );
#ifdef DBG_UTIL
                            try
                            {
                                xStandardFormats->getByKey(pLoopFormats->nKey);
                            }
                            catch(const Exception&)
                            {
                                OSL_ENSURE(sal_False, "OLimitedFormats::ensureTableInitialized: adding the key to the formats collection failed!");
                            }
#endif
                        }

                        // next
                        ++pLoopFormats;
                    }
                }
            }
        }
    }

    //---------------------------------------------------------------------
    void OLimitedFormats::clearTable(const sal_Int16 _nTableId)
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        const FormatEntry* pFormats = lcl_getFormatTable(_nTableId);
        FormatEntry* pResetLoop = const_cast<FormatEntry*>(pFormats);
        while (pResetLoop->pDescription)
        {
            pResetLoop->nKey = -1;
            ++pResetLoop;
        }
    }

    //---------------------------------------------------------------------
    void OLimitedFormats::setAggregateSet(const Reference< XFastPropertySet >& _rxAggregate, sal_Int32 _nOriginalPropertyHandle)
    {
        // changes (NULL -> not NULL) and (not NULL -> NULL) are allowed
        OSL_ENSURE(!m_xAggregate.is() || !_rxAggregate.is(), "OLimitedFormats::setAggregateSet: already have an aggregate!");
        OSL_ENSURE(_rxAggregate.is() || m_xAggregate.is(), "OLimitedFormats::setAggregateSet: invalid new aggregate!");

        m_xAggregate = _rxAggregate;
        m_nFormatEnumPropertyHandle = _nOriginalPropertyHandle;
#ifdef DBG_UTIL
        if (m_xAggregate.is())
        {
            try
            {
                m_xAggregate->getFastPropertyValue(m_nFormatEnumPropertyHandle);
            }
            catch(const Exception&)
            {
                OSL_ENSURE(sal_False, "OLimitedFormats::setAggregateSet: invalid handle!");
            }
        }
#endif
    }

    //---------------------------------------------------------------------
    void OLimitedFormats::getFormatKeyPropertyValue( Any& _rValue ) const
    {
        _rValue.clear();

        OSL_ENSURE(m_xAggregate.is() && (-1 != m_nFormatEnumPropertyHandle), "OLimitedFormats::getFormatKeyPropertyValue: not initialized!");
        if (m_xAggregate.is())
        {
            // get the aggregate's enum property value
            Any aEnumPropertyValue = m_xAggregate->getFastPropertyValue(m_nFormatEnumPropertyHandle);
            sal_Int32 nValue = -1;
            ::cppu::enum2int(nValue, aEnumPropertyValue);

            // get the translation table
            const FormatEntry* pFormats = lcl_getFormatTable(m_nTableId);

            // seek to the nValue'th entry
            sal_Int32 nLookup = 0;
            for	(	;
                    (NULL != pFormats->pDescription) && (nLookup < nValue);
                    ++pFormats, ++nLookup
                )
                ;
            OSL_ENSURE(NULL != pFormats->pDescription, "OLimitedFormats::getFormatKeyPropertyValue: did not find the value!");
            if (pFormats->pDescription)
                _rValue <<= pFormats->nKey;
        }

        // TODO: should use a standard format for the control type we're working for
    }

    //---------------------------------------------------------------------
    sal_Bool OLimitedFormats::convertFormatKeyPropertyValue(Any& _rConvertedValue, Any& _rOldValue, const Any& _rNewValue)
    {
        OSL_ENSURE(m_xAggregate.is() && (-1 != m_nFormatEnumPropertyHandle), "OLimitedFormats::convertFormatKeyPropertyValue: not initialized!");

        if (m_xAggregate.is())
        {
            // the new format key to set
            sal_Int32 nNewFormat = 0;
            if (!(_rNewValue >>= nNewFormat))
                throw IllegalArgumentException();

            // get the old (enum) value from the aggregate
            Any aEnumPropertyValue = m_xAggregate->getFastPropertyValue(m_nFormatEnumPropertyHandle);
            sal_Int32 nOldEnumValue = -1;
            ::cppu::enum2int(nOldEnumValue, aEnumPropertyValue);

            // get the translation table
            const FormatEntry* pFormats = lcl_getFormatTable(m_nTableId);

            _rOldValue.clear();
            _rConvertedValue.clear();

            // look for the entry with the given format key
            sal_Int32 nTablePosition = 0;
            for	(	;
                    (NULL != pFormats->pDescription) && (nNewFormat != pFormats->nKey);
                    ++pFormats, ++nTablePosition
                )
            {
                if (nTablePosition == nOldEnumValue)
                    _rOldValue <<= pFormats->nKey;
            }

            sal_Bool bFoundIt = (NULL != pFormats->pDescription);
            sal_Bool bModified = sal_False;
            if (bFoundIt)
            {
                _rConvertedValue <<= (sal_Int16)nTablePosition;
                bModified = nTablePosition != nOldEnumValue;
            }

            if (!_rOldValue.hasValue())
            {	// did not reach the end of the table (means we found nNewFormat)
                // -> go to the end to ensure that _rOldValue is set
                while (pFormats->pDescription)
                {
                    if (nTablePosition == nOldEnumValue)
                    {
                        _rOldValue <<= pFormats->nKey;
                        break;
                    }

                    ++pFormats;
                    ++nTablePosition;
                }
            }

            OSL_ENSURE(_rOldValue.hasValue(), "OLimitedFormats::convertFormatKeyPropertyValue: did not find the old enum value in the table!");

            if (!bFoundIt)
            {	// somebody gave us an format which we can't translate
                ::rtl::OUString sMessage = ::rtl::OUString::createFromAscii("This control supports only a very limited number of formats.");
                throw IllegalArgumentException(sMessage, NULL, 2);
            }

            return bModified;
        }

        return sal_False;
    }

    //---------------------------------------------------------------------
    void OLimitedFormats::setFormatKeyPropertyValue( const Any& _rNewValue )
    {
        OSL_ENSURE(m_xAggregate.is() && (-1 != m_nFormatEnumPropertyHandle), "OLimitedFormats::setFormatKeyPropertyValue: not initialized!");

        if (m_xAggregate.is())
        {	// this is to be called after convertFormatKeyPropertyValue, where
            // we translated the format key into a enum value.
            // So now we can simply forward this enum value to our aggreate
            m_xAggregate->setFastPropertyValue(m_nFormatEnumPropertyHandle, _rNewValue);
        }
    }

    //---------------------------------------------------------------------
    void OLimitedFormats::acquireSupplier(const Reference< XMultiServiceFactory >& _rxORB)
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if ((1 == ++s_nInstanceCount) && _rxORB.is())
        {	// create the standard formatter

            Sequence< Any > aInit(1);
            aInit[0] <<= getLocale(ltEnglishUS);

            Reference< XInterface > xSupplier = _rxORB->createInstanceWithArguments(FRM_NUMBER_FORMATS_SUPPLIER, aInit);
            OSL_ENSURE(xSupplier.is(), "OLimitedFormats::OLimitedFormats: could not create a formats supplier!");

            s_xStandardFormats = Reference< XNumberFormatsSupplier >(xSupplier, UNO_QUERY);
            OSL_ENSURE(s_xStandardFormats.is() || !xSupplier.is(), "OLimitedFormats::OLimitedFormats: missing an interface!");
        }
    }

    //---------------------------------------------------------------------
    void OLimitedFormats::releaseSupplier()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if (0 == --s_nInstanceCount)
        {
            ::comphelper::disposeComponent(s_xStandardFormats);
            s_xStandardFormats = NULL;

            clearTable(FormComponentType::TIMEFIELD);
            clearTable(FormComponentType::DATEFIELD);
        }
    }

//.........................................................................
}	// namespace frm
//.........................................................................

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
