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

#include <limitedformats.hxx>
#include <osl/diagnose.h>
#include <comphelper/types.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/util/NumberFormatsSupplier.hpp>
#include <span>

namespace frm
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::beans;

    sal_Int32                               OLimitedFormats::s_nInstanceCount(0);
    ::osl::Mutex                            OLimitedFormats::s_aMutex;
    Reference< XNumberFormatsSupplier >     OLimitedFormats::s_xStandardFormats;


    //=

    namespace {

    enum LocaleType
    {
        ltEnglishUS,
        ltGerman,
        ltSystem
    };

    }

    static const Locale& getLocale(LocaleType _eType)
    {
        static const Locale s_aEnglishUS( u"en"_ustr, u"us"_ustr, OUString() );
        static const Locale s_aGerman( u"de"_ustr, u"DE"_ustr, OUString() );
        static const Locale s_aSystem( u""_ustr, u""_ustr, u""_ustr );

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

    namespace {

    struct FormatEntry
    {
        OUString     aDescription;
        sal_Int32    nKey;
        LocaleType   eLocale;
    };

    }

    static std::span<FormatEntry> lcl_getFormatTable(sal_Int16 nTableId)
    {
        switch (nTableId)
        {
            case FormComponentType::TIMEFIELD:
            {
                static FormatEntry s_aFormats[] = {
                    { u"HH:MM"_ustr, -1, ltEnglishUS },
                    { u"HH:MM:SS"_ustr, -1, ltEnglishUS },
                    { u"HH:MM AM/PM"_ustr, -1, ltEnglishUS },
                    { u"HH:MM:SS AM/PM"_ustr, -1, ltEnglishUS }
                };
                return s_aFormats;
            }
            case FormComponentType::DATEFIELD:
            {
                static FormatEntry s_aFormats[] = {
                    { u"T-M-JJ"_ustr, -1, ltGerman },
                    { u"TT-MM-JJ"_ustr, -1, ltGerman },
                    { u"TT-MM-JJJJ"_ustr, -1, ltGerman },
                    { u"NNNNT. MMMM JJJJ"_ustr, -1, ltGerman },

                    { u"DD/MM/YY"_ustr, -1, ltEnglishUS },
                    { u"MM/DD/YY"_ustr, -1, ltEnglishUS },
                    { u"YY/MM/DD"_ustr, -1, ltEnglishUS },
                    { u"DD/MM/YYYY"_ustr, -1, ltEnglishUS },
                    { u"MM/DD/YYYY"_ustr, -1, ltEnglishUS },
                    { u"YYYY/MM/DD"_ustr, -1, ltEnglishUS },

                    { u"JJ-MM-TT"_ustr, -1, ltGerman },
                    { u"JJJJ-MM-TT"_ustr, -1, ltGerman }
                };
                return s_aFormats;
            }
        }

        OSL_FAIL("lcl_getFormatTable: invalid id!");
        return {};
    }

    OLimitedFormats::OLimitedFormats(const Reference< XComponentContext >& _rxContext, const sal_Int16 _nClassId)
        :m_nFormatEnumPropertyHandle(-1)
        ,m_nTableId(_nClassId)
    {
        OSL_ENSURE(_rxContext.is(), "OLimitedFormats::OLimitedFormats: invalid service factory!");
        acquireSupplier(_rxContext);
        ensureTableInitialized(m_nTableId);
    }


    OLimitedFormats::~OLimitedFormats()
    {
        releaseSupplier();
    }


    void OLimitedFormats::ensureTableInitialized(const sal_Int16 _nTableId)
    {
        std::span<FormatEntry> pFormatTable = lcl_getFormatTable(_nTableId);
        if (-1 != pFormatTable[0].nKey)
            return;

        ::osl::MutexGuard aGuard(s_aMutex);
        if (-1 != pFormatTable[0].nKey)
            return;

        // initialize the keys
        Reference<XNumberFormats> xStandardFormats;
        if (s_xStandardFormats.is())
            xStandardFormats = s_xStandardFormats->getNumberFormats();
        OSL_ENSURE(xStandardFormats.is(), "OLimitedFormats::ensureTableInitialized: don't have a formats supplier!");

        if (!xStandardFormats.is())
            return;

        // loop through the table
        for (FormatEntry & rLoopFormats : pFormatTable)
        {
            // get the key for the description
            rLoopFormats.nKey = xStandardFormats->queryKey(
                rLoopFormats.aDescription,
                getLocale(rLoopFormats.eLocale),
                false
            );

            if (-1 == rLoopFormats.nKey)
            {
                rLoopFormats.nKey = xStandardFormats->addNew(
                    rLoopFormats.aDescription,
                    getLocale(rLoopFormats.eLocale)
                );
#ifdef DBG_UTIL
                try
                {
                    xStandardFormats->getByKey(rLoopFormats.nKey);
                }
                catch(const Exception&)
                {
                    OSL_FAIL("OLimitedFormats::ensureTableInitialized: adding the key to the formats collection failed!");
                }
#endif
            }
        }
    }


    void OLimitedFormats::clearTable(const sal_Int16 _nTableId)
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        std::span<FormatEntry> pFormats = lcl_getFormatTable(_nTableId);
        for (FormatEntry & rResetLoop : pFormats)
            rResetLoop.nKey = -1;
    }


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


    void OLimitedFormats::getFormatKeyPropertyValue( Any& _rValue ) const
    {
        _rValue.clear();

        OSL_ENSURE(m_xAggregate.is() && (-1 != m_nFormatEnumPropertyHandle), "OLimitedFormats::getFormatKeyPropertyValue: not initialized!");
        if (!m_xAggregate.is())
            return;

        // get the aggregate's enum property value
        Any aEnumPropertyValue = m_xAggregate->getFastPropertyValue(m_nFormatEnumPropertyHandle);
        sal_Int32 nValue = -1;
        ::cppu::enum2int(nValue, aEnumPropertyValue);

        // get the translation table
        std::span<FormatEntry> pFormats = lcl_getFormatTable(m_nTableId);

        // seek to the nValue'th entry
        OSL_ENSURE(pFormats.size() < o3tl::make_unsigned(nValue), "OLimitedFormats::getFormatKeyPropertyValue: did not find the value!");
        if (pFormats.size() < o3tl::make_unsigned(nValue))
            _rValue <<= pFormats[nValue].nKey;

        // TODO: should use a standard format for the control type we're working for
    }


    bool OLimitedFormats::convertFormatKeyPropertyValue(Any& _rConvertedValue, Any& _rOldValue, const Any& _rNewValue)
    {
        OSL_ENSURE(m_xAggregate.is() && (-1 != m_nFormatEnumPropertyHandle), "OLimitedFormats::convertFormatKeyPropertyValue: not initialized!");

        if (!m_xAggregate)
            return false;

        // the new format key to set
        sal_Int32 nNewFormat = 0;
        if (!(_rNewValue >>= nNewFormat))
            throw IllegalArgumentException();

        // get the old (enum) value from the aggregate
        Any aEnumPropertyValue = m_xAggregate->getFastPropertyValue(m_nFormatEnumPropertyHandle);
        sal_Int32 nOldEnumValue = -1;
        ::cppu::enum2int(nOldEnumValue, aEnumPropertyValue);

        // get the translation table
        std::span<FormatEntry> pFormats = lcl_getFormatTable(m_nTableId);

        _rOldValue.clear();
        _rConvertedValue.clear();

        _rOldValue <<= pFormats[nOldEnumValue].nKey;
        bool bModified = false;
        bool bFoundIt = false;
        // look for the entry with the given format key
        sal_Int32 nTablePosition = 0;
        for (FormatEntry & rEntry : pFormats)
        {
            if (nNewFormat == rEntry.nKey)
            {
                bFoundIt = true;
                _rConvertedValue <<= static_cast<sal_Int16>(nTablePosition);
                bModified = nTablePosition != nOldEnumValue;
                break;
            }
            ++nTablePosition;
        }

        if (!_rOldValue.hasValue())
        {   // did not reach the end of the table (means we found nNewFormat)
            // -> go to the end to ensure that _rOldValue is set
            _rOldValue <<= pFormats[nOldEnumValue].nKey;
        }

        OSL_ENSURE(_rOldValue.hasValue(), "OLimitedFormats::convertFormatKeyPropertyValue: did not find the old enum value in the table!");

        if (!bFoundIt)
        {   // somebody gave us a format which we can't translate
            throw IllegalArgumentException(u"This control supports only a very limited number of formats."_ustr, nullptr, 2);
        }

        return bModified;
    }


    void OLimitedFormats::setFormatKeyPropertyValue( const Any& _rNewValue )
    {
        OSL_ENSURE(m_xAggregate.is() && (-1 != m_nFormatEnumPropertyHandle), "OLimitedFormats::setFormatKeyPropertyValue: not initialized!");

        if (m_xAggregate.is())
        {   // this is to be called after convertFormatKeyPropertyValue, where
            // we translated the format key into an enum value.
            // So now we can simply forward this enum value to our aggregate
            m_xAggregate->setFastPropertyValue(m_nFormatEnumPropertyHandle, _rNewValue);
        }
    }


    void OLimitedFormats::acquireSupplier(const Reference< XComponentContext >& _rxContext)
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if (1 == ++s_nInstanceCount)
        {   // create the standard formatter
            s_xStandardFormats = NumberFormatsSupplier::createWithLocale(_rxContext, getLocale(ltEnglishUS));
        }
    }


    void OLimitedFormats::releaseSupplier()
    {
        ::osl::MutexGuard aGuard(s_aMutex);
        if (0 == --s_nInstanceCount)
        {
            ::comphelper::disposeComponent(s_xStandardFormats);
            s_xStandardFormats = nullptr;

            clearTable(FormComponentType::TIMEFIELD);
            clearTable(FormComponentType::DATEFIELD);
        }
    }


}   // namespace frm


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
