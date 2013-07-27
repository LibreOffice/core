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

#include "limitedformats.hxx"
#include "services.hxx"
#include <osl/diagnose.h>
#include <comphelper/types.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>

//.........................................................................
namespace frm
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::beans;

    sal_Int32                               OLimitedFormats::s_nInstanceCount(0);
    ::osl::Mutex                            OLimitedFormats::s_aMutex;
    Reference< XNumberFormatsSupplier >     OLimitedFormats::s_xStandardFormats;

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
        static const Locale s_aEnglishUS( OUString("en"), OUString("us"), OUString() );
        static const Locale s_aGerman( OUString("de"), OUString("DE"), OUString() );
        static const OUString s_sEmptyString;
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

        OSL_FAIL("getLocale: invalid enum value!");
        return s_aSystem;
    }

    //---------------------------------------------------------------------
    struct FormatEntry
    {
        const sal_Char* pDescription;
        sal_Int32       nKey;
        LocaleType      eLocale;
    };

    //---------------------------------------------------------------------
    static FormatEntry* lcl_getFormatTable(sal_Int16 nTableId)
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
                    { NULL, -1, ltSystem }
                };
                // don't switch this table here to const. The compiler could be tempted to really place this
                // in a non-writeable segment, but we want to fill in the format keys later ....
                return s_aFormats;
            }
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

                    { NULL, -1, ltSystem }
                };
                return s_aFormats;
            }
        }

        OSL_FAIL("lcl_getFormatTable: invalid id!");
        return NULL;
    }

    //=====================================================================
    //= OLimitedFormats
    //=====================================================================
    //---------------------------------------------------------------------
    OLimitedFormats::OLimitedFormats(const Reference< XComponentContext >& _rxContext, const sal_Int16 _nClassId)
        :m_nFormatEnumPropertyHandle(-1)
        ,m_nTableId(_nClassId)
    {
        OSL_ENSURE(_rxContext.is(), "OLimitedFormats::OLimitedFormats: invalid service factory!");
        acquireSupplier(_rxContext);
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
        FormatEntry* pFormatTable = lcl_getFormatTable(_nTableId);
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
                    FormatEntry* pLoopFormats = pFormatTable;
                    while (pLoopFormats->pDescription)
                    {
                        // get the key for the description
                        pLoopFormats->nKey = xStandardFormats->queryKey(
                            OUString::createFromAscii(pLoopFormats->pDescription),
                            getLocale(pLoopFormats->eLocale),
                            sal_False
                        );

                        if (-1 == pLoopFormats->nKey)
                        {
                            pLoopFormats->nKey = xStandardFormats->addNew(
                                OUString::createFromAscii(pLoopFormats->pDescription),
                                getLocale(pLoopFormats->eLocale)
                            );
#ifdef DBG_UTIL
                            try
                            {
                                xStandardFormats->getByKey(pLoopFormats->nKey);
                            }
                            catch(const Exception&)
                            {
                                OSL_FAIL("OLimitedFormats::ensureTableInitialized: adding the key to the formats collection failed!");
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
        FormatEntry* pFormats = lcl_getFormatTable(_nTableId);
        FormatEntry* pResetLoop = pFormats;
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
                OSL_FAIL("OLimitedFormats::setAggregateSet: invalid handle!");
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
            for (   ;
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
            for (   ;
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
            {   // did not reach the end of the table (means we found nNewFormat)
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
            {   // somebody gave us an format which we can't translate
                OUString sMessage ("This control supports only a very limited number of formats.");
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
        {   // this is to be called after convertFormatKeyPropertyValue, where
            // we translated the format key into a enum value.
            // So now we can simply forward this enum value to our aggreate
            m_xAggregate->setFastPropertyValue(m_nFormatEnumPropertyHandle, _rNewValue);
        }
    }

    //---------------------------------------------------------------------
    void OLimitedFormats::acquireSupplier(const Reference< XComponentContext >& _rxContext)
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if (1 == ++s_nInstanceCount)
        {   // create the standard formatter
            s_xStandardFormats = NumberFormatsSupplier::createWithLocale(_rxContext, getLocale(ltEnglishUS));
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
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
