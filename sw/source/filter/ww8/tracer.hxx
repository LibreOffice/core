/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */

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
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
