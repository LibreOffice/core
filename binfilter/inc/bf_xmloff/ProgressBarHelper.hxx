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

#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#define _XMLOFF_PROGRESSBARHELPER_HXX

#include <com/sun/star/task/XStatusIndicator.hpp>
namespace binfilter {

#define XML_PROGRESSRANGE	"ProgressRange"
#define XML_PROGRESSMAX		"ProgressMax"
#define XML_PROGRESSCURRENT	"ProgressCurrent"
#define XML_PROGRESSREPEAT  "ProgressRepeat"

class ProgressBarHelper
{
            ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator > 	xStatusIndicator;
            sal_Int32   nRange;
            sal_Int32   nReference;
            sal_Int32   nValue;
            double      fOldPercent;
            sal_Bool    bStrict;
            // #96469#; if the value goes over the Range the progressbar starts again
            sal_Bool    bRepeat;

#ifdef DBG_UTIL
            sal_Bool    bFailure;
#endif
public:
            ProgressBarHelper(const ::com::sun::star::uno::Reference < ::com::sun::star::task::XStatusIndicator>& xStatusIndicator,
                                const sal_Bool bStrict);
            ~ProgressBarHelper();

            void SetText(::rtl::OUString& rText) { if (xStatusIndicator.is()) xStatusIndicator->setText(rText); }
            void SetRange(sal_Int32 nValueIn) { nRange = nValueIn; }
            void SetReference(sal_Int32 nValueIn) { nReference = nValueIn; }
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

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
