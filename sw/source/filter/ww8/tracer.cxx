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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
#include <tools/urlobj.hxx>         //INetURLObject
#include <sfx2/docfile.hxx>         //SfxMedium
#include <filter/msfilter/msfiltertracer.hxx>   //MSFilterTracer
#include "tracer.hxx"               //sw::log::Tracer

using rtl::OUString;
#define CAU(X) RTL_CONSTASCII_USTRINGPARAM(X)
#define C2O(X) OUString(CAU(X))
#define COMMENT(X) OUString()

namespace sw
{
    namespace log
    {
        Tracer::Tracer(const SfxMedium &rMed)
            : mpTrace(0)
        {
            using namespace ::com::sun::star::uno;
            using namespace ::com::sun::star::beans;
            Sequence<PropertyValue> aConfig(1);
            PropertyValue aPropValue;
            aPropValue.Value <<=
                OUString(rMed.GetURLObject().GetMainURL(
                    INetURLObject::NO_DECODE));
            aPropValue.Name = C2O("DocumentURL");
            aConfig[0] = aPropValue;
            OUString aTraceConfigPath(CAU("Office.Tracing/Import/Word"));
            mpTrace = new MSFilterTracer(aTraceConfigPath, &aConfig);
            if (mpTrace)
                mpTrace->StartTracing();
        }

        Tracer::~Tracer()
        {
            if (mpTrace)
            {
                mpTrace->EndTracing();
                delete mpTrace;
            }
        }

        void Tracer::Log(Problem eProblem)
        {
            if (!mpTrace)
                return;

            OUString sID(CAU("sw"));
            sID += rtl::OUString::valueOf(static_cast<sal_Int32>(eProblem));
            switch (eProblem)
            {
                case ePrinterMetrics:
                    mpTrace->Trace(sID, COMMENT("PrinterMetrics"));
                    break;
                case eExtraLeading:
                    mpTrace->Trace(sID, COMMENT("Extra Leading"));
                    break;
                case eTabStopDistance:
                    mpTrace->Trace(sID, COMMENT("Minimum Tab Distance"));
                    break;
                case eDontUseHTMLAutoSpacing:
                    mpTrace->Trace(sID, COMMENT("HTML AutoSpacing"));
                    break;
                case eAutoWidthFrame:
                    mpTrace->Trace(sID, COMMENT("AutoWidth"));
                    break;
                case eRowCanSplit:
                    mpTrace->Trace(sID, COMMENT("Splitable Row"));
                    break;
                case eSpacingBetweenCells:
                    mpTrace->Trace(sID, COMMENT("Spacing Between Cells"));
                    break;
                case eTabInNumbering:
                    mpTrace->Trace(sID, COMMENT("Tab In Numbering"));
                    break;
                case eNegativeVertPlacement:
                    mpTrace->Trace(sID,
                        COMMENT("Negative Vertical Placement"));
                    break;
                case eAutoColorBg:
                    mpTrace->Trace(sID,
                        COMMENT("Bad Background for Autocolour"));
                    break;
                case eTooWideAsChar:
                    mpTrace->Trace(sID,
                        COMMENT("Inline wider than TextArea"));
                    break;
                case eAnimatedText:
                    mpTrace->Trace(sID,
                        COMMENT("Animated Text"));
                    break;
                case eDontAddSpaceForEqualStyles:
                    mpTrace->Trace(sID,
                        COMMENT("Don't Add Space between Equal Style"));
                    break;
                case eBorderDistOutside:
                    mpTrace->Trace(sID,
                        COMMENT("Word draws the border outside"));
                    break;
                case eContainsVisualBasic:
                    mpTrace->Trace(sID, COMMENT("Contains VBA"));
                    break;
                case eContainsWordBasic:
                    mpTrace->Trace(sID, COMMENT("Contains Word Basic"));
                    break;
                default:
                    mpTrace->Trace(sID, COMMENT("UNKNOWN"));
                    break;
            }
        }

        rtl::OUString Tracer::GetContext(Environment eContext) const
        {
            rtl::OUString sContext;
            switch (eContext)
            {
                case eMacros:
                case eDocumentProperties:
                    sContext = C2O("Global");
                    break;
                case eMainText:
                    sContext = C2O("Text");
                    break;
                case eSubDoc:
                    sContext = C2O("Text");
                    break;
                case eTable:
                    sContext = C2O("Table");
                    break;
                default:
                    sContext = C2O("UNKNOWN");
                    break;
            }
            return sContext;
        }

        rtl::OUString Tracer::GetDetails(Environment eContext) const
        {
            rtl::OUString sDetails;
            switch (eContext)
            {
                case eDocumentProperties:
                    sDetails = C2O("Document Properties");
                    break;
                case eMacros:
                    sDetails = C2O("Macros");
                    break;
                case eMainText:
                    sDetails = C2O("MainDocument");
                    break;
                case eSubDoc:
                    sDetails = C2O("Unknown Subdocument");
                    break;
                default:
                    sDetails = C2O("UNKNOWN");
                    break;
            }
            return sDetails;
        }

        void Tracer::EnterEnvironment(Environment eContext)
        {
            mpTrace->AddAttribute(GetContext(eContext), GetDetails(eContext));
        }

        void Tracer::EnterEnvironment(Environment eContext,
            const rtl::OUString &rDetails)
        {
            mpTrace->AddAttribute(GetContext(eContext), rDetails);
        }

        void Tracer::LeaveEnvironment(Environment eContext)
        {
            mpTrace->RemoveAttribute(GetContext(eContext));
        }
    }
}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
