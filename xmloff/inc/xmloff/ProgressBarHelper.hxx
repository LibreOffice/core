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

#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#define _XMLOFF_PROGRESSBARHELPER_HXX

#include "sal/config.h"
#include "xmloff/dllapi.h"
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/frame/XModel.hpp>

#define XML_PROGRESSRANGE   "ProgressRange"
#define XML_PROGRESSMAX     "ProgressMax"
#define XML_PROGRESSCURRENT "ProgressCurrent"
#define XML_PROGRESSREPEAT  "ProgressRepeat"

class XMLOFF_DLLPUBLIC ProgressBarHelper
{
            ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator >   xStatusIndicator;
            sal_Int32                                                                       nRange;
            sal_Int32                                                                       nReference;
            sal_Int32                                                                       nValue;
            sal_Bool                                                                        bStrict;
            // #96469#; if the value goes over the Range the progressbar starts again
            sal_Bool                                                                        bRepeat;

#ifdef DBG_UTIL
            sal_Bool                                                                        bFailure;
#endif
public:
            ProgressBarHelper(const ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator>& xStatusIndicator,
                                const sal_Bool bStrict);
            ~ProgressBarHelper();

            void SetText(::rtl::OUString& rText) { if (xStatusIndicator.is()) xStatusIndicator->setText(rText); }
            void SetRange(sal_Int32 nVal) { nRange = nVal; }
            void SetReference(sal_Int32 nVal) { nReference = nVal; }
            void SetValue(sal_Int32 nValue);
            void SetRepeat(sal_Bool bValue) { bRepeat = bValue; }
            inline void Increment(sal_Int32 nInc = 1) { SetValue( nValue+nInc ); }
            void End() { if (xStatusIndicator.is()) xStatusIndicator->end(); }

            // set the new reference and returns the new value which gives the
            // Progress Bar the sam position as before
            sal_Int32 ChangeReference(sal_Int32 nNewReference);

            sal_Int32 GetReference() { return nReference; }
            sal_Int32 GetValue() { return nValue; }
            sal_Bool GetRepeat() { return bRepeat; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
