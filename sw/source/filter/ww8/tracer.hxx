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

#ifndef SW_TRACER_HXX
#define SW_TRACER_HXX

#include <rtl/ustring.hxx>      //rtl::OUString

class MSFilterTracer;
class SfxMedium;

namespace sw
{
    namespace log
    {

        enum Problem
        {
            ePrinterMetrics = 1,
            eExtraLeading,
            eTabStopDistance,
            eDontUseHTMLAutoSpacing,
            eAutoWidthFrame,
            eRowCanSplit,
            eSpacingBetweenCells,
            eTabInNumbering,
            eNegativeVertPlacement,
            eAutoColorBg,
            eTooWideAsChar,
            eAnimatedText,
            eDontAddSpaceForEqualStyles,
            eBorderDistOutside,
            eContainsVisualBasic,
            eContainsWordBasic
        };

        enum Environment
        {
            eDocumentProperties,
            eMacros,
            eMainText,
            eSubDoc,
            eTable
        };

        class Tracer
        {
        private:
            MSFilterTracer *mpTrace;
            rtl::OUString GetContext(Environment eContext) const;
            rtl::OUString GetDetails(Environment eContext) const;
        public:
            Tracer(const SfxMedium &rMedium);
            MSFilterTracer *GetTrace() const { return mpTrace; }
            void EnterEnvironment(Environment eContext);
            void EnterEnvironment(Environment eContext,
                const rtl::OUString &rDetails);
            void Log(Problem eProblem);
            void LeaveEnvironment(Environment eContext);
            ~Tracer();
        };
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
