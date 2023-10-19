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

#ifndef INCLUDED_XMLOFF_PROGRESSBARHELPER_HXX
#define INCLUDED_XMLOFF_PROGRESSBARHELPER_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <com/sun/star/task/XStatusIndicator.hpp>

inline constexpr OUString XML_PROGRESSRANGE = u"ProgressRange"_ustr;
inline constexpr OUString XML_PROGRESSMAX = u"ProgressMax"_ustr;
inline constexpr OUString XML_PROGRESSCURRENT = u"ProgressCurrent"_ustr;
inline constexpr OUString XML_PROGRESSREPEAT = u"ProgressRepeat"_ustr;

class XMLOFF_DLLPUBLIC ProgressBarHelper
{
            css::uno::Reference < css::task::XStatusIndicator >   m_xStatusIndicator;
            sal_Int32                                             m_nRange;
            sal_Int32                                             m_nReference;
            sal_Int32                                             m_nValue;
            double                                                m_fOldPercent;
            bool                                                  m_bStrict;
            // #96469#; if the value goes over the Range the progressbar starts again
            bool                                                  m_bRepeat;

#ifdef DBG_UTIL
            bool                                                  m_bFailure;
#endif
public:
            ProgressBarHelper(css::uno::Reference < css::task::XStatusIndicator> xStatusIndicator,
                                const bool bStrict);
            ~ProgressBarHelper();

            void SetRange(sal_Int32 nVal) { m_nRange = nVal; }
            void SetReference(sal_Int32 nVal) { m_nReference = nVal; }
            void SetValue(sal_Int32 nValue);
            void SetRepeat(bool bValue) { m_bRepeat = bValue; }
            void Increment(sal_Int32 nInc = 1) { SetValue( m_nValue+nInc ); }
            void End() { if (m_xStatusIndicator.is()) m_xStatusIndicator->end(); }

            // set the new reference and returns the new value which gives the
            // Progress Bar the same position as before
            void ChangeReference(sal_Int32 nNewReference);

            sal_Int32 GetReference() const { return m_nReference; }
            sal_Int32 GetValue() const { return m_nValue; }
            bool GetRepeat() const { return m_bRepeat; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
